#include "LayoutProfilePopups.hpp"

#include <algorithm>
#include <cctype>

using namespace geode::prelude;

namespace pause_menu_studio {
namespace {
std::string trim(std::string value) {
    auto whitespace = [](unsigned char c) { return std::isspace(c); };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), whitespace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), whitespace).base(), value.end());
    return value;
}

std::string itemName(CCObject* sender) {
    auto node = typeinfo_cast<CCNode*>(sender);
    auto value = node ? typeinfo_cast<CCString*>(node->getUserObject()) : nullptr;
    return value ? std::string(value->getCString()) : std::string();
}
}

LayoutNamePopup* LayoutNamePopup::create(Function<void(std::string)> callback) {
    auto ret = new LayoutNamePopup();
    if (ret && ret->init(std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LayoutNamePopup::init(Function<void(std::string)> callback) {
    if (!Popup::init(310.f, 155.f)) return false;
    m_callback = std::move(callback);
    setTitle("Save layout");

    m_input = TextInput::create(230.f, "Layout name");
    m_input->setCommonFilter(CommonFilter::Name);
    m_input->setMaxCharCount(24);
    m_input->setPosition({m_size.width / 2.f, 82.f});
    m_mainLayer->addChild(m_input);

    auto sprite = ButtonSprite::create("SAVE", 80, true, "bigFont.fnt", "GJ_button_01.png", 24.f, .55f);
    auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(LayoutNamePopup::onSave));
    auto saveMenu = CCMenu::create();
    saveMenu->setID("save-layout-menu");
    saveMenu->setPosition({m_size.width / 2.f, 35.f});
    saveMenu->addChild(button);
    m_mainLayer->addChild(saveMenu);
    m_input->focus();
    return true;
}

void LayoutNamePopup::onSave(CCObject*) {
    auto name = trim(m_input ? std::string(m_input->getString()) : std::string());
    if (name.empty()) {
        Notification::create("Enter a layout name", NotificationIcon::Warning)->show();
        return;
    }
    auto callback = std::move(m_callback);
    onClose(nullptr);
    if (callback) callback(name);
}

LayoutListPopup* LayoutListPopup::create(
    std::vector<std::string> names,
    std::string activeName,
    Function<void(std::string)> applyCallback,
    Function<void(std::string)> deleteCallback
) {
    auto ret = new LayoutListPopup();
    if (ret && ret->init(
        std::move(names), std::move(activeName),
        std::move(applyCallback), std::move(deleteCallback)
    )) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LayoutListPopup::init(
    std::vector<std::string> names,
    std::string activeName,
    Function<void(std::string)> applyCallback,
    Function<void(std::string)> deleteCallback
) {
    if (!Popup::init(370.f, 260.f)) return false;
    m_names = std::move(names);
    m_activeName = std::move(activeName);
    m_applyCallback = std::move(applyCallback);
    m_deleteCallback = std::move(deleteCallback);
    setTitle("Saved layouts");

    m_scroll = ScrollLayer::create({320.f, 190.f});
    m_scroll->setPosition({25.f, 25.f});
    m_scroll->setID("layout-list");
    m_mainLayer->addChild(m_scroll);
    rebuildList();
    return true;
}

void LayoutListPopup::rebuildList() {
    auto content = m_scroll->m_contentLayer;
    content->removeAllChildren();
    auto height = std::max(190.f, static_cast<float>(m_names.size()) * 38.f);
    content->setContentSize({320.f, height});
    content->setPositionY(190.f - height);

    if (m_names.empty()) {
        auto empty = CCLabelBMFont::create("No saved layouts", "bigFont.fnt");
        empty->setScale(.45f);
        empty->setOpacity(150);
        empty->setPosition({160.f, height - 90.f});
        content->addChild(empty);
        return;
    }

    for (size_t index = 0; index < m_names.size(); ++index) {
        auto const& name = m_names[index];
        auto y = height - 19.f - static_cast<float>(index) * 38.f;
        auto row = CCMenu::create();
        row->setPosition({0.f, 0.f});
        row->setContentSize({320.f, 38.f});

        auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        label->setAnchorPoint({0.f, .5f});
        label->setPosition({8.f, y});
        label->setScale(.42f);
        label->limitLabelWidth(165.f, .42f, .2f);
        if (name == m_activeName) label->setColor({100, 255, 170});
        row->addChild(label);

        auto applySprite = ButtonSprite::create(
            name == m_activeName ? "ACTIVE" : "APPLY",
            70, true, "bigFont.fnt", "GJ_button_04.png", 19.f, .4f
        );
        auto apply = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(LayoutListPopup::onApply));
        apply->setPosition({245.f, y});
        apply->setUserObject(CCString::create(name));
        row->addChild(apply);

        auto trashSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        trashSprite->setScale(.55f);
        auto trash = CCMenuItemSpriteExtra::create(trashSprite, this, menu_selector(LayoutListPopup::onDelete));
        trash->setPosition({300.f, y});
        trash->setUserObject(CCString::create(name));
        row->addChild(trash);
        content->addChild(row);
    }
    m_scroll->scrollToTop();
}

void LayoutListPopup::onApply(CCObject* sender) {
    auto name = itemName(sender);
    if (name.empty()) return;
    if (m_applyCallback) m_applyCallback(name);
    onClose(nullptr);
}

void LayoutListPopup::onDelete(CCObject* sender) {
    auto name = itemName(sender);
    if (name.empty()) return;
    auto self = WeakRef<LayoutListPopup>(this);
    createQuickPopup(
        "Delete layout",
        "Delete the saved layout <cy>" + name + "</c>?",
        "Cancel", "Delete",
        [self, name](FLAlertLayer*, bool confirmed) {
            if (!confirmed) return;
            auto popup = self.lock();
            if (!popup) return;
            if (popup->m_deleteCallback) popup->m_deleteCallback(name);
            std::erase(popup->m_names, name);
            if (popup->m_activeName == name) popup->m_activeName.clear();
            popup->rebuildList();
        }
    );
}
}
