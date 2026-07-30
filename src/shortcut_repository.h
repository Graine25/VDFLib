/**
 * @file    src/shortcut_repository.h
 * @brief   Declares shortcuts.vdf repository operations. Provides in-memory lookup and persistence methods.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <filesystem>
#include <vector>

#include "shortcut.h"

namespace vdflib {

class ShortcutRepository {
public:
    explicit ShortcutRepository(std::filesystem::path shortcutsVdfPath);

    void load();

    void save(bool createBackup = true) const;

    const std::vector<Shortcut>& shortcuts() const { return shortcuts_; }

    void addShortcut(Shortcut shortcut);
    bool removeShortcutByAppId(uint32_t appid);

    Shortcut* findByAppId(uint32_t appid);
    const Shortcut* findByAppId(uint32_t appid) const;
    Shortcut* findByName(const std::string& appName);

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
    std::vector<Shortcut> shortcuts_;
};

}
