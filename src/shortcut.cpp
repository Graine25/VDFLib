/**
 * @file    src/shortcut.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "shortcut.h"

#include "shortcut_id.h"

namespace vdflib {

namespace {

std::string quotedPath(const std::string& path) {
    if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
        return path;
    }
    return "\"" + path + "\"";
}

}

Shortcut Shortcut::create(const std::string& name, const std::string& exePath,
                           const std::string& startDirPath) {
    Shortcut shortcut;
    shortcut.appName = name;
    shortcut.exe = quotedPath(exePath);
    shortcut.startDir = quotedPath(startDirPath);
    shortcut.appid = generateShortcutAppId(name, shortcut.exe);
    return shortcut;
}

VdfValue Shortcut::toVdf() const {
    VdfObject obj;
    obj["appid"] = VdfValue(appid);
    obj["AppName"] = VdfValue(appName);
    obj["Exe"] = VdfValue(exe);
    obj["StartDir"] = VdfValue(startDir);
    obj["icon"] = VdfValue(icon);
    obj["ShortcutPath"] = VdfValue(shortcutPath);
    obj["LaunchOptions"] = VdfValue(launchOptions);
    obj["IsHidden"] = VdfValue(isHidden);
    obj["AllowDesktopConfig"] = VdfValue(allowDesktopConfig);
    obj["AllowOverlay"] = VdfValue(allowOverlay);
    obj["OpenVR"] = VdfValue(openVR);
    obj["Devkit"] = VdfValue(devkit);
    obj["DevkitGameID"] = VdfValue(devkitGameID);
    obj["DevkitOverrideAppID"] = VdfValue(devkitOverrideAppID);
    obj["LastPlayTime"] = VdfValue(lastPlayTime);
    obj["FlatpakAppID"] = VdfValue(flatpakAppID);
    obj["tags"] = VdfValue(tags);
    return VdfValue(obj);
}

Shortcut Shortcut::fromVdf(const VdfValue& value) {
    Shortcut shortcut;
    shortcut.appid = value.getUInt32("appid", 0);
    shortcut.appName = value.getString("AppName", "");
    shortcut.exe = value.getString("Exe", "");
    shortcut.startDir = value.getString("StartDir", "");
    shortcut.icon = value.getString("icon", "");
    shortcut.shortcutPath = value.getString("ShortcutPath", "");
    shortcut.launchOptions = value.getString("LaunchOptions", "");
    shortcut.isHidden = value.getUInt32("IsHidden", 0);
    shortcut.allowDesktopConfig = value.getUInt32("AllowDesktopConfig", 1);
    shortcut.allowOverlay = value.getUInt32("AllowOverlay", 1);
    shortcut.openVR = value.getUInt32("OpenVR", 0);
    shortcut.devkit = value.getUInt32("Devkit", 0);
    shortcut.devkitGameID = value.getString("DevkitGameID", "");
    shortcut.devkitOverrideAppID = value.getUInt32("DevkitOverrideAppID", 0);
    shortcut.lastPlayTime = value.getUInt32("LastPlayTime", 0);
    shortcut.flatpakAppID = value.getString("FlatpakAppID", "");

    if (value.isObject()) {
        if (const VdfValue* tagsValue = value.asObject().find("tags")) {
            if (tagsValue->isObject()) {
                shortcut.tags = tagsValue->asObject();
            }
        }
    }

    return shortcut;
}

}
