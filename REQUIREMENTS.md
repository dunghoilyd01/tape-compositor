# Tape Compositor — Requirements

> **Mandatory input for all code generation.** Before writing or changing code, check it against this file,
> [PLAN.md](PLAN.md) (design) and [CHECKLIST.md](CHECKLIST.md) (progress).
> If a change conflicts with a requirement here, update this file first (with date + reason). Don't silently diverge.
>
> Requirement IDs: **R** = functional, **I** = implied functional, **N** = non-functional, **D** = dependency.

---

## 1. Functional Requirements (from the user)

| ID | Requirement | Acceptance criteria | Plan § | Milestone |
|----|-------------|---------------------|--------|-----------|
| R1 | **xdg-desktop-portal backend** that behaves like a standard xdg-desktop-portal implementation, with **per-window selection** for screen sharing (e.g. Discord) | ScreenCast (monitor/window/virtual) and Screenshot portals work. Discord can share one specific window, and that window keeps streaming when covered, minimized or on another output | §9 | M8 |
| R2 | Compositor built on **QtWayland Compositor (Qt 6.12)**, with **KDE theming** | Shell, decorations and client apps follow the KDE color scheme, icons, cursors and fonts from `kdeglobals`, and update live | §4, §11 | M0, M5 |
| R3 | **Desktop effects**: wobbly windows, magic lamp, blur | Wobbly while dragging; magic lamp minimizes/restores to the dock icon; blur behind shell surfaces and clients that request it (KDE blur protocol) | §12 | M10 |
| R4 | **Minimal resource usage** | Meets the budgets in N1 | §17 | all, M13 |
| R5 | **Debug log toggle** | Can be toggled at runtime from Settings, a shortcut, `tapectl` and D-Bus. Per-category and Wayland protocol logging | §16 | M0, M4 |
| R6 | **Settings page** (placeholder pages at first) | `tape-settings` app with every page listed in PLAN §15. Pages start as placeholders and are filled in milestone by milestone | §15 | M4+ |
| R7 | **App dashboard** | Launchpad-style full-screen grid with search, categories and launching | §13.3 | M3 |
| R8 | **Global menu** | The focused app's menu appears in the top bar (Qt/KDE apps on Wayland, X11 apps via the registrar) | §13.5 | M9 |
| R9 | **elogind** as session manager (systemd-logind optional later) | Runs without root via elogind `TakeControl`/`TakeDevice`. VT switching, sleep handling. No systemd needed | §6 | M2 |
| R10 | **Built-in fcitx5** + **fcitx5 settings menu** | The compositor starts and supervises fcitx5. IME works in Qt5/Qt6/GTK/Chromium/Electron/XWayland apps. A settings page manages IM groups and options | §10 | M6 |
| R11 | **Display configuration**: resolution, refresh rate (+ position, rotation, enable) | Change at runtime; changes are tested first and auto-revert unless confirmed; settings persist per monitor (EDID) | §8 | M7 |
| R12 | **Create virtual displays** | Create/remove a virtual output with a chosen size and refresh rate. It acts as a normal `wl_output` and can be screen-cast | §8.5 | M7 |
| R13 | **Adaptive Sync (VRR)** | Per-output Never / Always / Automatic (Automatic = only while a fullscreen window is shown) | §8.4 | M7 |
| R14 | **Scaling** (integer + fractional) | Per-output scale in 0.25 steps; `wp_fractional_scale_v1` + viewporter; XWayland scaling modes | §8.3 | M7 |
| R15 | **Wallpaper** | Per-output image (fill/fit/stretch/center/tile), solid color, KDE wallpaper packages, slideshow | §13.2 | M3 |
| R16 | **Text & fonts in Settings** | Font families/sizes, anti-aliasing, hinting, subpixel order, forced DPI and text scaling, applied to Qt, GTK and X11 apps | §11.4 | M5 |
| R17 | **Notifications** | `org.freedesktop.Notifications` server, popups, notification center, Do Not Disturb, per-app rules | §13.6 | M3 |
| R18 | **Power management config** (sleep time, dim screen time) | Separate AC/battery profiles: dim, screen off, lock and suspend timers; respects idle inhibitors | §14 | M11 |
| R19 | **Breeze-themed top-bar + bottom-dock + dashboard layout** | Top panel (system menu, app name, global menu, status area, clock) + bottom dock (pinned/running apps, minimized windows) + full-screen app dashboard. Uses **KDE Breeze** as default theme for shell and window decorations; takes layout inspiration from global-menu/dock workflows without copying macOS window themes or visuals 1:1 | §13 | M3 |

