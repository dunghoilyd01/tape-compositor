# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.27)

project(tape-compositor
    VERSION 0.1.0
    DESCRIPTION "Tape Compositor — Lightweight KDE-themed Stacking Wayland Compositor"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Feature options (PLAN §22.9 & CHECKLIST M0.1)
option(TAPE_XWAYLAND "Enable XWayland compatibility layer" ON)
option(TAPE_KDECORATION "Enable KDecoration3 native decoration bridge" OFF)
option(TAPE_PIPEWIRE "Enable PipeWire screen capture and portal backend" ON)
option(TAPE_SYSTEMD "Enable optional systemd-logind user integration (default: elogind)" OFF)
option(TAPE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(TAPE_BUILD_TESTS "Build automated unit and headless protocol tests" ON)

include(CompilerWarnings)
include(Sanitizers)
include(WaylandProtocols)

# Target Qt version is 6.12 (REQUIREMENTS D10-D15); allow >= 6.11 on build hosts while 6.12 is rolling out
set(TAPE_TARGET_QT_VERSION "6.12")
find_package(Qt6 6.11 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    WaylandCompositor
    WaylandCompositorXdgShell
    DBus
    ShaderTools
    Concurrent
    Svg
)

if(Qt6_VERSION VERSION_LESS "${TAPE_TARGET_QT_VERSION}.0")
    message(STATUS "Tape targets Qt ${TAPE_TARGET_QT_VERSION}; building against installed Qt ${Qt6_VERSION}")
endif()

find_package(KF6Config REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_SERVER REQUIRED IMPORTED_TARGET wayland-server>=1.22)
pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client>=1.22)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon>=1.5)

qt_standard_project_setup(REQUIRES 6.8)

# Core static library shared between the compositor executable and test suite
add_library(tape_lib STATIC
    src/core/Logging.h
    src/core/Logging.cpp
    src/core/CommandLine.h
    src/core/CommandLine.cpp
    src/core/Config.h
    src/core/Config.cpp
    src/core/Compositor.h
    src/core/Compositor.cpp
    src/core/Application.h
    src/core/Application.cpp
    src/wm/Window.h
    src/wm/Window.cpp
    src/wm/WindowModel.h
    src/wm/WindowModel.cpp
)

target_include_directories(tape_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(tape_lib PUBLIC
    TAPE_VERSION_STRING="${PROJECT_VERSION}"
    TAPE_DEFAULT_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/taperc.default"
    QT_NO_CAST_FROM_ASCII
    QT_NO_FOREACH
)

target_link_libraries(tape_lib PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::WaylandCompositor
    Qt6::WaylandCompositorXdgShell
    Qt6::DBus
    Qt6::Concurrent
    Qt6::Svg
    KF6::ConfigCore
    PkgConfig::WAYLAND_SERVER
    PkgConfig::XKBCOMMON
)

tape_enable_warnings(tape_lib)
tape_enable_sanitizers(tape_lib)

# Embed QML shell scene into tape_lib
qt_add_qml_module(tape_lib
    URI Tape.Shell
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Tape/Shell"
    QML_FILES
        src/qml/OutputWindow.qml
        src/qml/ChromeWindow.qml
        src/qml/TopBar.qml
        src/qml/Dock.qml
)

# Main compositor executable
qt_add_executable(tape
    src/main.cpp
)

target_link_libraries(tape PRIVATE
    tape_lib
)

tape_enable_warnings(tape)
tape_enable_sanitizers(tape)

include(GNUInstallDirs)
install(TARGETS tape RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES data/taperc.default DESTINATION "${CMAKE_INSTALL_DATADIR}/tape")

if(TAPE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
