# SPDX-License-Identifier: GPL-3.0-or-later

function(tape_enable_sanitizers target_name)
    if(TAPE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PUBLIC
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${target_name} PUBLIC
            -fsanitize=address,undefined
        )
    endif()
endfunction()
