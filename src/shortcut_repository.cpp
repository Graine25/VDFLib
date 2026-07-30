/**
 * @file    src/shortcut_repository.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "shortcut_repository.h"

#include <algorithm>

#include "vdf_io.h"

namespace vdflib {

ShortcutRepository::ShortcutRepository(std::filesystem::path shortcutsVdfPath)
    : path_(std::move(shortcutsVdfPath)) {}

void ShortcutRepository::load() {
    shortcuts_.clear();

    if (!std::filesystem::exists(path_)) {
        return;
    }

    VdfValue root = parseVdf(path_);
    if (!root.isObject()) {
        return;
    }

    const VdfValue* shortcutsSection = root.asObject().find("shortcuts");
    if (!shortcutsSection || !shortcutsSection->isObject()) {
        return;
    }

    for (const auto& [key, entry] : shortcutsSection->asObject()) {
        if (!entry.isObject()) {
            continue;
        }
        shortcuts_.push_back(Shortcut::fromVdf(entry));
    }
}

void ShortcutRepository::save(bool createBackup) const {
    VdfObject shortcutsSection;
    for (size_t i = 0; i < shortcuts_.size(); ++i) {
        shortcutsSection[std::to_string(i)] = shortcuts_[i].toVdf();
    }

    VdfObject root;
    root["shortcuts"] = VdfValue(shortcutsSection);

    writeVdf(VdfValue(root), path_, createBackup);
}

void ShortcutRepository::addShortcut(Shortcut shortcut) { shortcuts_.push_back(std::move(shortcut)); }

bool ShortcutRepository::removeShortcutByAppId(uint32_t appid) {
    auto it = std::find_if(shortcuts_.begin(), shortcuts_.end(),
                            [&](const Shortcut& shortcut) { return shortcut.appid == appid; });
    if (it == shortcuts_.end()) {
        return false;
    }
    shortcuts_.erase(it);
    return true;
}

Shortcut* ShortcutRepository::findByAppId(uint32_t appid) {
    auto it = std::find_if(shortcuts_.begin(), shortcuts_.end(),
                            [&](const Shortcut& shortcut) { return shortcut.appid == appid; });
    return it == shortcuts_.end() ? nullptr : &*it;
}

const Shortcut* ShortcutRepository::findByAppId(uint32_t appid) const {
    auto it = std::find_if(shortcuts_.begin(), shortcuts_.end(),
                            [&](const Shortcut& shortcut) { return shortcut.appid == appid; });
    return it == shortcuts_.end() ? nullptr : &*it;
}

Shortcut* ShortcutRepository::findByName(const std::string& appName) {
    auto it = std::find_if(shortcuts_.begin(), shortcuts_.end(),
                            [&](const Shortcut& shortcut) { return shortcut.appName == appName; });
    return it == shortcuts_.end() ? nullptr : &*it;
}

}
