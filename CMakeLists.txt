
cmake_minimum_required(VERSION 3.16)
project(VDFLib VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VDFLIB_BUILD_TESTS "Build VDFLib's test suite" ${PROJECT_IS_TOP_LEVEL})
option(VDFLIB_BUILD_EXAMPLES "Build VDFLib's example programs" ${PROJECT_IS_TOP_LEVEL})
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

if(VDFLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(VDFLIB_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

include(GNUInstallDirs)
install(TARGETS vdflib
    EXPORT vdflibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
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
