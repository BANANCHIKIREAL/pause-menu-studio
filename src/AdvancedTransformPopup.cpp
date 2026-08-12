#include "AdvancedTransformPopup.hpp"
#include "ModIcons.hpp"

#include <Geode/ui/Label.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio {
AdvancedTransformPopup* AdvancedTransformPopup::create(
    float rotation,
    std::optional<int> opacity,
    int zOrder,
    Function<void(float, std::optional<int>, int)> callback
) {
    auto ret = new AdvancedTransformPopup();
    if (ret && ret->init(rotation, opacity, zOrder, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool AdvancedTransformPopup::init(
    float rotation,
    std::optional<int> opacity,
    int zOrder,
    Function<void(float, std::optional<int>, int)> callback
) {
    if (!Popup::init(360.f, 210.f)) return false;
    m_callback = std::move(callback);
    m_opacityAvailable = opacity.has_value();
    setTitle("Advanced transform");
    setCloseButtonSpr(mod_icons::create("close-icon.png", 24.f));

    auto subtitle = Label::create("EDIT THE SELECTED BLOCK'S VISUAL TRANSFORM", "chatFont.fnt");
    subtitle->setPosition({m_size.width / 2.f, 160.f});
    subtitle->setScale(.45f);
    subtitle->setColor({205, 190, 255});
    subtitle->setOpacity(210);
    m_mainLayer->addChild(subtitle);

    auto addField = [this](
        char const* title,
        char const* hint,
        float x,
        std::string const& value,
        CommonFilter filter,
        int maxChars
    ) {
        auto label = Label::create(title, "bigFont.fnt");
        label->setPosition({x, 137.f});
        label->setScale(.32f);
        label->setColor(ccWHITE);
        m_mainLayer->addChild(label);

        auto input = TextInput::create(86.f, title);
        input->setPosition({x, 108.f});
        input->setCommonFilter(filter);
        input->setMaxCharCount(maxChars);
        input->setString(value, false);
        m_mainLayer->addChild(input);

        auto help = Label::create(hint, "chatFont.fnt");
        help->setPosition({x, 82.f});
        help->setScale(.40f);
        help->setColor({155, 145, 205});
        m_mainLayer->addChild(help);
        return input;
    };

    m_rotationInput = addField(
        "ROTATION", "-360 TO 360", 69.f, fmt::format("{:.1f}", rotation),
        CommonFilter::Float, 7
    );
    m_opacityInput = addField(
        "OPACITY", m_opacityAvailable ? "0 TO 255" : "NOT SUPPORTED", 180.f,
        m_opacityAvailable ? std::to_string(*opacity) : std::string(),
        CommonFilter::Int, 3
    );
    m_opacityInput->setEnabled(m_opacityAvailable);
    m_zOrderInput = addField(
        "LAYER", "-1000 TO 1000", 291.f, std::to_string(zOrder),
        CommonFilter::Int, 5
    );

    auto applySprite = mod_icons::labeledButton(
        "transform-icon.png", "APPLY TRANSFORM", {158.f, 32.f}, 21.f, {76, 52, 128}
    );
    auto apply = CCMenuItemSpriteExtra::create(
        applySprite, this, menu_selector(AdvancedTransformPopup::onApply)
    );
    auto menu = CCMenu::create();
    menu->setPosition({m_size.width / 2.f, 38.f});
    menu->addChild(apply);
    m_mainLayer->addChild(menu);
    return true;
}

void AdvancedTransformPopup::onApply(CCObject*) {
    auto rotation = numFromString<float>(m_rotationInput->getString());
    auto zOrder = numFromString<int>(m_zOrderInput->getString());
    if (rotation.isErr() || zOrder.isErr()) {
        Notification::create("Enter valid rotation and layer values", NotificationIcon::Warning)->show();
        return;
    }

    std::optional<int> opacity;
    if (m_opacityAvailable) {
        auto parsed = numFromString<int>(m_opacityInput->getString());
        if (parsed.isErr()) {
            Notification::create("Enter a valid opacity", NotificationIcon::Warning)->show();
            return;
        }
        opacity = std::clamp(parsed.unwrap(), 0, 255);
    }

    auto callback = std::move(m_callback);
    onClose(nullptr);
    if (callback) {
        callback(
            std::clamp(rotation.unwrap(), -360.f, 360.f),
            opacity,
            std::clamp(zOrder.unwrap(), -1000, 1000)
        );
    }
}
}
