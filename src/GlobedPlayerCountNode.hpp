#pragma once

#include <Geode/Geode.hpp>

#include <limits>

namespace pause_menu_studio {
class GlobedPlayerCountNode final : public cocos2d::CCNode {
public:
    static GlobedPlayerCountNode* create();

private:
    cocos2d::CCLabelBMFont* m_label = nullptr;
    cocos2d::CCSprite* m_icon = nullptr;
    size_t m_displayedCount = std::numeric_limits<size_t>::max();

    bool init();
    void refresh(float);
    void ensureIcon();
};
}
