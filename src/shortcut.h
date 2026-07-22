/**
 * @file    src/shortcut.h
 * @brief   Declares the Steam shortcut model. Represents fields stored in shortcuts.vdf.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <cstdint>
#include <string>

#include "vdf_value.h"

namespace vdflib {

struct Shortcut {
    uint32_t appid = 0;
    std::string appName;
    std::string exe;
    std::string startDir;
    std::string icon;
    std::string shortcutPath;
    std::string launchOptions;
    uint32_t isHidden = 0;
    uint32_t allowDesktopConfig = 1;
    uint32_t allowOverlay = 1;
    uint32_t openVR = 0;
    uint32_t devkit = 0;
    std::string devkitGameID;
    uint32_t devkitOverrideAppID = 0;
    uint32_t lastPlayTime = 0;
    std::string flatpakAppID;
    VdfObject tags;

    static Shortcut create(const std::string& name, const std::string& exePath,
                            const std::string& startDirPath);

    VdfValue toVdf() const;
    static Shortcut fromVdf(const VdfValue& value);
};

}
