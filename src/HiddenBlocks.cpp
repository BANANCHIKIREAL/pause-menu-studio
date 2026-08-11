#include "HiddenBlocks.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio::hidden_blocks {
namespace {
std::string storageKey(std::string const& mode) {
    // Keep normal mode on the original key so existing Trash data migrates
    // without any rewrite. Every menu variant with additional controls gets
    // an isolated recycle bin.
    if (mode == "normal") return "hidden-blocks-v1";
    return "hidden-blocks-v1-" + mode;
}

matjson::Value readStorage(std::string const& mode) {
    auto value = Mod::get()->getSavedValue<matjson::Value>(storageKey(mode), matjson::Value::object());
    return value.type() == matjson::Type::Object ? value : matjson::Value::object();
}

void writeStorage(matjson::Value const& value, std::string const& mode) {
    Mod::get()->setSavedValue(storageKey(mode), value);
}

std::optional<Entry> decode(std::string id, matjson::Value const& encoded) {
    if (encoded.type() != matjson::Type::Object) return std::nullopt;
    auto label = encoded["label"].asString();
    auto const& membersValue = encoded["members"];
    if (label.isErr() || membersValue.type() != matjson::Type::Object) return std::nullopt;

    Entry result {std::move(id), label.unwrap(), {}};
    if (auto icon = encoded["icon-frame"].asString(); icon.isOk()) {
        result.iconFrame = icon.unwrap();
    }
    for (auto const& item : membersValue) {
        auto path = item.getKey();
        if (!path || item.type() != matjson::Type::Object) continue;
        auto x = item["x"].asDouble();
        auto y = item["y"].asDouble();
        auto scaleX = item["scale-x"].asDouble();
        auto scaleY = item["scale-y"].asDouble();
        if (x.isErr() || y.isErr() || scaleX.isErr() || scaleY.isErr()) continue;
        Member member {
            *path,
            {static_cast<float>(x.unwrap()), static_cast<float>(y.unwrap())},
            static_cast<float>(scaleX.unwrap()),
            static_cast<float>(scaleY.unwrap()),
        };
        if (auto rotation = item["rotation"].asDouble(); rotation.isOk()) {
            member.rotation = static_cast<float>(rotation.unwrap());
        }
        if (auto opacity = item["opacity"].asInt(); opacity.isOk()) {
            member.opacity = static_cast<int>(opacity.unwrap());
        }
        if (auto zOrder = item["z-order"].asInt(); zOrder.isOk()) {
            member.zOrder = static_cast<int>(zOrder.unwrap());
        }
        result.members.push_back(std::move(member));
    }
    if (result.members.empty()) return std::nullopt;
    return result;
}
}

std::vector<Entry> entries(std::string const& mode) {
    std::vector<Entry> result;
    for (auto const& item : readStorage(mode)) {
        auto id = item.getKey();
        if (!id) continue;
        if (auto entry = decode(*id, item)) result.push_back(std::move(*entry));
    }
    std::ranges::sort(result, [](Entry const& a, Entry const& b) {
        return geode::utils::string::toLower(a.label) < geode::utils::string::toLower(b.label);
    });
    return result;
}

std::optional<Entry> find(std::string const& id, std::string const& mode) {
    auto storage = readStorage(mode);
    if (!storage.contains(id)) return std::nullopt;
    return decode(id, storage[id]);
}

std::vector<std::string> memberPaths(std::string const& mode) {
    std::vector<std::string> result;
    for (auto const& entry : entries(mode)) {
        for (auto const& member : entry.members) result.push_back(member.path);
    }
    return result;
}

std::string uniqueID(std::string const& preferred, std::string const& mode) {
    auto base = preferred.empty() ? std::string("hidden-block") : preferred;
    auto storage = readStorage(mode);
    if (!storage.contains(base)) return base;
    for (size_t suffix = 2;; ++suffix) {
        auto candidate = base + "#" + std::to_string(suffix);
        if (!storage.contains(candidate)) return candidate;
    }
}

bool upsert(Entry const& entry, std::string const& mode) {
    if (entry.id.empty() || entry.members.empty()) return false;
    auto storage = readStorage(mode);
    auto encoded = matjson::Value::object();
    encoded.set("label", entry.label);
    if (!entry.iconFrame.empty()) encoded.set("icon-frame", entry.iconFrame);
    auto members = matjson::Value::object();
    for (auto const& member : entry.members) {
        auto transform = matjson::Value::object();
        transform.set("x", member.position.x);
        transform.set("y", member.position.y);
        transform.set("scale-x", member.scaleX);
        transform.set("scale-y", member.scaleY);
        if (member.rotation) transform.set("rotation", *member.rotation);
        if (member.opacity) transform.set("opacity", *member.opacity);
        if (member.zOrder) transform.set("z-order", *member.zOrder);
        members.set(member.path, std::move(transform));
    }
    encoded.set("members", std::move(members));
    storage.set(entry.id, std::move(encoded));
    writeStorage(storage, mode);
    return find(entry.id, mode).has_value();
}

void erase(std::string const& id, std::string const& mode) {
    auto storage = readStorage(mode);
    storage.erase(id);
    writeStorage(storage, mode);
}

void clear(std::string const& mode) {
    writeStorage(matjson::Value::object(), mode);
}
}
