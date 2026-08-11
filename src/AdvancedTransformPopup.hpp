#pragma once

#include <Geode/Geode.hpp>

#include <optional>

namespace pause_menu_studio {
class AdvancedTransformPopup final : public geode::Popup {
public:
    static AdvancedTransformPopup* create(
        float rotation,
        std::optional<int> opacity,
        int zOrder,
        geode::Function<void(float, std::optional<int>, int)> callback
    );

private:
    geode::TextInput* m_rotationInput = nullptr;
    geode::TextInput* m_opacityInput = nullptr;
    geode::TextInput* m_zOrderInput = nullptr;
    bool m_opacityAvailable = false;
    geode::Function<void(float, std::optional<int>, int)> m_callback;

    bool init(
        float rotation,
        std::optional<int> opacity,
        int zOrder,
        geode::Function<void(float, std::optional<int>, int)> callback
    );
    void onApply(cocos2d::CCObject*);
};
}
