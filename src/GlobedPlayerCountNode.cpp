#include "GlobedPlayerCountNode.hpp"

#include <dankmeme.globed2/include/globed/soft-link/API.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace pause_menu_studio {
namespace {
constexpr ccColor3B PLAYER_COUNT_COLOR = {90, 255, 225};

bool usesRainbowPlayerCount(size_t count) {
    switch (count) {
        case 14:
        case 25:
        case 42:
        case 52:
        case 67:
        case 69:
            return true;
        default:
            return false;
    }
}

ccColor3B rainbowColor(float hue) {
    hue = std::fmod(hue, 1.f);
    if (hue < 0.f) hue += 1.f;

    auto const sector = hue * 6.f;
    auto const x = 1.f - std::fabs(std::fmod(sector, 2.f) - 1.f);
    float red = 0.f;
    float green = 0.f;
    float blue = 0.f;

    if (sector < 1.f) {
        red = 1.f;
        green = x;
    } else if (sector < 2.f) {
        red = x;
        green = 1.f;
    } else if (sector < 3.f) {
        green = 1.f;
        blue = x;
    } else if (sector < 4.f) {
        green = x;
        blue = 1.f;
    } else if (sector < 5.f) {
        red = x;
        blue = 1.f;
    } else {
        red = 1.f;
        blue = x;
    }

    return {
        static_cast<GLubyte>(std::round(red * 255.f)),
        static_cast<GLubyte>(std::round(green * 255.f)),
        static_cast<GLubyte>(std::round(blue * 255.f)),
    };
}
}

GlobedPlayerCountNode* GlobedPlayerCountNode::create() {
    auto ret = new GlobedPlayerCountNode();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GlobedPlayerCountNode::init() {
    if (!CCNode::init()) return false;
    setID("globed-player-count");
    setContentSize({36.f, 15.f});

    m_label = CCLabelBMFont::create("", "bigFont.fnt");
    m_label->setID("count");
    m_label->setAnchorPoint({1.f, .5f});
    m_label->setPosition({23.f, 7.5f});
    m_label->setScale(.27f);
    m_label->setColor(PLAYER_COUNT_COLOR);
    addChild(m_label);

    ensureIcon();
    refresh(0.f);
    schedule(schedule_selector(GlobedPlayerCountNode::refresh), .5f);
    schedule(schedule_selector(GlobedPlayerCountNode::updateRainbow));
    return true;
}

void GlobedPlayerCountNode::ensureIcon() {
    if (m_icon || !globed::api::isAtLeast("v2.1.2") || !globed::api::available()) return;

    auto path = globed::api::misc::fullPathForFilename("icon-person.png", false);
    if (path.empty()) return;
    m_icon = CCSprite::create(path.c_str());
    if (!m_icon) return;

    m_icon->setID("globed-icon");
    auto largest = std::max(m_icon->getContentWidth(), m_icon->getContentHeight());
    if (largest > .001f) m_icon->setScale(8.5f / largest);
    m_icon->setColor(ccWHITE);
    m_icon->setOpacity(255);
    m_icon->setPosition({29.f, 7.5f});
    addChild(m_icon);
}

void GlobedPlayerCountNode::refresh(float) {
    // getPlayerCount() is the remote-player map size. Globed itself adds one
    // for the local player when exposing its TOTAL_PLAYERS game item.
    auto const active =
        globed::api::isAtLeast("v2.1.2") &&
        globed::api::available() &&
        globed::api::net::isConnected() &&
        globed::api::game::isActive();
    setVisible(active);
    if (!active) return;

    ensureIcon();
    auto const count = globed::api::game::getPlayerCount() + 1;
    if (count == m_displayedCount) return;

    auto const firstValue = m_displayedCount == std::numeric_limits<size_t>::max();
    m_displayedCount = count;
    m_label->setString(std::to_string(count).c_str());
    m_label->limitLabelWidth(19.f, .27f, .18f);
    if (!usesRainbowPlayerCount(count)) {
        m_rainbowHue = 0.f;
        m_label->setColor(PLAYER_COUNT_COLOR);
    }

    if (!firstValue) {
        m_label->stopAllActions();
        m_label->setScale(.34f);
        m_label->runAction(CCEaseSineOut::create(CCScaleTo::create(.16f, .27f)));
    }
}

void GlobedPlayerCountNode::updateRainbow(float dt) {
    if (!m_label || !isVisible() || !usesRainbowPlayerCount(m_displayedCount)) return;

    m_rainbowHue = std::fmod(m_rainbowHue + dt * .32f, 1.f);
    m_label->setColor(rainbowColor(m_rainbowHue));
}
}
