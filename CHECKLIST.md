# Tape Compositor — Implementation Checklist

> Companion to [PLAN.md](PLAN.md) and [REQUIREMENTS.md](REQUIREMENTS.md). Tick items as they are completed (`[x]`), mark in-progress with `[~]`.
> IDs (e.g. `M2.4`) are stable — reference them in commits: `M2.4: atomic modeset test-only commit`.
> Before coding any item, check the corresponding requirement (`R*`/`I*`/`N*`) and dependencies (`D*`, **Qt 6.12**) in [REQUIREMENTS.md](REQUIREMENTS.md).
> Each milestone ends with a **Verify** block that must pass before moving on.

Legend: 🔴 critical path · 🟡 important · 🟢 nice-to-have

---

## M0 — Scaffolding, nested compositor, logging
- [x] M0.1 🔴 Root `CMakeLists.txt`, `cmake/` modules, feature options (`TAPE_XWAYLAND`, `TAPE_KDECORATION`, `TAPE_PIPEWIRE`, `TAPE_SYSTEMD=OFF`, `TAPE_ASAN`)
- [x] M0.2 🔴 Dependency detection: Qt 6.12 (exact minor check; Core/Gui/Quick/WaylandCompositor/DBus/ShaderTools/Concurrent/Svg + private), KF6, wayland-server, xkbcommon
- [x] M0.3 🔴 Directory layout per PLAN §5, `.clang-format`, `.clang-tidy`, `.gitignore`, SPDX headers, `README.md`
- [x] M0.4 🔴 `protocols/` vendored XMLs + CMake helper to generate server sources (`qt6_generate_wayland_protocol_server_sources`)
- [x] M0.5 🔴 `core/Application` (startup sequence, signal handling, clean shutdown)
- [x] M0.6 🔴 `core/CommandLine` (`--nested`, `--headless`, `--outputs`, `--output-size`, `--debug`, `--config`)
- [x] M0.7 🔴 `core/Logging`: `tape.*` categories, message handler, rotating file log `~/.local/state/tape/tape.log`, `tape.log.old`
- [x] M0.8 🔴 `LogManager::setRules()` runtime toggle (internal API used later by settings/D-Bus/shortcut)
- [x] M0.9 🔴 `core/Config` (KConfig `taperc`, defaults file `data/taperc.default`, `KConfigWatcher` live reload)
- [x] M0.10 🔴 `QWaylandCompositor` + `QWaylandXdgShell` + `QWaylandQuickOutput`; nested backend (one window per output)
- [x] M0.11 🔴 Basic QML scene: background color, client surfaces as `QWaylandQuickItem`s, click-to-focus
- [x] M0.12 🟡 `--headless` mode (render-control outputs only) for automated tests
- [x] M0.13 🟡 Unit test target + CI script (build, clang-format check, tests)
- **Verify:** `tape --nested` runs; weston-terminal / Konsole / gtk4-demo open, render, receive keyboard/pointer; `--debug` produces categorized log file.

