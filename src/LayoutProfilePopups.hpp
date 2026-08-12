#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <vector>

namespace pause_menu_studio {
class LayoutNamePopup final : public geode::Popup {
public:
    static LayoutNamePopup* create(
        geode::Function<void(std::string)> callback,
        std::string title = "Save layout",
        std::string initialValue = ""
    );

private:
    geode::TextInput* m_input = nullptr;
    geode::Function<void(std::string)> m_callback;

    bool init(
        geode::Function<void(std::string)> callback,
        std::string title,
        std::string initialValue
    );
    void onSave(cocos2d::CCObject*);
};

class SaveLayoutPopup final : public geode::Popup {
public:
    static SaveLayoutPopup* create(
        std::vector<std::string> names,
        std::string activeName,
        std::string title,
        geode::Function<void(std::string)> saveNewCallback,
        geode::Function<void(std::string)> updateCallback
    );

private:
    std::vector<std::string> m_names;
    std::string m_activeName;
    geode::TextInput* m_input = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;
    geode::Function<void(std::string)> m_saveNewCallback;
    geode::Function<void(std::string)> m_updateCallback;

    bool init(
        std::vector<std::string> names,
        std::string activeName,
        std::string title,
        geode::Function<void(std::string)> saveNewCallback,
        geode::Function<void(std::string)> updateCallback
    );
    void rebuildList();
    void onSaveNew(cocos2d::CCObject*);
    void onUpdate(cocos2d::CCObject*);
};

class LayoutListPopup final : public geode::Popup {
public:
    static LayoutListPopup* create(
        std::vector<std::string> names,
        std::string activeName,
        std::string title,
        geode::Function<void(std::string)> applyCallback,
        geode::Function<bool(std::string, std::string)> renameCallback,
        geode::Function<bool(std::string, std::string)> duplicateCallback,
        geode::Function<void(std::string)> deleteCallback
    );

private:
    std::vector<std::string> m_names;
    std::string m_activeName;
    geode::Function<void(std::string)> m_applyCallback;
    geode::Function<bool(std::string, std::string)> m_renameCallback;
    geode::Function<bool(std::string, std::string)> m_duplicateCallback;
    geode::Function<void(std::string)> m_deleteCallback;
    geode::ScrollLayer* m_scroll = nullptr;

    bool init(
        std::vector<std::string> names,
        std::string activeName,
        std::string title,
        geode::Function<void(std::string)> applyCallback,
        geode::Function<bool(std::string, std::string)> renameCallback,
        geode::Function<bool(std::string, std::string)> duplicateCallback,
        geode::Function<void(std::string)> deleteCallback
    );
    void rebuildList();
    void onApply(cocos2d::CCObject* sender);
    void onRename(cocos2d::CCObject* sender);
    void onDuplicate(cocos2d::CCObject* sender);
    void onDelete(cocos2d::CCObject* sender);
};
}
