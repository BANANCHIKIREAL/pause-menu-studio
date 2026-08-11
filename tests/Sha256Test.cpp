#include "../src/Sha256.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string_view>

int main() {
    auto digest = [](std::string_view text) {
        auto bytes = std::span {
            reinterpret_cast<std::uint8_t const*>(text.data()), text.size()
        };
        return pause_menu_studio::crypto::sha256Hex(bytes);
    };

    assert(digest("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    assert(digest("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    assert(digest("Pause Menu Studio") == "4e2d7ca3acfadea4d7e643f5b1ac1e1ff19469ead0593eb07810263676b43991");
}
