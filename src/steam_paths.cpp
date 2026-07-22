/**
 * @file    src/steam_paths.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "steam_paths.h"

#include <cstdlib>

#if defined(_WIN32)
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace vdflib {

namespace {

std::optional<std::filesystem::path> envPath(const char* name) {
    if (const char* value = std::getenv(name); value && *value) {
        return std::filesystem::path(value);
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> homeDirectory() {
    if (auto home = envPath("HOME")) {
        return home;
    }
#if defined(_WIN32)
    if (auto profile = envPath("USERPROFILE")) {
        return profile;
    }
#else
    if (const passwd* entry = getpwuid(getuid())) {
        return std::filesystem::path(entry->pw_dir);
    }
#endif
    return std::nullopt;
}

#if defined(_WIN32)
std::optional<std::filesystem::path> steamPathFromRegistry() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return std::nullopt;
    }

    char buffer[MAX_PATH]{};
    DWORD size = sizeof(buffer);
    DWORD type = 0;
    LONG result = RegQueryValueExA(key, "SteamPath", nullptr, &type, reinterpret_cast<LPBYTE>(buffer), &size);
    RegCloseKey(key);

    if (result != ERROR_SUCCESS || type != REG_SZ) {
        return std::nullopt;
    }
    return std::filesystem::path(buffer);
}
#endif

}

std::optional<std::filesystem::path> findSteamInstallPath() {
    for (const char* var : {"STEAM_PATH", "STEAM_ROOT"}) {
        if (auto path = envPath(var); path && std::filesystem::exists(*path)) {
            return path;
        }
    }

#if defined(_WIN32)
    if (auto path = steamPathFromRegistry(); path && std::filesystem::exists(*path)) {
        return path;
    }
#else
    if (auto home = homeDirectory()) {
        const std::filesystem::path candidates[] = {
            *home / ".local/share/Steam",
            *home / ".steam/steam",
            *home / ".var/app/com.valvesoftware.Steam/.local/share/Steam",
#if defined(__APPLE__)
            *home / "Library/Application Support/Steam",
#endif
        };
        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }
    }
#endif

    return std::nullopt;
}

std::vector<std::string> listLocalSteamUserIds(const std::filesystem::path& steamPath) {
    std::vector<std::string> userIds;
    std::filesystem::path userdata = steamPath / "userdata";
    if (!std::filesystem::exists(userdata)) {
        return userIds;
    }

    for (const auto& entry : std::filesystem::directory_iterator(userdata)) {
        if (entry.is_directory()) {
            userIds.push_back(entry.path().filename().string());
        }
    }
    return userIds;
}

std::filesystem::path getShortcutsVdfPath(const std::filesystem::path& steamPath, const std::string& userId) {
    return steamPath / "userdata" / userId / "config" / "shortcuts.vdf";
}

std::filesystem::path getGridDirectory(const std::filesystem::path& steamPath, const std::string& userId) {
    return steamPath / "userdata" / userId / "config" / "grid";
}

}
