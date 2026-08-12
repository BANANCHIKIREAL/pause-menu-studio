#pragma once

#include <Geode/Geode.hpp>

#include <string>

namespace pause_menu_studio {
class UpdatePopup final : public geode::Popup {
public:
    static UpdatePopup* create(
        std::string version,
        std::string releaseNotes,
        geode::Function<void()> downloadCallback
    );

private:
    geode::Function<void()> m_downloadCallback;

    bool init(
        std::string version,
        std::string releaseNotes,
        geode::Function<void()> downloadCallback
    );
    void onLater(cocos2d::CCObject*);
    void onDownload(cocos2d::CCObject*);
};
}
