#include "LayoutProfiles.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace pause_menu_studio::profiles {
namespace {
constexpr char const* STORAGE_KEY = "named-layouts-v1";

matjson::Value readStorage() {
    auto value = Mod::get()->getSavedValue<matjson::Value>(STORAGE_KEY, matjson::Value::object());
    return value.type() == matjson::Type::Object ? value : matjson::Value::object();
}

void writeStorage(matjson::Value const& value) {
    Mod::get()->setSavedValue(STORAGE_KEY, value);
}
}

std::vector<std::string> names() {
    std::vector<std::string> result;
    for (auto const& entry : readStorage()) {
        if (auto key = entry.getKey()) result.push_back(*key);
    }
    std::ranges::sort(result, [](auto const& a, auto const& b) {
        return geode::utils::string::toLower(a) < geode::utils::string::toLower(b);
    });
    return result;
}

bool exists(std::string const& name) {
    return readStorage().contains(name);
}

std::optional<Snapshot> load(std::string const& name) {
    auto storage = readStorage();
    auto const& encoded = storage[name];
    if (encoded.type() != matjson::Type::Object) return std::nullopt;

    Snapshot snapshot;
    for (auto const& entry : encoded) {
        auto key = entry.getKey();
        if (!key || entry.type() != matjson::Type::Object) continue;
        auto x = entry["x"].asDouble();
        auto y = entry["y"].asDouble();
        if (x.isErr() || y.isErr()) continue;
        Transform transform;
        transform.position = CCPoint {
            static_cast<float>(x.unwrap()),
            static_cast<float>(y.unwrap()),
        };
        if (auto scaleX = entry["scale-x"].asDouble(); scaleX.isOk()) {
            transform.scaleX = static_cast<float>(scaleX.unwrap());
        }
        if (auto scaleY = entry["scale-y"].asDouble(); scaleY.isOk()) {
            transform.scaleY = static_cast<float>(scaleY.unwrap());
        }
        if (auto hidden = entry["hidden"].asBool(); hidden.isOk()) {
            transform.hidden = hidden.unwrap();
        }
        if (auto hiddenID = entry["hidden-id"].asString(); hiddenID.isOk()) {
            transform.hiddenID = hiddenID.unwrap();
        }
        if (auto hiddenLabel = entry["hidden-label"].asString(); hiddenLabel.isOk()) {
            transform.hiddenLabel = hiddenLabel.unwrap();
        }
        snapshot[*key] = transform;
    }
    return snapshot;
}

void save(std::string const& name, Snapshot const& snapshot) {
    auto storage = readStorage();
    auto encoded = matjson::Value::object();
    for (auto const& [path, transform] : snapshot) {
        auto point = matjson::Value::object();
        point.set("x", transform.position.x);
        point.set("y", transform.position.y);
        if (transform.scaleX) point.set("scale-x", *transform.scaleX);
        if (transform.scaleY) point.set("scale-y", *transform.scaleY);
        if (transform.hidden) point.set("hidden", *transform.hidden);
        if (transform.hiddenID) point.set("hidden-id", *transform.hiddenID);
        if (transform.hiddenLabel) point.set("hidden-label", *transform.hiddenLabel);
        encoded.set(path, std::move(point));
    }
    storage.set(name, std::move(encoded));
    writeStorage(storage);
}

void erase(std::string const& name) {
    auto storage = readStorage();
    storage.erase(name);
    writeStorage(storage);
}
}
