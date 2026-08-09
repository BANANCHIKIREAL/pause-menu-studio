#include <Geode/Geode.hpp>
#include <Geode/modify/CustomSongWidget.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/utils/web.hpp>
#include <hiimjustin000.demons_in_between/include/DemonsInBetweenAPI.hpp>

#include "LayoutProfilePopups.hpp"
#include "LayoutProfiles.hpp"
#include "HiddenBlocks.hpp"
#include "HiddenBlocksPopup.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <map>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;

namespace {
constexpr char const* EDITOR_ID = "pause-menu-editor";
constexpr char const* CARDS_ID = "pause-menu-cards";
constexpr float GRID = 5.f;
constexpr int LAYOUT_SCHEMA = 2;
constexpr std::array<std::string_view, 4> INFO_CARD_IDS {
    "level-card", "song-card", "coins-card", "difficulty-card"
};

std::map<int, LadderDemon> g_dibDemons;

std::string stableLevelName(PlayLayer* play, GJGameLevel* level);

std::string presetName() {
    return Mod::get()->getSettingValue<std::string>("preset");
}

std::string safeKey(std::string value) {
    for (auto& c : value) {
        if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
    }
    return value;
}

std::string nodePath(CCNode* node) {
    std::vector<std::string> parts;
    for (auto current = node; current && !typeinfo_cast<PauseLayer*>(current); current = current->getParent()) {
        auto id = current->getID();
        if (!id.empty()) {
            parts.push_back(id);
            continue;
        }
        auto parent = current->getParent();
        unsigned index = 0;
        if (parent) {
            for (auto sibling : CCArrayExt<CCNode*>(parent->getChildren())) {
                if (sibling == current) break;
                ++index;
            }
        }
        parts.push_back("unnamed-node-" + std::to_string(index));
    }
    std::reverse(parts.begin(), parts.end());
    std::string result;
    for (auto const& part : parts) result += "/" + safeKey(part);
    return result;
}

std::string nodeKey(CCNode* node) {
    if (node && node->getID() == "editor-toggle-button") {
        return presetName() + "/persistent-editor-toggle-button";
    }
    auto generation = Mod::get()->getSavedValue<int64_t>("layout-generation", 0);
    std::string result = presetName() + "/generation-" + std::to_string(generation);
    return result + nodePath(node);
}

std::string xKey(CCNode* node) { return "position-x/" + nodeKey(node); }
std::string yKey(CCNode* node) { return "position-y/" + nodeKey(node); }
std::string scaleXKey(CCNode* node) { return "scale-x/" + nodeKey(node); }
std::string scaleYKey(CCNode* node) { return "scale-y/" + nodeKey(node); }

bool isInformationCard(CCNode* node) {
    if (!node) return false;
    auto const id = node->getID();
    return id == "song-card" || id == "level-card" || id == "coins-card" || id == "difficulty-card";
}

std::string informationCardPath(std::string_view id) {
    return "/" + safeKey(CARDS_ID) + "/" + safeKey(std::string(id));
}

bool isInformationCardPath(std::string_view path) {
    return std::ranges::any_of(INFO_CARD_IDS, [path](auto id) {
        return path == informationCardPath(id);
    });
}

std::unordered_set<std::string> informationCardsInSnapshot(
    pause_menu_studio::profiles::Snapshot const& snapshot
) {
    std::unordered_set<std::string> result;
    for (auto id : INFO_CARD_IDS) {
        if (snapshot.contains(informationCardPath(id))) result.insert(std::string(id));
    }
    return result;
}

std::unordered_set<std::string> informationCardsInActiveLayout() {
    auto active = Mod::get()->getSavedValue<std::string>("active-named-layout", "");
    if (active.empty()) return {};
    if (auto snapshot = pause_menu_studio::profiles::load(active)) {
        return informationCardsInSnapshot(*snapshot);
    }
    return {};
}

void savePosition(CCNode* node) {
    if (!node) return;
    Mod::get()->setSavedValue<double>(xKey(node), node->getPositionX());
    Mod::get()->setSavedValue<double>(yKey(node), node->getPositionY());
    Mod::get()->setSavedValue<double>(scaleXKey(node), node->getScaleX());
    Mod::get()->setSavedValue<double>(scaleYKey(node), node->getScaleY());
}

void restorePositions(CCNode* node, CCNode* skip = nullptr) {
    if (!node || node == skip || node->getID() == EDITOR_ID) return;
    auto mod = Mod::get();
    auto keyX = xKey(node);
    auto keyY = yKey(node);
    if (mod->hasSavedValue(keyX) && mod->hasSavedValue(keyY)) {
        node->setPosition({
            static_cast<float>(mod->getSavedValue<double>(keyX)),
            static_cast<float>(mod->getSavedValue<double>(keyY)),
        });
    }
    if (mod->hasSavedValue(scaleXKey(node)) && mod->hasSavedValue(scaleYKey(node))) {
        node->setScaleX(static_cast<float>(mod->getSavedValue<double>(scaleXKey(node))));
        node->setScaleY(static_cast<float>(mod->getSavedValue<double>(scaleYKey(node))));
    }
    // Cards are logical blocks. Their internal Jukebox and difficulty nodes
    // rebuild themselves and must never receive old per-child editor offsets.
    if (isInformationCard(node)) return;
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) restorePositions(child, skip);
}

void applyHiddenVisibility(CCNode* node, std::unordered_set<std::string> const& paths) {
    if (!node || node->getID() == EDITOR_ID) return;
    if (!typeinfo_cast<PauseLayer*>(node) && paths.contains(nodePath(node))) {
        node->setVisible(false);
    }
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        applyHiddenVisibility(child, paths);
    }
}

void applyHiddenVisibility(CCNode* root) {
    auto storedPaths = pause_menu_studio::hidden_blocks::memberPaths();
    std::unordered_set<std::string> paths(storedPaths.begin(), storedPaths.end());
    applyHiddenVisibility(root, paths);
}

struct InitialPosition {
    WeakRef<CCNode> node;
    CCPoint position;
    float scaleX;
    float scaleY;
};

void captureResetPositions(CCNode* node, CCNode* owner, std::vector<InitialPosition>& positions) {
    if (!node || node->getID() == EDITOR_ID) return;
    // RESET must only touch nodes moved by this mod. Capturing every child also
    // captures the internal sprites of animated texture-pack buttons and can
    // tear those animations apart when their positions are restored.
    if (
        node != owner &&
        Mod::get()->hasSavedValue(xKey(node)) &&
        Mod::get()->hasSavedValue(yKey(node))
    ) {
        positions.push_back({
            WeakRef<CCNode>(node), node->getPosition(), node->getScaleX(), node->getScaleY()
        });
    }
    if (isInformationCard(node)) return;
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        captureResetPositions(child, owner, positions);
    }
}

void captureCurrentSnapshot(
    CCNode* node,
    CCNode* owner,
    pause_menu_studio::profiles::Snapshot& snapshot,
    std::unordered_map<std::string, std::pair<std::string, std::string>> const& hiddenMembership
) {
    if (!node || node->getID() == EDITOR_ID) return;
    if (
        node != owner &&
        Mod::get()->hasSavedValue(xKey(node)) &&
        Mod::get()->hasSavedValue(yKey(node))
    ) {
        auto path = nodePath(node);
        pause_menu_studio::profiles::Transform transform {
            node->getPosition(), node->getScaleX(), node->getScaleY()
        };
        auto hidden = hiddenMembership.find(path);
        transform.hidden = hidden != hiddenMembership.end();
        if (hidden != hiddenMembership.end()) {
            transform.hiddenID = hidden->second.first;
            transform.hiddenLabel = hidden->second.second;
        }
        snapshot[path] = std::move(transform);
    }
    if (isInformationCard(node)) return;
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        captureCurrentSnapshot(child, owner, snapshot, hiddenMembership);
    }
}

bool pointInside(CCNode* node, CCPoint world) {
    if (!node || !node->isVisible() || node->getContentWidth() <= 1.f || node->getContentHeight() <= 1.f) return false;
    auto local = node->convertToNodeSpace(world);
    return CCRect({0.f, 0.f}, node->getContentSize()).containsPoint(local);
}

bool isEditorContainer(CCNode* node) {
    return
        typeinfo_cast<PauseLayer*>(node) ||
        typeinfo_cast<CCScene*>(node) ||
        typeinfo_cast<CCMenu*>(node) ||
        typeinfo_cast<CCLayer*>(node) ||
        node->getID() == CARDS_ID;
}

bool hasLogicalID(CCNode* node, std::string_view wanted) {
    if (!node) return false;
    auto id = std::string(node->getID());
    return id == wanted || id.ends_with("/" + std::string(wanted));
}

CCNode* findByLogicalID(CCNode* root, std::string_view wanted) {
    if (!root || root->getID() == EDITOR_ID) return nullptr;
    if (hasLogicalID(root, wanted)) return root;
    for (auto child : CCArrayExt<CCNode*>(root->getChildren())) {
        if (auto found = findByLogicalID(child, wanted)) return found;
    }
    return nullptr;
}

void fixJukeboxDiscPosition(CCNode* root) {
    auto widget = typeinfo_cast<CustomSongWidget*>(
        hasLogicalID(root, "song-card") ? root : findByLogicalID(root, "song-card")
    );
    if (!widget) return;
    auto disc = findByLogicalID(widget, "nong-pin");
    if (!disc) return;

    CCNode* pinMenu = nullptr;
    for (auto current = disc->getParent(); current && current != widget; current = current->getParent()) {
        if (hasLogicalID(current, "nong-menu")) pinMenu = current;
    }
    if (!pinMenu || !widget->m_bgSpr) return;

    // Jukebox places nong-menu from the song-label position and nong-pin is a
    // sprite nested inside nong-button. Moving the menu origin directly to the
    // frame corner therefore preserves a title-dependent child offset. Shift
    // the menu by the actual visible disc-center delta instead.
    auto worldTopLeft = widget->m_bgSpr->convertToWorldSpace({
        0.f, widget->m_bgSpr->getContentHeight()
    });
    auto worldDiscCenter = disc->convertToWorldSpace(disc->getContentSize() / 2.f);
    auto menuParent = pinMenu->getParent();
    if (!menuParent) return;
    auto worldMenuPosition = menuParent->convertToWorldSpace(pinMenu->getPosition());
    pinMenu->setPosition(menuParent->convertToNodeSpace(
        worldMenuPosition + worldTopLeft - worldDiscCenter
    ));
}

void alignJukeboxSongBlock(CCNode* root) {
    auto widget = typeinfo_cast<CustomSongWidget*>(
        hasLogicalID(root, "song-card") ? root : findByLogicalID(root, "song-card")
    );
    if (!widget || !widget->m_bgSpr) return;

    auto cards = widget->getParent();
    if (!cards || cards->getID() != CARDS_ID) return;

    // A manually positioned music block belongs to the user's layout. Only
    // stabilize the default position after Jukebox rebuilds the widget.
    if (Mod::get()->hasSavedValue(xKey(widget)) && Mod::get()->hasSavedValue(yKey(widget))) return;

    auto visibleBounds = [&] {
        std::array<CCPoint, 4> corners {{
            cards->convertToNodeSpace(widget->m_bgSpr->convertToWorldSpace({0.f, 0.f})),
            cards->convertToNodeSpace(widget->m_bgSpr->convertToWorldSpace({widget->m_bgSpr->getContentWidth(), 0.f})),
            cards->convertToNodeSpace(widget->m_bgSpr->convertToWorldSpace(widget->m_bgSpr->getContentSize())),
            cards->convertToNodeSpace(widget->m_bgSpr->convertToWorldSpace({0.f, widget->m_bgSpr->getContentHeight()})),
        }};
        float minX = corners.front().x;
        float minY = corners.front().y;
        float maxX = minX;
        float maxY = minY;
        for (auto point : corners) {
            minX = std::min(minX, point.x);
            minY = std::min(minY, point.y);
            maxX = std::max(maxX, point.x);
            maxY = std::max(maxY, point.y);
        }
        return std::array<float, 4> {minX, minY, maxX, maxY};
    };

    auto bounds = visibleBounds();
    auto const width = bounds[2] - bounds[0];
    if (width > 285.f) {
        auto fit = 285.f / width;
        widget->setScaleX(widget->getScaleX() * fit);
        widget->setScaleY(widget->getScaleY() * fit);
        bounds = visibleBounds();
    }

    // The cards below reach y=52 at most. Keeping the real Jukebox background
    // above y=57 prevents its title from covering the level-name card when the
    // pause layer is destroyed and created again.
    auto const centerX = (bounds[0] + bounds[2]) / 2.f;
    widget->setPosition(widget->getPosition() + CCPoint {
        cards->getContentWidth() / 2.f - centerX,
        57.f - bounds[1],
    });
    fixJukeboxDiscPosition(widget);
}

