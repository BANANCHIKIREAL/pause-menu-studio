#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <string_view>

namespace pause_menu_studio::block_icons {
std::string frameForText(std::string_view text);
cocos2d::CCSprite* create(std::string_view text, float maxSize);
}
