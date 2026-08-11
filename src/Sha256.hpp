#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace pause_menu_studio::crypto {

std::string sha256Hex(std::span<std::uint8_t const> data);

}
