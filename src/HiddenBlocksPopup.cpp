#include "HiddenBlocksPopup.hpp"
#include "ModIcons.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio {
namespace {
std::string entryID(CCObject* sender) {
    auto node = typeinfo_cast<CCNode*>(sender);
    auto value = node ? typeinfo_cast<CCString*>(node->getUserObject()) : nullptr;
    return value ? std::string(value->getCString()) : std::string();
}

CCSprite* iconForEntry(hidden_blocks::Entry const&) {
    return mod_icons::create("generic-block-icon.png", 38.f);
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
    if (auto close = mod_icons::create("close-icon.png", 24.f)) {
        setCloseButtonSpr(close, close->getScale());
    }

    m_scroll = ScrollLayer::create({390.f, 220.f});
    m_scroll->setPosition({25.f, 31.f});
    m_scroll->setID("hidden-blocks-list");
    m_mainLayer->addChild(m_scroll);

    m_pageMenu = CCMenu::create();
    m_pageMenu->setPosition({m_size.width / 2.f, 16.f});
    m_pageMenu->setID("hidden-blocks-pages");
    auto previousSprite = mod_icons::create("arrow-icon.png", 22.f);
    previousSprite->setFlipX(true);
    auto previous = CCMenuItemSpriteExtra::create(
        previousSprite, this, menu_selector(HiddenBlocksPopup::onPage)
    );
    previous->setTag(-1);
    previous->setPositionX(-62.f);
    m_pageMenu->addChild(previous);
    auto nextSprite = mod_icons::create("arrow-icon.png", 22.f);
    auto next = CCMenuItemSpriteExtra::create(
        nextSprite, this, menu_selector(HiddenBlocksPopup::onPage)
    );
    next->setTag(1);
    next->setPositionX(62.f);
    m_pageMenu->addChild(next);
    m_mainLayer->addChild(m_pageMenu, 3);

    m_pageLabel = Label::create("", "chatFont.fnt");
    m_pageLabel->setPosition({m_size.width / 2.f, 16.f});
    m_pageLabel->setScale(.42f);
    m_mainLayer->addChild(m_pageLabel, 3);
    rebuildList();
    return true;
}

void HiddenBlocksPopup::rebuildList() {
    auto content = m_scroll->m_contentLayer;
    content->removeAllChildren();
    constexpr float cardWidth = 185.f;
    constexpr float cardHeight = 104.f;
    constexpr size_t columns = 2;
    constexpr size_t pageSize = 4;
    auto pageCount = std::max<size_t>(1, (m_entries.size() + pageSize - 1) / pageSize);
    m_page = std::min(m_page, pageCount - 1);
    auto start = m_page * pageSize;
    auto end = std::min(start + pageSize, m_entries.size());
    constexpr float height = 220.f;
    content->setContentSize({390.f, height});
    content->setPositionY(0.f);

    if (m_pageMenu) m_pageMenu->setVisible(pageCount > 1);
    if (m_pageLabel) {
        m_pageLabel->setString(
            pageCount > 1
                ? fmt::format("{} / {}  -  {} BLOCKS", m_page + 1, pageCount, m_entries.size()).c_str()
                : fmt::format("{} BLOCKS", m_entries.size()).c_str()
        );
    }

    if (m_entries.empty()) {
        auto empty = Label::create("Trash is empty", "bigFont.fnt");
        empty->setScale(.45f);
        empty->setOpacity(150);
        empty->setPosition({195.f, height - 110.f});
        content->addChild(empty);
        return;
    }

    for (size_t index = start; index < end; ++index) {
        auto const& entry = m_entries[index];
        auto pageIndex = index - start;
        auto column = pageIndex % columns;
        auto rowIndex = pageIndex / columns;
        auto x = 4.f + static_cast<float>(column) * 193.f;
        auto y = height - 50.f - static_cast<float>(rowIndex) * cardHeight;
        auto row = CCMenu::create();
        row->setPosition(CCPointZero);
        row->setContentSize({cardWidth, 92.f});

        auto card = CCScale9Sprite::create("GJ_square02.png");
        card->setContentSize({cardWidth, 94.f});
        card->setPosition({x + cardWidth / 2.f, y});
        card->setColor({36, 31, 78});
        card->setOpacity(235);
        row->addChild(card, -2);

        auto iconPlate = CCScale9Sprite::create("GJ_square01.png");
        iconPlate->setContentSize({54.f, 54.f});
        iconPlate->setPosition({x + 35.f, y});
        iconPlate->setColor({12, 11, 28});
        iconPlate->setOpacity(220);
        row->addChild(iconPlate, -1);
        if (auto icon = iconForEntry(entry)) {
            icon->setPosition({x + 35.f, y});
            row->addChild(icon);
        }

        auto label = Label::create(entry.label, "bigFont.fnt");
        label->setAnchorPoint({0.f, .5f});
        label->setPosition({x + 69.f, y + 17.f});
        label->setScale(.36f);
        label->setLimitLabelWidth(105.f, .36f, .2f);
        row->addChild(label);

        auto sprite = mod_icons::labeledButton(
            "restore-icon.png", "RESTORE", {104.f, 28.f}, 19.f, {54, 108, 88}
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

void HiddenBlocksPopup::onPage(CCObject* sender) {
    auto node = typeinfo_cast<CCNode*>(sender);
    if (!node) return;
    if (node->getTag() < 0) {
        if (m_page > 0) --m_page;
    } else {
        auto pageCount = std::max<size_t>(1, (m_entries.size() + 3) / 4);
        if (m_page + 1 < pageCount) ++m_page;
    }
    rebuildList();
}

void HiddenBlocksPopup::onRestore(CCObject* sender) {
    auto id = entryID(sender);
    if (id.empty()) return;
    if (!m_restoreCallback || !m_restoreCallback(id)) return;
    std::erase_if(m_entries, [&id](hidden_blocks::Entry const& entry) { return entry.id == id; });
    rebuildList();
}
}