## M1 — Window management core (stacking)
- [~] M1.1 🔴 `wm/Window` abstraction (`XdgToplevel` done; `XWaylandWindow`, `LayerSurfaceWindow` pending)
- [~] M1.2 🔴 `wm/WindowModel` (global logical coordinates done; multi-output primary view selection pending)
- [ ] M1.3 🔴 `wm/StackingOrder` (layers below/normal/above, raise on activate, transient-above-parent, modal handling)
- [ ] M1.4 🔴 `wm/FocusPolicy` (click-to-focus done; focus-follows-mouse option, keyboard focus history pending)
- [ ] M1.5 🔴 Interactive move/resize (titlebar drag move done; client-initiated move/resize, resize borders, `Super+LMB/RMB` pending)
- [~] M1.6 🔴 Maximize / minimize / fullscreen / restore states (maximize/minimize/restore done; fullscreen pending)
- [ ] M1.7 🔴 xdg popups: positioner constraints, grabs, dismissal
- [~] M1.8 🔴 `zxdg_decoration_manager_v1` (done) + `org_kde_kwin_server_decoration_manager` (pending)
- [~] M1.9 🔴 QML server-side decorations (KDE Breeze default theme: left app icon/title, right-aligned Breeze minimize/maximize/close buttons, border done; interactive resize borders & shadow pending)
- [ ] M1.10 🟡 `wm/Placement` (cascade placement done; center dialogs on parent, smart placement pending)
- [ ] M1.11 🟡 Edge/quarter snapping with preview + `Super+Arrow` shortcuts
- [ ] M1.12 🔴 `input/ShortcutManager` (basic QML shortcuts done; configurable `[Shortcuts]` C++ manager pending)
- [ ] M1.13 🟡 Alt+Tab window switcher (lazy QML)
- [ ] M1.14 🔴 `zwlr_layer_shell_v1` (layers, anchors, exclusive zones, keyboard interactivity)
- [ ] M1.15 🔴 `xdg_activation_v1` (tokens, focus-stealing prevention, urgency fallback)
- [~] M1.16 🟡 `zxdg_output_manager_v1` (done), clipboard / primary selection / DnD between clients (pending)
- [ ] M1.17 🟡 Usable-area computation (exclusive zones) for maximize/snap
- **Verify:** multiple apps stack/raise/focus correctly; dialogs stay above parents; layer-shell client (e.g. `waybar` or test client) reserves space; shortcuts work.

## M2 — DRM/KMS platform, elogind session, libinput
- [ ] M2.1 🔴 `session/SessionBackend` interface
- [ ] M2.2 🔴 `session/LogindSession` (elogind via `org.freedesktop.login1`): session lookup, `TakeControl`, `TakeDevice`/`ReleaseDevice`, `PauseDevice`/`ResumeDevice`, active state
- [ ] M2.3 🔴 VT switching (`Seat.SwitchTo`, `Ctrl+Alt+Fn`)
- [ ] M2.4 🔴 `platform/drm` QPA plugin skeleton (integration, screen, window, EGL context on GBM)
- [ ] M2.5 🔴 Device enumeration via udev, GPU selection (boot_vga / config override), fds from session backend
- [ ] M2.6 🔴 Atomic KMS: CRTC/connector/plane assignment, initial modeset (preferred mode), page-flip event loop
- [ ] M2.7 🔴 Swap path: GBM surface → front buffer → framebuffer cache → atomic flip
- [ ] M2.8 🔴 Hardware cursor plane (+ software fallback); cursor motion without scene repaint
- [ ] M2.9 🔴 Hotplug (udev monitor) → screen add/remove, windows migrated to remaining outputs
- [ ] M2.10 🔴 Pause/resume on VT switch (drop/restore master, restore CRTC state, full repaint)
- [ ] M2.11 🔴 Native interface `TapePlatformInterface` (modes, apply/test config, VRR, DPMS, cursor)
- [ ] M2.12 🔴 `input/LibinputBackend` (udev seat, `open_restricted` via session), event dispatch into `QWaylandSeat`
- [ ] M2.13 🔴 Keyboard: xkbcommon keymap from config, repeat rate/delay, LED state, NumLock option
- [ ] M2.14 🟡 Pointer/touchpad config (accel, natural scroll, tap, DWT, left-handed, scroll method)
- [ ] M2.15 🟡 Touch input basic support
- [ ] M2.16 🔴 `zwp_linux_dmabuf_v1` client buffers (format/modifier advertisement)
- [ ] M2.17 🟡 `wp_presentation` feedback
- [ ] M2.18 🔴 `data/tape-session` script (env vars, `dbus-run-session` fallback) + `data/tape.desktop`
- [ ] M2.19 🔴 `dbus-update-activation-environment` after socket + D-Bus names ready
- [ ] M2.20 🔴 `core/Autostart` (XDG autostart parsing) + `core/ProcessLauncher` (setsid, activation token, env)
- [ ] M2.21 🟡 Optional spawn of pipewire / wireplumber / pipewire-pulse when absent
- [ ] M2.22 🟡 Crash safety: restore TTY/KD mode on crash, backtrace to log
- **Verify:** start from SDDM/greetd on an elogind system (e.g. Artix/Gentoo/Void); 2 monitors; unplug/replug; VT switch away and back; Firefox/Chromium use dmabuf; no root needed.

