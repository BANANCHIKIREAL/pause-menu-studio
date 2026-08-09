#include "BlockIcons.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio::block_icons {
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
