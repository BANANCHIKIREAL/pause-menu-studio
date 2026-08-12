#include "ModIcons.hpp"

#include <Geode/ui/Label.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

using namespace geode::prelude;

namespace pause_menu_studio::mod_icons {
namespace {
CCDrawNode* roundedStencil(CCSize size, float radius) {
    auto stencil = CCDrawNode::create();
    std::vector<CCPoint> points;
    constexpr int cornerSegments = 6;

    auto appendCorner = [&](CCPoint center, float startDegrees) {
        for (int index = 0; index <= cornerSegments; ++index) {
            auto degrees = startDegrees + 90.f * static_cast<float>(index) / cornerSegments;
            auto radians = degrees * std::numbers::pi_v<float> / 180.f;
            points.emplace_back(
                center.x + std::cos(radians) * radius,
                center.y + std::sin(radians) * radius
            );
        }
    };

    appendCorner({size.width - radius, radius}, -90.f);
    appendCorner({size.width - radius, size.height - radius}, 0.f);
    appendCorner({radius, size.height - radius}, 90.f);
    appendCorner({radius, radius}, 180.f);
    stencil->drawPolygon(
        points.data(), static_cast<unsigned int>(points.size()),
        {1.f, 1.f, 1.f, 1.f}, 0.f, {0.f, 0.f, 0.f, 0.f}
    );
    return stencil;
}
}

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

    // Keep the original compact action-tile texture. Clip only its outer corners;
    // swapping to a different GD panel sprite distorts into strips at 30x26.
    auto tile = CCScale9Sprite::create("square02_001.png");
    tile->setContentSize(size);
    tile->setPosition({size.width / 2.f, size.height / 2.f});
    tile->setColor(color);
    tile->setOpacity(210);
    auto clip = CCClippingNode::create(roundedStencil(size, 4.5f));
    clip->setAlphaThreshold(.05f);
    clip->setContentSize(size);
    clip->addChild(tile);
    visual->addChild(clip, -1);

    if (auto icon = create(iconName, iconSize)) {
        icon->setPosition({size.width / 2.f, size.height / 2.f});
        visual->addChild(icon);
    }
    return visual;
}
}
