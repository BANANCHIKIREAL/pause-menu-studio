#include "LayoutProfilePopups.hpp"
#include "LayoutProfiles.hpp"

#include <algorithm>
#include <cctype>
#include <ctime>

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

std::string savedDate(std::string const& name) {
    auto timestamp = profiles::savedAt(name);
    if (timestamp <= 0) return "LEGACY LAYOUT";
    std::time_t raw = static_cast<std::time_t>(timestamp);
    std::tm local {};
#ifdef GEODE_IS_WINDOWS
    localtime_s(&local, &raw);
#else
    localtime_r(&raw, &local);
#endif
    char buffer[32] {};
    if (std::strftime(buffer, sizeof(buffer), "SAVED %Y-%m-%d", &local) == 0) return "SAVED LAYOUT";
    return buffer;
}

CCDrawNode* createPreview(std::string const& name, ccColor4F color) {
    auto preview = CCDrawNode::create();
    auto snapshot = profiles::load(name);
    if (!snapshot || snapshot->empty()) return preview;
    size_t drawn = 0;
    for (auto const& [path, transform] : *snapshot) {
        if (transform.hidden.value_or(false)) continue;
        auto x = std::clamp(transform.position.x / 568.f, 0.f, 1.f);
        auto y = std::clamp(transform.position.y / 320.f, 0.f, 1.f);
        preview->drawDot({5.f + x * 64.f, 4.f + y * 34.f}, 1.8f, color);
        if (++drawn >= 28) break;
    }
    return preview;
}
}

