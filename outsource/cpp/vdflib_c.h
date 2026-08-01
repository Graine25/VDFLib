#ifndef VDFLIB_C_H
#define VDFLIB_C_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(VDFLIB_C_SHARED)
#  if defined(vdflib_c_EXPORTS)
#    define VDFLIB_C_API __declspec(dllexport)
#  else
#    define VDFLIB_C_API __declspec(dllimport)
#  endif
#else
#  define VDFLIB_C_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdflib_repository vdflib_repository;

typedef enum vdflib_status {
    VDFLIB_OK = 0,
    VDFLIB_INVALID_ARGUMENT = 1,
    VDFLIB_NOT_FOUND = 2,
    VDFLIB_IO_ERROR = 3,
    VDFLIB_ERROR = 4
} vdflib_status;

typedef struct vdflib_shortcut_options {
    const char* name;
    const char* exe;
    const char* start_dir;
    const char* icon;
    const char* launch_options;
    const char* flatpak_app_id;
    uint32_t allow_overlay;
} vdflib_shortcut_options;

VDFLIB_C_API const char* vdflib_last_error(void);
VDFLIB_C_API uint32_t vdflib_generate_shortcut_app_id(const char* name,
                                                       const char* exe);
VDFLIB_C_API vdflib_repository* vdflib_repository_create(const char* path);
VDFLIB_C_API void vdflib_repository_destroy(vdflib_repository* repository);
VDFLIB_C_API vdflib_status vdflib_repository_load(vdflib_repository* repository);
VDFLIB_C_API vdflib_status vdflib_repository_save(vdflib_repository* repository,
                                                   int create_backup);
VDFLIB_C_API size_t vdflib_repository_count(const vdflib_repository* repository);
VDFLIB_C_API uint32_t vdflib_repository_app_id(
    const vdflib_repository* repository, size_t index);
VDFLIB_C_API const char* vdflib_repository_name(
    const vdflib_repository* repository, size_t index);
VDFLIB_C_API vdflib_status vdflib_repository_add(
    vdflib_repository* repository, const vdflib_shortcut_options* options,
    uint32_t* app_id);
VDFLIB_C_API vdflib_status vdflib_repository_remove(
    vdflib_repository* repository, uint32_t app_id);

#ifdef __cplusplus
}
#endif

#endif
