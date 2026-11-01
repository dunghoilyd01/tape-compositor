# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt6 REQUIRED COMPONENTS Test)

# 1. Unit tests for CommandLine, Logging, Config, and WindowModel
qt_add_executable(test_config_and_logging
    test_config_and_logging.cpp
)
target_link_libraries(test_config_and_logging PRIVATE
    tape_lib
    Qt6::Test
)
tape_enable_warnings(test_config_and_logging)
tape_enable_sanitizers(test_config_and_logging)
add_test(NAME test_config_and_logging COMMAND test_config_and_logging)

# 2. Headless Wayland compositor + libwayland-client protocol verification test
qt_add_executable(test_compositor_headless
    test_compositor_headless.cpp
)
target_link_libraries(test_compositor_headless PRIVATE
    tape_lib
    Qt6::Test
    PkgConfig::WAYLAND_CLIENT
)
tape_enable_warnings(test_compositor_headless)
tape_enable_sanitizers(test_compositor_headless)
add_test(NAME test_compositor_headless COMMAND test_compositor_headless)
