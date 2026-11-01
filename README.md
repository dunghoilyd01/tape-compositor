# Tape Compositor

**Tape** is a lightweight, KDE-themed stacking Wayland compositor built on **QtWayland Compositor** (targeting **Qt 6.12**), using **KDE Breeze** as its default visual and window decoration theme with a top-bar + bottom-dock + app-dashboard layout, **elogind** session management, built-in **fcitx5** input method integration, per-window **xdg-desktop-portal** screen sharing (for Discord/OBS), and hardware-accelerated desktop effects (wobbly windows, magic lamp, dual-Kawase blur).

See:
- [PLAN.md](PLAN.md) — Master Architecture & Implementation Plan
- [REQUIREMENTS.md](REQUIREMENTS.md) — Functional, Non-Functional & Dependency Requirements
- [CHECKLIST.md](CHECKLIST.md) — Milestone Checklist

---

## 1. Building

From inside `tape-compositor/`:

```bash
cd /home/huynguyen/compositor/tape-compositor

# Configure CMake with Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Compile the compositor and test suite
cmake --build build
```

Optional CMake flags:
- `-DTAPE_ASAN=ON` — build with AddressSanitizer + UndefinedBehaviorSanitizer
- `-DTAPE_BUILD_TESTS=ON` (default `ON`) — build unit and headless protocol tests
- `-DTAPE_XWAYLAND=ON`, `-DTAPE_PIPEWIRE=ON`, `-DTAPE_KDECORATION=OFF`, `-DTAPE_SYSTEMD=OFF`

---

## 2. Running

### 2.1 Nested Mode (inside your current Wayland / X11 desktop)

```bash
# Single 1280x800 output with debug logging enabled
./build/tape --nested --output-size 1280x800 --debug

# Dual outputs with 1.25x scaling on a named Wayland socket
./build/tape --nested --outputs 2 --output-size 1280x800@1.25 --socket tape-dev

# Start Tape and immediately launch a client inside it (e.g. Konsole or weston-terminal)
./build/tape --nested --socket tape-dev -c "konsole"
```

To manually launch a Wayland client into a running Tape instance from another terminal:

```bash
WAYLAND_DISPLAY=tape-dev QT_QPA_PLATFORM=wayland konsole
# or
WAYLAND_DISPLAY=tape-dev weston-terminal
```

### 2.2 Headless Mode (no host window required)

```bash
./build/tape --headless --socket tape-headless --debug --protocol-log
```

### 2.3 CLI Options (`./build/tape --help`)

| Flag | Description |
|------|-------------|
| `--nested` | Run nested inside an existing Wayland or X11 session |
| `--headless` | Run in offscreen/headless mode |
| `--outputs <count>` | Number of outputs in nested/headless mode (`1..8`, default `1`) |
| `--output-size <WxH[@scale]>` | Output geometry and scale (e.g. `1280x800` or `1920x1080@1.5`) |
| `-s, --socket <name>` | Wayland socket name (e.g. `tape-0`) |
| `-c, --command <cmd>` | Shell command to launch once the Wayland socket is ready |
| `--debug` | Enable runtime debug logging at startup |
| `--debug-categories <list>` | Comma-separated categories (e.g. `core,wm,output,protocol`) |
| `--protocol-log` | Enable Wayland server protocol message logging |
| `--config <path>` | Custom config file path (default `~/.config/taperc`) |

### 2.4 Keyboard Shortcuts & UI Controls (M0)

- **`Super+Shift+D`** (or click the **`DBG ON/OFF`** pill in the top-right bar): Toggle debug logging at runtime
- **`Alt+F4`**: Close the active window
- **`Super+M`** (or double-click titlebar / green button): Maximize or restore the active window
- **`Super+H`** (or yellow button / click active icon in dock): Minimize the active window to the dock
- **Log file location:** `~/.local/state/tape/tape.log` (previous session saved as `tape.log.old`)

---

## 3. Testing

Run the automated CTest suite:

```bash
cd /home/huynguyen/compositor/tape-compositor

# Run all tests with verbose output on failure
ctest --test-dir build --output-on-failure -V
```

Or run the individual test binaries directly:

```bash
# Unit tests: CLI parser, LogManager runtime toggle + log file, KConfig defaults, WindowModel stacking
./build/tests/test_config_and_logging

# Headless compositor + libwayland-client protocol verification test
./build/tests/test_compositor_headless
```

---

## License

SPDX-License-Identifier: `GPL-3.0-or-later`
