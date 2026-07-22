/**
 * @file    src/vdf_io.h
 * @brief   Declares binary VDF parsing and writing. Reads and writes Steam's shortcuts.vdf format.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <filesystem>
#include <stdexcept>

#include "vdf_value.h"

namespace vdflib {

class VdfIoError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

VdfValue parseVdf(const std::filesystem::path& filePath);

void writeVdf(const VdfValue& value, const std::filesystem::path& filePath);

}
