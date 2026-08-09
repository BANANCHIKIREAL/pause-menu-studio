#include "LayoutProfiles.hpp"

#include <algorithm>
#include <chrono>

using namespace geode::prelude;

namespace pause_menu_studio::profiles {
namespace {
constexpr char const* STORAGE_KEY = "named-layouts-v1";
constexpr char const* METADATA_KEY = "named-layout-metadata-v1";

matjson::Value readStorage() {
    auto value = Mod::get()->getSavedValue<matjson::Value>(STORAGE_KEY, matjson::Value::object());
    return value.type() == matjson::Type::Object ? value : matjson::Value::object();
}

void writeStorage(matjson::Value const& value) {
    Mod::get()->setSavedValue(STORAGE_KEY, value);
}

matjson::Value readMetadata() {
    auto value = Mod::get()->getSavedValue<matjson::Value>(METADATA_KEY, matjson::Value::object());
    return value.type() == matjson::Type::Object ? value : matjson::Value::object();
}

void writeMetadata(matjson::Value const& value) {
    Mod::get()->setSavedValue(METADATA_KEY, value);
}

int64_t nowUnix() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
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
        if (auto hiddenIcon = entry["hidden-icon"].asString(); hiddenIcon.isOk()) {
            transform.hiddenIcon = hiddenIcon.unwrap();
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
        if (transform.hiddenIcon) point.set("hidden-icon", *transform.hiddenIcon);
        encoded.set(path, std::move(point));
    }
    storage.set(name, std::move(encoded));
    writeStorage(storage);

    auto metadata = readMetadata();
    auto item = matjson::Value::object();
    item.set("saved-at", nowUnix());
    metadata.set(name, std::move(item));
    writeMetadata(metadata);
}

int64_t savedAt(std::string const& name) {
    auto metadata = readMetadata();
    auto value = metadata[name]["saved-at"].asInt();
    return value.isOk() ? value.unwrap() : 0;
}

bool rename(std::string const& oldName, std::string const& newName) {
    if (oldName.empty() || newName.empty() || oldName == newName || exists(newName)) return false;
    auto storage = readStorage();
    if (!storage.contains(oldName)) return false;
    auto snapshot = storage[oldName];
    storage.set(newName, std::move(snapshot));
    storage.erase(oldName);
    writeStorage(storage);

    auto metadata = readMetadata();
    if (metadata.contains(oldName)) {
        auto item = metadata[oldName];
        metadata.set(newName, std::move(item));
        metadata.erase(oldName);
        writeMetadata(metadata);
    }
    return true;
}

bool duplicate(std::string const& sourceName, std::string const& newName) {
    if (sourceName.empty() || newName.empty() || exists(newName)) return false;
    auto snapshot = load(sourceName);
    if (!snapshot) return false;
    save(newName, *snapshot);
    return true;
}

void erase(std::string const& name) {
    auto storage = readStorage();
    storage.erase(name);
    writeStorage(storage);
    auto metadata = readMetadata();
    metadata.erase(name);
    writeMetadata(metadata);
}
}