LayoutNamePopup* LayoutNamePopup::create(
    Function<void(std::string)> callback,
    std::string title,
    std::string initialValue
) {
    auto ret = new LayoutNamePopup();
    if (ret && ret->init(std::move(callback), std::move(title), std::move(initialValue))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LayoutNamePopup::init(
    Function<void(std::string)> callback,
    std::string title,
    std::string initialValue
) {
    if (!Popup::init(310.f, 155.f)) return false;
    m_callback = std::move(callback);
    setTitle(title.c_str());

    m_input = TextInput::create(230.f, "Layout name");
    m_input->setCommonFilter(CommonFilter::Name);
    m_input->setMaxCharCount(24);
    m_input->setString(initialValue);
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
    Function<bool(std::string, std::string)> renameCallback,
    Function<bool(std::string, std::string)> duplicateCallback,
    Function<void(std::string)> deleteCallback
) {
    auto ret = new LayoutListPopup();
    if (ret && ret->init(
        std::move(names), std::move(activeName),
        std::move(applyCallback), std::move(renameCallback),
        std::move(duplicateCallback), std::move(deleteCallback)
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
    Function<bool(std::string, std::string)> renameCallback,
    Function<bool(std::string, std::string)> duplicateCallback,
    Function<void(std::string)> deleteCallback
) {
    if (!Popup::init(440.f, 310.f)) return false;
    m_names = std::move(names);
    m_activeName = std::move(activeName);
    m_applyCallback = std::move(applyCallback);
    m_renameCallback = std::move(renameCallback);
    m_duplicateCallback = std::move(duplicateCallback);
    m_deleteCallback = std::move(deleteCallback);
    setTitle("Saved layouts");

    m_scroll = ScrollLayer::create({390.f, 235.f});
    m_scroll->setPosition({25.f, 25.f});
    m_scroll->setID("layout-list");
    m_mainLayer->addChild(m_scroll);
    rebuildList();
    return true;
}

void LayoutListPopup::rebuildList() {
    auto content = m_scroll->m_contentLayer;
    content->removeAllChildren();
    constexpr float rowHeight = 72.f;
    auto height = std::max(235.f, static_cast<float>(m_names.size()) * rowHeight);
    content->setContentSize({390.f, height});
    content->setPositionY(235.f - height);

    if (m_names.empty()) {
        auto empty = CCLabelBMFont::create("No saved layouts", "bigFont.fnt");
        empty->setScale(.45f);
        empty->setOpacity(150);
        empty->setPosition({195.f, height - 110.f});
        content->addChild(empty);
        return;
    }

    for (size_t index = 0; index < m_names.size(); ++index) {
        auto const& name = m_names[index];
        auto y = height - rowHeight / 2.f - static_cast<float>(index) * rowHeight;
        auto row = CCMenu::create();
        row->setPosition({0.f, 0.f});
        row->setContentSize({390.f, rowHeight});

        auto card = CCScale9Sprite::create("square02_001.png");
        card->setContentSize({382.f, 64.f});
        card->setPosition({195.f, y});
        card->setColor(name == m_activeName ? ccColor3B {45, 105, 82} : ccColor3B {35, 31, 57});
        card->setOpacity(210);
        row->addChild(card, -2);

        auto previewBG = CCLayerColor::create({8, 8, 18, 185}, 74.f, 44.f);
        previewBG->setPosition({8.f, y - 22.f});
        row->addChild(previewBG, -1);
        auto preview = createPreview(
            name, name == m_activeName ? ccColor4F {.3f, 1.f, .65f, 1.f} : ccColor4F {.25f, .85f, 1.f, 1.f}
        );
        preview->setPosition({8.f, y - 19.f});
        row->addChild(preview);

        auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        label->setAnchorPoint({0.f, .5f});
        label->setPosition({92.f, y + 12.f});
        label->setScale(.44f);
        label->limitLabelWidth(170.f, .44f, .2f);
        if (name == m_activeName) label->setColor({100, 255, 170});
        row->addChild(label);

        auto date = CCLabelBMFont::create(savedDate(name).c_str(), "chatFont.fnt");
        date->setAnchorPoint({0.f, .5f});
        date->setPosition({92.f, y - 9.f});
        date->setScale(.42f);
        date->setOpacity(150);
        row->addChild(date);

        auto applySprite = ButtonSprite::create(
            name == m_activeName ? "ACTIVE" : "APPLY",
            64, true, "bigFont.fnt", "GJ_button_04.png", 18.f, .36f
        );
        auto apply = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(LayoutListPopup::onApply));
        apply->setPosition({300.f, y + 13.f});
        apply->setUserObject(CCString::create(name));
        row->addChild(apply);

        auto renameSprite = ButtonSprite::create("REN", 42, true, "bigFont.fnt", "GJ_button_05.png", 16.f, .3f);
        auto rename = CCMenuItemSpriteExtra::create(renameSprite, this, menu_selector(LayoutListPopup::onRename));
        rename->setPosition({278.f, y - 17.f});
        rename->setUserObject(CCString::create(name));
        row->addChild(rename);

        auto copySprite = ButtonSprite::create("COPY", 50, true, "bigFont.fnt", "GJ_button_05.png", 16.f, .28f);
        auto copy = CCMenuItemSpriteExtra::create(copySprite, this, menu_selector(LayoutListPopup::onDuplicate));
        copy->setPosition({327.f, y - 17.f});
        copy->setUserObject(CCString::create(name));
        row->addChild(copy);

        auto trashSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        trashSprite->setScale(.42f);
        auto trash = CCMenuItemSpriteExtra::create(trashSprite, this, menu_selector(LayoutListPopup::onDelete));
        trash->setPosition({365.f, y - 15.f});
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

void LayoutListPopup::onRename(CCObject* sender) {
    auto oldName = itemName(sender);
    if (oldName.empty()) return;
    auto self = WeakRef<LayoutListPopup>(this);
    if (auto popup = LayoutNamePopup::create(
        [self, oldName](std::string newName) {
            auto list = self.lock();
            if (!list || !list->m_renameCallback) return;
            if (!list->m_renameCallback(oldName, newName)) {
                Notification::create("That layout name is unavailable", NotificationIcon::Warning)->show();
                return;
            }
            for (auto& name : list->m_names) if (name == oldName) name = newName;
            if (list->m_activeName == oldName) list->m_activeName = newName;
            std::ranges::sort(list->m_names);
            list->rebuildList();
        },
        "Rename layout", oldName
    )) popup->show();
}

void LayoutListPopup::onDuplicate(CCObject* sender) {
    auto sourceName = itemName(sender);
    if (sourceName.empty()) return;
    auto self = WeakRef<LayoutListPopup>(this);
    if (auto popup = LayoutNamePopup::create(
        [self, sourceName](std::string newName) {
            auto list = self.lock();
            if (!list || !list->m_duplicateCallback) return;
            if (!list->m_duplicateCallback(sourceName, newName)) {
                Notification::create("That layout name is unavailable", NotificationIcon::Warning)->show();
                return;
            }
            list->m_names.push_back(newName);
            std::ranges::sort(list->m_names);
            list->rebuildList();
        },
        "Duplicate layout", sourceName + " copy"
    )) popup->show();
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
