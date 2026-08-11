#pragma once

#include <Geode/Geode.hpp>

#include <optional>
#include <string>
#include <vector>

namespace pause_menu_studio::hidden_blocks {
struct Member {
    std::string path;
    cocos2d::CCPoint position;
    float scaleX;
    float scaleY;
    std::optional<float> rotation;
    std::optional<int> opacity;
    std::optional<int> zOrder;
};

struct Entry {
    std::string id;
    std::string label;
    std::vector<Member> members;
    std::string iconFrame;
};

std::vector<Entry> entries(std::string const& mode = "normal");
std::optional<Entry> find(std::string const& id, std::string const& mode = "normal");
std::vector<std::string> memberPaths(std::string const& mode = "normal");
std::string uniqueID(std::string const& preferred, std::string const& mode = "normal");
bool upsert(Entry const& entry, std::string const& mode = "normal");
void erase(std::string const& id, std::string const& mode = "normal");
void clear(std::string const& mode = "normal");
}
