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
};

struct Entry {
    std::string id;
    std::string label;
    std::vector<Member> members;
    std::string iconFrame;
};

std::vector<Entry> entries();
std::optional<Entry> find(std::string const& id);
std::vector<std::string> memberPaths();
std::string uniqueID(std::string const& preferred);
bool upsert(Entry const& entry);
void erase(std::string const& id);
void clear();
}
