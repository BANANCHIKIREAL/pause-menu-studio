#include "HiddenBlocksPopup.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio {
namespace {
std::string entryID(CCObject* sender) {
    auto node = typeinfo_cast<CCNode*>(sender);
    auto value = node ? typeinfo_cast<CCString*>(node->getUserObject()) : nullptr;
    return value ? std::string(value->getCString()) : std::string();
}

CCSprite* iconForEntry(hidden_blocks::Entry const& entry) {
    auto text = utils::string::toLower(entry.label + " " + entry.id);
    char const* frame = "GJ_viewListsBtn_001.png";
    if (text.find("resume") != std::string::npos || text.find("play") != std::string::npos) {
        frame = "GJ_playBtn2_001.png";
    } else if (text.find("practice") != std::string::npos) {
        frame = "GJ_practiceBtn_001.png";
    } else if (text.find("retry") != std::string::npos || text.find("restart") != std::string::npos) {
        frame = "GJ_replayBtn_001.png";
    } else if (text.find("option") != std::string::npos || text.find("setting") != std::string::npos) {
        frame = "GJ_optionsBtn_001.png";
    } else if (text.find("music") != std::string::npos) {
        frame = "GJ_musicOnBtn_001.png";
    } else if (text.find("sfx") != std::string::npos) {
        frame = "GJ_sfxOnBtn_001.png";
    } else if (text.find("edit") != std::string::npos) {
        frame = "GJ_editBtn_001.png";
    }
    auto icon = CCSprite::createWithSpriteFrameName(frame);
    if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_viewListsBtn_001.png");
    return icon;
}
}

HiddenBlocksPopup* HiddenBlocksPopup::create(
    std::vector<hidden_blocks::Entry> entries,
    Function<bool(std::string)> restoreCallback
) {
    auto ret = new HiddenBlocksPopup();
    if (ret && ret->init(std::move(entries), std::move(restoreCallback))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool HiddenBlocksPopup::init(
    std::vector<hidden_blocks::Entry> entries,
    Function<bool(std::string)> restoreCallback
) {
    if (!Popup::init(440.f, 310.f)) return false;
    m_entries = std::move(entries);
    m_restoreCallback = std::move(restoreCallback);
    setTitle("Hidden blocks");

    m_scroll = ScrollLayer::create({390.f, 235.f});
    m_scroll->setPosition({25.f, 25.f});
    m_scroll->setID("hidden-blocks-list");
    m_mainLayer->addChild(m_scroll);
    rebuildList();
    return true;
}

void HiddenBlocksPopup::rebuildList() {
    auto content = m_scroll->m_contentLayer;
    content->removeAllChildren();
    constexpr float cardWidth = 185.f;
    constexpr float cardHeight = 98.f;
    constexpr size_t columns = 2;
    auto rows = (m_entries.size() + columns - 1) / columns;
    auto height = std::max(235.f, static_cast<float>(rows) * cardHeight);
    content->setContentSize({390.f, height});
    content->setPositionY(235.f - height);

    if (m_entries.empty()) {
        auto empty = CCLabelBMFont::create("Trash is empty", "bigFont.fnt");
        empty->setScale(.45f);
        empty->setOpacity(150);
        empty->setPosition({195.f, height - 110.f});
        content->addChild(empty);
        return;
    }

    for (size_t index = 0; index < m_entries.size(); ++index) {
        auto const& entry = m_entries[index];
        auto column = index % columns;
        auto rowIndex = index / columns;
        auto x = 4.f + static_cast<float>(column) * 193.f;
        auto y = height - 49.f - static_cast<float>(rowIndex) * cardHeight;
        auto row = CCMenu::create();
        row->setPosition(CCPointZero);
        row->setContentSize({cardWidth, 92.f});

        auto card = CCScale9Sprite::create("square02_001.png");
        card->setContentSize({cardWidth, 90.f});
        card->setPosition({x + cardWidth / 2.f, y});
        card->setColor({39, 32, 64});
        card->setOpacity(220);
        row->addChild(card, -2);

        auto iconPlate = CCLayerColor::create({8, 8, 18, 175}, 54.f, 54.f);
        iconPlate->setPosition({x + 8.f, y - 27.f});
        row->addChild(iconPlate, -1);
        if (auto icon = iconForEntry(entry)) {
            auto largest = std::max(icon->getContentWidth(), icon->getContentHeight());
            if (largest > .001f) icon->setScale(42.f / largest);
            icon->setPosition({x + 35.f, y});
            row->addChild(icon);
        }

        auto label = CCLabelBMFont::create(entry.label.c_str(), "bigFont.fnt");
        label->setAnchorPoint({0.f, .5f});
        label->setPosition({x + 69.f, y + 17.f});
        label->setScale(.36f);
        label->limitLabelWidth(105.f, .36f, .2f);
        row->addChild(label);

        auto sprite = ButtonSprite::create(
            "RESTORE", 104, true, "bigFont.fnt", "GJ_button_01.png", 19.f, .34f
        );
        auto restore = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(HiddenBlocksPopup::onRestore)
        );
        restore->setPosition({x + 121.f, y - 17.f});
        restore->setUserObject(CCString::create(entry.id));
        row->addChild(restore);
        content->addChild(row);
        row->setScale(.92f);
        row->runAction(CCEaseBackOut::create(CCScaleTo::create(.18f, 1.f)));
    }
    m_scroll->scrollToTop();
}

void HiddenBlocksPopup::onRestore(CCObject* sender) {
    auto id = entryID(sender);
    if (id.empty()) return;
    if (!m_restoreCallback || !m_restoreCallback(id)) return;
    std::erase_if(m_entries, [&id](hidden_blocks::Entry const& entry) { return entry.id == id; });
    rebuildList();
}
}
