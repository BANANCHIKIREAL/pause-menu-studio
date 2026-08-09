#include "BlockIcons.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace geode::prelude;

namespace pause_menu_studio::block_icons {
namespace {
bool sameValue(float lhs, float rhs) {
    return std::abs(lhs - rhs) <= .01f;
}

bool sameFrame(CCSpriteFrame* lhs, CCSpriteFrame* rhs) {
    if (!lhs || !rhs || lhs->getTexture() != rhs->getTexture() || lhs->isRotated() != rhs->isRotated()) {
        return false;
    }
    auto const& a = lhs->getRect();
    auto const& b = rhs->getRect();
    return
        sameValue(a.origin.x, b.origin.x) && sameValue(a.origin.y, b.origin.y) &&
        sameValue(a.size.width, b.size.width) && sameValue(a.size.height, b.size.height);
}

std::string cachedName(CCSpriteFrame* frame) {
    if (!frame) return {};
    auto direct = std::string(frame->getFrameName());
    if (!direct.empty()) return direct;

    auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
    if (!cache || !cache->m_pSpriteFrames) return {};
    for (auto [name, candidate] : CCDictionaryExt<std::string_view, CCSpriteFrame*, false>(cache->m_pSpriteFrames)) {
        if (sameFrame(frame, candidate)) return std::string(name);
    }
    return {};
}

float frameScore(std::string const& name, CCSprite* sprite) {
    auto lower = utils::string::toLower(name);
    auto has = [&lower](std::string_view term) { return lower.find(term) != std::string::npos; };
    if (
        has("gj_square") || has("button_0") || has("slidergroove") ||
        has("sliderbar") || has("background") || has("ground") || has("gradient")
    ) return -100000.f;

    float score = 0.f;
    if (has("icon")) score += 15000.f;
    if (has("btn")) score += 9000.f;
    if (
        has("coin") || has("star") || has("demon") || has("skull") || has("music") ||
        has("sfx") || has("play") || has("practice") || has("replay") || has("trash") ||
        has("edit") || has("info") || has("setting") || has("option") || has("download")
    ) score += 12000.f;
    if (sprite) {
        score += std::min(5000.f, sprite->getContentWidth() * sprite->getContentHeight());
    }
    return score;
}

void inspectNode(CCNode* node, std::string& bestName, float& bestScore) {
    if (!node) return;
    if (auto sprite = typeinfo_cast<CCSprite*>(node)) {
        auto name = cachedName(sprite->displayFrame());
        if (!name.empty()) {
            auto score = frameScore(name, sprite);
            if (score > bestScore) {
                bestScore = score;
                bestName = std::move(name);
            }
        }
    }
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        inspectNode(child, bestName, bestScore);
    }
}
}

std::string frameForText(std::string_view value) {
    auto text = utils::string::toLower(std::string(value));
    auto has = [&text](std::string_view term) { return text.find(term) != std::string::npos; };

    if (has("practice") || has("checkpoint")) return "GJ_practiceBtn_001.png";
    if (has("retry") || has("restart") || has("replay")) return "GJ_replayBtn_001.png";
    if (has("resume") || has("play-button") || has("playbtn")) return "GJ_playBtn2_001.png";
    if (has("option") || has("setting") || has("config")) return "GJ_optionsBtn_001.png";
    if (has("jukebox") || has("music") || has("song")) return "GJ_musicOnBtn_001.png";
    if (has("sfx") || has("sound") || has("volume") || has("mute")) return "GJ_sfxOnBtn_001.png";
    if (has("comment") || has("chat") || has("emote")) return "GJ_chatBtn_001.png";
    if (has("coin")) return "GJ_coinsIcon_001.png";
    if (has("demon") || has("skull") || has("death") || has("difficulty")) return "GJ_demonIcon_001.png";
    if (has("star") || has("rating") || has("rate")) return "GJ_starsIcon_001.png";
    if (has("thumbnail") || has("level-info") || has("level-card") || has("level_")) {
        return "GJ_viewLevelsBtn_001.png";
    }
    if (has("info")) return "GJ_infoBtn_001.png";
    if (has("download") || has("upload")) return "GJ_downloadBtn_001.png";
    if (has("edit")) return "GJ_editBtn_001.png";
    if (has("trash") || has("delete")) return "GJ_trashBtn_001.png";
    return "GJ_viewListsBtn_001.png";
}

std::string frameForNode(CCNode* node) {
    std::string bestName;
    auto bestScore = -std::numeric_limits<float>::infinity();
    inspectNode(node, bestName, bestScore);
    return bestScore > -99999.f ? bestName : std::string();
}

std::string frameForNodes(std::vector<CCNode*> const& nodes) {
    std::string bestName;
    auto bestScore = -std::numeric_limits<float>::infinity();
    for (auto node : nodes) inspectNode(node, bestName, bestScore);
    return bestScore > -99999.f ? bestName : std::string();
}

CCSprite* create(std::string_view text, float maxSize) {
    auto frame = frameForText(text);
    auto icon = CCSprite::createWithSpriteFrameName(frame.c_str());
    if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_viewListsBtn_001.png");
    if (!icon) return nullptr;
    auto largest = std::max(icon->getContentWidth(), icon->getContentHeight());
    if (largest > .001f) icon->setScale(maxSize / largest);
    return icon;
}
}
