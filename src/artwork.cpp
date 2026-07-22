/**
 * @file    src/artwork.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "artwork.h"

namespace vdflib {

std::string artworkFileName(uint32_t shortcutAppId, ArtworkSlot slot, const std::string& extension) {
    std::string base = std::to_string(shortcutAppId);
    switch (slot) {
        case ArtworkSlot::Portrait:
            return base + "p" + extension;
        case ArtworkSlot::Hero:
            return base + "_hero" + extension;
        case ArtworkSlot::Logo:
            return base + "_logo" + extension;
        case ArtworkSlot::Capsule:
            return base + extension;
    }
    return base + extension;
}

bool installLocalArtwork(const std::filesystem::path& gridDirectory, uint32_t shortcutAppId, ArtworkSlot slot,
                          const std::filesystem::path& sourceImage) {
    if (!std::filesystem::exists(sourceImage)) {
        return false;
    }

    std::filesystem::create_directories(gridDirectory);

    std::string fileName = artworkFileName(shortcutAppId, slot, sourceImage.extension().string());
    std::filesystem::copy_file(sourceImage, gridDirectory / fileName,
                                std::filesystem::copy_options::overwrite_existing);
    return true;
}

}
