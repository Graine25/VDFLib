/**
 * @file    src/shortcut_id.h
 * @brief   Declares non-Steam shortcut ID generation. Produces stable IDs from executable and name values.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <cstdint>
#include <string>

namespace vdflib {

uint32_t generateShortcutAppId(const std::string& name, const std::string& exe);

}
