#pragma once

#include <Geode/Geode.hpp>

#include <string_view>

namespace pause_menu_studio::mod_icons {
cocos2d::CCSprite* create(std::string_view name, float maxSize);

cocos2d::CCNode* labeledButton(
    std::string_view iconName,
    std::string_view caption,
    cocos2d::CCSize size,
    float iconSize,
    cocos2d::ccColor3B color
);

cocos2d::CCNode* iconButton(
    std::string_view iconName,
    cocos2d::CCSize size,
    float iconSize,
    cocos2d::ccColor3B color
);
}
