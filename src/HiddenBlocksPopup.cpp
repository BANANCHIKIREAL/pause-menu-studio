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
    if (!Popup::init(370.f, 260.f)) return false;
    m_entries = std::move(entries);
    m_restoreCallback = std::move(restoreCallback);
    setTitle("Hidden blocks");

    m_scroll = ScrollLayer::create({320.f, 190.f});
    m_scroll->setPosition({25.f, 25.f});
    m_scroll->setID("hidden-blocks-list");
    m_mainLayer->addChild(m_scroll);
    rebuildList();
    return true;
}

void HiddenBlocksPopup::rebuildList() {
    auto content = m_scroll->m_contentLayer;
    content->removeAllChildren();
    auto height = std::max(190.f, static_cast<float>(m_entries.size()) * 38.f);
    content->setContentSize({320.f, height});
    content->setPositionY(190.f - height);

    if (m_entries.empty()) {
        auto empty = CCLabelBMFont::create("Trash is empty", "bigFont.fnt");
        empty->setScale(.45f);
        empty->setOpacity(150);
        empty->setPosition({160.f, height - 90.f});
        content->addChild(empty);
        return;
    }

    for (size_t index = 0; index < m_entries.size(); ++index) {
        auto const& entry = m_entries[index];
        auto y = height - 19.f - static_cast<float>(index) * 38.f;
        auto row = CCMenu::create();
        row->setPosition(CCPointZero);
        row->setContentSize({320.f, 38.f});

        auto label = CCLabelBMFont::create(entry.label.c_str(), "bigFont.fnt");
        label->setAnchorPoint({0.f, .5f});
        label->setPosition({8.f, y});
        label->setScale(.42f);
        label->limitLabelWidth(190.f, .42f, .2f);
        row->addChild(label);

        auto sprite = ButtonSprite::create(
            "RESTORE", 88, true, "bigFont.fnt", "GJ_button_01.png", 20.f, .38f
        );
        auto restore = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(HiddenBlocksPopup::onRestore)
        );
        restore->setPosition({267.f, y});
        restore->setUserObject(CCString::create(entry.id));
        row->addChild(restore);
        content->addChild(row);
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
