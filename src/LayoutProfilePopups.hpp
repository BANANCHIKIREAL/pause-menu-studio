#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <vector>

namespace pause_menu_studio {
class LayoutNamePopup final : public geode::Popup {
public:
    static LayoutNamePopup* create(geode::Function<void(std::string)> callback);

private:
    geode::TextInput* m_input = nullptr;
    geode::Function<void(std::string)> m_callback;

    bool init(geode::Function<void(std::string)> callback);
    void onSave(cocos2d::CCObject*);
};

class LayoutListPopup final : public geode::Popup {
public:
    static LayoutListPopup* create(
        std::vector<std::string> names,
        std::string activeName,
        geode::Function<void(std::string)> applyCallback,
        geode::Function<void(std::string)> deleteCallback
    );

private:
    std::vector<std::string> m_names;
    std::string m_activeName;
    geode::Function<void(std::string)> m_applyCallback;
    geode::Function<void(std::string)> m_deleteCallback;
    geode::ScrollLayer* m_scroll = nullptr;

    bool init(
        std::vector<std::string> names,
        std::string activeName,
        geode::Function<void(std::string)> applyCallback,
        geode::Function<void(std::string)> deleteCallback
    );
    void rebuildList();
    void onApply(cocos2d::CCObject* sender);
    void onDelete(cocos2d::CCObject* sender);
};
}
