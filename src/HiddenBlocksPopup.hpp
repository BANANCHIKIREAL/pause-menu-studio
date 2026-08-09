#pragma once

#include "HiddenBlocks.hpp"

#include <Geode/Geode.hpp>

#include <string>
#include <vector>

namespace pause_menu_studio {
class HiddenBlocksPopup final : public geode::Popup {
public:
    static HiddenBlocksPopup* create(
        std::vector<hidden_blocks::Entry> entries,
        geode::Function<bool(std::string)> restoreCallback
    );

private:
    std::vector<hidden_blocks::Entry> m_entries;
    geode::Function<bool(std::string)> m_restoreCallback;
    geode::ScrollLayer* m_scroll = nullptr;
    cocos2d::CCMenu* m_pageMenu = nullptr;
    cocos2d::CCLabelBMFont* m_pageLabel = nullptr;
    size_t m_page = 0;

    bool init(
        std::vector<hidden_blocks::Entry> entries,
        geode::Function<bool(std::string)> restoreCallback
    );
    void rebuildList();
    void onRestore(cocos2d::CCObject* sender);
    void onPage(cocos2d::CCObject* sender);
};
}