## M3 — Shell UI
- [ ] M3.1 🔴 Shell QML structure per output with lazy `Loader`s + idle unload helper
- [~] M3.2 🔴 Breeze-styled Top bar (layer `top`): system menu button, focused app name, global-menu slot (placeholder until M9), debug pill, clock (tray/notification toggle pending)
- [~] M3.3 🔴 `shell/TaskModel` + Breeze-styled bottom Dock (running & minimized windows, dashboard launcher done; pinned apps, autohide + magnification pending)
- [ ] M3.4 🔴 Wallpaper: image modes (fill/fit/stretch/center/tile), solid color, per-output, `sourceSize` decoding
- [ ] M3.5 🟡 Wallpaper: KDE wallpaper packages, slideshow folder
- [ ] M3.6 🔴 `shell/AppModel` (.desktop parsing via KService or own parser, inotify refresh, icons)
- [ ] M3.7 🔴 App dashboard: full-screen grid, search (fuzzy), categories, paging, keyboard nav
- [ ] M3.8 🟡 Dashboard: frequently used, pin to dock, desktop actions context menu
- [ ] M3.9 🟡 Launch feedback (dock bounce / busy cursor) via activation token
- [ ] M3.10 🔴 `org.kde.StatusNotifierWatcher` + host, `shell/TrayModel`, icon/pixmap/attention rendering
- [ ] M3.11 🔴 DBusMenu client (`shell/MenuModel`, shared with M9) + compositor menu popup component
- [ ] M3.12 🔴 `org.freedesktop.Notifications` server (Notify, Close, Capabilities, ServerInformation, signals)
- [ ] M3.13 🔴 Notification popups (stacking, timeouts per urgency, hover pause, actions, images, inline reply)
- [ ] M3.14 🟡 Notification center (history, grouping, clear), Do Not Disturb (manual + auto during fullscreen/screencast)
- [ ] M3.15 🟡 Per-app notification rules (`[Notifications][App:<id>]`)
- [ ] M3.16 🟡 OSD component (volume, brightness, debug toggle, layout change)
- [ ] M3.17 🟢 Basic indicators: battery (UPower), volume (wpctl), network (optional NM D-Bus)
- **Verify:** `notify-send` with actions/images works; Discord/fcitx5 tray icons show with menus; dashboard launches apps; wallpaper set per output; idle CPU ~0%.

## M4 — Settings app skeleton, D-Bus API, debug toggle
- [ ] M4.1 🔴 `org.tape.Compositor` D-Bus service (PLAN §19) with caller checks
- [ ] M4.2 🔴 `settings/` app: QQC2 `org.kde.desktop` style, sidebar, single instance (`org.tape.Settings`), deep links
- [ ] M4.3 🔴 `PlaceholderPage.qml` + all pages registered as placeholders (Display, Appearance, Wallpaper, Text & Fonts, Input Method, Keyboard, Mouse & Touchpad, Notifications, Power, Desktop Effects, Window Management, Dock & Top bar, Debug, About)
- [ ] M4.4 🔴 **Debug page**: master switch, per-category toggles, protocol logging toggle, open log file, debug overlay toggle
- [ ] M4.5 🔴 `Super+Shift+D` debug toggle shortcut + OSD
- [x] M4.6 🔴 `wl_display_add_protocol_logger` runtime install/remove
- [ ] M4.7 🔴 `ctl/tapectl`: `debug on|off [--category]`, `output list`, `output virtual add|remove`, `windows`, `reconfigure`, `lock`, `screenshot`
- [ ] M4.8 🟡 Debug overlay (FPS, frame time, window count, damage flash)
- [ ] M4.9 🟡 About page (versions, GPU/driver, session type, elogind version)
- [ ] M4.10 🟡 Dashboard/top bar entry to open Settings
- **Verify:** toggling debug from Settings, shortcut and `tapectl` all change log output live; placeholder pages navigable.

