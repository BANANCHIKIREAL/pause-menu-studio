#include <Geode/Geode.hpp>
#include <Geode/modify/CustomSongWidget.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/utils/web.hpp>
#include <hiimjustin000.demons_in_between/include/DemonsInBetweenAPI.hpp>

#include "LayoutProfilePopups.hpp"
#include "LayoutProfiles.hpp"
#include "HiddenBlocks.hpp"
#include "HiddenBlocksPopup.hpp"
#include "BlockIcons.hpp"
#include "Sha256.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <functional>
#include <map>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;

namespace {
constexpr char const* EDITOR_ID = "pause-menu-editor";
constexpr char const* CARDS_ID = "pause-menu-cards";
constexpr char const* UPDATE_RELEASES_URL =
    "https://api.github.com/repos/BANANCHIKIREAL/pause-menu-studio/releases?per_page=10";
constexpr char const* UPDATE_USER_AGENT = "Pause-Menu-Studio-Updater/4.0.3";
constexpr char const* UPDATE_CACHE_KEY = "available-update-version";
constexpr float GRID = 5.f;
constexpr int LAYOUT_SCHEMA = 2;
constexpr std::array<std::string_view, 4> INFO_CARD_IDS {
    "level-card", "song-card", "coins-card", "difficulty-card"
};

std::map<int, LadderDemon> g_dibDemons;
bool g_startupUpdateCheckStarted = false;
std::optional<std::string> g_cachedAvailableUpdateVersion;

struct UpdateCandidate {
    VersionInfo version;
    std::string url;
    std::string digest;
};

template <class Releases>
std::optional<UpdateCandidate> latestUpdateCandidate(Releases const& releases) {
    std::optional<UpdateCandidate> best;
    auto const current = Mod::get()->getVersion();
    for (auto const& release : releases) {
        auto draft = release.template get<bool>("draft");
        if (draft.isOk() && draft.unwrap()) continue;
        auto tag = release.template get<std::string>("tag_name");
        if (tag.isErr()) continue;
        auto version = VersionInfo::parse(tag.unwrap());
        if (version.isErr() || version.unwrap() <= current) continue;

        auto assets = release.template get<std::vector<matjson::Value>>("assets");
        if (assets.isErr()) continue;
        std::string selectedURL;
        std::string selectedDigest;
        for (auto const& asset : assets.unwrap()) {
            auto name = asset.template get<std::string>("name");
            auto url = asset.template get<std::string>("browser_download_url");
            if (name.isErr() || url.isErr() || !name.unwrap().ends_with(".geode")) continue;
            auto digest = asset.template get<std::string>("digest");
            auto const exact = name.unwrap() == "bananchikireal.pause-menu-studio.geode";
            if (selectedURL.empty() || exact) {
                selectedURL = url.unwrap();
                selectedDigest = digest.isOk() ? digest.unwrap() : "";
            }
            if (exact) break;
        }
        if (selectedURL.empty()) continue;
        if (!best || version.unwrap() > best->version) {
            best = UpdateCandidate {version.unwrap(), std::move(selectedURL), std::move(selectedDigest)};
        }
    }
    return best;
}

void rememberAvailableUpdate(std::optional<UpdateCandidate> const& candidate) {
    auto value = candidate ? candidate->version.toVString() : std::string();
    g_cachedAvailableUpdateVersion = value;
    Mod::get()->setSavedValue<std::string>(UPDATE_CACHE_KEY, value);
}

std::string cachedAvailableUpdateVersion() {
    if (g_cachedAvailableUpdateVersion.has_value()) return *g_cachedAvailableUpdateVersion;
    auto cached = Mod::get()->getSavedValue<std::string>(UPDATE_CACHE_KEY, "");
    auto version = VersionInfo::parse(cached);
    if (version.isOk() && version.unwrap() > Mod::get()->getVersion()) {
        g_cachedAvailableUpdateVersion = version.unwrap().toVString();
    } else {
        g_cachedAvailableUpdateVersion = std::string();
        if (!cached.empty()) Mod::get()->setSavedValue<std::string>(UPDATE_CACHE_KEY, "");
    }
    return *g_cachedAvailableUpdateVersion;
}

TaskHolder<web::WebResponse>& startupUpdateRequest() {
    static TaskHolder<web::WebResponse> request;
    return request;
}

void showLoadingUpdateBanner(
    CCNode* banner,
    CCLabelBMFont* label,
    std::string const& version
) {
    if (!banner || !label || version.empty()) return;
    label->setString(fmt::format("PAUSE MENU STUDIO {} AVAILABLE!", version).c_str());
    auto scale = .38f;
    auto const availableWidth = banner->getContentWidth() - 18.f;
    auto const textWidth = label->getContentWidth();
    if (textWidth * scale > availableWidth && textWidth > .001f) {
        scale = availableWidth / textWidth;
    }
    label->setScale(scale);
    banner->setVisible(true);
}

std::string stableLevelName(PlayLayer* play, GJGameLevel* level);

std::string presetName() {
    return Mod::get()->getSettingValue<std::string>("preset");
}

bool isPlatformerPause() {
    auto play = PlayLayer::get();
    return play && play->m_level && play->m_level->isPlatformer();
}

bool isCreatorPause() {
    auto play = PlayLayer::get();
    return play && play->m_level && play->m_level->m_levelType == GJLevelType::Editor;
}

std::string pauseModeKey() {
    auto const platformer = isPlatformerPause();
    auto const creator = isCreatorPause();
    if (platformer && creator) return "platformer-creator";
    if (platformer) return "platformer";
    if (creator) return "creator";
    return "normal";
}

std::string activeLayoutStorageKey() {
    auto const mode = pauseModeKey();
    if (mode == "platformer") return "active-named-layout-platformer";
    if (mode == "creator") return "active-named-layout-creator";
    if (mode == "platformer-creator") return "active-named-layout-platformer-creator";
    return "active-named-layout";
}

std::string layoutGenerationStorageKey() {
    auto const mode = pauseModeKey();
    if (mode == "platformer") return "layout-generation-platformer";
    if (mode == "creator") return "layout-generation-creator";
    if (mode == "platformer-creator") return "layout-generation-platformer-creator";
    return "layout-generation";
}

std::string positionNamespace() {
    auto result = presetName();
    // Normal-mode keys intentionally keep their legacy shape. Only platformer
    // and creator variants get suffixes, so upgrading does not reset an
    // existing normal layout or mix menus with different button sets.
    auto const mode = pauseModeKey();
    if (mode != "normal") result += "/" + mode;
    return result;
}

std::string editorModeLabel() {
    auto const mode = pauseModeKey();
    if (mode == "platformer-creator") return "CREATOR PLATFORMER";
    if (mode == "platformer") return "PLATFORMER MODE";
    if (mode == "creator") return "CREATOR MODE";
    return "EDIT MODE";
}

std::string saveLayoutTitle() {
    auto const mode = pauseModeKey();
    if (mode == "platformer-creator") return "Save platformer creator layout";
    if (mode == "platformer") return "Save platformer layout";
    if (mode == "creator") return "Save creator layout";
    return "Save layout";
}

std::string layoutListTitle() {
    auto const mode = pauseModeKey();
    if (mode == "platformer-creator") return "Platformer creator layouts";
    if (mode == "platformer") return "Platformer layouts";
    if (mode == "creator") return "Creator layouts";
    return "Saved layouts";
}

ccColor3B editorAccentColor() {
    auto const mode = pauseModeKey();
    if (mode == "platformer-creator") return {255, 105, 185};
    if (mode == "creator") return {255, 145, 70};
    if (mode == "platformer") return {255, 205, 75};
    return {70, 235, 255};
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
        return positionNamespace() + "/persistent-editor-toggle-button";
    }
    auto generation = Mod::get()->getSavedValue<int64_t>(layoutGenerationStorageKey(), 0);
    std::string result = positionNamespace() + "/generation-" + std::to_string(generation);
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
    auto active = Mod::get()->getSavedValue<std::string>(activeLayoutStorageKey(), "");
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
    auto storedPaths = pause_menu_studio::hidden_blocks::memberPaths(pauseModeKey());
    std::unordered_set<std::string> paths(storedPaths.begin(), storedPaths.end());
    applyHiddenVisibility(root, paths);
}

struct InitialPosition {
    WeakRef<CCNode> node;
    CCPoint position;
    float scaleX;
    float scaleY;
};