bool hasAncestorWithLogicalID(CCNode* node, CCNode* owner, std::string_view wanted) {
    for (auto current = node; current && current != owner; current = current->getParent()) {
        if (hasLogicalID(current, wanted)) return true;
    }
    return false;
}

CCNode* findDeepest(CCNode* root, CCPoint world) {
    if (!root || !root->isVisible() || root->getID() == EDITOR_ID) return nullptr;
    auto children = root->getChildren();
    if (children) {
        for (int index = static_cast<int>(children->count()) - 1; index >= 0; --index) {
            if (auto found = findDeepest(static_cast<CCNode*>(children->objectAtIndex(index)), world)) return found;
        }
    }
    // Menus and layers frequently cover the entire screen. Selecting one when
    // the user clicks empty space makes every contained button move at once.
    if (isEditorContainer(root)) return nullptr;
    return pointInside(root, world) ? root : nullptr;
}

CCNode* promoteSelection(CCNode* node, PauseLayer* owner) {
    if (!node) return nullptr;
    // The information widgets are single logical controls. Selecting a button,
    // label, flame, or icon inside one must move the whole widget.
    for (auto current = node; current && current != owner; current = current->getParent()) {
        auto const id = current->getID();
        if (id == "song-card" || id == "level-card" || id == "coins-card" || id == "difficulty-card") {
            return current;
        }
    }
    // Empty space inside the shared cards row is not an element. Returning the
    // row itself here used to move level, song, and difficulty together.
    for (auto current = node; current && current != owner; current = current->getParent()) {
        if (current->getID() == CARDS_ID) return nullptr;
    }
    for (auto current = node; current && current != owner; current = current->getParent()) {
        if (typeinfo_cast<CCMenuItem*>(current)) return current;
    }
    for (auto current = node; current && current != owner; current = current->getParent()) {
        if (!current->getID().empty() && current->getID() != CARDS_ID) return current;
    }
    return node;
}

std::vector<CCNode*> logicalSelection(CCNode* hit, PauseLayer* owner) {
    if (!hit || !owner) return {};

    auto volumeGroup = [&](char const* type) -> std::vector<CCNode*> {
        auto sliderID = std::string(type) + "-slider";
        auto labelID = std::string(type) + "-label";
        auto labelMenuID = std::string(type) + "-label-menu";
        auto inputID = std::string(type) + "-input";
        auto percentID = std::string(type) + "-percent-label";
        auto muteMenuID = std::string(type) + "-mute-menu";
        auto muteID = std::string(type) + "-mute-toggle";
        auto belongs =
            hasAncestorWithLogicalID(hit, owner, sliderID) ||
            hasAncestorWithLogicalID(hit, owner, labelID) ||
            hasAncestorWithLogicalID(hit, owner, labelMenuID) ||
            hasAncestorWithLogicalID(hit, owner, inputID) ||
            hasAncestorWithLogicalID(hit, owner, percentID) ||
            hasAncestorWithLogicalID(hit, owner, muteMenuID) ||
            hasAncestorWithLogicalID(hit, owner, muteID);
        if (!belongs) return {};

        std::vector<CCNode*> members;
        if (auto slider = findByLogicalID(owner, sliderID)) members.push_back(slider);
        if (auto labelMenu = findByLogicalID(owner, labelMenuID)) {
            members.push_back(labelMenu);
        } else if (auto label = findByLogicalID(owner, labelID); label && label->isVisible()) {
            members.push_back(label);
        }
        if (auto muteMenu = findByLogicalID(owner, muteMenuID)) members.push_back(muteMenu);
        return members;
    };

    if (auto music = volumeGroup("music"); !music.empty()) return music;
    if (auto sfx = volumeGroup("sfx"); !sfx.empty()) return sfx;
    if (auto promoted = promoteSelection(hit, owner)) return {promoted};
    return {};
}

void collectSelectableHits(CCNode* root, CCPoint world, std::vector<CCNode*>& hits) {
    if (!root || !root->isVisible() || root->getID() == EDITOR_ID) return;
    auto children = root->getChildren();
    if (children) {
        for (int index = static_cast<int>(children->count()) - 1; index >= 0; --index) {
            collectSelectableHits(static_cast<CCNode*>(children->objectAtIndex(index)), world, hits);
        }
    }
    if (!isEditorContainer(root) && pointInside(root, world)) hits.push_back(root);
}

bool sameLogicalGroup(std::vector<CCNode*> const& first, std::vector<CCNode*> const& second) {
    if (first.size() != second.size()) return false;
    return std::ranges::all_of(first, [&second](CCNode* node) {
        return std::ranges::find(second, node) != second.end();
    });
}

std::vector<std::vector<CCNode*>> logicalSelectionsAt(PauseLayer* owner, CCPoint world) {
    std::vector<CCNode*> hits;
    collectSelectableHits(owner, world, hits);
    std::vector<std::vector<CCNode*>> result;
    for (auto hit : hits) {
        auto group = logicalSelection(hit, owner);
        if (group.empty()) continue;
        auto duplicate = std::ranges::any_of(result, [&group](auto const& existing) {
            return sameLogicalGroup(existing, group);
        });
        if (!duplicate) result.push_back(std::move(group));
    }
    return result;
}

void stabilizeBetterVolumeSliderGeometry(CCNode* root) {
    if (!root || !Loader::get()->getLoadedMod("nwo5.better_volume")) return;
    for (auto id : {std::string_view("music-slider"), std::string_view("sfx-slider")}) {
        auto slider = typeinfo_cast<Slider*>(findByLogicalID(root, id));
        if (
            !slider || !slider->isVisible() || !slider->m_groove ||
            !slider->m_touchLogic || !slider->m_touchLogic->m_thumb
        ) continue;

        auto thumb = slider->m_touchLogic->m_thumb;
        auto thumbParent = thumb->getParent();
        if (!thumbParent) continue;
        auto grooveCenter = slider->m_groove->convertToWorldSpace(slider->m_groove->getContentSize() / 2.f);
        auto thumbCenter = thumbParent->convertToWorldSpace(thumb->getPosition());
        if (std::abs(thumbCenter.y - grooveCenter.y) <= 2.f) continue;

        // Menu animation mods may animate the technical thumb node separately
        // from its groove. Rebuild only slider geometry, then put the thumb on
        // the groove's world-space centerline without stopping sprite actions.
        slider->setValue(slider->getValue());
        slider->updateBar();
        auto target = thumbParent->convertToNodeSpace(grooveCenter);
        thumb->setPositionY(target.y);
    }
}

