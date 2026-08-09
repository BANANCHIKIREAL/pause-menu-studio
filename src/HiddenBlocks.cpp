#include "HiddenBlocks.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio::hidden_blocks {
namespace {
constexpr char const* STORAGE_KEY = "hidden-blocks-v1";

matjson::Value readStorage() {
    auto value = Mod::get()->getSavedValue<matjson::Value>(STORAGE_KEY, matjson::Value::object());
    return value.type() == matjson::Type::Object ? value : matjson::Value::object();
}

void writeStorage(matjson::Value const& value) {
    Mod::get()->setSavedValue(STORAGE_KEY, value);
}

std::optional<Entry> decode(std::string id, matjson::Value const& encoded) {
    if (encoded.type() != matjson::Type::Object) return std::nullopt;
    auto label = encoded["label"].asString();
    auto const& membersValue = encoded["members"];
    if (label.isErr() || membersValue.type() != matjson::Type::Object) return std::nullopt;

    Entry result {std::move(id), label.unwrap(), {}};
    for (auto const& item : membersValue) {
        auto path = item.getKey();
        if (!path || item.type() != matjson::Type::Object) continue;
        auto x = item["x"].asDouble();
        auto y = item["y"].asDouble();
        auto scaleX = item["scale-x"].asDouble();
        auto scaleY = item["scale-y"].asDouble();
        if (x.isErr() || y.isErr() || scaleX.isErr() || scaleY.isErr()) continue;
        result.members.push_back({
            *path,
            {static_cast<float>(x.unwrap()), static_cast<float>(y.unwrap())},
            static_cast<float>(scaleX.unwrap()),
            static_cast<float>(scaleY.unwrap()),
        });
    }
    if (result.members.empty()) return std::nullopt;
    return result;
}
}

std::vector<Entry> entries() {
    std::vector<Entry> result;
    for (auto const& item : readStorage()) {
        auto id = item.getKey();
        if (!id) continue;
        if (auto entry = decode(*id, item)) result.push_back(std::move(*entry));
    }
    std::ranges::sort(result, [](Entry const& a, Entry const& b) {
        return geode::utils::string::toLower(a.label) < geode::utils::string::toLower(b.label);
    });
    return result;
}

std::optional<Entry> find(std::string const& id) {
    auto storage = readStorage();
    if (!storage.contains(id)) return std::nullopt;
    return decode(id, storage[id]);
}

std::vector<std::string> memberPaths() {
    std::vector<std::string> result;
    for (auto const& entry : entries()) {
        for (auto const& member : entry.members) result.push_back(member.path);
    }
    return result;
}

std::string uniqueID(std::string const& preferred) {
    auto base = preferred.empty() ? std::string("hidden-block") : preferred;
    auto storage = readStorage();
    if (!storage.contains(base)) return base;
    for (size_t suffix = 2;; ++suffix) {
        auto candidate = base + "#" + std::to_string(suffix);
        if (!storage.contains(candidate)) return candidate;
    }
}

bool upsert(Entry const& entry) {
    if (entry.id.empty() || entry.members.empty()) return false;
    auto storage = readStorage();
    auto encoded = matjson::Value::object();
    encoded.set("label", entry.label);
    auto members = matjson::Value::object();
    for (auto const& member : entry.members) {
        auto transform = matjson::Value::object();
        transform.set("x", member.position.x);
        transform.set("y", member.position.y);
        transform.set("scale-x", member.scaleX);
        transform.set("scale-y", member.scaleY);
        members.set(member.path, std::move(transform));
    }
    encoded.set("members", std::move(members));
    storage.set(entry.id, std::move(encoded));
    writeStorage(storage);
    return find(entry.id).has_value();
}

void erase(std::string const& id) {
    auto storage = readStorage();
    storage.erase(id);
    writeStorage(storage);
}

void clear() {
    writeStorage(matjson::Value::object());
}
}
