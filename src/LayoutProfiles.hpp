#pragma once

#include <Geode/Geode.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace pause_menu_studio::profiles {
struct Transform {
    cocos2d::CCPoint position;
    std::optional<float> scaleX;
    std::optional<float> scaleY;
    std::optional<bool> hidden;
    std::optional<std::string> hiddenID;
    std::optional<std::string> hiddenLabel;
    std::optional<std::string> hiddenIcon;
    std::optional<std::string> previewIcon;
    std::optional<cocos2d::CCPoint> previewPosition;
};

using Snapshot = std::map<std::string, Transform>;

std::vector<std::string> names();
bool exists(std::string const& name);
std::optional<Snapshot> load(std::string const& name);
void save(std::string const& name, Snapshot const& snapshot);
int64_t savedAt(std::string const& name);
bool rename(std::string const& oldName, std::string const& newName);
bool duplicate(std::string const& sourceName, std::string const& newName);
void erase(std::string const& name);
}