class VolumeGeometryGuard final : public CCNode {
public:
    static VolumeGeometryGuard* create(CCNode* owner) {
        auto ret = new VolumeGeometryGuard();
        if (ret && ret->init(owner)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(CCNode* owner) {
        if (!CCNode::init() || !owner) return false;
        m_owner = owner;
        setID("better-volume-geometry-guard");
        scheduleUpdateWithPriority(std::numeric_limits<int>::max());
        return true;
    }

    void update(float) override {
        stabilizeBetterVolumeSliderGeometry(m_owner);
    }

private:
    CCNode* m_owner = nullptr;
};

CCSprite* createGDDPDifficulty(GJGameLevel* level) {
    auto gddp = Loader::get()->getLoadedMod("minemaker0430.gddp_integration");
    if (!gddp || !level || !gddp->getSettingValue<bool>("custom-difficulty-faces")) return nullptr;

    auto const activeOutsideMenu =
        gddp->getSavedValue<bool>("in-gddp", false) ||
        gddp->getSettingValue<bool>("show-outside-menus");
    if (!activeOutsideMenu) return nullptr;

    auto data = gddp->getSavedValue<matjson::Value>("cached-data");
    auto const levelID = std::to_string(level->m_levelID.value());
    if (!data.isObject() || !data["level-data"].contains(levelID)) return nullptr;

    auto const difficulty = data["level-data"][levelID]["difficulty"].as<int>().unwrapOr(0);
    auto spriteName = data["main"][difficulty]["sprite"].asString().unwrapOr("DP_Unknown");
    if (
        level->m_isEpic == 1 &&
        gddp->getSettingValue<bool>("replace-epic")
    ) {
        spriteName = data["main"][difficulty]["plusSprite"].asString().unwrapOr(spriteName);
    }
    if (spriteName == "DP_Invisible") return nullptr;

    auto const frame = gddp->expandSpriteName(spriteName + ".png");
    auto sprite = CCSprite::createWithSpriteFrameName(frame.data());
    if (sprite) sprite->setID("gddp-difficulty");
    return sprite;
}

CCSprite* createDIBDifficulty(GJGameLevel* level) {
    auto dib = Loader::get()->getLoadedMod("hiimjustin000.demons_in_between");
    if (!dib || !level || !dib->getSettingValue<bool>("enable-difficulties")) return nullptr;
    if (level->m_stars.value() < 10) return nullptr;

    auto found = g_dibDemons.find(level->m_levelID.value());
    if (found == g_dibDemons.end()) return nullptr;

    auto state = level->m_featured != 0
        ? static_cast<GJFeatureState>(level->m_isEpic + 1)
        : GJFeatureState::None;
    if (state == GJFeatureState::Legendary && !dib->getSettingValue<bool>("enable-legendary")) {
        state = GJFeatureState::None;
    }
    if (state == GJFeatureState::Mythic && !dib->getSettingValue<bool>("enable-mythic")) {
        state = GJFeatureState::None;
    }

    auto const suffix = state == GJFeatureState::Legendary
        ? "_4"
        : state == GJFeatureState::Mythic ? "_5" : "";
    auto const frame = dib->expandSpriteName(fmt::format(
        "DIB_{:02d}{}_btn_001.png", found->second.difficulty, suffix
    ));
    auto sprite = CCSprite::createWithSpriteFrameName(frame.data());
    if (sprite) sprite->setID("between-difficulty-sprite");
    return sprite;
}

CCNode* createDifficultyVisual(GJGameLevel* level) {
    auto gddp = createGDDPDifficulty(level);
    auto dibMod = Loader::get()->getLoadedMod("hiimjustin000.demons_in_between");
    auto const dibOverridesGDDP = dibMod && dibMod->getSettingValue<bool>("gddp-integration-override");

    if (gddp && !dibOverridesGDDP) return gddp;
    if (auto dib = createDIBDifficulty(level)) return dib;
    if (gddp) return gddp;

    auto difficulty = level->getAverageDifficulty();
    if (
        difficulty <= static_cast<int>(GJDifficulty::Auto) &&
        !level->m_autoLevel &&
        level->m_levelType == GJLevelType::Main
    ) {
        auto stars = level->m_stars.value();
        if (stars <= 2) difficulty = static_cast<int>(GJDifficulty::Easy);
        else if (stars == 3) difficulty = static_cast<int>(GJDifficulty::Normal);
        else if (stars <= 5) difficulty = static_cast<int>(GJDifficulty::Hard);
        else if (stars <= 7) difficulty = static_cast<int>(GJDifficulty::Harder);
        else if (stars <= 9) difficulty = static_cast<int>(GJDifficulty::Insane);
        else difficulty = static_cast<int>(GJDifficulty::Demon);
    }
    if (static_cast<int>(level->m_demon) != 0) {
        difficulty = GJGameLevel::demonIconForDifficulty(
            static_cast<DemonDifficultyType>(level->m_demonDifficulty)
        );
    }
    auto icon = GJDifficultySprite::create(difficulty, GJDifficultyName::Short);
    auto const featureState = level->m_featured != 0
        ? static_cast<GJFeatureState>(level->m_isEpic + 1)
        : GJFeatureState::None;
    // Call updateFeatureState directly: GodlikeFaces hooks this exact method
    // to swap its Legendary/Mythic face while GD creates the matching fire.
    icon->updateFeatureState(featureState);
    return icon;
}

char const* difficultyName(GJGameLevel* level) {
    if (!level) return "Unknown";
    if (level->m_autoLevel) return "Auto";
    if (static_cast<int>(level->m_demon) != 0) {
        switch (level->m_demonDifficulty) {
            case 3: return "Easy Demon";
            case 4: return "Medium Demon";
            case 5: return "Insane Demon";
            case 6: return "Extreme Demon";
            default: return "Hard Demon";
        }
    }
    // Official levels frequently keep m_difficulty as NA. Use the same value
    // Geometry Dash uses to build the difficulty face so the text agrees with
    // the icon for both official and online levels.
    auto difficulty = level->getAverageDifficulty();
    if (
        difficulty <= static_cast<int>(GJDifficulty::Auto) &&
        level->m_levelType == GJLevelType::Main
    ) {
        auto stars = level->m_stars.value();
        if (stars <= 2) difficulty = static_cast<int>(GJDifficulty::Easy);
        else if (stars == 3) difficulty = static_cast<int>(GJDifficulty::Normal);
        else if (stars <= 5) difficulty = static_cast<int>(GJDifficulty::Hard);
        else if (stars <= 7) difficulty = static_cast<int>(GJDifficulty::Harder);
        else if (stars <= 9) difficulty = static_cast<int>(GJDifficulty::Insane);
        else difficulty = static_cast<int>(GJDifficulty::Demon);
    }
    switch (static_cast<GJDifficulty>(difficulty)) {
        case GJDifficulty::Auto: return "Auto";
        case GJDifficulty::Easy: return "Easy";
        case GJDifficulty::Normal: return "Normal";
        case GJDifficulty::Hard: return "Hard";
        case GJDifficulty::Harder: return "Harder";
        case GJDifficulty::Insane: return "Insane";
        default: return "Unrated";
    }
}

CCScale9Sprite* panel(CCSize size) {
    auto bg = CCScale9Sprite::create("square02_001.png");
    bg->setContentSize(size);
    bg->setColor({20, 15, 35});
    bg->setOpacity(215);
    return bg;
}

void addLabel(CCNode* parent, std::string const& text, CCPoint pos, float scale, ccColor3B color = {255, 255, 255}) {
    auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    label->setPosition(pos);
    label->setScale(scale);
    label->setColor(color);
    label->limitLabelWidth(parent->getContentWidth() - 14.f, scale, .1f);
    parent->addChild(label);
}

class DemonRankNode final : public CCNode {
public:
    static DemonRankNode* create(GJGameLevel* level) {
        auto ret = new DemonRankNode();
        if (ret && ret->init(level)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

private:
    TaskHolder<web::WebResponse> m_request;
    CCLabelBMFont* m_label = nullptr;

    bool init(GJGameLevel* level) {
        if (!CCNode::init() || !level) return false;
        setID("demonlist-rank");
        setContentSize({108.f, 10.f});

        m_label = CCLabelBMFont::create("", "chatFont.fnt");
        m_label->setPosition({54.f, 5.f});
        m_label->setScale(.55f);
        m_label->setColor({255, 210, 80});
        addChild(m_label);

        auto integrated = Loader::get()->getLoadedMod("hiimjustin000.integrated_demonlist");
        if (!integrated || !integrated->getSettingValue<bool>("enable-rank")) return true;

        auto const platformer = level->isPlatformer();
        auto const demonDifficulty = static_cast<int>(level->m_demonDifficulty);
        auto const eligible =
            level->m_levelType != GJLevelType::Editor &&
            level->m_demon.value() > 0 &&
            (platformer ? (demonDifficulty == 0 || demonDifficulty >= 5) : demonDifficulty >= 6);
        if (!eligible) return true;

        auto const levelID = level->m_levelID.value();
        auto const url = platformer
            ? fmt::format("https://pemonlist.com/api/level/{}?version=2", levelID)
            : fmt::format("https://api.aredl.net/v2/api/aredl/levels/{}", levelID);

        m_request.spawn(web::WebRequest().get(url), [this, platformer](web::WebResponse response) {
            if (!response.ok()) return;
            auto json = response.json();
            if (!json.isOk()) return;
            auto position = json.unwrap().get<int>(platformer ? "placement" : "position");
            if (!position.isOk() || (platformer && position.unwrap() > 150)) return;
            m_label->setString(fmt::format(
                "#{} {}", position.unwrap(), platformer ? "Pemonlist" : "AREDL"
            ).c_str());
        });
        return true;
    }
};

CustomSongWidget* createJukeboxSongBlock(GJGameLevel* level) {
    if (!level) return nullptr;

    auto const isRobTopSong = level->m_songID <= 0;
    auto const songIDs = std::string(level->m_songIDs);
    auto const sfxIDs = std::string(level->m_sfxIDs);
    auto const hasMultiAssets = !songIDs.empty() || !sfxIDs.empty();
    SongInfoObject* info = nullptr;
    if (hasMultiAssets) {
        auto const creator = level->m_creatorName.empty()
            ? gd::string("Unknown")
            : level->m_creatorName;
        info = SongInfoObject::create(
            level->m_songID,
            level->m_levelName,
            creator,
            0,
            0.f,
            "",
            "",
            "",
            "",
            0,
            "",
            false,
            0,
            0
        );
    }
    else if (isRobTopSong) {
        info = LevelTools::getSongObject(level->m_audioTrack);
    }
    else {
        info = MusicDownloadManager::sharedState()->getSongInfoObject(level->m_songID);
        if (!info) info = SongInfoObject::create(level->m_songID);
    }
    if (!info) return nullptr;

    // This is the real Geometry Dash CustomSongWidget. Jukebox 3.6.2 hooks
    // this exact class and adds JB_PinDisc.png, NONG metadata, and its popup.
    auto widget = CustomSongWidget::create(
        info,
        nullptr,
        false,
        false,
        true,
        isRobTopSong,
        false,
        false,
        0
    );
    if (widget) {
        widget->setID("song-card");
        if (hasMultiAssets) {
            // This calls Geometry Dash's real multi-asset mode. Jukebox hooks
            // the same method to calculate the combined song/SFX size.
            widget->updateWithMultiAssets(level->m_songIDs, level->m_sfxIDs, level->m_songSize);
        }
        fixJukeboxDiscPosition(widget);
    }
    return widget;
}

CCNode* createInfoCards(std::unordered_set<std::string> const& forcedCards = {}) {
    auto play = PlayLayer::get();
    auto level = play ? play->m_level : nullptr;
    if (!level) return nullptr;

    auto cards = CCNode::create();
    cards->setID(CARDS_ID);
    cards->setContentSize({360.f, 130.f});
    cards->setAnchorPoint({.5f, .5f});
    cards->ignoreAnchorPointForPosition(false);

    std::vector<CCNode*> built;
    if (Mod::get()->getSettingValue<bool>("show-level") || forcedCards.contains("level-card")) {
        auto card = panel({145.f, 60.f});
        card->setID("level-card");
        auto const levelName = stableLevelName(play, level);
        addLabel(card, levelName.c_str(), {72.5f, 40.f}, .55f);
        auto creator = level->m_creatorName.empty() ? "Local level" : "by " + std::string(level->m_creatorName.c_str());
        addLabel(card, creator, {72.5f, 18.f}, .32f, {190, 120, 255});
        built.push_back(card);
    }
    if (Mod::get()->getSettingValue<bool>("show-song") || forcedCards.contains("song-card")) {
        if (auto song = createJukeboxSongBlock(level)) {
            song->setPosition({0.f, 0.f});
            cards->addChild(song);
            alignJukeboxSongBlock(song);
        }
    }
    if (Mod::get()->getSettingValue<bool>("show-coins") || forcedCards.contains("coins-card")) {
        auto coinCount = std::clamp(level->m_coins, 0, 3);
        if (coinCount > 0) {
            auto cardWidth = 18.f + coinCount * 24.f;
            auto card = panel({cardWidth, 42.f});
            card->setID("coins-card");
            for (int index = 0; index < coinCount; ++index) {
                // Gold User Coins swaps this frame with secretCoinUI2_001.png
                // during MenuLayer setup, so use its supported UI alias.
                auto coin = CCSprite::createWithSpriteFrameName("secretCoinUI_001.png");
                coin->setID("coin-" + std::to_string(index + 1));
                coin->setColor(ccWHITE);
                coin->setScale(.52f);
                coin->setPosition({21.f + index * 24.f, 21.f});
                card->addChild(coin);
            }
            built.push_back(card);
        }
    }
    if (Mod::get()->getSettingValue<bool>("show-difficulty") || forcedCards.contains("difficulty-card")) {
        auto card = panel({108.f, 72.f});
        card->setID("difficulty-card");
        auto icon = createDifficultyVisual(level);
        icon->setID("difficulty-icon");
        auto const maxIconHeight = 45.f;
        if (icon->getScaledContentHeight() > maxIconHeight) {
            icon->setScale(icon->getScale() * maxIconHeight / icon->getScaledContentHeight());
        }
        icon->setPosition({54.f, 45.f});
        card->addChild(icon);

        auto stars = CCLabelBMFont::create(std::to_string(level->m_stars.value()).c_str(), "bigFont.fnt");
        stars->setID("star-count");
        stars->setAnchorPoint({1.f, .5f});
        stars->setPosition({53.f, 15.f});
        stars->setScale(.3f);
        card->addChild(stars);

        auto starIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        starIcon->setID("star-icon");
        starIcon->setAnchorPoint({0.f, .5f});
        starIcon->setPosition({57.f, 15.f});
        starIcon->setScale(.3f);
        card->addChild(starIcon);

        if (Mod::get()->getSettingValue<bool>("show-demon-rank")) {
            if (auto rank = DemonRankNode::create(level)) {
                rank->setPosition({0.f, 0.f});
                card->addChild(rank);
            }
        }
        built.push_back(card);
    }

    float gap = 5.f;
    float total = -gap;
    for (auto card : built) total += card->getContentWidth() + gap;
    float x = (cards->getContentWidth() - total) / 2.f;
    for (auto card : built) {
        card->setAnchorPoint({0.f, .5f});
        card->setPosition({x, 16.f});
        cards->addChild(card);
        x += card->getContentWidth() + gap;
    }
    if (total > cards->getContentWidth() && total > 0.f) {
        auto rowScale = cards->getContentWidth() / total;
        for (auto card : built) card->setScale(rowScale);
    }
    return cards;
}

class PauseEditor final : public CCLayer {
public:
    static PauseEditor* create(PauseLayer* owner, std::vector<InitialPosition> initialPositions) {
        auto ret = new PauseEditor();
        if (ret && ret->init(owner, std::move(initialPositions))) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(PauseLayer* owner, std::vector<InitialPosition> initialPositions) {
        if (!CCLayer::init() || !owner) return false;
        m_owner = owner;
        m_initialPositions = std::move(initialPositions);
        setID(EDITOR_ID);
        setContentSize(CCDirector::sharedDirector()->getWinSize());
        setTouchEnabled(true);
        setTouchMode(kCCTouchesOneByOne);
        setTouchPriority(-1000);
        m_undoKey = KeyboardInputEvent(KEY_Z).listen([this](KeyboardInputData& input) {
            if (
                !m_editing ||
                input.action != KeyboardInputData::Action::Press ||
                !(input.modifiers & KeyboardModifier::Control)
            ) return false;
            undo();
            return true;
        });
        m_redoKey = KeyboardInputEvent(KEY_Y).listen([this](KeyboardInputData& input) {
            if (
                !m_editing ||
                input.action != KeyboardInputData::Action::Press ||
                !(input.modifiers & KeyboardModifier::Control)
            ) return false;
            redo();
            return true;
        });
        auto bindArrow = [this](enumKeyCodes key, CCPoint delta) {
            return KeyboardInputEvent(key).listen([this, delta](KeyboardInputData& input) {
                if (
                    !m_editing || m_selectedNodes.empty() ||
                    (input.action != KeyboardInputData::Action::Press &&
                     input.action != KeyboardInputData::Action::Repeat)
                ) return false;
                auto amount = (input.modifiers & KeyboardModifier::Shift) ? 5.f : 1.f;
                nudgeSelection(delta * amount);
                return true;
            });
        };
        m_leftKey = bindArrow(KEY_Left, {-1.f, 0.f});
        m_rightKey = bindArrow(KEY_Right, {1.f, 0.f});
        m_upKey = bindArrow(KEY_Up, {0.f, 1.f});
        m_downKey = bindArrow(KEY_Down, {0.f, -1.f});
        m_resetKey = KeyboardInputEvent(KEY_R).listen([this](KeyboardInputData& input) {
            if (
                !m_editing || m_selectedNodes.empty() ||
                input.action != KeyboardInputData::Action::Press
            ) return false;
            requestResetSelection();
            return true;
        });
        auto bindRemove = [this](enumKeyCodes key) {
            return KeyboardInputEvent(key).listen([this](KeyboardInputData& input) {
                if (
                    !m_editing || m_selectedNodes.empty() ||
                    input.action != KeyboardInputData::Action::Press
                ) return false;
                hideSelection();
                return true;
            });
        };
        m_deleteKey = bindRemove(KEY_Delete);
        m_backspaceKey = bindRemove(KEY_Backspace);
        buildControls();
        setEditing(Mod::get()->getSettingValue<bool>("edit-on-open"));
        return true;
    }

    bool isDragging() const { return m_editing && m_dragging; }
    bool shouldWarnBeforeExit() const { return m_editing && m_hasEdits; }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        if (!m_editing || !m_owner) return false;
        auto world = touch->getLocation();
        // In Edit mode the DONE button itself is selectable. A click still
        // exits Edit mode, while a real drag moves the button.
        if (m_toggle && pointInside(m_toggle, world)) {
            m_hasLastSelectionPoint = false;
            selectOnly({m_toggle});
            beginPendingDrag(world, true);
            return true;
        }
        for (auto control : m_controlItems) {
            if (pointInside(control, world)) return false;
        }
        auto ctrl = CCKeyboardDispatcher::get()->getControlKeyPressed();
        auto selections = logicalSelectionsAt(m_owner, world);
        if (selections.empty()) {
            if (!ctrl) clearSelection();
            m_hasLastSelectionPoint = false;
            m_pendingDrag = false;
            m_dragging = false;
            updateSelectionLabel();
            updateSelectionOutline();
            return true;
        }
        if (ctrl) {
            addToSelection(selections.front());
            m_hasLastSelectionPoint = false;
        } else {
            size_t selectionIndex = 0;
            auto const clickDelta = world - m_lastSelectionPoint;
            auto const repeatedPoint =
                m_hasLastSelectionPoint &&
                clickDelta.x * clickDelta.x + clickDelta.y * clickDelta.y <= 16.f;
            if (repeatedPoint) {
                for (size_t index = 0; index < selections.size(); ++index) {
                    if (!sameLogicalGroup(selections[index], m_selectedNodes)) continue;
                    selectionIndex = (index + 1) % selections.size();
                    break;
                }
            }
            selectOnly(selections[selectionIndex]);
            m_lastSelectionPoint = world;
            m_hasLastSelectionPoint = true;
        }
        beginPendingDrag(world, false);
        updateSelectionLabel();
        updateSelectionOutline();
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_editing || m_selectedNodes.empty() || (!m_pendingDrag && !m_dragging)) return;
        auto world = touch->getLocation();
        if (m_pendingDrag) {
            auto delta = world - m_pressWorld;
            if (std::sqrt(delta.x * delta.x + delta.y * delta.y) < 4.f) return;
            m_hasLastSelectionPoint = false;
            m_pendingDrag = false;
            m_dragging = true;
        }
        moveFromStartTo(world);
        updateSelectionOutline();
    }

    void ccTouchEnded(CCTouch*, CCEvent*) override {
        finishDrag();
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) override {
        finishDrag();
    }

    void finishDrag() {
        if (m_pendingDrag) {
            m_pendingDrag = false;
            if (m_editControlPressed) {
                m_editControlPressed = false;
                setEditing(false);
            }
            return;
        }
        if (m_selectedNodes.empty() || !m_dragging) return;
        if (Mod::get()->getSettingValue<bool>("snap-grid")) {
            for (auto const& group : selectedLogicalGroups()) {
                if (group.empty() || !group.front() || !group.front()->getParent()) continue;
                auto anchor = group.front();
                auto position = anchor->getPosition();
                auto snapped = CCPoint {
                    std::round(position.x / GRID) * GRID,
                    std::round(position.y / GRID) * GRID,
                };
                auto parent = anchor->getParent();
                auto worldDelta =
                    parent->convertToWorldSpace(snapped) - parent->convertToWorldSpace(position);
                for (auto node : group) applyWorldCorrection(node, worldDelta);
            }
        }
        keepSelectedGroupsReachable();
        auto after = captureStates(m_selectedNodes);
        if (statesDiffer(m_dragStart, after)) {
            m_undo.push_back({m_dragStart, std::move(after)});
            m_redo.clear();
            m_hasEdits = true;
            for (auto node : m_selectedNodes) markTransformDirty(node);
        }
        m_dragging = false;
        m_editControlPressed = false;
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

private:
    struct NodeState {
        Ref<CCNode> node;
        CCPoint position;
        float scaleX;
        float scaleY;
    };

    struct EditAction {
        std::vector<NodeState> before;
        std::vector<NodeState> after;
    };

    PauseLayer* m_owner = nullptr;
    CCNode* m_selected = nullptr;
    std::vector<CCNode*> m_selectedNodes;
    CCPoint m_pressWorld;
    std::vector<NodeState> m_dragStart;
    bool m_editing = false;
    bool m_dragging = false;
    bool m_pendingDrag = false;
    bool m_editControlPressed = false;
    bool m_hasEdits = false;
    bool m_hasLastSelectionPoint = false;
    CCPoint m_lastSelectionPoint = CCPointZero;
    CCLabelBMFont* m_modeLabel = nullptr;
    CCLabelBMFont* m_selectionLabel = nullptr;
    CCMenuItemSpriteExtra* m_toggle = nullptr;
    CCMenu* m_historyMenu = nullptr;
    CCMenuItemSpriteExtra* m_undoButton = nullptr;
    CCMenuItemSpriteExtra* m_redoButton = nullptr;
    CCMenuItemSpriteExtra* m_resetButton = nullptr;
    CCMenuItemSpriteExtra* m_hideButton = nullptr;
    CCMenuItemSpriteExtra* m_trashButton = nullptr;
    CCSprite* m_toggleEditIcon = nullptr;
    CCSprite* m_toggleDoneIcon = nullptr;
    CCMenu* m_profileMenu = nullptr;
    CCMenu* m_scaleMenu = nullptr;
    CCDrawNode* m_selectionOutline = nullptr;
    std::vector<CCDrawNode*> m_gridLines;
    std::vector<CCMenuItemSpriteExtra*> m_controlItems;
    std::vector<InitialPosition> m_initialPositions;
    std::vector<EditAction> m_undo;
    std::vector<EditAction> m_redo;
    std::vector<WeakRef<CCNode>> m_dirtyTransforms;
    ListenerHandle m_undoKey;
    ListenerHandle m_redoKey;
    ListenerHandle m_leftKey;
    ListenerHandle m_rightKey;
    ListenerHandle m_upKey;
    ListenerHandle m_downKey;
    ListenerHandle m_resetKey;
    ListenerHandle m_deleteKey;
    ListenerHandle m_backspaceKey;

    static bool isAncestorOf(CCNode* possibleAncestor, CCNode* node) {
        for (auto current = node ? node->getParent() : nullptr; current; current = current->getParent()) {
            if (current == possibleAncestor) return true;
        }
        return false;
    }

    void clearSelection() {
        m_selectedNodes.clear();
        m_selected = nullptr;
    }

    void addNodeToSelection(CCNode* node) {
        if (!node || node == this || node->getID() == EDITOR_ID) return;
        if (std::ranges::find(m_selectedNodes, node) != m_selectedNodes.end()) return;
        for (auto existing : m_selectedNodes) {
            if (isAncestorOf(existing, node)) return;
        }
        std::erase_if(m_selectedNodes, [node](CCNode* existing) {
            return isAncestorOf(node, existing);
        });
        m_selectedNodes.push_back(node);
        m_selected = node;
        rememberInitialPosition(node);
    }

    void selectOnly(std::vector<CCNode*> const& nodes) {
        clearSelection();
        for (auto node : nodes) addNodeToSelection(node);
    }

    void addToSelection(std::vector<CCNode*> const& nodes) {
        for (auto node : nodes) addNodeToSelection(node);
    }

    bool isVolumeGroupMember(CCNode* node, std::string_view type) const {
        if (!node) return false;
        auto prefix = std::string(type);
        return
            hasLogicalID(node, prefix + "-slider") ||
            hasLogicalID(node, prefix + "-label") ||
            hasLogicalID(node, prefix + "-label-menu") ||
            hasLogicalID(node, prefix + "-mute-menu");
    }

    std::vector<CCNode*> selectedVolumeGroup(std::string_view type) const {
        std::vector<CCNode*> result;
        for (auto node : m_selectedNodes) {
            if (isVolumeGroupMember(node, type)) result.push_back(node);
        }
        return result;
    }

    size_t logicalSelectionCount() const {
        auto count = m_selectedNodes.size();
        for (auto type : {std::string_view("music"), std::string_view("sfx")}) {
            auto const members = selectedVolumeGroup(type).size();
            if (members > 1) count -= members - 1;
        }
        return count;
    }

    std::vector<std::vector<CCNode*>> selectedLogicalGroups() const {
        std::vector<std::vector<CCNode*>> result;
        std::unordered_set<CCNode*> grouped;
        for (auto type : {std::string_view("music"), std::string_view("sfx")}) {
            auto members = selectedVolumeGroup(type);
            if (members.empty()) continue;
            for (auto node : members) grouped.insert(node);
            result.push_back(std::move(members));
        }
        for (auto node : m_selectedNodes) {
            if (node && !grouped.contains(node)) result.push_back({node});
        }
        return result;
    }

    std::string groupLabel(std::vector<CCNode*> const& group) const {
        if (group.empty()) return "Unnamed block";
        if (isVolumeGroupMember(group.front(), "music")) return "Music controls";
        if (isVolumeGroupMember(group.front(), "sfx")) return "SFX controls";
        auto node = group.front();
        if (hasLogicalID(node, "level-card")) return "Level info";
        if (hasLogicalID(node, "song-card")) return "Music info";
        if (hasLogicalID(node, "coins-card")) return "Coins";
        if (hasLogicalID(node, "difficulty-card")) return "Difficulty";
        if (node == m_toggle) return "Edit button";
        auto id = std::string(node->getID());
        return id.empty() ? "Unnamed block" : id;
    }

    bool hasHistoricalTransform(CCNode* node) const {
        if (!node) return false;
        auto mod = Mod::get();
        if (mod->hasSavedValue(xKey(node)) && mod->hasSavedValue(yKey(node))) return true;

        auto path = nodePath(node);
        if (path.empty()) return false;
        constexpr std::string_view xPrefix = "position-x/";
        constexpr std::string_view yPrefix = "position-y/";
        auto const& saved = mod->getSaveContainer();
        for (auto const& item : saved) {
            auto storedKey = item.getKey();
            if (!storedKey) continue;
            std::string_view key = *storedKey;
            if (!key.starts_with(xPrefix) || !key.ends_with(path)) continue;
            auto transformKey = key.substr(xPrefix.size());
            if (saved.contains(std::string(yPrefix) + std::string(transformKey))) return true;
        }
        return false;
    }

    size_t mergeHiddenSnapshot(
        pause_menu_studio::profiles::Snapshot const& snapshot,
        std::map<std::string, pause_menu_studio::hidden_blocks::Entry>& entriesByID
    ) const {
        std::unordered_set<std::string> trackedPaths;
        for (auto const& [id, entry] : entriesByID) {
            for (auto const& member : entry.members) trackedPaths.insert(member.path);
        }

        size_t recovered = 0;
        for (auto const& [path, transform] : snapshot) {
            if (!transform.hidden.value_or(false) || trackedPaths.contains(path)) continue;
            auto preferredID = transform.hiddenID.value_or(path);
            auto id = preferredID;
            if (auto found = entriesByID.find(id); found == entriesByID.end()) {
                id = pause_menu_studio::hidden_blocks::uniqueID(preferredID);
            }
            auto& entry = entriesByID[id];
            entry.id = id;
            if (entry.label.empty()) entry.label = transform.hiddenLabel.value_or(path);
            auto node = findNodeByStoredPath(m_owner, path);
            entry.members.push_back({
                path,
                transform.position,
                transform.scaleX.value_or(node ? node->getScaleX() : 1.f),
                transform.scaleY.value_or(node ? node->getScaleY() : 1.f),
            });
            trackedPaths.insert(path);
            ++recovered;
        }
        return recovered;
    }

    size_t reconcileInvisibleManagedBlocks() {
        auto storedEntries = pause_menu_studio::hidden_blocks::entries();
        std::map<std::string, pause_menu_studio::hidden_blocks::Entry> entriesByID;
        std::unordered_map<std::string, std::string> pathToID;
        for (auto const& entry : storedEntries) {
            entriesByID[entry.id] = entry;
            for (auto const& member : entry.members) pathToID[member.path] = entry.id;
        }

        std::unordered_set<std::string> changedIDs;
        std::unordered_set<std::string> processedGroups;
        size_t recovered = 0;
        std::function<void(CCNode*)> visit = [&](CCNode* node) {
            if (!node || node->getID() == EDITOR_ID) return;
            if (
                node != m_owner && node != m_toggle && !node->isVisible() &&
                hasHistoricalTransform(node)
            ) {
                auto group = logicalSelection(node, m_owner);
                std::vector<CCNode*> missing;
                std::vector<std::string> missingPaths;
                std::string existingID;
                for (auto member : group) {
                    if (!member || member == m_toggle || member->isVisible() || !hasHistoricalTransform(member)) {
                        continue;
                    }
                    auto path = nodePath(member);
                    if (path.empty()) continue;
                    if (auto found = pathToID.find(path); found != pathToID.end()) {
                        if (existingID.empty()) existingID = found->second;
                        continue;
                    }
                    missing.push_back(member);
                    missingPaths.push_back(std::move(path));
                }
                if (!missing.empty()) {
                    auto signatureParts = missingPaths;
                    std::ranges::sort(signatureParts);
                    std::string signature;
                    for (auto const& path : signatureParts) signature += path + "\n";
                    if (processedGroups.insert(signature).second) {
                        auto preferredID = *std::ranges::min_element(missingPaths);
                        auto id = existingID.empty()
                            ? pause_menu_studio::hidden_blocks::uniqueID(preferredID)
                            : existingID;
                        auto& entry = entriesByID[id];
                        entry.id = id;
                        if (entry.label.empty()) entry.label = groupLabel(group);
                        for (size_t index = 0; index < missing.size(); ++index) {
                            auto member = missing[index];
                            entry.members.push_back({
                                missingPaths[index], member->getPosition(), member->getScaleX(), member->getScaleY()
                            });
                            pathToID[missingPaths[index]] = id;
                            ++recovered;
                        }
                        changedIDs.insert(id);
                    }
                }
            }
            if (isInformationCard(node)) return;
            for (auto child : CCArrayExt<CCNode*>(node->getChildren())) visit(child);
        };
        visit(m_owner);

        for (auto const& id : changedIDs) {
            pause_menu_studio::hidden_blocks::upsert(entriesByID[id]);
        }
        return recovered;
    }

    size_t reconcileHiddenBlocks() {
        size_t recovered = 0;
        auto entries = pause_menu_studio::hidden_blocks::entries();
        std::map<std::string, pause_menu_studio::hidden_blocks::Entry> entriesByID;
        for (auto const& entry : entries) entriesByID[entry.id] = entry;

        auto active = Mod::get()->getSavedValue<std::string>("active-named-layout", "");
        if (!active.empty()) {
            if (auto snapshot = pause_menu_studio::profiles::load(active)) {
                auto before = entriesByID;
                recovered += mergeHiddenSnapshot(*snapshot, entriesByID);
                for (auto const& [id, entry] : entriesByID) {
                    auto old = before.find(id);
                    if (old == before.end() || old->second.members.size() != entry.members.size()) {
                        pause_menu_studio::hidden_blocks::upsert(entry);
                    }
                }
            }
        }
        recovered += reconcileInvisibleManagedBlocks();
        return recovered;
    }

    std::vector<NodeState> captureStates(std::vector<CCNode*> const& nodes) const {
        std::vector<NodeState> result;
        result.reserve(nodes.size());
        for (auto node : nodes) {
            if (!node) continue;
            result.push_back({node, node->getPosition(), node->getScaleX(), node->getScaleY()});
        }
        return result;
    }

    static void applyStates(std::vector<NodeState> const& states) {
        for (auto const& state : states) {
            if (!state.node) continue;
            state.node->setPosition(state.position);
            state.node->setScaleX(state.scaleX);
            state.node->setScaleY(state.scaleY);
        }
    }

    void markTransformDirty(CCNode* node) {
        if (!node) return;
        auto found = std::ranges::any_of(m_dirtyTransforms, [node](auto const& reference) {
            auto stored = reference.lock();
            return stored && stored.data() == node;
        });
        if (!found) m_dirtyTransforms.emplace_back(node);
    }

    void commitDirtyTransforms() {
        if (m_dirtyTransforms.empty()) return;
        for (auto const& reference : m_dirtyTransforms) {
            if (auto node = reference.lock()) savePosition(node.data());
        }
        m_dirtyTransforms.clear();
        Mod::get()->setSavedValue<std::string>("active-named-layout", "");
    }

    static bool statesDiffer(std::vector<NodeState> const& before, std::vector<NodeState> const& after) {
        if (before.size() != after.size()) return true;
        for (size_t index = 0; index < before.size(); ++index) {
            if (
                before[index].node.data() != after[index].node.data() ||
                !before[index].position.equals(after[index].position) ||
                std::abs(before[index].scaleX - after[index].scaleX) > .0001f ||
                std::abs(before[index].scaleY - after[index].scaleY) > .0001f
            ) return true;
        }
        return false;
    }

    void beginPendingDrag(CCPoint world, bool editControl) {
        if (m_selectedNodes.empty()) return;
        m_pressWorld = world;
        m_dragStart = captureStates(m_selectedNodes);
        m_pendingDrag = true;
        m_dragging = false;
        m_editControlPressed = editControl;
    }

    static CCPoint localDelta(CCNode* node, CCPoint fromWorld, CCPoint toWorld) {
        if (!node || !node->getParent()) return CCPointZero;
        auto parent = node->getParent();
        return parent->convertToNodeSpace(toWorld) - parent->convertToNodeSpace(fromWorld);
    }

    void moveFromStartTo(CCPoint world) {
        for (auto const& state : m_dragStart) {
            if (!state.node) continue;
            state.node->setPosition(state.position + localDelta(state.node, m_pressWorld, world));
        }
        keepSelectedGroupsReachable();
    }

    void nudgeSelection(CCPoint worldDelta) {
        if (m_selectedNodes.empty()) return;
        if (m_dragging || m_pendingDrag) finishDrag();
        auto before = captureStates(m_selectedNodes);
        for (auto node : m_selectedNodes) {
            node->setPosition(node->getPosition() + localDelta(node, CCPointZero, worldDelta));
        }
        keepSelectedGroupsReachable();
        auto after = captureStates(m_selectedNodes);
        if (statesDiffer(before, after)) {
            m_undo.push_back({std::move(before), std::move(after)});
            m_redo.clear();
            m_hasEdits = true;
            for (auto node : m_selectedNodes) markTransformDirty(node);
        }
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void scaleSelection(float factor) {
        if (!m_editing || m_selectedNodes.empty()) return;
        if (m_dragging || m_pendingDrag) finishDrag();
        auto before = captureStates(m_selectedNodes);
        for (auto node : m_selectedNodes) {
            rememberInitialPosition(node);
            auto largest = std::max(std::abs(node->getScaleX()), std::abs(node->getScaleY()));
            if (largest <= .0001f) largest = 1.f;
            auto target = std::clamp(largest * factor, .15f, 3.5f);
            auto applied = target / largest;
            node->setScaleX(node->getScaleX() * applied);
            node->setScaleY(node->getScaleY() * applied);
        }
        keepSelectedGroupsReachable();
        auto after = captureStates(m_selectedNodes);
        if (statesDiffer(before, after)) {
            m_undo.push_back({std::move(before), std::move(after)});
            m_redo.clear();
            m_hasEdits = true;
            for (auto node : m_selectedNodes) markTransformDirty(node);
        }
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void resetSelectionScale() {
        if (!m_editing || m_selectedNodes.empty()) return;
        if (m_dragging || m_pendingDrag) finishDrag();
        auto before = captureStates(m_selectedNodes);
        for (auto node : m_selectedNodes) {
            rememberInitialPosition(node);
            auto found = std::ranges::find_if(m_initialPositions, [node](auto const& initial) {
                auto remembered = initial.node.lock();
                return remembered && remembered.data() == node;
            });
            if (found == m_initialPositions.end()) continue;
            node->setScaleX(found->scaleX);
            node->setScaleY(found->scaleY);
        }
        keepSelectedGroupsReachable();
        auto after = captureStates(m_selectedNodes);
        if (statesDiffer(before, after)) {
            m_undo.push_back({std::move(before), std::move(after)});
            m_redo.clear();
            m_hasEdits = true;
            for (auto node : m_selectedNodes) markTransformDirty(node);
        }
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void buildControls() {
        auto size = getContentSize();
        auto makeIcon = [](char const* frame, float maxSize) {
            auto icon = CCSprite::createWithSpriteFrameName(frame);
            auto largest = std::max(icon->getContentWidth(), icon->getContentHeight());
            if (largest > .001f) icon->setScale(maxSize / largest);
            return icon;
        };

        auto menu = CCMenu::create();
        menu->setID("editor-controls");
        menu->setPosition({size.width - 38.f, size.height - 22.f});
        auto toggleVisual = CCNode::create();
        toggleVisual->setContentSize({34.f, 34.f});
        m_toggleEditIcon = makeIcon("GJ_editBtn_001.png", 34.f);
        m_toggleDoneIcon = makeIcon("GJ_checkOn_001.png", 31.f);
        for (auto icon : {m_toggleEditIcon, m_toggleDoneIcon}) {
            icon->setPosition(toggleVisual->getContentSize() / 2.f);
            toggleVisual->addChild(icon);
        }
        m_toggle = CCMenuItemSpriteExtra::create(toggleVisual, this, menu_selector(PauseEditor::onToggle));
        m_toggle->setID("editor-toggle-button");
        menu->addChild(m_toggle);
        m_controlItems.push_back(m_toggle);
        addChild(menu, 20);
        m_initialPositions.push_back({
            WeakRef<CCNode>(m_toggle), m_toggle->getPosition(), m_toggle->getScaleX(), m_toggle->getScaleY()
        });
        restorePositions(menu);

        m_historyMenu = CCMenu::create();
        m_historyMenu->setID("editor-history-controls");
        m_historyMenu->setPosition({size.width / 2.f, 12.f});

        auto makeControl = [this, &makeIcon](
            CCMenu* targetMenu,
            char const* frame,
            float x,
            SEL_MenuHandler callback,
            char const* id,
            float maxSize = 28.f
        ) {
            auto icon = makeIcon(frame, maxSize);
            auto button = CCMenuItemSpriteExtra::create(icon, this, callback);
            button->setID(id);
            button->setPositionX(x);
            targetMenu->addChild(button);
            m_controlItems.push_back(button);
            return button;
        };
        m_undoButton = makeControl(m_historyMenu, "GJ_undoBtn_001.png", -42.f, menu_selector(PauseEditor::onUndo), "undo-button");
        m_redoButton = makeControl(m_historyMenu, "GJ_redoBtn_001.png", 0.f, menu_selector(PauseEditor::onRedo), "redo-button");
        m_resetButton = makeControl(m_historyMenu, "GJ_replayBtn_001.png", 42.f, menu_selector(PauseEditor::onReset), "reset-button");
        addChild(m_historyMenu, 20);

        m_profileMenu = CCMenu::create();
        m_profileMenu->setID("editor-layout-profile-controls");
        m_profileMenu->setPosition({size.width / 2.f, 45.f});
        makeControl(m_profileMenu, "GJ_downloadBtn_001.png", -63.f, menu_selector(PauseEditor::onSaveProfile), "save-layout-button");
        makeControl(m_profileMenu, "GJ_viewListsBtn_001.png", -21.f, menu_selector(PauseEditor::onLayouts), "saved-layouts-button");
        m_hideButton = makeControl(m_profileMenu, "hideBtn_001.png", 21.f, menu_selector(PauseEditor::onHide), "hide-block-button");
        m_trashButton = makeControl(m_profileMenu, "GJ_trashBtn_001.png", 63.f, menu_selector(PauseEditor::onTrash), "hidden-blocks-button");
        addChild(m_profileMenu, 20);

        m_scaleMenu = CCMenu::create();
        m_scaleMenu->setID("editor-scale-controls");
        m_scaleMenu->setPosition({size.width / 2.f, size.height - 17.f});
        makeControl(m_scaleMenu, "GJ_zoomOutBtn_001.png", -42.f, menu_selector(PauseEditor::onScaleDown), "scale-down-button");
        makeControl(m_scaleMenu, "edit_eResetBtn_001.png", 0.f, menu_selector(PauseEditor::onScaleReset), "scale-reset-button");
        makeControl(m_scaleMenu, "GJ_zoomInBtn_001.png", 42.f, menu_selector(PauseEditor::onScaleUp), "scale-up-button");
        addChild(m_scaleMenu, 20);

        m_modeLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_modeLabel->setAnchorPoint({1.f, .5f});
        m_modeLabel->setPosition({size.width - 8.f, size.height - 43.f});
        m_modeLabel->setScale(.25f);
        m_modeLabel->setColor({100, 255, 170});
        addChild(m_modeLabel, 20);

        m_selectionLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_selectionLabel->setAnchorPoint({0.f, .5f});
        m_selectionLabel->setPosition({6.f, size.height - 8.f});
        m_selectionLabel->setScale(.4f);
        m_selectionLabel->setOpacity(190);
        addChild(m_selectionLabel, 20);

        m_selectionOutline = CCDrawNode::create();
        m_selectionOutline->setID("selection-outline");
        addChild(m_selectionOutline, 19);
    }

    void onToggle(CCObject*) { setEditing(!m_editing); }
    void onUndo(CCObject*) { undo(); }
    void onRedo(CCObject*) { redo(); }
    void onScaleDown(CCObject*) { scaleSelection(.9f); }
    void onScaleUp(CCObject*) { scaleSelection(1.1f); }
    void onScaleReset(CCObject*) { resetSelectionScale(); }
    void onHide(CCObject*) { hideSelection(); }
    void onTrash(CCObject*) {
        auto recovered = reconcileHiddenBlocks();
        if (recovered > 0) {
            Notification::create(
                recovered == 1
                    ? "Missing hidden block returned to Trash"
                    : std::to_string(recovered) + " missing hidden blocks returned to Trash",
                NotificationIcon::Success
            )->show();
        }
        auto self = WeakRef<PauseEditor>(this);
        if (auto popup = pause_menu_studio::HiddenBlocksPopup::create(
            pause_menu_studio::hidden_blocks::entries(),
            [self](std::string id) -> bool {
                if (auto editor = self.lock()) return editor->restoreHiddenBlock(id);
                return false;
            }
        )) popup->show();
    }
    void onSaveProfile(CCObject*) {
        auto self = WeakRef<PauseEditor>(this);
        if (auto popup = pause_menu_studio::LayoutNamePopup::create([self](std::string name) {
            if (auto editor = self.lock()) editor->saveNamedLayout(name);
        })) popup->show();
    }
    void onLayouts(CCObject*) {
        auto self = WeakRef<PauseEditor>(this);
        auto active = Mod::get()->getSavedValue<std::string>("active-named-layout", "");
        if (auto popup = pause_menu_studio::LayoutListPopup::create(
            pause_menu_studio::profiles::names(),
            active,
            [self](std::string name) {
                if (auto editor = self.lock()) editor->applyNamedLayout(name);
            },
            [self](std::string name) {
                pause_menu_studio::profiles::erase(name);
                if (Mod::get()->getSavedValue<std::string>("active-named-layout", "") == name) {
                    Mod::get()->setSavedValue<std::string>("active-named-layout", "");
                }
                if (auto editor = self.lock()) {
                    Notification::create("Layout deleted", NotificationIcon::Success)->show();
                }
            }
        )) popup->show();
    }
    void onReset(CCObject*) {
        if (!m_selectedNodes.empty()) {
            requestResetSelection();
            return;
        }
        auto self = WeakRef<PauseEditor>(this);
        createQuickPopup(
            "Reset layout",
            "Reset all moved elements and theme-only cards to their <cy>default state</c>?",
            "Cancel", "Reset",
            [self](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                if (auto editor = self.lock()) editor->resetLayout();
            }
        );
    }

    void requestResetSelection() {
        if (!m_editing || m_selectedNodes.empty()) return;
        auto count = logicalSelectionCount();
        auto self = WeakRef<PauseEditor>(this);
        createQuickPopup(
            count == 1 ? "Reset block" : "Reset blocks",
            count == 1
                ? "Reset the selected block to its <cy>default position and size</c>?"
                : fmt::format(
                    "Reset <cy>{}</c> selected blocks to their default positions and sizes?", count
                ),
            "Cancel", "Reset",
            [self](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                if (auto editor = self.lock()) editor->resetSelectedBlocks();
            }
        );
    }

    void setEditing(bool editing) {
        auto leavingEditor = m_editing && !editing;
        if (leavingEditor) {
            commitDirtyTransforms();
            m_hasEdits = false;
        }
        m_editing = editing;
        m_dragging = false;
        m_pendingDrag = false;
        if (!editing) clearSelection();
        if (!editing) m_hasLastSelectionPoint = false;
        m_modeLabel->setString(editing ? "EDIT MODE" : "");
        if (m_toggleEditIcon) m_toggleEditIcon->setVisible(!editing);
        if (m_toggleDoneIcon) m_toggleDoneIcon->setVisible(editing);
        for (auto line : m_gridLines) line->setVisible(editing);
        m_historyMenu->setVisible(editing);
        m_profileMenu->setVisible(editing);
        m_scaleMenu->setVisible(editing);
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void undo() {
        if (!m_editing) return;
        if (m_dragging) finishDrag();
        if (m_undo.empty()) return;

        auto action = std::move(m_undo.back());
        m_undo.pop_back();
        applyStates(action.before);
        for (auto const& state : action.before) if (state.node) markTransformDirty(state.node.data());
        clearSelection();
        for (auto const& state : action.before) if (state.node) addNodeToSelection(state.node);
        m_redo.push_back(std::move(action));
        m_hasEdits = true;
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void redo() {
        if (!m_editing) return;
        if (m_dragging) finishDrag();
        if (m_redo.empty()) return;

        auto action = std::move(m_redo.back());
        m_redo.pop_back();
        applyStates(action.after);
        for (auto const& state : action.after) if (state.node) markTransformDirty(state.node.data());
        clearSelection();
        for (auto const& state : action.after) if (state.node) addNodeToSelection(state.node);
        m_undo.push_back(std::move(action));
        m_hasEdits = true;
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void resetLayout() {
        if (!m_editing) return;
        m_hasEdits = true;
        for (auto const& entry : pause_menu_studio::hidden_blocks::entries()) {
            for (auto const& member : entry.members) {
                if (auto node = findNodeByStoredPath(m_owner, member.path)) node->setVisible(true);
            }
        }
        pause_menu_studio::hidden_blocks::clear();
        auto generation = Mod::get()->getSavedValue<int64_t>("layout-generation", 0);
        Mod::get()->setSavedValue<int64_t>("layout-generation", generation + 1);
        clearSelection();
        rebuildInformationCards({});
        for (auto const& initial : m_initialPositions) {
            if (auto node = initial.node.lock()) {
                node->setPosition(initial.position);
                node->setScaleX(initial.scaleX);
                node->setScaleY(initial.scaleY);
                if (node->getID() == "editor-toggle-button") savePosition(node);
            }
        }
        m_dragging = false;
        m_pendingDrag = false;
        m_undo.clear();
        m_redo.clear();
        Mod::get()->setSavedValue<std::string>("active-named-layout", "");
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
        Notification::create("Pause layout reset", NotificationIcon::Success)->show();
    }

    void resetSelectedBlocks() {
        if (!m_editing || m_selectedNodes.empty()) return;
        if (m_dragging || m_pendingDrag) finishDrag();
        auto before = captureStates(m_selectedNodes);
        for (auto node : m_selectedNodes) {
            rememberInitialPosition(node);
            auto found = std::ranges::find_if(m_initialPositions, [node](auto const& initial) {
                auto remembered = initial.node.lock();
                return remembered && remembered.data() == node;
            });
            if (found == m_initialPositions.end()) continue;
            node->setPosition(found->position);
            node->setScaleX(found->scaleX);
            node->setScaleY(found->scaleY);
            markTransformDirty(node);
        }
        auto after = captureStates(m_selectedNodes);
        if (statesDiffer(before, after)) {
            m_undo.push_back({std::move(before), std::move(after)});
            m_redo.clear();
            m_hasEdits = true;
        }
        Mod::get()->setSavedValue<std::string>("active-named-layout", "");
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
        Notification::create("Selected block reset", NotificationIcon::Success)->show();
    }

    void hideSelection() {
        if (!m_editing || m_selectedNodes.empty()) return;
        if (m_dragging || m_pendingDrag) finishDrag();
        size_t hiddenCount = 0;
        bool skippedEditButton = false;
        for (auto const& group : selectedLogicalGroups()) {
            if (group.empty()) continue;
            if (std::ranges::find(group, m_toggle) != group.end()) {
                skippedEditButton = true;
                continue;
            }

            pause_menu_studio::hidden_blocks::Entry entry;
            entry.label = groupLabel(group);
            for (auto node : group) {
                if (!node) continue;
                rememberInitialPosition(node);
                savePosition(node);
                entry.members.push_back({
                    nodePath(node), node->getPosition(), node->getScaleX(), node->getScaleY()
                });
            }
            if (entry.members.empty()) continue;
            auto preferredID = std::ranges::min_element(
                entry.members, {}, &pause_menu_studio::hidden_blocks::Member::path
            )->path;
            entry.id = pause_menu_studio::hidden_blocks::uniqueID(preferredID);
            // Never make a node disappear unless its complete restore record
            // can immediately be read back from the mod's saved storage.
            if (!pause_menu_studio::hidden_blocks::upsert(entry)) {
                Notification::create("Could not move block to trash", NotificationIcon::Error)->show();
                continue;
            }
            for (auto node : group) if (node) node->setVisible(false);
            ++hiddenCount;
        }

        clearSelection();
        m_dragging = false;
        m_pendingDrag = false;
        Mod::get()->setSavedValue<std::string>("active-named-layout", "");
        updateSelectionLabel();
        updateSelectionOutline();
        if (hiddenCount > 0) {
            m_hasEdits = true;
            Notification::create(
                hiddenCount == 1 ? "Block moved to trash" : "Blocks moved to trash",
                NotificationIcon::Success
            )->show();
        } else if (skippedEditButton) {
            Notification::create("The Edit button cannot be hidden", NotificationIcon::Warning)->show();
        }
    }

    CCNode* findNodeByStoredPath(CCNode* node, std::string const& path) const {
        if (!node || node->getID() == EDITOR_ID) return nullptr;
        if (node != m_owner && nodePath(node) == path) return node;
        for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
            if (auto found = findNodeByStoredPath(child, path)) return found;
        }
        return nullptr;
    }

    bool restoreHiddenBlock(std::string const& id) {
        auto entry = pause_menu_studio::hidden_blocks::find(id);
        if (!entry) {
            Notification::create("Hidden block could not be loaded", NotificationIcon::Error)->show();
            return false;
        }
        size_t restored = 0;
        for (auto const& member : entry->members) {
            auto node = findNodeByStoredPath(m_owner, member.path);
            if (!node) continue;
            rememberInitialPosition(node);
            node->setPosition(member.position);
            node->setScaleX(member.scaleX);
            node->setScaleY(member.scaleY);
            node->setVisible(true);
            savePosition(node);
            ++restored;
        }
        if (restored == 0) {
            Notification::create(
                "Block is unavailable in the current pause menu", NotificationIcon::Warning
            )->show();
            return false;
        }
        pause_menu_studio::hidden_blocks::erase(id);
        Mod::get()->setSavedValue<std::string>("active-named-layout", "");
        m_hasEdits = true;
        Notification::create("Block restored: " + entry->label, NotificationIcon::Success)->show();
        return true;
    }

    void rememberInitialPosition(CCNode* node) {
        if (!node) return;
        auto alreadyRemembered = std::ranges::any_of(m_initialPositions, [node](auto const& initial) {
            auto remembered = initial.node.lock();
            return remembered && remembered.data() == node;
        });
        if (!alreadyRemembered) {
            m_initialPositions.push_back({
                WeakRef<CCNode>(node), node->getPosition(), node->getScaleX(), node->getScaleY()
            });
        }
    }

    void saveNamedLayout(std::string const& name) {
        if (pause_menu_studio::profiles::exists(name)) {
            auto self = WeakRef<PauseEditor>(this);
            createQuickPopup(
                "Overwrite layout",
                "A layout named <cy>" + name + "</c> already exists. Replace it?",
                "Cancel", "Replace",
                [self, name](FLAlertLayer*, bool confirmed) {
                    if (!confirmed) return;
                    if (auto editor = self.lock()) editor->writeNamedLayout(name);
                }
            );
            return;
        }
        writeNamedLayout(name);
    }

    void writeNamedLayout(std::string const& name) {
        reconcileInvisibleManagedBlocks();
        pause_menu_studio::profiles::Snapshot snapshot;
        std::unordered_map<std::string, std::pair<std::string, std::string>> hiddenMembership;
        for (auto const& entry : pause_menu_studio::hidden_blocks::entries()) {
            for (auto const& member : entry.members) {
                hiddenMembership[member.path] = {entry.id, entry.label};
            }
        }
        captureCurrentSnapshot(m_owner, m_owner, snapshot, hiddenMembership);
        // Card presence belongs to the layout even when a card was never moved.
        // This lets Apply recreate a card whose normal mod setting is disabled.
        if (auto cards = m_owner->getChildByID(CARDS_ID)) {
            for (auto id : INFO_CARD_IDS) {
                auto card = cards->getChildByID(std::string(id));
                if (!card) continue;
                auto path = nodePath(card);
                pause_menu_studio::profiles::Transform transform {
                    card->getPosition(), card->getScaleX(), card->getScaleY()
                };
                if (auto hidden = hiddenMembership.find(path); hidden != hiddenMembership.end()) {
                    transform.hidden = true;
                    transform.hiddenID = hidden->second.first;
                    transform.hiddenLabel = hidden->second.second;
                } else {
                    transform.hidden = false;
                }
                snapshot[path] = std::move(transform);
            }
        }
        // Keep every Trash member in the profile even when a dynamic mod node
        // is temporarily absent from this particular pause-menu instance.
        for (auto const& entry : pause_menu_studio::hidden_blocks::entries()) {
            for (auto const& member : entry.members) {
                auto found = snapshot.find(member.path);
                if (found == snapshot.end()) {
                    pause_menu_studio::profiles::Transform transform {
                        member.position, member.scaleX, member.scaleY
                    };
                    transform.hidden = true;
                    transform.hiddenID = entry.id;
                    transform.hiddenLabel = entry.label;
                    snapshot[member.path] = std::move(transform);
                } else {
                    found->second.hidden = true;
                    found->second.hiddenID = entry.id;
                    found->second.hiddenLabel = entry.label;
                }
            }
        }
        pause_menu_studio::profiles::save(name, snapshot);
        Mod::get()->setSavedValue<std::string>("active-named-layout", name);
        Notification::create("Layout saved: " + name, NotificationIcon::Success)->show();
    }

    void applyNamedLayout(std::string const& name) {
        auto snapshot = pause_menu_studio::profiles::load(name);
        if (!snapshot) {
            Notification::create("Layout could not be loaded", NotificationIcon::Error)->show();
            return;
        }

        reconcileInvisibleManagedBlocks();
        rebuildInformationCards(informationCardsInSnapshot(*snapshot));
        auto hasHiddenState = std::ranges::any_of(*snapshot, [](auto const& item) {
            return item.second.hidden.has_value();
        });
        std::map<std::string, pause_menu_studio::hidden_blocks::Entry> hiddenEntries;
        if (hasHiddenState) {
            // A new-format profile owns the Trash state. Reveal only nodes
            // hidden by this mod before replacing that state with the profile.
            // Entries whose dynamic nodes are unavailable are retained instead
            // of being silently deleted from Trash.
            for (auto const& entry : pause_menu_studio::hidden_blocks::entries()) {
                pause_menu_studio::hidden_blocks::Entry unavailable {
                    entry.id, entry.label, {}
                };
                for (auto const& member : entry.members) {
                    if (auto node = findNodeByStoredPath(m_owner, member.path)) {
                        node->setVisible(true);
                    } else if (!isInformationCardPath(member.path) || snapshot->contains(member.path)) {
                        unavailable.members.push_back(member);
                    }
                }
                if (!unavailable.members.empty()) hiddenEntries[unavailable.id] = std::move(unavailable);
            }
            pause_menu_studio::hidden_blocks::clear();
        }
        auto generation = Mod::get()->getSavedValue<int64_t>("layout-generation", 0);
        Mod::get()->setSavedValue<int64_t>("layout-generation", generation + 1);
        applySnapshot(m_owner, *snapshot);
        if (hasHiddenState) {
            mergeHiddenSnapshot(*snapshot, hiddenEntries);
            for (auto& [id, entry] : hiddenEntries) {
                if (entry.id.empty()) entry.id = pause_menu_studio::hidden_blocks::uniqueID("hidden-block");
                pause_menu_studio::hidden_blocks::upsert(entry);
            }
        }
        // Old profiles have no visibility metadata, so their Apply operation
        // preserves the current Trash instead of revealing hidden controls.
        applyHiddenVisibility(m_owner);
        m_hasEdits = true;
        Mod::get()->setSavedValue<std::string>("active-named-layout", name);
        clearSelection();
        m_dragging = false;
        m_pendingDrag = false;
        m_undo.clear();
        m_redo.clear();
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
        Notification::create("Layout applied: " + name, NotificationIcon::Success)->show();
    }

    void rebuildInformationCards(std::unordered_set<std::string> const& forcedCards) {
        if (!m_owner) return;
        if (auto oldCards = m_owner->getChildByID(CARDS_ID)) {
            oldCards->removeFromParentAndCleanup(true);
        }
        if (auto cards = createInfoCards(forcedCards)) {
            auto size = CCDirector::sharedDirector()->getWinSize();
            cards->setPosition({size.width / 2.f, 72.f});
            m_owner->addChild(cards, 10);
            alignJukeboxSongBlock(cards);
            fixJukeboxDiscPosition(cards);
        }
    }

    void applySnapshot(CCNode* node, pause_menu_studio::profiles::Snapshot const& snapshot) {
        if (!node || node->getID() == EDITOR_ID) return;
        if (node != m_owner) {
            if (auto found = snapshot.find(nodePath(node)); found != snapshot.end()) {
                rememberInitialPosition(node);
                node->setPosition(found->second.position);
                if (found->second.scaleX) node->setScaleX(*found->second.scaleX);
                if (found->second.scaleY) node->setScaleY(*found->second.scaleY);
                savePosition(node);
            }
        }
        if (isInformationCard(node)) return;
        for (auto child : CCArrayExt<CCNode*>(node->getChildren())) applySnapshot(child, snapshot);
    }

    void updateHistoryButtons() {
        auto update = [](CCMenuItemSpriteExtra* button, bool enabled) {
            if (!button) return;
            button->setEnabled(enabled);
            if (auto image = typeinfo_cast<CCRGBAProtocol*>(button->getNormalImage())) {
                image->setOpacity(enabled ? 255 : 90);
            }
        };
        update(m_undoButton, !m_undo.empty());
        update(m_redoButton, !m_redo.empty());
    }

    void updateSelectionLabel() {
        if (!m_editing) { m_selectionLabel->setString(""); return; }
        if (m_selectedNodes.empty()) { m_selectionLabel->setString("Click to select | Ctrl+click: group"); return; }
        auto const logicalCount = logicalSelectionCount();
        if (logicalCount > 1) {
            m_selectionLabel->setString(fmt::format("Selected: {} elements", logicalCount).c_str());
            return;
        }
        if (!selectedVolumeGroup("music").empty()) {
            m_selectionLabel->setString("Selected: music controls");
            return;
        }
        if (!selectedVolumeGroup("sfx").empty()) {
            m_selectionLabel->setString("Selected: SFX controls");
            return;
        }
        auto id = m_selected->getID();
        auto name = id.empty() ? std::string("unnamed node") : std::string(id);
        m_selectionLabel->setString(("Selected: " + name).c_str());
    }

    struct OutlineBounds {
        float minX = 0.f;
        float minY = 0.f;
        float maxX = 0.f;
        float maxY = 0.f;
        bool valid = false;
    };

    void includeOutlineCorners(OutlineBounds& bounds, std::array<CCPoint, 4> const& corners) {
        for (auto const point : corners) {
            if (!bounds.valid) {
                bounds.minX = bounds.maxX = point.x;
                bounds.minY = bounds.maxY = point.y;
                bounds.valid = true;
                continue;
            }
            bounds.minX = std::min(bounds.minX, point.x);
            bounds.minY = std::min(bounds.minY, point.y);
            bounds.maxX = std::max(bounds.maxX, point.x);
            bounds.maxY = std::max(bounds.maxY, point.y);
        }
    }

    void includeVisibleBounds(CCNode* node, OutlineBounds& bounds) {
        if (!node || !node->isVisible()) return;

        // A Geometry Dash Slider uses a large technical touch node. Its node
        // content size is not the visible slider bar and produced a nearly
        // screen-sized editor outline. Only the groove and thumb are visual.
        if (auto slider = typeinfo_cast<Slider*>(node)) {
            if (slider->m_groove && slider->m_groove->isVisible()) {
                includeOutlineCorners(bounds, selectionCorners(slider->m_groove));
            }
            if (
                slider->m_touchLogic && slider->m_touchLogic->m_thumb &&
                slider->m_touchLogic->m_thumb->isVisible()
            ) {
                includeOutlineCorners(bounds, selectionCorners(slider->m_touchLogic->m_thumb));
            }
            return;
        }

        auto const size = node->getContentSize();
        auto const children = node->getChildrenCount();
        auto const visualNode =
            typeinfo_cast<CCSprite*>(node) ||
            typeinfo_cast<CCLabelBMFont*>(node) ||
            typeinfo_cast<CCLabelTTF*>(node) ||
            typeinfo_cast<CCMenuItem*>(node) ||
            typeinfo_cast<CCScale9Sprite*>(node) ||
            typeinfo_cast<CCLayerColor*>(node) ||
            children == 0;
        if (visualNode && size.width > 1.f && size.height > 1.f) {
            includeOutlineCorners(bounds, selectionCorners(node));
        }
        for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
            includeVisibleBounds(child, bounds);
        }
    }

    void drawOutlineBounds(OutlineBounds const& bounds, ccColor4F const& color) {
        if (!bounds.valid) return;
        std::array<CCPoint, 4> const corners {{
            {bounds.minX, bounds.minY},
            {bounds.maxX, bounds.minY},
            {bounds.maxX, bounds.maxY},
            {bounds.minX, bounds.maxY},
        }};
        for (size_t index = 0; index < corners.size(); ++index) {
            m_selectionOutline->drawSegment(corners[index], corners[(index + 1) % corners.size()], .8f, color);
        }
    }

    std::array<CCPoint, 4> selectionCorners(CCNode* node) {
        auto size = node->getContentSize();
        return {
            convertToNodeSpace(node->convertToWorldSpace({0.f, 0.f})),
            convertToNodeSpace(node->convertToWorldSpace({size.width, 0.f})),
            convertToNodeSpace(node->convertToWorldSpace({size.width, size.height})),
            convertToNodeSpace(node->convertToWorldSpace({0.f, size.height})),
        };
    }

    void updateSelectionOutline() {
        m_selectionOutline->clear();
        if (!m_editing || m_selectedNodes.empty()) return;
        auto const multiple = logicalSelectionCount() > 1;
        ccColor4F const color = multiple
            ? ccColor4F {1.f, .48f, .08f, 1.f}
            : ccColor4F {.15f, 1.f, 1.f, 1.f};

        std::unordered_set<CCNode*> groupedNodes;
        for (auto type : {std::string_view("music"), std::string_view("sfx")}) {
            auto const members = selectedVolumeGroup(type);
            if (members.empty()) continue;
            OutlineBounds bounds;
            for (auto node : members) {
                groupedNodes.insert(node);
                includeVisibleBounds(node, bounds);
            }
            drawOutlineBounds(bounds, color);
        }

        for (auto node : m_selectedNodes) {
            if (groupedNodes.contains(node)) continue;
            OutlineBounds bounds;
            includeVisibleBounds(node, bounds);
            drawOutlineBounds(bounds, color);
        }
    }

    void applyWorldCorrection(CCNode* node, CCPoint correction) {
        if (!node || !node->getParent() || correction.equals(CCPointZero)) return;
        auto parent = node->getParent();
        auto localOrigin = parent->convertToNodeSpace(CCPointZero);
        auto localCorrection = parent->convertToNodeSpace(correction) - localOrigin;
        node->setPosition(node->getPosition() + localCorrection);
    }

    void keepSelectedGroupsReachable() {
        auto screen = getContentSize();
        constexpr float visibleEdge = 18.f;
        for (auto const& group : selectedLogicalGroups()) {
            OutlineBounds bounds;
            for (auto node : group) includeVisibleBounds(node, bounds);
            if (!bounds.valid) continue;

            CCPoint correction {0.f, 0.f};
            if (bounds.maxX < visibleEdge) correction.x = visibleEdge - bounds.maxX;
            else if (bounds.minX > screen.width - visibleEdge) {
                correction.x = screen.width - visibleEdge - bounds.minX;
            }
            if (bounds.maxY < visibleEdge) correction.y = visibleEdge - bounds.maxY;
            else if (bounds.minY > screen.height - visibleEdge) {
                correction.y = screen.height - visibleEdge - bounds.minY;
            }
            for (auto node : group) applyWorldCorrection(node, correction);
        }
    }
};

void applyPreset(PauseLayer* layer) {
    auto preset = presetName();
    if (preset == "Classic") return;
    float scale = preset == "Compact" ? .82f : .9f;
    auto cards = layer->getChildByID(CARDS_ID);
    for (auto child : CCArrayExt<CCNode*>(layer->getChildren())) {
        if (child == cards || child->getID() == EDITOR_ID) continue;
        if (typeinfo_cast<CCLayerColor*>(child)) continue;
        child->setScaleX(child->getScaleX() * scale);
        child->setScaleY(child->getScaleY() * scale);
    }
}
}

class $modify(PauseMenuStudioPlayLayer, PlayLayer) {
    struct Fields {
        GJGameLevel* level = nullptr;
        gd::string originalLevelName;
    };

public:
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        auto originalLevelName = level ? level->m_levelName : gd::string();
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        m_fields->level = level;
        m_fields->originalLevelName = std::move(originalLevelName);
        return true;
    }

    std::string originalLevelName(GJGameLevel* level) {
        if (m_fields->level == level && !m_fields->originalLevelName.empty()) {
            return std::string(m_fields->originalLevelName);
        }
        return level ? std::string(level->m_levelName) : std::string();
    }
};

namespace {
std::string stableLevelName(PlayLayer* play, GJGameLevel* level) {
    if (!play || !level) return {};
    return static_cast<PauseMenuStudioPlayLayer*>(play)->originalLevelName(level);
}
}

$on_mod(Loaded) {
    if (!Mod::get()->getSavedValue<bool>("v1.4-card-defaults-applied", false)) {
        Mod::get()->setSettingValue<bool>("show-level", false);
        Mod::get()->setSettingValue<bool>("show-song", false);
        Mod::get()->setSavedValue<bool>("v1.4-card-defaults-applied", true);
    }
    // Demons In Between publishes its loaded GDDL table through this API event.
    // Keeping a copy lets the pause card use the mod's exact difficulty index.
    DemonsInBetweenLoadedEvent().listen([](std::map<int, LadderDemon> const& demons) {
        g_dibDemons = demons;
        return false;
    }).leak();
}

class $modify(PauseMenuStudioSongWidget, CustomSongWidget) {
    void refreshJukeboxPlacement() {
        alignJukeboxSongBlock(this);
        fixJukeboxDiscPosition(this);
    }

    void scheduleJukeboxPlacementRefresh() {
        constexpr int actionTag = 0x504D53;
        stopActionByTag(actionTag);
        auto action = CCSequence::create(
            CCCallFunc::create(this, callfunc_selector(PauseMenuStudioSongWidget::refreshJukeboxPlacement)),
            CCDelayTime::create(.05f),
            CCCallFunc::create(this, callfunc_selector(PauseMenuStudioSongWidget::refreshJukeboxPlacement)),
            CCDelayTime::create(.25f),
            CCCallFunc::create(this, callfunc_selector(PauseMenuStudioSongWidget::refreshJukeboxPlacement)),
            nullptr
        );
        action->setTag(actionTag);
        runAction(action);
    }

    void updateSongInfo() {
        CustomSongWidget::updateSongInfo();
        // Jukebox removes and recreates its pin menu when the active NONG or
        // title changes. Repair it over the next few frames so later popup and
        // layout callbacks cannot leave the new disc attached to the label.
        scheduleJukeboxPlacementRefresh();
    }

    void updateWithMultiAssets(gd::string songs, gd::string sfx, int size) {
        CustomSongWidget::updateWithMultiAssets(songs, sfx, size);
        scheduleJukeboxPlacementRefresh();
    }
};

class EntranceLayoutGuard final : public CCNode {
public:
    static EntranceLayoutGuard* create(PauseLayer* owner) {
        auto ret = new EntranceLayoutGuard();
        if (ret && ret->init(owner)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(PauseLayer* owner) {
        if (!CCNode::init() || !owner) return false;
        setID("entrance-layout-guard");
        // Cocos actions use a system priority. A large positive priority runs
        // after them, so animated menus cannot be drawn at an intermediate
        // position before the saved layout is reapplied.
        scheduleUpdateWithPriority(std::numeric_limits<int>::max() - 1);
        return true;
    }

    void update(float dt) override {
        // This guard is a direct child of PauseLayer. Do not keep a WeakRef to
        // that parent: Geode's WeakRefPool retains CCObjects internally, which
        // would form PauseLayer -> guard -> PauseLayer and keep the pause menu
        // alive when it is resumed before this one-second guard expires.
        auto owner = typeinfo_cast<PauseLayer*>(getParent());
        if (!owner) {
            unscheduleUpdate();
            return;
        }

        auto hasTemporaryEdit = false;
        if (auto editor = typeinfo_cast<PauseEditor*>(owner->getChildByID(EDITOR_ID))) {
            hasTemporaryEdit = editor->isDragging() || editor->shouldWarnBeforeExit();
        }
        if (!hasTemporaryEdit) {
            restorePositions(owner);
            applyHiddenVisibility(owner);
            alignJukeboxSongBlock(owner);
            fixJukeboxDiscPosition(owner);
        }

        m_elapsed += std::max(0.f, dt);
        if (m_elapsed >= 1.f) {
            unscheduleUpdate();
            removeFromParent();
        }
    }

private:
    float m_elapsed = 0.f;
};

class $modify(PauseMenuStudioLayer, PauseLayer) {
    void continueQuitAfterWarning() {
        // PauseLayer::onQuit performs more than PlayLayer::onQuit: it also
        // tears down the pause UI and starts the exit transition. Calling the
        // PlayLayer directly leaves an orphan PauseLayer whose next Resume
        // action dereferences GameManager::m_playLayer after it became null.
        PauseLayer::onQuit(this);
    }

    void tryQuit(CCObject* sender) {
        auto betterEscape = Loader::get()->getLoadedMod("ecuet.better-escape");
        auto shift = CCKeyboardDispatcher::get()->getShiftKeyPressed();
        auto actualExit = !betterEscape || shift || sender != nullptr;
        if (!actualExit) {
            PauseLayer::tryQuit(sender);
            return;
        }

        auto editor = typeinfo_cast<PauseEditor*>(getChildByID(EDITOR_ID));
        if (!editor || !editor->shouldWarnBeforeExit()) {
            PauseLayer::tryQuit(sender);
            return;
        }

        auto self = WeakRef<PauseMenuStudioLayer>(this);
        createQuickPopup(
            "Exit level?",
            "You changed the pause-menu layout in <cy>Edit Mode</c>. Exit the level anyway?",
            "Stay", "Exit",
            [self](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                if (auto layer = self.lock()) layer->continueQuitAfterWarning();
            }
        );
    }

    void customSetup() {
        PauseLayer::customSetup();
        if (!Mod::get()->getSettingValue<bool>("enabled")) return;

        auto const savedSchema = Mod::get()->getSavedValue<int>("layout-schema", 0);
        if (savedSchema < LAYOUT_SCHEMA) {
            auto generation = Mod::get()->getSavedValue<int64_t>("layout-generation", 0);
            Mod::get()->setSavedValue<int64_t>("layout-generation", generation + 1);
            Mod::get()->setSavedValue<int>("layout-schema", LAYOUT_SCHEMA);
        }

        auto size = CCDirector::sharedDirector()->getWinSize();
        if (auto cards = createInfoCards(informationCardsInActiveLayout())) {
            cards->setPosition({size.width / 2.f, 72.f});
            addChild(cards, 10);
        }
        if (auto guard = VolumeGeometryGuard::create(this)) addChild(guard, 999);
        applyPreset(this);
        std::vector<InitialPosition> initialPositions;
        captureResetPositions(this, this, initialPositions);
        restorePositions(this);
        applyHiddenVisibility(this);
        alignJukeboxSongBlock(this);
        fixJukeboxDiscPosition(this);
        if (auto editor = PauseEditor::create(this, std::move(initialPositions))) addChild(editor, 1000);
        if (auto guard = EntranceLayoutGuard::create(this)) addChild(guard, 998);
    }
};
