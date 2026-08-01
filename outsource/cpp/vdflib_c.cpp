#include "vdflib_c.h"

#include <exception>
#include <filesystem>
#include <new>
#include <string>

#include "shortcut_id.h"
#include "shortcut_repository.h"
#include "vdf_io.h"

struct vdflib_repository {
    explicit vdflib_repository(const char* path) : value(std::filesystem::u8path(path)) {}
    vdflib::ShortcutRepository value;
};

namespace {

thread_local std::string lastError;

vdflib_status fail(vdflib_status status, const char* message) {
    lastError = message;
    return status;
}

template <typename Function>
vdflib_status guarded(Function&& function) {
    try {
        function();
        lastError.clear();
        return VDFLIB_OK;
    } catch (const vdflib::VdfIoError& error) {
        return fail(VDFLIB_IO_ERROR, error.what());
    } catch (const std::exception& error) {
        return fail(VDFLIB_ERROR, error.what());
    } catch (...) {
        return fail(VDFLIB_ERROR, "Unknown VDFLib error");
    }
}

const vdflib::Shortcut* shortcutAt(const vdflib_repository* repository, size_t index) {
    if (!repository || index >= repository->value.shortcuts().size()) {
        return nullptr;
    }
    return &repository->value.shortcuts()[index];
}

}

extern "C" {

const char* vdflib_last_error(void) { return lastError.c_str(); }

uint32_t vdflib_generate_shortcut_app_id(const char* name, const char* exe) {
    if (!name || !exe) {
        fail(VDFLIB_INVALID_ARGUMENT, "name and exe must not be null");
        return 0;
    }
    lastError.clear();
    return vdflib::generateShortcutAppId(name, exe);
}

vdflib_repository* vdflib_repository_create(const char* path) {
    if (!path) {
        fail(VDFLIB_INVALID_ARGUMENT, "path must not be null");
        return nullptr;
    }
    try {
        auto* repository = new vdflib_repository(path);
        lastError.clear();
        return repository;
    } catch (const std::exception& error) {
        fail(VDFLIB_ERROR, error.what());
        return nullptr;
    }
}

void vdflib_repository_destroy(vdflib_repository* repository) { delete repository; }

vdflib_status vdflib_repository_load(vdflib_repository* repository) {
    if (!repository) return fail(VDFLIB_INVALID_ARGUMENT, "repository must not be null");
    return guarded([&] { repository->value.load(); });
}

vdflib_status vdflib_repository_save(vdflib_repository* repository, int createBackup) {
    if (!repository) return fail(VDFLIB_INVALID_ARGUMENT, "repository must not be null");
    return guarded([&] { repository->value.save(createBackup != 0); });
}

size_t vdflib_repository_count(const vdflib_repository* repository) {
    return repository ? repository->value.shortcuts().size() : 0;
}

uint32_t vdflib_repository_app_id(const vdflib_repository* repository, size_t index) {
    const auto* shortcut = shortcutAt(repository, index);
    return shortcut ? shortcut->appid : 0;
}

const char* vdflib_repository_name(const vdflib_repository* repository, size_t index) {
    const auto* shortcut = shortcutAt(repository, index);
    return shortcut ? shortcut->appName.c_str() : nullptr;
}

vdflib_status vdflib_repository_add(vdflib_repository* repository,
                                     const vdflib_shortcut_options* options,
                                     uint32_t* appId) {
    if (!repository || !options || !options->name || !options->exe ||
        !options->start_dir) {
        return fail(VDFLIB_INVALID_ARGUMENT,
                    "repository, name, exe, and start_dir are required");
    }
    return guarded([&] {
        auto shortcut =
            vdflib::Shortcut::create(options->name, options->exe, options->start_dir);
        if (options->icon) shortcut.icon = options->icon;
        if (options->launch_options) shortcut.launchOptions = options->launch_options;
        if (options->flatpak_app_id) shortcut.flatpakAppID = options->flatpak_app_id;
        shortcut.allowOverlay = options->allow_overlay;
        if (repository->value.findByAppId(shortcut.appid)) {
            throw std::invalid_argument("A shortcut with this app ID already exists");
        }
        if (appId) *appId = shortcut.appid;
        repository->value.addShortcut(std::move(shortcut));
    });
}

vdflib_status vdflib_repository_remove(vdflib_repository* repository,
                                        uint32_t appId) {
    if (!repository) return fail(VDFLIB_INVALID_ARGUMENT, "repository must not be null");
    if (!repository->value.removeShortcutByAppId(appId)) {
        return fail(VDFLIB_NOT_FOUND, "Shortcut app ID was not found");
    }
    lastError.clear();
    return VDFLIB_OK;
}

}