struct HiddenSnapshotInfo {
    std::string id;
    std::string label;
    std::string iconFrame;
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
    std::unordered_map<std::string, HiddenSnapshotInfo> const& hiddenMembership
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
            transform.hiddenID = hidden->second.id;
            transform.hiddenLabel = hidden->second.label;
            if (!hidden->second.iconFrame.empty()) transform.hiddenIcon = hidden->second.iconFrame;
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
    for (auto current = node->getParent(); current; current = current->getParent()) {
        if (!current->isVisible()) return false;
    }
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
                !m_editing || m_preview ||
                input.action != KeyboardInputData::Action::Press ||
                !(input.modifiers & KeyboardModifier::Control)
            ) return false;
            undo();
            return true;
        });
        m_redoKey = KeyboardInputEvent(KEY_Y).listen([this](KeyboardInputData& input) {
            if (
                !m_editing || m_preview ||
                input.action != KeyboardInputData::Action::Press ||
                !(input.modifiers & KeyboardModifier::Control)
            ) return false;
            redo();
            return true;
        });
        auto bindArrow = [this](enumKeyCodes key, CCPoint delta) {
            return KeyboardInputEvent(key).listen([this, delta](KeyboardInputData& input) {
                if (
                    !m_editing || m_preview || !m_moveEnabled || m_selectedNodes.empty() ||
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
                !m_editing || m_preview || m_selectedNodes.empty() ||
                input.action != KeyboardInputData::Action::Press
            ) return false;
            requestResetSelection();
            return true;
        });
        auto bindRemove = [this](enumKeyCodes key) {
            return KeyboardInputEvent(key).listen([this](KeyboardInputData& input) {
                if (
                    !m_editing || m_preview || m_selectedNodes.empty() ||
                    input.action != KeyboardInputData::Action::Press
                ) return false;
                hideSelection();
                return true;
            });
        };
        m_deleteKey = bindRemove(KEY_Delete);
        buildControls();
        schedule(schedule_selector(PauseEditor::pollUpdateBadge), .5f);
        setEditing(Mod::get()->getSettingValue<bool>("edit-on-open"));
        return true;
    }

    bool isDragging() const { return m_editing && m_dragging; }
    bool shouldWarnBeforeExit() const { return m_editing && m_hasEdits; }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        if (!m_editing || !m_owner) return false;
        auto world = touch->getLocation();
        if (m_preview) {
            auto returnButton = m_previewReturnMenu
                ? m_previewReturnMenu->getChildByID("preview-return-button")
                : nullptr;
            return returnButton && pointInside(returnButton, world) ? false : true;
        }
        // In Edit mode the DONE button itself is selectable. A click still
        // exits Edit mode, while a real drag moves the button.
        if (m_toggle && pointInside(m_toggle, world)) {
            if (!m_moveEnabled) return false;
            m_hasLastSelectionPoint = false;
            selectOnly({m_toggle});
            beginPendingDrag(world, true);
            return true;
        }
        for (auto control : m_controlItems) {
            if (pointInside(control, world)) return false;
        }
        if (m_scaleSlider && m_contextPanel && m_contextPanel->isVisible()) {
            auto onGroove = m_scaleSlider->m_groove && pointInside(m_scaleSlider->m_groove, world);
            auto thumb = m_scaleSlider->m_touchLogic ? m_scaleSlider->m_touchLogic->m_thumb : nullptr;
            auto onThumb = thumb && pointInside(thumb, world);
            if (onGroove || onThumb) return false;
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
            if (!sameLogicalGroup(selections[selectionIndex], m_selectedNodes)) {
                selectOnly(selections[selectionIndex]);
            }
            m_lastSelectionPoint = world;
            m_hasLastSelectionPoint = true;
        }
        if (m_moveEnabled) beginPendingDrag(world, false);
        updateSelectionLabel();
        updateSelectionOutline();
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_editing || !m_moveEnabled || m_selectedNodes.empty() || (!m_pendingDrag && !m_dragging)) return;
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
        pulseContextPanel();
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
    bool m_preview = false;
    bool m_dragging = false;
    bool m_pendingDrag = false;
    bool m_editControlPressed = false;
    bool m_moveEnabled = false;
    bool m_animatingExit = false;
    bool m_updatingScaleControls = false;
    bool m_hasEdits = false;
    bool m_hasLastSelectionPoint = false;
    CCPoint m_lastSelectionPoint = CCPointZero;
    Label* m_modeLabel = nullptr;
    Label* m_statusLabel = nullptr;
    Label* m_selectionLabel = nullptr;
    Label* m_selectionMetaLabel = nullptr;
    CCMenuItemSpriteExtra* m_toggle = nullptr;
    CCMenu* m_toggleMenu = nullptr;
    CCMenu* m_historyMenu = nullptr;
    CCMenuItemSpriteExtra* m_undoButton = nullptr;
    CCMenuItemSpriteExtra* m_redoButton = nullptr;
    CCMenuItemSpriteExtra* m_updateButton = nullptr;
    CCNode* m_updateBadge = nullptr;
    CCMenuItemSpriteExtra* m_resetButton = nullptr;
    CCMenuItemSpriteExtra* m_hideButton = nullptr;
    CCMenuItemSpriteExtra* m_trashButton = nullptr;
    CCMenuItemSpriteExtra* m_moveButton = nullptr;
    CCSprite* m_moveIcon = nullptr;
    Label* m_moveLabel = nullptr;
    CCSprite* m_toggleEditIcon = nullptr;
    CCSprite* m_toggleDoneIcon = nullptr;
    CCScale9Sprite* m_toggleBackground = nullptr;
    CCScale9Sprite* m_modeBadge = nullptr;
    CCMenu* m_profileMenu = nullptr;
    CCMenu* m_scaleMenu = nullptr;
    Slider* m_scaleSlider = nullptr;
    TextInput* m_scaleInput = nullptr;
    CCScale9Sprite* m_bottomPanel = nullptr;
    CCScale9Sprite* m_contextPanel = nullptr;
    CCLayerColor* m_bottomAccent = nullptr;
    CCLayerColor* m_contextAccent = nullptr;
    CCMenu* m_previewReturnMenu = nullptr;
    bool m_contextWasVisible = false;
    CCDrawNode* m_selectionOutline = nullptr;
    std::array<CCLayerColor*, 4> m_focusShade {};
    std::vector<CCDrawNode*> m_gridLines;
    std::vector<CCNode*> m_controlItems;
    std::vector<InitialPosition> m_initialPositions;
    std::vector<EditAction> m_undo;
    std::vector<EditAction> m_redo;
    std::vector<WeakRef<CCNode>> m_dirtyTransforms;
    std::unordered_map<std::string, bool> m_moveModeByNode;
    std::unordered_map<CCMenuItemSpriteExtra*, CCScale9Sprite*> m_controlTiles;
    std::unordered_map<CCMenuItemSpriteExtra*, ccColor3B> m_controlColors;
    TaskHolder<web::WebResponse> m_updateCheckRequest;
    TaskHolder<web::WebResponse> m_updateDownloadRequest;
    Ref<Notification> m_updateNotice;
    bool m_updateBusy = false;
    std::string m_pendingUpdateVersion;
    ListenerHandle m_undoKey;
    ListenerHandle m_redoKey;
    ListenerHandle m_leftKey;
    ListenerHandle m_rightKey;
    ListenerHandle m_upKey;
    ListenerHandle m_downKey;
    ListenerHandle m_resetKey;
    ListenerHandle m_deleteKey;

    static bool isAncestorOf(CCNode* possibleAncestor, CCNode* node) {
        for (auto current = node ? node->getParent() : nullptr; current; current = current->getParent()) {
            if (current == possibleAncestor) return true;
        }
        return false;
    }

    void clearSelection() {
        m_selectedNodes.clear();
        m_selected = nullptr;
        m_moveEnabled = false;
        updateMoveToggle();
    }

    void restoreMoveModeForSelection() {
        if (m_selectedNodes.empty()) {
            m_moveEnabled = false;
        } else {
            m_moveEnabled = std::ranges::all_of(m_selectedNodes, [this](CCNode* node) {
                auto found = m_moveModeByNode.find(nodePath(node));
                return found != m_moveModeByNode.end() && found->second;
            });
        }
        updateMoveToggle();
    }

    void rememberMoveModeForSelection() {
        for (auto node : m_selectedNodes) {
            auto path = nodePath(node);
            if (!path.empty()) m_moveModeByNode[path] = m_moveEnabled;
        }
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
        restoreMoveModeForSelection();
    }

    void addToSelection(std::vector<CCNode*> const& nodes) {
        for (auto node : nodes) addNodeToSelection(node);
        restoreMoveModeForSelection();
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
        if (hasLogicalID(node, "play-time")) return "Platformer time";
        if (hasLogicalID(node, "full-restart-button")) return "Full restart";
        if (hasLogicalID(node, "retry-button")) return "Retry button";
        if (hasLogicalID(node, "play-button")) return "Resume button";
        if (hasLogicalID(node, "exit-button")) return "Exit button";
        if (hasLogicalID(node, "options-button")) return "Options button";
        if (hasLogicalID(node, "practice-button")) return "Practice button";
        if (hasLogicalID(node, "edit-button")) return "Level editor button";
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
                id = pause_menu_studio::hidden_blocks::uniqueID(preferredID, pauseModeKey());
            }
            auto& entry = entriesByID[id];
            entry.id = id;
            if (entry.label.empty()) entry.label = transform.hiddenLabel.value_or(path);
            if (entry.iconFrame.empty()) {
                entry.iconFrame = transform.hiddenIcon.value_or("GJ_infoBtn_001.png");
            }
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
        auto storedEntries = pause_menu_studio::hidden_blocks::entries(pauseModeKey());
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
                            ? pause_menu_studio::hidden_blocks::uniqueID(preferredID, pauseModeKey())
                            : existingID;
                        auto& entry = entriesByID[id];
                        entry.id = id;
                        if (entry.label.empty()) entry.label = groupLabel(group);
                        if (entry.iconFrame.empty()) {
                            entry.iconFrame = pause_menu_studio::block_icons::frameForNodes(missing);
                            if (entry.iconFrame.empty()) entry.iconFrame = "GJ_infoBtn_001.png";
                        }
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
            pause_menu_studio::hidden_blocks::upsert(entriesByID[id], pauseModeKey());
        }
        return recovered;
    }

    size_t reconcileHiddenBlocks() {
        size_t recovered = 0;
        auto entries = pause_menu_studio::hidden_blocks::entries(pauseModeKey());
        std::map<std::string, pause_menu_studio::hidden_blocks::Entry> entriesByID;
        for (auto const& entry : entries) entriesByID[entry.id] = entry;

        auto active = Mod::get()->getSavedValue<std::string>(activeLayoutStorageKey(), "");
        if (!active.empty()) {
            if (auto snapshot = pause_menu_studio::profiles::load(active)) {
                auto before = entriesByID;
                recovered += mergeHiddenSnapshot(*snapshot, entriesByID);
                for (auto const& [id, entry] : entriesByID) {
                    auto old = before.find(id);
                    if (old == before.end() || old->second.members.size() != entry.members.size()) {
                        pause_menu_studio::hidden_blocks::upsert(entry, pauseModeKey());
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
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), "");
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
        if (!m_moveEnabled || m_selectedNodes.empty()) return;
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
            CCSprite* icon = nullptr;
            if (std::string_view(frame).find('/') != std::string_view::npos) {
                icon = CCSprite::create(frame);
            } else {
                icon = CCSprite::createWithSpriteFrameName(frame);
            }
            if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
            auto largest = std::max(icon->getContentWidth(), icon->getContentHeight());
            if (largest > .001f) icon->setScale(maxSize / largest);
            return icon;
        };

        auto addPanelDepth = [](CCScale9Sprite* panel, CCSize panelSize) {
            auto shadow = CCScale9Sprite::create("square02_001.png");
            shadow->setAnchorPoint({0.f, 0.f});
            shadow->setPosition({3.f, -3.f});
            shadow->setContentSize(panelSize);
            shadow->setColor({0, 0, 0});
            shadow->setOpacity(145);
            panel->addChild(shadow, -3);
        };

        m_toggleMenu = CCMenu::create();
        m_toggleMenu->setID("editor-controls");
        m_toggleMenu->setPosition({size.width - 35.f, size.height - 25.f});
        auto toggleVisual = CCNode::create();
        toggleVisual->setContentSize({42.f, 42.f});
        // Keep the pause-menu EDIT button as the original GD icon. A stretched
        // panel texture behind it clashes with texture packs and looks like a
        // broken square around the button.
        m_toggleBackground = nullptr;
        m_toggleEditIcon = makeIcon("GJ_editBtn_001.png", 32.f);
        m_toggleDoneIcon = makeIcon("GJ_checkOn_001.png", 29.f);
        for (auto icon : {m_toggleEditIcon, m_toggleDoneIcon}) {
            icon->setPosition({21.f, 21.f});
            toggleVisual->addChild(icon);
        }
        m_toggle = CCMenuItemSpriteExtra::create(toggleVisual, this, menu_selector(PauseEditor::onToggle));
        m_toggle->setID("editor-toggle-button");
        m_toggleMenu->addChild(m_toggle);
        m_controlItems.push_back(m_toggle);
        addChild(m_toggleMenu, 20);
        m_initialPositions.push_back({
            WeakRef<CCNode>(m_toggle), m_toggle->getPosition(), m_toggle->getScaleX(), m_toggle->getScaleY()
        });
        restorePositions(m_toggleMenu);

        auto toolbarWidth = std::min(520.f, size.width - 8.f);
        m_bottomPanel = CCScale9Sprite::create("square02_001.png");
        m_bottomPanel->setContentSize({toolbarWidth, 70.f});
        m_bottomPanel->setColor({52, 35, 105});
        m_bottomPanel->setOpacity(190);
        m_bottomPanel->setCascadeOpacityEnabled(false);
        m_bottomPanel->setID("editor-bottom-toolbar");
        m_bottomPanel->setPosition({(size.width - toolbarWidth) / 2.f, 3.f});
        m_bottomPanel->setAnchorPoint({0.f, 0.f});
        addPanelDepth(m_bottomPanel, m_bottomPanel->getContentSize());
        auto const initialAccent = editorAccentColor();
        m_bottomAccent = CCLayerColor::create(
            {initialAccent.r, initialAccent.g, initialAccent.b, 255}, toolbarWidth - 12.f, 2.f
        );
        m_bottomAccent->setPosition({6.f, 65.f});
        m_bottomAccent->setOpacity(235);
        m_bottomPanel->addChild(m_bottomAccent, 4);

        auto brand = Label::create("PAUSE MENU STUDIO", "bigFont.fnt");
        brand->setAnchorPoint({0.f, .5f});
        brand->setPosition({10.f, 60.5f});
        brand->setScale(.25f);
        brand->setColor(editorAccentColor());
        brand->setOpacity(255);
        m_bottomPanel->addChild(brand, 4);
        auto beta = Label::create("V4.0.3", "bigFont.fnt");
        beta->setAnchorPoint({1.f, .5f});
        beta->setPosition({toolbarWidth - 10.f, 60.5f});
        beta->setScale(.20f);
        beta->setColor({190, 155, 255});
        beta->setOpacity(255);
        m_bottomPanel->addChild(beta, 4);

        addChild(m_bottomPanel, 20);
        m_historyMenu = CCMenu::create();
        m_historyMenu->setID("editor-history-controls");
        m_historyMenu->setPosition({toolbarWidth / 2.f, 29.f});
        m_bottomPanel->addChild(m_historyMenu);
        auto downloadIconFrame = Mod::get()->expandSpriteName("download-icon.png");
        auto layoutsIconFrame = Mod::get()->expandSpriteName("layouts-icon.png");
        auto moveIconFrame = Mod::get()->expandSpriteName("move-icon.png");
        auto resetIconFrame = Mod::get()->expandSpriteName("reset-icon.png");
        auto viewIconFrame = Mod::get()->expandSpriteName("view-icon.png");

        auto makeControl = [this, &makeIcon](
            CCMenu* targetMenu,
            char const* frame,
            float x,
            SEL_MenuHandler callback,
            char const* id,
            float maxSize,
            ccColor3B tileColor
        ) {
            auto icon = makeIcon(frame, maxSize);
            icon->setID(std::string(id) + "-icon");
            auto visual = CCNode::create();
            visual->setContentSize({48.f, 50.f});
            auto tile = CCScale9Sprite::create("square02_001.png");
            tile->setContentSize({46.f, 48.f});
            tile->setPosition({24.f, 25.f});
            tile->setColor(tileColor);
            tile->setOpacity(170);
            tile->setID(std::string(id) + "-tile");
            visual->addChild(tile, -2);
            icon->setPosition({24.f, 30.f});
            visual->addChild(icon);
            std::string caption;
            auto controlID = std::string(id);
            if (controlID == "undo-button") caption = "UNDO";
            else if (controlID == "redo-button") caption = "REDO";
            else if (controlID == "save-layout-button") caption = "SAVE";
            else if (controlID == "saved-layouts-button") caption = "LAYOUTS";
            else if (controlID == "updates-button") caption = "UPDATES";
            else if (controlID == "hidden-blocks-button") caption = "TRASH";
            else if (controlID == "reset-button") caption = "RESET";
            else if (controlID == "preview-button") caption = "VIEW";
            else if (controlID == "move-hint-button") caption = "MOVE";
            else if (controlID == "scale-reset-button") caption = "RESET";
            else if (controlID == "hide-block-button") caption = "HIDE";
            if (!caption.empty()) {
                auto label = Label::create(caption, "bigFont.fnt");
                label->setPosition({24.f, 6.5f});
                label->setScale(.22f);
                label->setColor(ccWHITE);
                label->setOpacity(255);
                label->setLimitLabelWidth(42.f, .22f, .16f);
                label->setID(controlID + "-label");
                visual->addChild(label);
            }
            auto button = CCMenuItemSpriteExtra::create(visual, this, callback);
            button->setID(id);
            button->setPositionX(x);
            targetMenu->addChild(button);
            m_controlItems.push_back(button);
            m_controlTiles[button] = tile;
            m_controlColors[button] = tileColor;
            return button;
        };
        std::array<float, 8> controlPositions {};
        auto const controlStep = (toolbarWidth - 72.f) / 7.f;
        for (size_t index = 0; index < controlPositions.size(); ++index) {
            controlPositions[index] = (static_cast<float>(index) - 3.5f) * controlStep;
        }
        m_undoButton = makeControl(m_historyMenu, "GJ_undoBtn_001.png", controlPositions[0], menu_selector(PauseEditor::onUndo), "undo-button", 26.f, {48, 62, 112});
        m_redoButton = makeControl(m_historyMenu, "GJ_redoBtn_001.png", controlPositions[1], menu_selector(PauseEditor::onRedo), "redo-button", 26.f, {48, 62, 112});
        makeControl(m_historyMenu, "GJ_downloadBtn_001.png", controlPositions[2], menu_selector(PauseEditor::onSaveProfile), "save-layout-button", 26.f, {45, 105, 88});
        makeControl(m_historyMenu, layoutsIconFrame.c_str(), controlPositions[3], menu_selector(PauseEditor::onLayouts), "saved-layouts-button", 26.f, {76, 52, 128});
        m_updateButton = makeControl(m_historyMenu, downloadIconFrame.c_str(), controlPositions[4], menu_selector(PauseEditor::onUpdates), "updates-button", 26.f, {38, 118, 105});
        if (auto visual = m_updateButton ? m_updateButton->getNormalImage() : nullptr) {
            auto badge = CCScale9Sprite::create("square02_001.png");
            badge->setID("updates-available-badge");
            badge->setContentSize({15.f, 15.f});
            badge->setPosition({39.f, 43.f});
            badge->setColor({225, 45, 55});
            badge->setOpacity(255);
            auto mark = Label::create("!", "bigFont.fnt");
            mark->setPosition({7.5f, 7.5f});
            mark->setScale(.38f);
            mark->setColor(ccWHITE);
            mark->setOpacity(255);
            badge->addChild(mark);
            badge->setVisible(false);
            visual->addChild(badge, 8);
            m_updateBadge = badge;
        }
        refreshUpdateBadge();
        m_trashButton = makeControl(m_historyMenu, "GJ_trashBtn_001.png", controlPositions[5], menu_selector(PauseEditor::onTrash), "hidden-blocks-button", 26.f, {116, 52, 86});
        m_resetButton = makeControl(m_historyMenu, resetIconFrame.c_str(), controlPositions[6], menu_selector(PauseEditor::onReset), "reset-button", 24.f, {132, 76, 38});
        makeControl(m_historyMenu, viewIconFrame.c_str(), controlPositions[7], menu_selector(PauseEditor::onPreview), "preview-button", 24.f, {38, 96, 132});

        for (auto pair : {std::pair {1u, 2u}, std::pair {3u, 4u}, std::pair {5u, 6u}}) {
            auto const localX = (controlPositions[pair.first] + controlPositions[pair.second]) / 2.f;
            auto separator = CCLayerColor::create({130, 100, 210, 75}, 1.f, 38.f);
            separator->setPosition({toolbarWidth / 2.f + localX, 8.f});
            m_bottomPanel->addChild(separator, 3);
        }

        auto contextWidth = std::min(404.f, size.width - 8.f);
        m_contextPanel = CCScale9Sprite::create("square02_001.png");
        m_contextPanel->setContentSize({contextWidth, 82.f});
        m_contextPanel->setColor({50, 34, 102});
        m_contextPanel->setOpacity(190);
        m_contextPanel->setCascadeOpacityEnabled(false);
        m_contextPanel->setAnchorPoint({0.f, 0.f});
        m_contextPanel->setID("selection-context-panel");
        m_contextPanel->setVisible(false);
        addPanelDepth(m_contextPanel, m_contextPanel->getContentSize());
        m_contextAccent = CCLayerColor::create(
            {initialAccent.r, initialAccent.g, initialAccent.b, 255}, contextWidth - 12.f, 2.f
        );
        m_contextAccent->setPosition({6.f, 77.f});
        m_contextAccent->setOpacity(245);
        m_contextPanel->addChild(m_contextAccent, 4);
        addChild(m_contextPanel, 22);
        m_scaleMenu = CCMenu::create();
        m_scaleMenu->setID("editor-scale-controls");
        m_scaleMenu->setPosition({contextWidth / 2.f, 27.f});
        m_moveButton = makeControl(
            m_scaleMenu, moveIconFrame.c_str(), -contextWidth / 2.f + 31.f,
            menu_selector(PauseEditor::onMoveToggle), "move-hint-button", 23.f, {62, 54, 118}
        );
        if (auto visual = m_moveButton ? m_moveButton->getNormalImage() : nullptr) {
            m_moveIcon = typeinfo_cast<CCSprite*>(visual->getChildByID("move-hint-button-icon"));
            m_moveLabel = typeinfo_cast<Label*>(visual->getChildByID("move-hint-button-label"));
        }
        makeControl(
            m_scaleMenu, resetIconFrame.c_str(), contextWidth / 2.f - 82.f,
            menu_selector(PauseEditor::onScaleReset), "scale-reset-button", 22.f, {124, 72, 38}
        );
        m_hideButton = makeControl(
            m_scaleMenu, "hideBtn_001.png", contextWidth / 2.f - 24.f,
            menu_selector(PauseEditor::onHide), "hide-block-button", 22.f, {112, 48, 76}
        );
        m_contextPanel->addChild(m_scaleMenu);
        m_scaleSlider = Slider::create(this, menu_selector(PauseEditor::onScaleSlider), .56f);
        m_scaleSlider->setID("selection-scale-slider");
        m_scaleSlider->setPosition({168.f, 27.f});
        m_scaleSlider->setLiveDragging(true);
        m_contextPanel->addChild(m_scaleSlider, 3);
        m_scaleInput = TextInput::create(62.f, "Scale");
        m_scaleInput->setID("selection-scale-input");
        m_scaleInput->setCommonFilter(CommonFilter::Float);
        m_scaleInput->setMaxCharCount(4);
        m_scaleInput->setScale(.65f);
        m_scaleInput->setPosition({269.f, 27.f});
        m_scaleInput->setCallback([this](std::string const& value) { onScaleInput(value); });
        m_contextPanel->addChild(m_scaleInput, 5);
        m_controlItems.push_back(m_scaleInput);

        auto scaleCaption = Label::create("SCALE", "bigFont.fnt");
        scaleCaption->setPosition({168.f, 49.f});
        scaleCaption->setScale(.30f);
        scaleCaption->setColor(ccWHITE);
        scaleCaption->setOpacity(255);
        m_contextPanel->addChild(scaleCaption, 3);

        // Keep the divider in the gap between the scale input and Reset.
        // contextWidth - 95 placed the line inside the Reset tile.
        for (auto x : {59.f, contextWidth - 110.f}) {
            auto separator = CCLayerColor::create({130, 100, 210, 65}, 1.f, 38.f);
            separator->setPosition({x, 8.f});
            m_contextPanel->addChild(separator, 3);
        }

        auto const mode = pauseModeKey();
        auto requestedBadgeWidth = 92.f;
        if (mode == "creator") requestedBadgeWidth = 112.f;
        else if (mode == "platformer") requestedBadgeWidth = 124.f;
        else if (mode == "platformer-creator") requestedBadgeWidth = 158.f;
        auto badgeWidth = std::min(requestedBadgeWidth, size.width - 92.f);
        m_modeBadge = CCScale9Sprite::create("square02_001.png");
        m_modeBadge->setContentSize({badgeWidth, 25.f});
        m_modeBadge->setPosition({size.width - 62.f - badgeWidth / 2.f, size.height - 24.f});
        m_modeBadge->setColor({43, 31, 88});
        m_modeBadge->setOpacity(200);
        m_modeBadge->setCascadeOpacityEnabled(true);
        m_modeBadge->setVisible(false);
        addChild(m_modeBadge, 20);
        auto badgeAccent = CCLayerColor::create(
            {initialAccent.r, initialAccent.g, initialAccent.b, 255}, 3.f, 15.f
        );
        badgeAccent->setPosition({7.f, 5.f});
        m_modeBadge->addChild(badgeAccent, 2);
        m_modeLabel = Label::create("", "bigFont.fnt");
        m_modeLabel->setAnchorPoint({0.f, .5f});
        m_modeLabel->setPosition({15.f, 13.f});
        m_modeLabel->setScale(.21f);
        m_modeLabel->setColor(editorAccentColor());
        m_modeLabel->setLimitLabelWidth(badgeWidth - 24.f, .21f, .14f);
        m_modeBadge->addChild(m_modeLabel, 3);
        m_statusLabel = nullptr;

        m_selectionLabel = Label::create("", "chatFont.fnt");
        m_selectionLabel->setAnchorPoint({0.f, .5f});
        m_selectionLabel->setPosition({12.f, 68.f});
        m_selectionLabel->setScale(.34f);
        m_selectionLabel->setColor(editorAccentColor());
        m_selectionLabel->setOpacity(245);
        m_selectionLabel->setLimitLabelWidth(contextWidth - 170.f, .34f, .2f);
        m_contextPanel->addChild(m_selectionLabel, 2);
        m_selectionMetaLabel = Label::create("", "chatFont.fnt");
        m_selectionMetaLabel->setAnchorPoint({1.f, .5f});
        m_selectionMetaLabel->setPosition({contextWidth - 10.f, 68.f});
        m_selectionMetaLabel->setScale(.34f);
        m_selectionMetaLabel->setColor({225, 215, 255});
        m_selectionMetaLabel->setOpacity(255);
        m_selectionMetaLabel->setLimitLabelWidth(150.f, .34f, .24f);
        m_contextPanel->addChild(m_selectionMetaLabel, 2);

        m_selectionOutline = CCDrawNode::create();
        m_selectionOutline->setID("selection-outline");
        addChild(m_selectionOutline, 19);
        for (size_t index = 0; index < m_focusShade.size(); ++index) {
            auto shade = CCLayerColor::create({0, 0, 0, 52}, 1.f, 1.f);
            shade->setID("selection-focus-shade-" + std::to_string(index));
            shade->setVisible(false);
            m_focusShade[index] = shade;
            addChild(shade, 18);
        }

        m_previewReturnMenu = CCMenu::create();
        m_previewReturnMenu->setID("preview-return-menu");
        m_previewReturnMenu->setPosition({size.width - 70.f, size.height - 25.f});
        auto returnSprite = ButtonSprite::create("BACK TO EDIT", 118, true, "bigFont.fnt", "GJ_button_01.png", 20.f, .34f);
        auto returnButton = CCMenuItemSpriteExtra::create(returnSprite, this, menu_selector(PauseEditor::onPreview));
        returnButton->setID("preview-return-button");
        m_previewReturnMenu->addChild(returnButton);
        m_previewReturnMenu->setVisible(false);
        addChild(m_previewReturnMenu, 30);
    }

    void onToggle(CCObject*) { setEditing(!m_editing); }
    void onUndo(CCObject*) { undo(); }
    void onRedo(CCObject*) { redo(); }
    void onScaleDown(CCObject*) { scaleSelection(.9f); }
    void onScaleUp(CCObject*) { scaleSelection(1.1f); }
    void onScaleReset(CCObject*) { resetSelectionScale(); }
    void applyScaleTarget(float target) {
        if (!m_editing || m_preview || m_selectedNodes.empty()) return;
        target = std::clamp(target, .15f, 3.5f);
        auto before = captureStates(m_selectedNodes);
        for (auto node : m_selectedNodes) {
            rememberInitialPosition(node);
            auto largest = std::max(std::abs(node->getScaleX()), std::abs(node->getScaleY()));
            if (largest <= .0001f) largest = 1.f;
            auto factor = target / largest;
            node->setScaleX(node->getScaleX() * factor);
            node->setScaleY(node->getScaleY() * factor);
            markTransformDirty(node);
        }
        keepSelectedGroupsReachable();
        auto after = captureStates(m_selectedNodes);
        if (statesDiffer(before, after)) {
            m_undo.push_back({std::move(before), std::move(after)});
            m_redo.clear();
            m_hasEdits = true;
        }
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }
    void onScaleSlider(CCObject* sender) {
        auto slider = typeinfo_cast<Slider*>(sender);
        if (!slider) slider = m_scaleSlider;
        if (!slider || m_updatingScaleControls) return;
        auto target = .15f + std::clamp(slider->getValue(), 0.f, 1.f) * 3.35f;
        applyScaleTarget(target);
    }
    void onScaleInput(std::string const& value) {
        if (m_updatingScaleControls || value.empty()) return;
        auto parsed = numFromString<float>(value);
        if (parsed.isErr()) return;
        applyScaleTarget(parsed.unwrap());
    }
    void updateMoveToggle() {
        if (m_moveIcon) m_moveIcon->setColor(m_moveEnabled ? ccColor3B {90, 255, 170} : ccWHITE);
        if (m_moveLabel) {
            m_moveLabel->setString(m_moveEnabled ? "MOVE ON" : "MOVE");
            m_moveLabel->setColor(m_moveEnabled ? ccColor3B {90, 255, 170} : ccWHITE);
            m_moveLabel->setLimitLabelWidth(40.f, .3f, .2f);
        }
        if (auto found = m_controlTiles.find(m_moveButton); found != m_controlTiles.end()) {
            found->second->setColor(m_moveEnabled ? ccColor3B {38, 132, 102} : m_controlColors[m_moveButton]);
            found->second->setOpacity(m_moveEnabled ? 220 : 170);
        }
    }

    void updateChromeState() {
        auto accent = editorAccentColor();
        if (m_bottomAccent) m_bottomAccent->setColor(accent);
        if (m_contextAccent) m_contextAccent->setColor(accent);
        if (m_selectionLabel) m_selectionLabel->setColor(accent);
        if (m_modeLabel) m_modeLabel->setColor(accent);
        if (m_statusLabel) {
            m_statusLabel->setString(m_hasEdits ? "UNSAVED" : "READY");
            m_statusLabel->setColor(m_hasEdits ? ccColor3B {255, 155, 70} : ccColor3B {100, 255, 170});
        }
        if (m_modeBadge) {
            m_modeBadge->setColor(m_hasEdits ? ccColor3B {78, 40, 58} : ccColor3B {43, 31, 88});
        }
        if (m_toggleBackground) {
            m_toggleBackground->setColor(
                m_editing ? (m_hasEdits ? ccColor3B {112, 54, 76} : ccColor3B {36, 105, 92})
                          : ccColor3B {45, 32, 92}
            );
        }
    }
    void onMoveToggle(CCObject*) {
        if (!m_editing || m_preview || m_selectedNodes.empty()) return;
        if (m_dragging || m_pendingDrag) finishDrag();
        m_moveEnabled = !m_moveEnabled;
        rememberMoveModeForSelection();
        m_hasLastSelectionPoint = false;
        updateMoveToggle();
        updateSelectionLabel();
        pulseContextPanel();
    }
    void onHide(CCObject*) { hideSelection(); }
    void onPreview(CCObject*) { setPreview(!m_preview); }
    void pollUpdateBadge(float) { refreshUpdateBadge(); }
    void refreshUpdateBadge() {
        if (m_updateBadge) {
            m_updateBadge->setVisible(
                m_pendingUpdateVersion.empty() && !cachedAvailableUpdateVersion().empty()
            );
        }
    }
    void showUpdateNotice(std::string const& text, NotificationIcon icon, float time = 0.f) {
        if (m_updateNotice) m_updateNotice->cancel();
        m_updateNotice = Notification::create(text, icon, time);
        m_updateNotice->show();
    }

    void showRestartForUpdate(std::string const& version) {
        createQuickPopup(
            "Update installed",
            "Pause Menu Studio <cg>" + version + "</c> is ready. Restart Geometry Dash to load it.",
            "Later", "Restart",
            [](FLAlertLayer*, bool restart) {
                if (restart) utils::game::restart(true);
            }
        );
    }

    void failUpdate(std::string const& message) {
        m_updateBusy = false;
        showUpdateNotice(message, NotificationIcon::Error, NOTIFICATION_LONG_TIME);
    }

    void installDownloadedUpdate(
        web::WebResponse response,
        VersionInfo expectedVersion,
        std::string const& expectedDigest
    ) {
        if (!response.ok()) {
            failUpdate(fmt::format("Update download failed (HTTP {})", response.code()));
            return;
        }
        auto const& data = response.data();
        if (data.empty() || data.size() > 100u * 1024u * 1024u) {
            failUpdate("Downloaded update has an invalid size");
            return;
        }
        if (expectedDigest.starts_with("sha256:")) {
            auto actual = pause_menu_studio::crypto::sha256Hex(
                std::span<std::uint8_t const>(data.data(), data.size())
            );
            if (actual != expectedDigest.substr(7)) {
                failUpdate("Update checksum verification failed");
                return;
            }
        }

        auto target = Mod::get()->getPackagePath();
        auto temporary = target;
        temporary += ".download";
        auto backup = target;
        backup += ".pre-update";
        std::error_code error;
        std::filesystem::remove(temporary, error);
        error.clear();
        auto write = response.into(temporary);
        if (write.isErr()) {
            failUpdate("Could not write the downloaded update");
            return;
        }

        auto metadata = ModMetadata::createFromGeodeFile(temporary);
        auto const correctID = std::string(metadata.getID()) == std::string(Mod::get()->getID());
        auto const correctVersion = metadata.getVersion() == expectedVersion;
        auto targetCheck = metadata.checkTargetVersions();
        if (metadata.hasErrors() || !correctID || !correctVersion || targetCheck.isErr()) {
            std::filesystem::remove(temporary, error);
            failUpdate("Downloaded package is not a valid compatible update");
            return;
        }

        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(target, backup, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            failUpdate("Could not prepare the installed mod for replacement");
            return;
        }
        std::filesystem::rename(temporary, target, error);
        if (error) {
            std::error_code restoreError;
            std::filesystem::rename(backup, target, restoreError);
            std::filesystem::remove(temporary, restoreError);
            failUpdate("Could not install the downloaded package");
            return;
        }
        std::filesystem::remove(backup, error);

        m_updateBusy = false;
        m_pendingUpdateVersion = expectedVersion.toVString();
        rememberAvailableUpdate(std::nullopt);
        refreshUpdateBadge();
        showUpdateNotice("Update installed - restart required", NotificationIcon::Success, 3.f);
        showRestartForUpdate(m_pendingUpdateVersion);
    }

    void startUpdateDownload(
        std::string const& url,
        VersionInfo version,
        std::string const& digest
    ) {
        m_updateBusy = true;
        showUpdateNotice("Downloading update: 0%", NotificationIcon::Loading);
        auto self = WeakRef<PauseEditor>(this);
        auto request = web::WebRequest();
        request.userAgent(UPDATE_USER_AGENT);
        request.header("Accept", "application/octet-stream");
        request.timeout(std::chrono::seconds(120));
        request.onProgress([self](web::WebProgress const& progress) {
            auto editor = self.lock();
            if (!editor || !editor->m_updateNotice) return;
            auto percent = progress.downloadProgress();
            if (percent) editor->m_updateNotice->setString(fmt::format("Downloading update: {:.0f}%", *percent));
        });
        m_updateDownloadRequest.spawn(
            request.get(url),
            [self, version, digest](web::WebResponse response) {
                if (auto editor = self.lock()) {
                    editor->installDownloadedUpdate(std::move(response), version, digest);
                }
            }
        );
    }

    void handleUpdateCheck(web::WebResponse response) {
        m_updateBusy = false;
        if (!response.ok()) {
            failUpdate(fmt::format("Update check failed (HTTP {})", response.code()));
            return;
        }
        auto parsed = response.json();
        if (parsed.isErr()) {
            failUpdate("Update server returned invalid data");
            return;
        }
        auto releases = parsed.unwrap().asArray();
        if (releases.isErr()) {
            failUpdate("Update server returned an invalid release list");
            return;
        }

        auto best = latestUpdateCandidate(releases.unwrap());

        if (!best) {
            rememberAvailableUpdate(std::nullopt);
            refreshUpdateBadge();
            showUpdateNotice("Pause Menu Studio is up to date", NotificationIcon::Success, 2.5f);
            return;
        }
        rememberAvailableUpdate(best);
        refreshUpdateBadge();
        if (m_updateNotice) m_updateNotice->cancel();
        auto self = WeakRef<PauseEditor>(this);
        auto versionText = best->version.toVString();
        createQuickPopup(
            "Update available",
            "Pause Menu Studio <cg>" + versionText + "</c> is available. Download and install it for the next restart?",
            "Later", "Download",
            [self, candidate = *best](FLAlertLayer*, bool download) {
                if (!download) return;
                if (auto editor = self.lock()) {
                    editor->startUpdateDownload(candidate.url, candidate.version, candidate.digest);
                }
            }
        );
    }

    void onUpdates(CCObject*) {
        if (!m_pendingUpdateVersion.empty()) {
            showRestartForUpdate(m_pendingUpdateVersion);
            return;
        }
        if (m_updateBusy) {
            showUpdateNotice("Update request is already running", NotificationIcon::Info, 2.f);
            return;
        }
        m_updateBusy = true;
        showUpdateNotice("Checking for updates...", NotificationIcon::Loading);
        auto self = WeakRef<PauseEditor>(this);
        auto request = web::WebRequest();
        request.userAgent(UPDATE_USER_AGENT);
        request.header("Accept", "application/vnd.github+json");
        request.header("X-GitHub-Api-Version", "2022-11-28");
        request.timeout(std::chrono::seconds(20));
        m_updateCheckRequest.spawn(
            request.get(UPDATE_RELEASES_URL),
            [self](web::WebResponse response) {
                if (auto editor = self.lock()) editor->handleUpdateCheck(std::move(response));
            }
        );
    }

    void onTrash(CCObject*) {
        auto recovered = reconcileHiddenBlocks();
        refreshHiddenBlockIcons();
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
            pause_menu_studio::hidden_blocks::entries(pauseModeKey()),
            [self](std::string id) -> bool {
                if (auto editor = self.lock()) return editor->restoreHiddenBlock(id);
                return false;
            }
        )) popup->show();
    }
    void onSaveProfile(CCObject*) {
        auto self = WeakRef<PauseEditor>(this);
        auto title = saveLayoutTitle();
        if (auto popup = pause_menu_studio::LayoutNamePopup::create(
            [self](std::string name) {
                if (auto editor = self.lock()) editor->saveNamedLayout(name);
            },
            title
        )) popup->show();
    }
    void onLayouts(CCObject*) {
        auto self = WeakRef<PauseEditor>(this);
        auto mode = pauseModeKey();
        auto activeKey = activeLayoutStorageKey();
        auto active = Mod::get()->getSavedValue<std::string>(activeKey, "");
        if (auto popup = pause_menu_studio::LayoutListPopup::create(
            pause_menu_studio::profiles::names(mode),
            active,
            layoutListTitle(),
            [self](std::string name) {
                if (auto editor = self.lock()) editor->applyNamedLayout(name);
            },
            [activeKey](std::string oldName, std::string newName) {
                if (!pause_menu_studio::profiles::rename(oldName, newName)) return false;
                if (Mod::get()->getSavedValue<std::string>(activeKey, "") == oldName) {
                    Mod::get()->setSavedValue<std::string>(activeKey, newName);
                }
                Notification::create("Layout renamed", NotificationIcon::Success)->show();
                return true;
            },
            [](std::string sourceName, std::string newName) {
                if (!pause_menu_studio::profiles::duplicate(sourceName, newName)) return false;
                Notification::create("Layout duplicated", NotificationIcon::Success)->show();
                return true;
            },
            [self, activeKey](std::string name) {
                pause_menu_studio::profiles::erase(name);
                if (Mod::get()->getSavedValue<std::string>(activeKey, "") == name) {
                    Mod::get()->setSavedValue<std::string>(activeKey, "");
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

    void setPreview(bool preview) {
        if (preview && !m_editing) return;
        m_preview = preview;
        if (m_previewReturnMenu) m_previewReturnMenu->setVisible(preview);
        if (m_toggleMenu) m_toggleMenu->setVisible(!preview);
        if (m_bottomPanel) m_bottomPanel->setVisible(m_editing && !preview);
        if (m_modeBadge) m_modeBadge->setVisible(m_editing && !preview);
        if (preview) {
            if (m_contextPanel) m_contextPanel->setVisible(false);
            for (auto shade : m_focusShade) if (shade) shade->setVisible(false);
            m_contextWasVisible = false;
            if (m_selectionOutline) m_selectionOutline->clear();
        } else {
            updateSelectionLabel();
            updateSelectionOutline();
        }
    }

    void setEditing(bool editing) {
        if (!editing && m_preview) setPreview(false);
        auto leavingEditor = m_editing && !editing;
        if (leavingEditor) {
            commitDirtyTransforms();
            m_hasEdits = false;
            m_animatingExit = true;
        }
        if (editing) m_animatingExit = false;
        m_editing = editing;
        m_dragging = false;
        m_pendingDrag = false;
        if (!editing) clearSelection();
        if (!editing) m_hasLastSelectionPoint = false;
        if (editing) {
            m_modeLabel->setString(editorModeLabel().c_str());
        }
        else if (!leavingEditor) m_modeLabel->setString("");
        if (m_modeBadge) {
            m_modeBadge->stopAllActions();
            if (editing) {
                m_modeBadge->setVisible(true);
                m_modeBadge->setOpacity(0);
                m_modeBadge->setScale(.9f);
                m_modeBadge->runAction(CCSpawn::create(
                    CCFadeTo::create(.18f, 200),
                    CCEaseBackOut::create(CCScaleTo::create(.2f, 1.f)),
                    nullptr
                ));
            } else if (leavingEditor) {
                m_modeBadge->setVisible(true);
                m_modeBadge->runAction(CCSpawn::create(
                    CCFadeOut::create(.16f),
                    CCEaseSineIn::create(CCScaleTo::create(.16f, .92f)),
                    nullptr
                ));
            } else {
                m_modeBadge->setVisible(false);
            }
        }
        if (m_toggleEditIcon) m_toggleEditIcon->setVisible(!editing);
        if (m_toggleDoneIcon) m_toggleDoneIcon->setVisible(editing);
        updateChromeState();
        for (auto line : m_gridLines) line->setVisible(editing);
        if (m_bottomPanel) {
            m_bottomPanel->stopAllActions();
            if (editing) {
                m_bottomPanel->setVisible(true);
                m_bottomPanel->setOpacity(190);
                m_bottomPanel->setPositionY(-m_bottomPanel->getContentHeight());
                m_bottomPanel->runAction(CCEaseBackOut::create(CCMoveTo::create(
                    .26f, {m_bottomPanel->getPositionX(), 3.f}
                )));
                if (m_historyMenu) {
                    size_t index = 0;
                    for (auto child : CCArrayExt<CCNode*>(m_historyMenu->getChildren())) {
                        child->stopAllActions();
                        child->setScale(.62f);
                        child->runAction(CCSequence::create(
                            CCDelayTime::create(.025f * static_cast<float>(index++)),
                            CCEaseBackOut::create(CCScaleTo::create(.2f, 1.f)),
                            nullptr
                        ));
                    }
                }
            } else if (leavingEditor) {
                m_bottomPanel->setVisible(true);
                m_bottomPanel->runAction(CCSequence::create(
                    CCEaseIn::create(CCMoveTo::create(
                        .2f, {m_bottomPanel->getPositionX(), -m_bottomPanel->getContentHeight()}
                    ), 2.f),
                    CCHide::create(),
                    CCCallFunc::create(this, callfunc_selector(PauseEditor::finishEditorExitAnimation)),
                    nullptr
                ));
            } else {
                m_bottomPanel->setVisible(false);
            }
        }
        if (m_contextPanel) {
            m_contextPanel->stopAllActions();
            if (editing) {
                m_contextPanel->setOpacity(190);
                m_contextPanel->setScale(1.f);
            } else if (leavingEditor && m_contextPanel->isVisible()) {
                m_contextPanel->runAction(CCSpawn::create(
                    CCFadeOut::create(.16f),
                    CCEaseSineIn::create(CCScaleTo::create(.16f, .92f)),
                    nullptr
                ));
            } else if (!leavingEditor) {
                m_contextPanel->setVisible(false);
            }
        }
        if (!leavingEditor) m_contextWasVisible = false;
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
    }

    void finishEditorExitAnimation() {
        m_animatingExit = false;
        if (m_bottomPanel) m_bottomPanel->setVisible(false);
        if (m_contextPanel) {
            m_contextPanel->setVisible(false);
            m_contextPanel->setOpacity(190);
            m_contextPanel->setScale(1.f);
        }
        if (m_modeLabel) {
            m_modeLabel->setString("");
            m_modeLabel->setOpacity(255);
        }
        if (m_modeBadge) {
            m_modeBadge->setVisible(false);
            m_modeBadge->setOpacity(200);
            m_modeBadge->setScale(1.f);
        }
        m_contextWasVisible = false;
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
        restoreMoveModeForSelection();
        m_redo.push_back(std::move(action));
        m_hasEdits = true;
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
        pulseContextPanel();
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
        restoreMoveModeForSelection();
        m_undo.push_back(std::move(action));
        m_hasEdits = true;
        updateHistoryButtons();
        updateSelectionLabel();
        updateSelectionOutline();
        pulseContextPanel();
    }

    void resetLayout() {
        if (!m_editing) return;
        m_hasEdits = true;
        for (auto const& entry : pause_menu_studio::hidden_blocks::entries(pauseModeKey())) {
            for (auto const& member : entry.members) {
                if (auto node = findNodeByStoredPath(m_owner, member.path)) node->setVisible(true);
            }
        }
        pause_menu_studio::hidden_blocks::clear(pauseModeKey());
        auto generationKey = layoutGenerationStorageKey();
        auto generation = Mod::get()->getSavedValue<int64_t>(generationKey, 0);
        Mod::get()->setSavedValue<int64_t>(generationKey, generation + 1);
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
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), "");
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
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), "");
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
            entry.id = pause_menu_studio::hidden_blocks::uniqueID(preferredID, pauseModeKey());
            entry.iconFrame = pause_menu_studio::block_icons::frameForNodes(group);
            if (entry.iconFrame.empty()) entry.iconFrame = "GJ_infoBtn_001.png";
            // Never make a node disappear unless its complete restore record
            // can immediately be read back from the mod's saved storage.
            if (!pause_menu_studio::hidden_blocks::upsert(entry, pauseModeKey())) {
                Notification::create("Could not move block to trash", NotificationIcon::Error)->show();
                continue;
            }
            for (auto node : group) if (node) node->setVisible(false);
            ++hiddenCount;
        }

        clearSelection();
        m_dragging = false;
        m_pendingDrag = false;
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), "");
        updateSelectionLabel();
        updateSelectionOutline();
        if (hiddenCount > 0) {
            m_hasEdits = true;
            updateChromeState();
            if (m_trashButton) {
                m_trashButton->stopAllActions();
                m_trashButton->setScale(1.f);
                m_trashButton->runAction(CCSequence::create(
                    CCEaseBackOut::create(CCScaleTo::create(.12f, 1.28f)),
                    CCScaleTo::create(.12f, 1.f),
                    nullptr
                ));
            }
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

    void refreshHiddenBlockIcons() {
        for (auto entry : pause_menu_studio::hidden_blocks::entries(pauseModeKey())) {
            std::vector<CCNode*> nodes;
            nodes.reserve(entry.members.size());
            for (auto const& member : entry.members) {
                if (auto node = findNodeByStoredPath(m_owner, member.path)) nodes.push_back(node);
            }
            auto captured = pause_menu_studio::block_icons::frameForNodes(nodes);
            if (captured.empty()) captured = "GJ_infoBtn_001.png";
            if (entry.iconFrame == captured) continue;
            entry.iconFrame = std::move(captured);
            pause_menu_studio::hidden_blocks::upsert(entry, pauseModeKey());
        }
    }

    bool restoreHiddenBlock(std::string const& id) {
        auto entry = pause_menu_studio::hidden_blocks::find(id, pauseModeKey());
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
        pause_menu_studio::hidden_blocks::erase(id, pauseModeKey());
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), "");
        m_hasEdits = true;
        updateChromeState();
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
            if (pause_menu_studio::profiles::mode(name) != pauseModeKey()) {
                Notification::create(
                    "That name belongs to the other editor mode", NotificationIcon::Warning
                )->show();
                return;
            }
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
        refreshHiddenBlockIcons();
        pause_menu_studio::profiles::Snapshot snapshot;
        std::unordered_map<std::string, HiddenSnapshotInfo> hiddenMembership;
        for (auto const& entry : pause_menu_studio::hidden_blocks::entries(pauseModeKey())) {
            for (auto const& member : entry.members) {
                hiddenMembership[member.path] = {entry.id, entry.label, entry.iconFrame};
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
                    transform.hiddenID = hidden->second.id;
                    transform.hiddenLabel = hidden->second.label;
                    if (!hidden->second.iconFrame.empty()) transform.hiddenIcon = hidden->second.iconFrame;
                } else {
                    transform.hidden = false;
                }
                snapshot[path] = std::move(transform);
            }
        }
        // Keep every Trash member in the profile even when a dynamic mod node
        // is temporarily absent from this particular pause-menu instance.
        for (auto const& entry : pause_menu_studio::hidden_blocks::entries(pauseModeKey())) {
            for (auto const& member : entry.members) {
                auto found = snapshot.find(member.path);
                if (found == snapshot.end()) {
                    pause_menu_studio::profiles::Transform transform {
                        member.position, member.scaleX, member.scaleY
                    };
                    transform.hidden = true;
                    transform.hiddenID = entry.id;
                    transform.hiddenLabel = entry.label;
                    if (!entry.iconFrame.empty()) transform.hiddenIcon = entry.iconFrame;
                    snapshot[member.path] = std::move(transform);
                } else {
                    found->second.hidden = true;
                    found->second.hiddenID = entry.id;
                    found->second.hiddenLabel = entry.label;
                    if (!entry.iconFrame.empty()) found->second.hiddenIcon = entry.iconFrame;
                }
            }
        }
        pause_menu_studio::profiles::save(name, snapshot, pauseModeKey());
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), name);
        Notification::create("Layout saved: " + name, NotificationIcon::Success)->show();
    }

    void applyNamedLayout(std::string const& name) {
        if (pause_menu_studio::profiles::mode(name) != pauseModeKey()) {
            Notification::create(
                "This layout belongs to the other editor mode", NotificationIcon::Warning
            )->show();
            return;
        }
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
            for (auto const& entry : pause_menu_studio::hidden_blocks::entries(pauseModeKey())) {
                for (auto const& member : entry.members) {
                    if (auto node = findNodeByStoredPath(m_owner, member.path)) node->setVisible(true);
                }
            }
            pause_menu_studio::hidden_blocks::clear(pauseModeKey());
        }
        auto generationKey = layoutGenerationStorageKey();
        auto generation = Mod::get()->getSavedValue<int64_t>(generationKey, 0);
        Mod::get()->setSavedValue<int64_t>(generationKey, generation + 1);
        applySnapshot(m_owner, *snapshot);
        if (hasHiddenState) {
            mergeHiddenSnapshot(*snapshot, hiddenEntries);
            for (auto& [id, entry] : hiddenEntries) {
                if (entry.id.empty()) {
                    entry.id = pause_menu_studio::hidden_blocks::uniqueID("hidden-block", pauseModeKey());
                }
                pause_menu_studio::hidden_blocks::upsert(entry, pauseModeKey());
            }
        }
        // Old profiles have no visibility metadata, so their Apply operation
        // preserves the current Trash instead of revealing hidden controls.
        applyHiddenVisibility(m_owner);
        m_hasEdits = true;
        Mod::get()->setSavedValue<std::string>(activeLayoutStorageKey(), name);
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
        auto update = [this](CCMenuItemSpriteExtra* button, bool enabled) {
            if (!button) return;
            button->setEnabled(enabled);
            if (auto image = typeinfo_cast<CCRGBAProtocol*>(button->getNormalImage())) {
                image->setOpacity(enabled ? 255 : 90);
            }
            if (auto found = m_controlTiles.find(button); found != m_controlTiles.end()) {
                found->second->setColor(enabled ? m_controlColors[button] : ccColor3B {34, 31, 55});
                found->second->setOpacity(enabled ? 170 : 75);
            }
        };
        update(m_undoButton, !m_undo.empty());
        update(m_redoButton, !m_redo.empty());
        updateChromeState();
    }

    void updateSelectionLabel() {
        if (!m_editing || m_preview || m_selectedNodes.empty()) {
            m_selectionLabel->setString("");
            if (m_selectionMetaLabel) m_selectionMetaLabel->setString("");
            return;
        }
        if (m_scaleSlider && m_selected) {
            auto scale = std::max(std::abs(m_selected->getScaleX()), std::abs(m_selected->getScaleY()));
            m_updatingScaleControls = true;
            m_scaleSlider->setValue(std::clamp((scale - .15f) / 3.35f, 0.f, 1.f));
            if (
                m_scaleInput &&
                (!m_scaleInput->getInputNode() || !m_scaleInput->getInputNode()->m_selected)
            ) {
                m_scaleInput->setString(fmt::format("{:.2f}", scale), false);
            }
            m_updatingScaleControls = false;
            if (m_selectionMetaLabel) {
                m_selectionMetaLabel->setString(fmt::format(
                    "{:.2f}x  /  {}", scale, m_moveEnabled ? "MOVE ON" : "LOCKED"
                ).c_str());
                m_selectionMetaLabel->setColor(
                    m_moveEnabled ? ccColor3B {100, 255, 170} : ccColor3B {200, 185, 240}
                );
            }
        }
        auto const logicalCount = logicalSelectionCount();
        if (logicalCount > 1) {
            m_selectionLabel->setString(fmt::format("{} BLOCKS SELECTED", logicalCount).c_str());
            return;
        }
        if (!selectedVolumeGroup("music").empty()) {
            m_selectionLabel->setString("MUSIC CONTROLS");
            return;
        }
        if (!selectedVolumeGroup("sfx").empty()) {
            m_selectionLabel->setString("SFX CONTROLS");
            return;
        }
        auto groups = selectedLogicalGroups();
        auto name = groups.empty() ? std::string("UNNAMED BLOCK") : groupLabel(groups.front());
        m_selectionLabel->setString(utils::string::toUpper(name).c_str());
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
            typeinfo_cast<Label*>(node) ||
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
            m_selectionOutline->drawDot(corners[index], 2.4f, color);
        }
    }

    void pulseContextPanel() {
        if (!m_contextPanel || !m_contextPanel->isVisible()) return;
        m_contextPanel->stopAllActions();
        m_contextPanel->setScale(1.f);
        m_contextPanel->runAction(CCSequence::create(
            CCEaseSineOut::create(CCScaleTo::create(.08f, 1.035f)),
            CCEaseSineIn::create(CCScaleTo::create(.12f, 1.f)),
            nullptr
        ));
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
        if (!m_editing || m_preview || m_selectedNodes.empty()) {
            if (m_contextPanel && !m_animatingExit) m_contextPanel->setVisible(false);
            for (auto shade : m_focusShade) if (shade) shade->setVisible(false);
            m_contextWasVisible = false;
            return;
        }
        auto const multiple = logicalSelectionCount() > 1;
        auto const accent = editorAccentColor();
        ccColor4F const color = multiple
            ? ccColor4F {1.f, .48f, .08f, 1.f}
            : ccColor4F {
                accent.r / 255.f, accent.g / 255.f, accent.b / 255.f, 1.f
            };

        OutlineBounds contextBounds;
        std::unordered_set<CCNode*> groupedNodes;
        for (auto type : {std::string_view("music"), std::string_view("sfx")}) {
            auto const members = selectedVolumeGroup(type);
            if (members.empty()) continue;
            OutlineBounds bounds;
            for (auto node : members) {
                groupedNodes.insert(node);
                includeVisibleBounds(node, bounds);
                includeVisibleBounds(node, contextBounds);
            }
            drawOutlineBounds(bounds, color);
        }

        for (auto node : m_selectedNodes) {
            if (groupedNodes.contains(node)) continue;
            OutlineBounds bounds;
            includeVisibleBounds(node, bounds);
            includeVisibleBounds(node, contextBounds);
            drawOutlineBounds(bounds, color);
        }
        if (m_contextPanel && contextBounds.valid) {
            auto screen = getContentSize();
            auto minX = std::clamp(contextBounds.minX, 0.f, screen.width);
            auto maxX = std::clamp(contextBounds.maxX, 0.f, screen.width);
            auto minY = std::clamp(contextBounds.minY, 0.f, screen.height);
            auto maxY = std::clamp(contextBounds.maxY, 0.f, screen.height);
            auto setShade = [](CCLayerColor* shade, CCPoint position, CCSize size) {
                if (!shade) return;
                shade->setPosition(position);
                shade->setContentSize({std::max(0.f, size.width), std::max(0.f, size.height)});
                shade->setVisible(size.width > .5f && size.height > .5f);
            };
            setShade(m_focusShade[0], {0.f, 0.f}, {minX, screen.height});
            setShade(m_focusShade[1], {maxX, 0.f}, {screen.width - maxX, screen.height});
            setShade(m_focusShade[2], {minX, 0.f}, {maxX - minX, minY});
            setShade(m_focusShade[3], {minX, maxY}, {maxX - minX, screen.height - maxY});
            auto panelSize = m_contextPanel->getContentSize();
            auto x = (contextBounds.minX + contextBounds.maxX - panelSize.width) / 2.f;
            x = std::clamp(x, 4.f, std::max(4.f, screen.width - panelSize.width - 4.f));
            auto y = contextBounds.maxY + 8.f;
            if (y + panelSize.height > screen.height - 4.f) {
                y = contextBounds.minY - panelSize.height - 8.f;
            }
            y = std::clamp(y, 78.f, std::max(78.f, screen.height - panelSize.height - 4.f));
            m_contextPanel->setPosition({x, y});
            m_contextPanel->setVisible(true);
            if (!m_contextWasVisible) {
                m_contextPanel->setOpacity(0);
                m_contextPanel->setScale(.9f);
                m_contextPanel->runAction(CCSpawn::create(
                    CCFadeTo::create(.16f, 190),
                    CCEaseBackOut::create(CCScaleTo::create(.19f, 1.f)),
                    nullptr
                ));
            }
            m_contextWasVisible = true;
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

class $modify(PauseMenuStudioLoadingLayer, LoadingLayer) {
    struct Fields {
        CCNode* updateBanner = nullptr;
        CCLabelBMFont* updateLabel = nullptr;
    };

    void buildUpdateBanner() {
        auto const screen = CCDirector::sharedDirector()->getWinSize();
        auto const width = std::min(360.f, screen.width - 20.f);
        auto banner = CCNode::create();
        banner->setID("pause-menu-studio-update-banner");
        banner->setContentSize({width, 30.f});
        banner->setAnchorPoint({.5f, .5f});
        banner->setPosition({screen.width / 2.f, 44.f});
        banner->setVisible(false);

        auto border = CCLayerColor::create({70, 235, 255, 255}, width, 30.f);
        banner->addChild(border);
        auto background = CCLayerColor::create({25, 15, 55, 235}, width - 4.f, 26.f);
        background->setPosition({2.f, 2.f});
        banner->addChild(background, 1);

        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setPosition({width / 2.f, 15.f});
        label->setColor({255, 235, 75});
        label->setOpacity(255);
        banner->addChild(label, 2);

        m_fields->updateBanner = banner;
        m_fields->updateLabel = label;
        addChild(banner, 1000);
    }

    void showAvailableUpdate(std::string const& version) {
        showLoadingUpdateBanner(m_fields->updateBanner, m_fields->updateLabel, version);
    }

    bool init(bool refresh) {
        if (!LoadingLayer::init(refresh)) return false;
        buildUpdateBanner();
        showAvailableUpdate(cachedAvailableUpdateVersion());

        if (!g_startupUpdateCheckStarted) {
            g_startupUpdateCheckStarted = true;
            auto request = web::WebRequest();
            request.userAgent(UPDATE_USER_AGENT);
            request.header("Accept", "application/vnd.github+json");
            request.header("X-GitHub-Api-Version", "2022-11-28");
            request.timeout(std::chrono::seconds(20));
            // Keep the request alive after the loading scene closes so the
            // Pause Edit Mode badge is still updated during this launch. The
            // WeakRefs retain only the child banner and label, never their
            // LoadingLayer parent, so they cannot keep the loading scene open.
            auto banner = WeakRef<CCNode>(m_fields->updateBanner);
            auto label = WeakRef<CCLabelBMFont>(m_fields->updateLabel);
            startupUpdateRequest().spawn(
                request.get(UPDATE_RELEASES_URL),
                [banner, label](web::WebResponse response) {
                    if (!response.ok()) return;
                    auto parsed = response.json();
                    if (parsed.isErr()) return;
                    auto releases = parsed.unwrap().asArray();
                    if (releases.isErr()) return;
                    auto candidate = latestUpdateCandidate(releases.unwrap());
                    rememberAvailableUpdate(candidate);
                    auto bannerNode = banner.lock();
                    auto labelNode = label.lock();
                    if (candidate && bannerNode && labelNode && bannerNode->getParent()) {
                        showLoadingUpdateBanner(
                            bannerNode.data(), labelNode.data(), candidate->version.toVString()
                        );
                    } else if (!candidate && bannerNode) {
                        bannerNode->setVisible(false);
                    }
                }
            );
        }
        return true;
    }
};

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
        // Platformer adds a different top-level play-time node and a different
        // center-button set. Ask the registered NodeIDs provider to finish its
        // IDs before the editor captures paths or builds logical selections.
        NodeIDs::provideFor(this);
        if (!Mod::get()->getSettingValue<bool>("enabled")) return;

        // Jukebox intentionally overrides LevelTools::getAudioTitle() with the
        // active NONG title. Geometry Dash also uses that function for the
        // stock PauseLayer heading of official levels, so after Jukebox has
        // initialized a second pause can incorrectly show the remix title.
        // The real level name was captured before PlayLayer::init; restore only
        // the already-created pause label and leave Jukebox's song metadata
        // and selected NONG untouched.
        if (auto play = PlayLayer::get()) {
            if (auto level = play->m_level) {
                auto const name = stableLevelName(play, level);
                if (!name.empty()) {
                    if (auto title = typeinfo_cast<CCLabelBMFont*>(getChildByIDRecursive("level-name"))) {
                        title->setString(name.c_str());
                    }
                }
            }
        }

        auto const savedSchema = Mod::get()->getSavedValue<int>("layout-schema", 0);
        if (savedSchema < LAYOUT_SCHEMA) {
            auto generationKey = layoutGenerationStorageKey();
            auto generation = Mod::get()->getSavedValue<int64_t>(generationKey, 0);
            Mod::get()->setSavedValue<int64_t>(generationKey, generation + 1);
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
