#include "UpdatePopup.hpp"
#include "ModIcons.hpp"

#include <Geode/ui/Label.hpp>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>
#include <vector>

using namespace geode::prelude;

namespace pause_menu_studio {
namespace {
std::string trim(std::string value) {
    auto whitespace = [](unsigned char c) { return std::isspace(c); };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), whitespace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), whitespace).base(), value.end());
    return value;
}

void eraseMarkdownPairs(std::string& value, std::string_view marker) {
    size_t position = 0;
    while ((position = value.find(marker, position)) != std::string::npos) {
        value.erase(position, marker.size());
    }
}

std::string plainReleaseLine(std::string value) {
    value = trim(std::move(value));
    while (!value.empty() && value.front() == '#') value.erase(value.begin());
    value = trim(std::move(value));

    // Preserve the readable label from Markdown links and discard only the URL.
    size_t searchFrom = 0;
    while (true) {
        auto openLabel = value.find('[', searchFrom);
        if (openLabel == std::string::npos) break;
        auto closeLabel = value.find(']', openLabel + 1);
        if (closeLabel == std::string::npos || closeLabel + 1 >= value.size() || value[closeLabel + 1] != '(') {
            searchFrom = openLabel + 1;
            continue;
        }
        auto closeURL = value.find(')', closeLabel + 2);
        if (closeURL == std::string::npos) break;
        auto label = value.substr(openLabel + 1, closeLabel - openLabel - 1);
        value.replace(openLabel, closeURL - openLabel + 1, label);
        searchFrom = openLabel + label.size();
    }

    eraseMarkdownPairs(value, "**");
    eraseMarkdownPairs(value, "__");
    eraseMarkdownPairs(value, "`");
    return trim(std::move(value));
}

std::vector<std::string> wrapLine(std::string const& line, size_t limit = 54) {
    if (line.size() <= limit) return {line};
    std::vector<std::string> result;
    std::istringstream words(line);
    std::string word;
    std::string current;
    while (words >> word) {
        if (!current.empty() && current.size() + word.size() + 1 > limit) {
            result.push_back(current);
            current = word;
        } else {
            if (!current.empty()) current += ' ';
            current += word;
        }
    }
    if (!current.empty()) result.push_back(current);
    return result;
}

struct ReleaseItem {
    std::string title;
    std::vector<std::string> lines;
};

std::vector<ReleaseItem> displayItems(std::string const& releaseNotes) {
    std::vector<ReleaseItem> result;
    std::istringstream input(releaseNotes);
    std::string raw;
    while (std::getline(input, raw)) {
        if (!raw.empty() && raw.back() == '\r') raw.pop_back();
        auto original = trim(raw);
        auto heading = original.starts_with('#');
        auto bullet = original.starts_with("- ") || original.starts_with("* ");
        if (heading || original.empty()) continue;
        if (bullet) original.erase(0, 2);
        raw = std::move(original);
        auto line = plainReleaseLine(std::move(raw));
        if (line.empty()) continue;

        ReleaseItem item;
        if (auto colon = line.find(':'); colon != std::string::npos && colon <= 28) {
            item.title = trim(line.substr(0, colon));
            line = trim(line.substr(colon + 1));
        }
        item.lines = wrapLine(line);
        if (item.lines.empty()) item.lines.emplace_back("Update details available.");
        result.push_back(std::move(item));
    }
    if (result.empty()) {
        result.push_back({"UPDATE", {"Release notes were not provided for this update."}});
    }
    return result;
}
}

