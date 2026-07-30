
cmake_minimum_required(VERSION 3.16)
project(VDFLib VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build VDFLib as a shared library" OFF)

add_library(vdflib
    src/vdf_value.cpp
    src/vdf_io.cpp
    src/shortcut_id.cpp
    src/shortcut.cpp
    src/shortcut_repository.cpp
    src/steam_paths.cpp
    src/artwork.cpp
)
add_library(vdflib::vdflib ALIAS vdflib)

add_library(vdflib_c outsource/cpp/vdflib_c.cpp)
add_library(vdflib::c ALIAS vdflib_c)
target_link_libraries(vdflib_c PRIVATE vdflib)
set_target_properties(vdflib vdflib_c PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(vdflib_c
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/outsource/cpp>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(vdflib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32)
    target_link_libraries(vdflib PRIVATE advapi32)
endif()

if(MSVC)
    target_compile_options(vdflib PRIVATE /W4)
else()
    target_compile_options(vdflib PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS vdflib vdflib_c
    EXPORT vdflibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES outsource/cpp/vdflib_c.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES
    src/artwork.h
    src/shortcut.h
    src/shortcut_id.h
    src/shortcut_repository.h
    src/steam_paths.h
    src/vdf_io.h
    src/vdf_value.h
    src/vdflib.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vdflib
)
install(EXPORT vdflibTargets
    FILE vdflibTargets.cmake
    NAMESPACE vdflib::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vdflib
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vdflibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/vdflibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vdflib
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/vdflibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vdflibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/vdflibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vdflib
)