## M5 — KDE theming, Text & Fonts
- [ ] M5.1 🔴 `theme/KdeTheme` singleton: KColorScheme palettes, `[WM]` colors, accent, animation factor, live reload
- [ ] M5.2 🔴 Apply theme to shell QML + decorations
- [ ] M5.3 🔴 Icon theme from kdeglobals (`QIcon::setThemeName`), fallback chain
- [ ] M5.4 🔴 Cursor theme/size from kcminputrc, libXcursor loading, `XCURSOR_*` env, `wp_cursor_shape_v1`
- [ ] M5.5 🔴 GTK propagation: gtk-3.0/4.0 `settings.ini`, gsettings `org.gnome.desktop.interface`
- [ ] M5.6 🟡 XWayland propagation: xrdb (`Xft.*`, `Xcursor.*`), optional xsettingsd
- [ ] M5.7 🔴 **Appearance page**: color scheme list (from `color-schemes/*.colors`), icon theme, cursor theme/size, accent color, decoration choice
- [ ] M5.8 🔴 **Text & Fonts page**: general/fixed/small/toolbar/menu/title fonts, AA, subpixel order, hinting, force DPI, text scaling, live preview
- [ ] M5.9 🔴 Font settings writers: kdeglobals, fontconfig `99-tape.conf`, Xft, GTK
- [ ] M5.10 🟢 KDecoration3 bridge (Breeze/Aurorae SSD), behind `TAPE_KDECORATION`
- **Verify:** switching Breeze Light/Dark updates shell, Qt apps (plasma-integration), GTK apps, decorations live; font changes visible in Kate/GTK/X11 apps.

## M6 — Built-in fcitx5
- [ ] M6.1 🔴 Global filter infrastructure (privileged globals only for trusted clients)
- [ ] M6.2 🔴 `zwp_input_method_v1` + `zwp_input_method_context_v1` + `zwp_input_panel_v1`
- [ ] M6.3 🔴 `zwp_text_input_v3` (Qt built-in if available, else custom)
- [ ] M6.4 🔴 `zwp_text_input_v2` (Qt `QWaylandTextInputManager`)
- [ ] M6.5 🟡 `zwp_text_input_v1` (Chromium/Electron)
- [ ] M6.6 🔴 `TextInputRouter`: focus tracking, enable/disable, surrounding text, content type, cursor rect, preedit/commit forwarding
- [ ] M6.7 🔴 Candidate panel positioning (input_panel_surface at cursor rect, clamped)
- [ ] M6.8 🔴 `zwp_virtual_keyboard_v1` (privileged)
- [ ] M6.9 🔴 `Fcitx5Launcher`: spawn with `WAYLAND_SOCKET`, restart back-off, enable/disable config
- [ ] M6.10 🔴 `Fcitx5Controller` D-Bus client (`org.fcitx.Fcitx.Controller1`)
- [ ] M6.11 🔴 **Input Method page**: IM groups, add/remove/reorder IMs, switch group, current IM
- [ ] M6.12 🟡 Generic fcitx config renderer (global options, addon/IM config from config descriptions)
- [ ] M6.13 🟡 Addon enable/disable, Restart/Reload, "Advanced…" → `fcitx5-config-qt`
- [ ] M6.14 🔴 Password fields / lock screen disable IM
- [ ] M6.15 🟡 Keyboard page (layouts, variants, options, repeat) — coordinated with fcitx5 keyboard IMs
- **Verify (test matrix):** Pinyin/Mozc/Unikey/Hangul input in Kate (Qt6), Qt5 app, gedit/GTK4, Firefox, Chromium, Discord (Electron Wayland), XWayland app (XIM); candidate window positioned correctly on scaled outputs.

