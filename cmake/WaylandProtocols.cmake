# SPDX-License-Identifier: GPL-3.0-or-later
# Helper wrapper around qt6_generate_wayland_protocol_server_sources (CHECKLIST M0.4)

function(tape_add_wayland_protocol target_name protocol_xml)
    qt6_generate_wayland_protocol_server_sources(${target_name}
        FILES "${protocol_xml}"
    )
endfunction()
