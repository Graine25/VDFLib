/**
 * @file    src/artwork.h
 * @brief   Declares Steam shortcut artwork helpers. Defines the artwork slots and expected filenames.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace vdflib {

enum class ArtworkSlot {
    Portrait,
    Hero,
    Logo,
    Capsule,
};

bool installLocalArtwork(const std::filesystem::path& gridDirectory, uint32_t shortcutAppId, ArtworkSlot slot,
                          const std::filesystem::path& sourceImage);

std::string artworkFileName(uint32_t shortcutAppId, ArtworkSlot slot, const std::string& extension);

}