## 2. Implied Functional Requirements

| ID | Requirement | Reason | Milestone |
|----|-------------|--------|-----------|
| I1 | **Stacking window management** (move/resize, raise, focus, maximize/minimize/fullscreen, snapping, Alt+Tab) | Core of a stacking compositor | M1 |
| I2 | **Server-side decorations** styled by the KDE theme | Theming (R2) | M1, M5 |
| I3 | **XWayland** | Steam, games, X11-only apps, X11 Discord | M12 |
| I4 | **System tray** (StatusNotifierItem + DBusMenu) | fcitx5 and Discord tray icons | M3 |
| I5 | **Lock screen** (PAM) + `ext_session_lock_v1` | Lock before sleep (R18) | M11 |
| I6 | **XDG autostart & helper supervision** | No systemd user session under elogind | M2 |
| I7 | **Layer-shell** (`zwlr_layer_shell_v1`) | External bars, wallpapers, tools | M1 |
| I8 | **Multi-monitor + hotplug** | Display config (R11) | M2 |
| I9 | **Clipboard / primary selection / DnD** (Wayland ↔ X11) | Basic usability | M1, M12 |
| I10 | **Nested development mode** (run inside an existing session) | Fast iteration and testing | M0 |
| I11 | **`tapectl` CLI + `org.tape.Compositor` D-Bus API** | Debug toggle, virtual displays, automation | M4 |

## 3. Non-Functional Requirements

| ID | Requirement |
|----|-------------|
| N1 | **Resource budget** (1×1080p, top bar + dock + wallpaper, no clients): idle CPU ≈ 0 % and **0 wakeups/s**; compositor RSS ≤ 120 MB (stretch goal ≤ 90 MB); GPU memory ≤ 64 MB; first frame ≤ 1 s; pointer motion does not repaint the scene (HW cursor); disabled effects cost nothing |
| N2 | **Never block the compositor thread** (async D-Bus, async IO, delayed D-Bus replies for the portal) |
| N3 | **No root**: all privileged device access goes through elogind |
| N4 | **Security**: privileged Wayland globals (input-method, virtual-keyboard, screencopy, foreign-toplevel, session-lock) are only exposed to trusted clients via `wl_display_set_global_filter`. Mutating D-Bus calls check the caller |
| N5 | **Stability**: helpers (fcitx5, Xwayland) restart with back-off; the TTY is restored on crash; the previous session's log is kept |
| N6 | **Configurability**: every option lives in `~/.config/taperc` (KConfig format), documented in `data/taperc.default`, and reloads live |
| N7 | **Code quality**: C++20, Qt 6.12 APIs, clang-format/clang-tidy, a logging category per module, tests per protocol/model (PLAN §20, §22) |
| N8 | **License**: GPL-3.0-or-later, SPDX header in every file |
| N9 | **Portability across elogind distros** (Artix, Gentoo, Void, Devuan…); systemd integration optional (`TAPE_SYSTEMD=OFF` by default) |
| N10 | **Hardware**: Intel / AMD (Mesa) as the main targets; NVIDIA proprietary (GBM) best effort |

---

## 4. Software Dependencies

Minimum versions are **planned targets**. Check them against the distro at M0 and update here if they change.
Package names are for **Artix/Arch** (elogind-based). Other distros: see the notes column.

### 4.1 Build toolchain

| ID | Dependency | Min version | Artix/Arch package | Notes |
|----|-----------|-------------|--------------------|-------|
| D1 | C++ compiler with C++20 | GCC 13 / Clang 17 | `gcc` / `clang` | |
| D2 | CMake | 3.27 | `cmake` | |
| D3 | Ninja | 1.11 | `ninja` | |
| D4 | pkg-config | — | `pkgconf` | |
| D5 | Extra CMake Modules | matching KF6 | `extra-cmake-modules` | Needed for KF6 `find_package` |
| D6 | clang-format / clang-tidy | 17 | `clang` | Dev only |

### 4.2 Qt 6.12 (pinned minor version, private headers used)

