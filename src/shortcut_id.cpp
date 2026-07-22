/**
 * @file    src/shortcut_id.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "shortcut_id.h"

#include <array>
#include <cstdint>

namespace vdflib {

namespace {

constexpr std::array<uint32_t, 256> makeCrc32Table() {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int bit = 0; bit < 8; ++bit) {
            c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}

constexpr std::array<uint32_t, 256> kCrc32Table = makeCrc32Table();

uint32_t crc32(const std::string& data) {
    uint32_t crc = 0xFFFFFFFFu;
    for (unsigned char byte : data) {
        crc = kCrc32Table[(crc ^ byte) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

}

uint32_t generateShortcutAppId(const std::string& name, const std::string& exe) {
    uint32_t crc = crc32(exe + name);
    return crc | 0x80000000u;
}

}