## M7 — Display configuration, scaling, VRR, virtual displays
- [ ] M7.1 🔴 `output/OutputDevice` + `OutputConfig` model, EDID parsing (make/model/serial)
- [ ] M7.2 🔴 `OutputManager::apply()` with TEST_ONLY → commit → persist; error reporting
- [ ] M7.3 🔴 Runtime mode change (resolution + refresh), position, transform (rotation), enable/disable
- [ ] M7.4 🔴 "Keep display settings?" confirmation overlay with 15 s auto-revert
- [ ] M7.5 🔴 Persistence `outputs.json` keyed by EDID, per-setup profiles, apply on hotplug
- [ ] M7.6 🔴 Integer scaling (`wl_output.scale`) + QPA DPR change at runtime
- [ ] M7.7 🔴 `wp_fractional_scale_v1` + `wp_viewporter`; fractional DPR rendering
- [ ] M7.8 🔴 VRR: detect `vrr_capable`, set `VRR_ENABLED`, policy Never/Always/Automatic (fullscreen)
- [ ] M7.9 🟡 `wp_content_type_v1` (game hint for VRR Automatic), `wp_tearing_control_v1` (optional async flips)
- [ ] M7.10 🔴 `output/VirtualOutput` via `QQuickRenderControl` + GBM-backed FBO, damage-driven frame clock
- [ ] M7.11 🔴 Virtual output create/remove via D-Bus, `tapectl`, Settings; optional persistence
- [ ] M7.12 🔴 **Display page**: arrangement canvas (drag outputs), resolution, refresh, scale, rotation, VRR, primary, enable; virtual display section
- [ ] M7.13 🟡 DPMS API (used by M11)
- **Verify:** change 1080p60→1440p144 and back with revert; fractional 1.25/1.5 crisp in Qt6/GTK4/Firefox; VRR active in a fullscreen game (check `/sys/kernel/debug/dri/*/state` or monitor OSD); virtual display appears as a `wl_output` and windows can be moved onto it.

## M8 — Screen capture, xdg-desktop-portal, window picker
- [ ] M8.1 🔴 `capture/PipeWireCore` integrated into Qt event loop (pw_loop fd → `QSocketNotifier`)
- [ ] M8.2 🔴 `ScreenCastStream`: format/modifier negotiation, DMA-BUF buffers (GBM BOs), SHM/memfd fallback, renegotiation on resize
- [ ] M8.3 🔴 `OutputSource` (output frame blit, damage-driven)
- [ ] M8.4 🔴 `WindowSource`: secondary `QWaylandQuickItem` view in render-control window; subsurfaces + popups; works occluded/minimized/other output
- [ ] M8.5 🔴 Keep frame callbacks alive for captured hidden windows (at stream rate)
- [ ] M8.6 🔴 `VirtualSource` (stream virtual output buffers directly; create-on-demand for `VIRTUAL` type)
- [ ] M8.7 🔴 Cursor modes: hidden / embedded / metadata (`SPA_META_Cursor`)
- [ ] M8.8 🔴 D-Bus `org.freedesktop.impl.portal.desktop.tape` name acquired before autostart/env update
- [ ] M8.9 🔴 `impl.portal.ScreenCast` (CreateSession, SelectSources, Start, properties, v5 features) with delayed replies
- [ ] M8.10 🔴 `impl.portal.Session` (Close, cleanup streams)
- [ ] M8.11 🔴 **Picker UI**: Screens / Windows / Virtual tabs, live thumbnails, search, keyboard nav, multiple selection
- [ ] M8.12 🟡 `persist_mode` + restore tokens (output EDID, window appId/title heuristic)
- [ ] M8.13 🔴 Active-stream indicator in top bar (list + stop)
- [ ] M8.14 🔴 `impl.portal.Screenshot` (interactive/non-interactive) + `PickColor`
- [ ] M8.15 🟡 `Print` / `Shift+Print` screenshots → file + clipboard + notification; region selector
- [ ] M8.16 🔴 `zxdg_exporter_v2` / `zxdg_importer_v2` (xdg-foreign) for portal dialog parenting
- [ ] M8.17 🔴 `data/tape.portal` + `data/tape-portals.conf` (ScreenCast/Screenshot=tape, default=kde;gtk)
- [ ] M8.18 🟡 `ext_foreign_toplevel_list_v1` (task info for external tools)
- **Verify:** Discord (native Wayland) → Screen Share → "Windows" tab → pick a specific window → viewers see only that window, still updating when covered/minimized; OBS PipeWire capture; Firefox WebRTC test page; screenshot portal from Flatpak app; restore token re-share without prompt.

