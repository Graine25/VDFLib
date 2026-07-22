/**
 * @file    src/steam_paths.h
 * @brief   Declares local Steam installation discovery. Builds paths for user shortcut and artwork data.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace vdflib {

std::optional<std::filesystem::path> findSteamInstallPath();

std::vector<std::string> listLocalSteamUserIds(const std::filesystem::path& steamPath);

std::filesystem::path getShortcutsVdfPath(const std::filesystem::path& steamPath, const std::string& userId);

std::filesystem::path getGridDirectory(const std::filesystem::path& steamPath, const std::string& userId);

}
