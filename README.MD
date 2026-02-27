# hyprmax

A native Hyprland plugin that implements fake maximize for floating WM setups, avoiding the z-order bugs caused by the native fullscreen/maximize state.

---

## The Problem

When using Hyprland as a floating WM, the native `fullscreen, 1` (maximize) state causes a z-order bug: opening a new window in front of a maximized window and then switching back leaves a ghost window that appears on screen but cannot receive input. This is only resolved by un-maximizing the window.

`hyprmax` solves this by simulating maximize through repositioning and resizing the window to fill the screen.

---

## Features

- Native C++ plugin — no external dependencies (no `bash`, `jq`, or shell scripts)
- Fake maximize that reads `general:gaps_out` directly from your config (supports CSS-style gaps)
- Respects reserved areas automatically (Waybar, panels, etc.)
- Restores the original window geometry on toggle
- Tiled windows are converted to floating when triggered (tiled already behaves like maximized)
- Tags windows with `hyprmax` for use in window rules
- In-memory state with automatic cleanup on window close
- Works with any window switcher (Rofi, Hyprshell, etc.)
- Compatible with all systems that Hyprland supports

---

## Installation

### Plugin (recommended)

```bash
hyprpm add https://github.com/hashylog/hyprmax
hyprpm enable hyprmax
```

Add to your `hyprland.conf`:

```ini
# Load plugin on startup
exec-once = hyprpm reload

# Keybind
bind = $mainMod, F, hyprmax,
```

To update:

```bash
hyprpm update
```

### Manual build

**Requirements:** C++ compiler with C++23 support, `meson`, `ninja`, Hyprland development headers

```bash
git clone https://github.com/hashylog/hyprmax
cd hyprmax
meson setup build --buildtype=release
ninja -C build
```

Load manually:

```bash
hyprctl plugin load /absolute/path/to/hyprmax/build/libhyprmax.so
```

### Script (legacy)

The original shell script is still available in `scripts/hyprmax.sh` but is no longer recommended. The plugin is faster, has no external dependencies, and reads your config automatically.

<details>
<summary>Script installation</summary>

**Dependencies:** `bash`, `jq`, `hyprctl`

```bash
mkdir -p ~/.config/hypr/scripts
cp scripts/hyprmax.sh ~/.config/hypr/scripts/
chmod +x ~/.config/hypr/scripts/hyprmax.sh
```

```ini
bind = $mainMod, F, exec, ~/.config/hypr/scripts/hyprmax.sh
```

**Note:** The script has `GAPS_OUT` hardcoded to `20`. Edit the script if you use a different value.

</details>

---

## How It Works

| Window State | Action |
|---|---|
| Floating | Saves geometry, resizes to fill screen (minus gaps and reserved areas) |
| Fakemaxed | Restores original geometry |
| Tiled | Converts to floating (tiled already fills the screen) |

---

## Window Rules

Fakemaxed windows are tagged with `hyprmax`, which can be used in window rules:

```ini
windowrule = bordercolor rgb(ff0000), tag:hyprmax
```