## M9 — Global menu
- [ ] M9.1 🔴 `org_kde_kwin_appmenu_manager` protocol (service + object path per surface)
- [ ] M9.2 🔴 `com.canonical.AppMenu.Registrar` service (+ verify plasma-integration's detection requirements)
- [ ] M9.3 🔴 `MenuModel` full DBusMenu support (GetLayout, AboutToShow, Event, LayoutUpdated, ItemsPropertiesUpdated, shortcuts, toggles, icons, submenus)
- [ ] M9.4 🔴 Top bar global menu rendering for focused window, popup menus, keyboard navigation (Alt / F10, mnemonics)
- [ ] M9.5 🟡 Fallback menu (app name, desktop actions, Quit) for apps without menus
- [ ] M9.6 🟡 Setting: global menu on/off (in-window menus when off)
- **Verify:** Kate, Dolphin, Konsole, Qt5 apps show menus in the top bar; XWayland app with appmenu-gtk-module works.

## M10 — Desktop effects
- [ ] M10.1 🔴 `EffectManager` (registry, config, speed factor, reduced motion), zero-cost-when-idle design
- [ ] M10.2 🔴 Shader build pipeline (`qt_add_shaders`, `shaders/`)
- [ ] M10.3 🔴 `DeformableMeshNode` (grid mesh, texture provider incl. subsurfaces via temporary layer)
- [ ] M10.4 🔴 Wobbly windows (spring-mass physics, grab anchor, settle detection, multi-output shared state)
- [ ] M10.5 🔴 Magic lamp minimize/restore (dock icon target, dock-edge aware, fallback scale/fade)
- [ ] M10.6 🔴 Dual-Kawase blur render node (configurable strength/noise)
- [ ] M10.7 🔴 Blur behind shell surfaces (bar, dock, dashboard, notifications, picker)
- [ ] M10.8 🔴 `org_kde_kwin_blur_manager` client blur regions
- [ ] M10.9 🟡 Blur caching when background static
- [ ] M10.10 🟡 Basic effects: open/close fade+scale, dashboard zoom, switcher fade
- [ ] M10.11 🔴 **Desktop Effects page** (toggles + parameters: wobbliness, lamp duration, blur strength)
- [ ] M10.12 🟡 Effect performance measurement recorded in `docs/perf.md`
- **Verify:** wobbly while dragging Konsole at 144 Hz without drops; magic lamp toward dock icon; translucent Konsole blurred; disabling all effects returns to baseline memory/GPU.

## M11 — Power management & lock screen
- [ ] M11.1 🔴 `power/IdleManager` (input activity, rate-limited reset, multi-stage timers)
- [ ] M11.2 🔴 Inhibitors: `zwp_idle_inhibit_v1`, `org.freedesktop.ScreenSaver` (Inhibit/UnInhibit/GetActive…), elogind `BlockInhibited`
- [ ] M11.3 🔴 `SetIdleHint` to elogind; `ext_idle_notify_v1`
- [ ] M11.4 🔴 `power/UPowerClient` (OnBattery, battery %, lid present) → AC/Battery profile switching
- [ ] M11.5 🔴 Dim screen: backlight via elogind `SetBrightness` (feature-detected, sysfs fallback) / overlay for external monitors
- [ ] M11.6 🔴 Screen off via DPMS (`ACTIVE=0`), wake on input
- [ ] M11.7 🔴 Suspend after timeout (`Manager.Suspend`), `PrepareForSleep` handling with `delay` inhibitor
- [ ] M11.8 🔴 Lock screen QML (all outputs, input grab) + PAM auth worker thread + `data/pam/tape-lock`
- [ ] M11.9 🔴 elogind `Lock`/`Unlock` signal handling, `Super+L`, lock-before-sleep
- [ ] M11.10 🟡 `ext_session_lock_v1` for external lockers
- [ ] M11.11 🟡 Lid / power button custom actions (block inhibitor only when configured), power menu (logout/suspend/reboot/shutdown)
- [ ] M11.12 🟡 Brightness & volume keys + OSD
- [ ] M11.13 🟢 power-profiles-daemon integration
- [ ] M11.14 🔴 **Power page**: AC/Battery tabs — dim after, dim level, screen off after, suspend after, lock options, lid/power button actions
- **Verify:** timers fire in order (dim → off → lock → suspend) on AC and battery; video in mpv/Firefox inhibits; resume shows lock screen first; elogind `loginctl lock-session` locks.

## M12 — XWayland
- [ ] M12.1 🔴 Lazy Xwayland launch (listening sockets, `-rootless -wm -listenfd`), restart on crash
- [ ] M12.2 🔴 `Xwm` (xcb): map/unmap/configure, ICCCM, EWMH, window types, transient-for, `_NET_WM_STATE`, `_NET_ACTIVE_WINDOW`
- [ ] M12.3 🔴 `xwayland_shell_v1` surface association
- [ ] M12.4 🔴 `XWaylandWindow` in WindowModel (stacking, focus, decorations for non-CSD)
- [ ] M12.5 🔴 Clipboard + primary selection bridging X11 ↔ Wayland
- [ ] M12.6 🟡 DnD bridging
- [ ] M12.7 🔴 Scaling modes (compositor upscale vs app-scaled with Xft.dpi)
- [ ] M12.8 🔴 `zwp_pointer_constraints_v1` + `zwp_relative_pointer_v1` (games, also native Wayland)
- [ ] M12.9 🟡 XWayland windows in task model, screencast window list, global menu registrar
- **Verify:** Steam + a Proton game (fullscreen, pointer lock, VRR), X11 Discord, xterm, LibreOffice (X11) copy/paste both ways.

## M13 — Optimization, extras, packaging
- [ ] M13.1 🔴 Resource budget audit vs PLAN §17 (heaptrack, perf, powertop, wakeups) → `docs/perf.md`
- [ ] M13.2 🟡 Direct scanout of fullscreen opaque client buffers
- [ ] M13.3 🟡 Hidden-window frame callback throttling audit, QML lazy-unload tuning, qmltc where beneficial
- [ ] M13.4 🟡 `ext_image_capture_source_v1` + `ext_image_copy_capture_v1` (privileged / configurable)
- [ ] M13.5 🟡 `zwlr_foreign_toplevel_manager_v1`
- [ ] M13.6 🟢 `zwlr_output_management_v1` (wlr-randr / kanshi)
- [ ] M13.7 🟢 `impl.portal.RemoteDesktop`, `impl.portal.GlobalShortcuts`
- [ ] M13.8 🟡 Optional systemd integration (`TAPE_SYSTEMD`): graphical-session.target, transient scopes, journald
- [ ] M13.9 🔴 Packaging: install rules, Arch PKGBUILD, Gentoo/Artix notes, Void template (elogind distros)
- [ ] M13.10 🔴 README: install, session setup, Discord/Electron flags, fcitx5 notes, troubleshooting, debug logging how-to
- [ ] M13.11 🟡 Translation infrastructure (KI18n / Qt Linguist)

---

## Cross-cutting (continuous)
- [ ] X.1 Every new module has its `tape.*` logging category
- [ ] X.2 Every new config key: default in `taperc.default` + doc comment + settings UI (or placeholder)
- [ ] X.3 No blocking calls on the compositor thread (review checklist)
- [ ] X.4 Unit/headless tests added for each protocol and model
- [ ] X.5 Resource check (idle CPU/RSS) at the end of each milestone
- [ ] X.6 Update PLAN.md and REQUIREMENTS.md when a requirement, dependency, or design decision changes (with date + reason)
