# VDFLib

C++ library for making applications register themselves as non-Steam shortcuts,
without configuring them through Steam's UI.

It grew out of [Steam-Shelf](https://github.com/The-Sofishticated-Man/steam-shelf), a Python tool that batch-imports a folder of games into Steam. VDFLib covers the same underlying shortcut/VDF mechanics as a reusable, embeddable library rather than a standalone tool. No real reverse  engineering work was done for this project, as Steam-Shelf already had most of the information and system laid out.

This project is mainly meant for the usage of static recompilations, namely the ReXGlue Team's [RE:Blue](https://github.com/zolaware/reblue) and [EdgeOfTimeRecompiled](https://github.com/goliathret/reeot). Steam-Shelf still has significantly more features when it comes to grabbing images from the cloud and accurately parsing the VDF, but the project's current state is good enough for the usecases we have. I have no plans on updating this repository too much, as it works for what we need. More then welcome to accept PR's and discuss about certain issues

## Features

- **Binary VDF parsing and writing**: format Steam uses for `shortcuts.vdf` and other files like `appinfo.vdf`. Exposed as a generic `VdfValue`/`VdfObject` tree, not hard-coded to the shortcuts schema, so it's usable for other binary VDF files too.
- **Steam's real shortcut-appid algorithm**: CRC-32 of `exe + name` with the high bit set, verified byte-for-byte against Python's `zlib.crc32` for the same inputs, so IDs generated here match what Steam itself computes.
- **`Shortcut` struct**: typed struct covering every field Steam stores per non-Steam shortcut, with conversion to/from the raw VDF tree.
- **ShortcutRepository**: load, add, remove, and save a user's `shortcuts.vdf` without hand-building VDF trees.
- **Steam & user discovery**: finds the local Steam install (env var, Windows registry, or the usual Linux/macOS/Flatpak install locations) and lists local Steam user IDs.
- **Local artwork installation**: copies portrait/hero/ logo/capsule art you already have on disk into Steam's grid folder with the filename Steam expects. No network calls and no CDN are implemented atm — artwork is BYO but it is possible to integrate steamgriddb, given Steam-Shelf already contains it

## Usage

Install VDFLib, then link it from your CMake project:

```cmake
find_package(vdflib CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE vdflib::vdflib)
```

If VDFLib is included directly in your source tree, add it before linking:

```cmake
add_subdirectory(<path/to/VDFLib>)
target_link_libraries(my_app PRIVATE vdflib::vdflib)
```

Close Steam before editing `shortcuts.vdf`. The following adds a shortcut only
when it is not already present:

```cpp
#include <vdflib/vdflib.h>

const auto steamPath = vdflib::findSteamInstallPath();
if (!steamPath) {
    return 1;
}

const auto users = vdflib::listLocalSteamUserIds(*steamPath);
if (users.empty()) {
    return 1;
}

auto shortcut = vdflib::Shortcut::create("My App", "/path/to/my-app", "/path/to");
vdflib::ShortcutRepository shortcuts(vdflib::getShortcutsVdfPath(*steamPath, users.front()));
shortcuts.load();

if (!shortcuts.findByAppId(shortcut.appid)) {
    shortcuts.addShortcut(shortcut);
    shortcuts.save();
}
```

Restart Steam after saving. Use `installLocalArtwork` only if you want custom
library artwork; it is not needed for a functioning shortcut.

When VDFLib is used through `add_subdirectory`, include headers directly from
the source directory (for example, `#include "vdflib.h"`).

## Credits
- [Steam-Shelf](https://github.com/The-Sofishticated-Man/steam-shelf): Biggest knowledgebase on how the shortcut system works