UpdatePopup* UpdatePopup::create(
    std::string version,
    std::string releaseNotes,
    Function<void()> downloadCallback
) {
    auto ret = new UpdatePopup();
    if (ret && ret->init(
        std::move(version), std::move(releaseNotes), std::move(downloadCallback)
    )) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool UpdatePopup::init(
    std::string version,
    std::string releaseNotes,
    Function<void()> downloadCallback
) {
    if (!Popup::init(440.f, 320.f)) return false;
    m_downloadCallback = std::move(downloadCallback);
    setTitle("Update available");
    if (auto close = mod_icons::create("close-icon.png", 24.f)) {
        setCloseButtonSpr(close, close->getScale());
    }

    auto versionLabel = Label::create("PAUSE MENU STUDIO  " + version, "bigFont.fnt");
    versionLabel->setScale(.48f);
    versionLabel->setColor({95, 255, 180});
    versionLabel->setPosition({m_size.width / 2.f, 258.f});
    m_mainLayer->addChild(versionLabel);

    auto heading = Label::create("WHAT'S NEW & FIXED", "bigFont.fnt");
    heading->setScale(.38f);
    heading->setColor({80, 225, 255});
    heading->setAnchorPoint({0.f, .5f});
    heading->setPosition({31.f, 231.f});
    m_mainLayer->addChild(heading);

    auto frame = CCScale9Sprite::create("GJ_square02.png");
    frame->setContentSize({390.f, 174.f});
    frame->setPosition({m_size.width / 2.f, 137.f});
    frame->setColor({20, 18, 52});
    frame->setOpacity(220);
    m_mainLayer->addChild(frame);

    auto scroll = ScrollLayer::create({374.f, 158.f});
    scroll->setPosition({33.f, 58.f});
    scroll->setID("update-release-notes");
    m_mainLayer->addChild(scroll);

    auto items = displayItems(releaseNotes);
    constexpr float lineHeight = 12.f;
    float requiredHeight = 8.f;
    for (auto const& item : items) {
        requiredHeight += 12.f + static_cast<float>(item.lines.size()) * lineHeight;
        if (!item.title.empty()) requiredHeight += 13.f;
        requiredHeight += 5.f;
    }
    auto contentHeight = std::max(158.f, requiredHeight);
    auto content = scroll->m_contentLayer;
    content->setContentSize({374.f, contentHeight});
    content->setPositionY(158.f - contentHeight);

    float cursor = contentHeight - 5.f;
    for (size_t index = 0; index < items.size(); ++index) {
        auto const& item = items[index];
        auto cardHeight = 12.f + static_cast<float>(item.lines.size()) * lineHeight;
        if (!item.title.empty()) cardHeight += 13.f;

        auto card = CCScale9Sprite::create("GJ_square02.png");
        card->setContentSize({358.f, cardHeight});
        card->setPosition({187.f, cursor - cardHeight / 2.f});
        card->setColor(index % 2 == 0 ? ccColor3B {30, 26, 68} : ccColor3B {25, 31, 70});
        card->setOpacity(205);
        content->addChild(card);

        auto accent = CCLayerColor::create(
            index % 2 == 0 ? ccColor4B {80, 225, 255, 255} : ccColor4B {185, 135, 255, 255},
            3.f, cardHeight - 8.f
        );
        accent->setPosition({12.f, cursor - cardHeight + 4.f});
        content->addChild(accent, 2);

        if (auto icon = mod_icons::create("done-icon.png", 14.f)) {
            icon->setPosition({27.f, cursor - cardHeight / 2.f});
            icon->setOpacity(235);
            content->addChild(icon, 3);
        }

        auto textY = cursor - 9.f;
        if (!item.title.empty()) {
            auto title = Label::create(utils::string::toUpper(item.title), "bigFont.fnt");
            title->setAnchorPoint({0.f, .5f});
            title->setPosition({40.f, textY});
            title->setScale(.30f);
            title->setColor(index % 2 == 0 ? ccColor3B {80, 225, 255} : ccColor3B {205, 160, 255});
            title->setLimitLabelWidth(315.f, .30f, .22f);
            content->addChild(title, 3);
            textY -= 14.f;
        }

        for (auto const& text : item.lines) {
            auto label = Label::create(text, "chatFont.fnt");
            label->setAnchorPoint({0.f, .5f});
            label->setScale(.52f);
            label->setColor({242, 240, 255});
            label->setOpacity(245);
            label->setPosition({40.f, textY});
            content->addChild(label, 3);
            textY -= lineHeight;
        }
        cursor -= cardHeight + 5.f;
    }
    scroll->scrollToTop();

    auto menu = CCMenu::create();
    menu->setPosition({m_size.width / 2.f, 31.f});
    auto laterSprite = mod_icons::labeledButton(
        "arrow-icon.png", "LATER", {102.f, 32.f}, 20.f, {60, 52, 108}
    );
    auto later = CCMenuItemSpriteExtra::create(
        laterSprite, this, menu_selector(UpdatePopup::onLater)
    );
    later->setPositionX(-67.f);
    menu->addChild(later);

    auto downloadSprite = mod_icons::labeledButton(
        "download-icon.png", "DOWNLOAD", {122.f, 32.f}, 20.f, {45, 105, 88}
    );
    auto download = CCMenuItemSpriteExtra::create(
        downloadSprite, this, menu_selector(UpdatePopup::onDownload)
    );
    download->setPositionX(67.f);
    menu->addChild(download);
    m_mainLayer->addChild(menu);
    return true;
}

void UpdatePopup::onLater(CCObject*) {
    onClose(nullptr);
}

void UpdatePopup::onDownload(CCObject*) {
    auto callback = std::move(m_downloadCallback);
    onClose(nullptr);
    if (callback) callback();
}
}
