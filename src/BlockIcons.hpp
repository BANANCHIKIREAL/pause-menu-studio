#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace pause_menu_studio::block_icons {
std::string frameForText(std::string_view text);
std::string frameForNode(cocos2d::CCNode* node);
std::string frameForNodes(std::vector<cocos2d::CCNode*> const& nodes);
cocos2d::CCSprite* create(std::string_view text, float maxSize);
}