| ID | Module | Artix/Arch package | Used for |
|----|--------|--------------------|----------|
| D10 | Qt Core, Gui, DBus, Concurrent (+ **GuiPrivate**) | `qt6-base` | Base, QPA plugin, D-Bus services |
| D11 | Qt Quick, Qml (+ QuickPrivate where needed) | `qt6-declarative` | Scene graph, shell QML, render control |
| D12 | Qt Wayland Compositor (+ **WaylandCompositorPrivate**) | `qt6-wayland` | Wayland server, `qtwaylandscanner` |
| D13 | Qt Shader Tools | `qt6-shadertools` | `qt_add_shaders` (effects) |
| D14 | Qt Svg | `qt6-svg` | SVG icons / KDE wallpapers |
| D15 | Qt Linguist tools (optional) | `qt6-tools` | Translations (M13) |

> **Rule:** target **Qt 6.12 exactly** (`find_package(Qt6 6.12 REQUIRED ...)` and refuse other minor versions when private API is used). Upgrading Qt is a deliberate task that updates this file.

### 4.3 KDE Frameworks 6 / KDE components

| ID | Dependency | Artix/Arch package | Used for |
|----|-----------|--------------------|----------|
| D20 | KConfig (Core + KConfigWatcher) | `kconfig` | `taperc`, kdeglobals, live reload |
| D21 | KColorScheme | `kcolorscheme` | Palettes from color schemes |
| D22 | KGuiAddons | `kguiaddons` | Color utils, icon helpers |
| D23 | KIconThemes | `kiconthemes` | Icon theme handling |
| D24 | KService | `kservice` | `.desktop` parsing / app model |
| D25 | KI18n | `ki18n` | Translations |
| D26 | KDecoration3 (optional) | `kdecoration` | Breeze/Aurorae decoration bridge (`TAPE_KDECORATION`) |
| D27 | plasma-wayland-protocols | `plasma-wayland-protocols` | appmenu, blur, server-decoration XMLs |
| D28 | qqc2-desktop-style (runtime) | `qqc2-desktop-style` | Native look for `tape-settings` |
| D29 | plasma-integration (runtime) | `plasma-integration` | `QT_QPA_PLATFORMTHEME=kde` for Qt apps, global menu export |
| D30 | Breeze theme + icons (runtime) | `breeze`, `breeze-icons` | Default theme |
| D31 | kde-gtk-config (optional runtime) | `kde-gtk-config` | GTK theme sync |

### 4.4 Graphics, input, Wayland

| ID | Dependency | Min version | Artix/Arch package | Used for |
|----|-----------|-------------|--------------------|----------|
| D40 | libwayland-server | 1.23 | `wayland` | Protocol logger, global filter |
| D41 | wayland-protocols | 1.41 | `wayland-protocols` | xdg-shell, fractional-scale, ext-* protocols |
| D42 | wlr-protocols (vendored) | — | (in `protocols/`) | layer-shell, foreign-toplevel, output-management |
| D43 | libdrm | 2.4.120 | `libdrm` | Atomic KMS |
| D44 | Mesa GBM + EGL + GLESv2 | 24.0 | `mesa`, `libglvnd` | Buffers, rendering |
| D45 | libinput | 1.26 | `libinput` | Input devices |
| D46 | libudev | — | `udev` / `eudev` (distro-specific) | Device enumeration, hotplug |
| D47 | xkbcommon | 1.7 | `libxkbcommon` | Keymaps |
| D48 | libXcursor | 1.2 | `libxcursor` | Cursor themes |
| D49 | libdisplay-info (optional) | 0.2 | `libdisplay-info` | EDID parsing (fallback: own parser) |

### 4.5 Session, system services

| ID | Dependency | Min version | Artix/Arch package | Used for |
|----|-----------|-------------|--------------------|----------|
| D50 | **elogind** | 252 | `elogind` (+ init-specific service, e.g. `elogind-openrc`) | Session, devices, sleep, brightness |
| D51 | D-Bus daemon | 1.14 | `dbus` | Session bus (`dbus-run-session`) |
| D52 | UPower | 1.90 | `upower` | AC/battery state |
| D53 | PAM | — | `pam` | Lock screen auth |
| D54 | power-profiles-daemon (optional) | 0.20 | `power-profiles-daemon` | Power profiles |
| D55 | systemd-logind (optional, later) | — | — | Alternative backend (`TAPE_SYSTEMD`) |

### 4.6 Screen capture & portals

