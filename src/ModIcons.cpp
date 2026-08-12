#include "ModIcons.hpp"

#include <Geode/ui/Label.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio::mod_icons {
CCSprite* create(std::string_view name, float maxSize) {
    auto path = Mod::get()->expandSpriteName(name);
    auto icon = CCSprite::create(path.c_str());
    if (!icon) return nullptr;
    auto largest = std::max(icon->getContentWidth(), icon->getContentHeight());
    if (largest > .001f) icon->setScale(maxSize / largest);
    icon->setColor(ccWHITE);
    icon->setOpacity(255);
    return icon;
}

CCNode* labeledButton(
    std::string_view iconName,
    std::string_view caption,
    CCSize size,
    float iconSize,
    ccColor3B color
) {
    auto visual = CCNode::create();
    visual->setContentSize(size);

    auto tile = CCScale9Sprite::create("square02_001.png");
    tile->setContentSize(size);
    tile->setPosition({size.width / 2.f, size.height / 2.f});
    tile->setColor(color);
    tile->setOpacity(210);
    visual->addChild(tile, -1);

    auto iconX = caption.empty() ? size.width / 2.f : 17.f;
    if (auto icon = create(iconName, iconSize)) {
        icon->setPosition({iconX, size.height / 2.f});
        visual->addChild(icon);
    }

    if (!caption.empty()) {
        auto label = Label::create(std::string(caption), "bigFont.fnt");
        label->setPosition({(size.width + 25.f) / 2.f, size.height / 2.f});
        label->setScale(.30f);
        label->setColor(ccWHITE);
        label->setLimitLabelWidth(size.width - 37.f, .30f, .19f);
        visual->addChild(label);
    }
    return visual;
}

CCNode* iconButton(
    std::string_view iconName,
    CCSize size,
    float iconSize,
    ccColor3B color
) {
    auto visual = CCNode::create();
    visual->setContentSize(size);

    // Layout row actions are deliberately rounder than the wide text buttons.
    // GJ_square02 keeps its corner radius even at the compact 30x26 size.
    auto tile = CCScale9Sprite::create("GJ_square02.png");
    tile->setContentSize(size);
    tile->setPosition({size.width / 2.f, size.height / 2.f});
    tile->setColor(color);
    tile->setOpacity(210);
    visual->addChild(tile, -1);

    if (auto icon = create(iconName, iconSize)) {
        icon->setPosition({size.width / 2.f, size.height / 2.f});
        visual->addChild(icon);
    }
    return visual;
}
}