| ID | Dependency | Min version | Artix/Arch package | Used for |
|----|-----------|-------------|--------------------|----------|
| D60 | PipeWire (libpipewire + daemon) | 1.0 | `pipewire` | Screencast streams |
| D61 | WirePlumber (runtime) | 0.5 | `wireplumber` | PipeWire session manager |
| D62 | xdg-desktop-portal (frontend) | 1.18 | `xdg-desktop-portal` | Portal frontend |
| D63 | xdg-desktop-portal-kde (runtime) | 6.x | `xdg-desktop-portal-kde` | FileChooser, Settings, Notification, … |
| D64 | xdg-desktop-portal-gtk (optional) | — | `xdg-desktop-portal-gtk` | Fallback portals |

### 4.7 Input method

| ID | Dependency | Min version | Artix/Arch package | Used for |
|----|-----------|-------------|--------------------|----------|
| D70 | fcitx5 (with waylandim addon) | 5.1 | `fcitx5` | Built-in IME |
| D71 | fcitx5-qt / fcitx5-gtk (runtime) | 5.1 | `fcitx5-qt`, `fcitx5-gtk` | Legacy/X11 toolkit IM modules |
| D72 | fcitx5-configtool (optional) | 5.1 | `fcitx5-configtool` | "Advanced…" button |
| D73 | IM engines (user choice) | — | e.g. `fcitx5-chinese-addons`, `fcitx5-mozc`, `fcitx5-unikey`, `fcitx5-hangul` | Testing matrix |

### 4.8 X11 compatibility

| ID | Dependency | Min version | Artix/Arch package | Used for |
|----|-----------|-------------|--------------------|----------|
| D80 | Xwayland | 24.1 | `xorg-xwayland` | X11 apps |
| D81 | libxcb (+ composite, xfixes, res, render, shape) | 1.16 | `libxcb` | XWM |
| D82 | xcb-util-wm (icccm, ewmh) | 0.4.2 | `xcb-util-wm` | XWM |
| D83 | xrdb (runtime) | — | `xorg-xrdb` | Xft / Xcursor resources |
| D84 | xsettingsd (optional) | — | `xsettingsd` | GTK settings for X11 apps |

### 4.9 Development & testing tools (not runtime)

| ID | Tool | Package | Purpose |
|----|------|---------|---------|
| D90 | weston (test clients) | `weston` | `weston-terminal`, `weston-simple-*` |
| D91 | wayland-utils | `wayland-utils` | `wayland-info` |
| D92 | heaptrack, perf, powertop | `heaptrack`, `perf`, `powertop` | Resource budget (N1) |
| D93 | gdb, valgrind | `gdb`, `valgrind` | Debugging |
| D94 | Test apps | `konsole`, `kate`, `firefox`, `chromium`, `gtk4-demos`, `mpv`, `obs-studio`, Discord | Integration matrix |

---

## 5. System / Hardware Requirements

| ID | Requirement |
|----|-------------|
| S1 | Linux kernel ≥ 6.6 with an **atomic-KMS** DRM driver (i915/xe, amdgpu, nouveau, nvidia-drm with `modeset=1 fbdev=1`) |
| S2 | GPU with GBM + EGL + OpenGL ES 3.0 |
| S3 | VRR: kernel/driver exposing the `vrr_capable` connector property; FreeSync/G-Sync Compatible display |
| S4 | elogind running as the session/seat manager. The user's session is registered via `pam_elogind` (`XDG_RUNTIME_DIR` and `XDG_SESSION_ID` set) |
| S5 | Display manager that supports `wayland-sessions` (SDDM, greetd, LightDM) or a TTY login |

---

## 6. Runtime Environment (set by `tape-session` / compositor)

| Variable | Value | Reason |
|----------|-------|--------|
| `XDG_CURRENT_DESKTOP` | `Tape` | Portal routing (`tape-portals.conf`) |
| `XDG_SESSION_TYPE` / `XDG_SESSION_DESKTOP` | `wayland` / `tape` | |
| `QT_QPA_PLATFORMTHEME` | `kde` | KDE theming for Qt apps (R2) |
| `XMODIFIERS` | `@im=fcitx` | fcitx5 for XWayland apps (R10) |
| `GTK_IM_MODULE` / `QT_IM_MODULE` | *unset* by default | Wayland apps use text-input; optional legacy override |
| `XCURSOR_THEME` / `XCURSOR_SIZE` | from `kcminputrc` | Cursor theming |
| `ELECTRON_OZONE_PLATFORM_HINT` | `auto` | Native Wayland Electron (Discord) |
| `MOZ_ENABLE_WAYLAND` | `1` | Older Firefox |
| `WAYLAND_DISPLAY` / `DISPLAY` | set by compositor | Exported via `dbus-update-activation-environment` |
