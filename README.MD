# hyprmax

A fake maximize implementation for [Hyprland](https://hyprland.org/) that avoids the native fullscreen/maximize state, which causes z-order bugs in floating WM setups.

> **Plugin version coming soon.** For now, the script installation is the recommended method.

---

## The Problem

When using Hyprland as a floating WM, the native `fullscreen, 1` (maximize) state causes a z-order bug: opening a new window in front of a maximized window and then switching back leaves a ghost window that appears on screen but cannot receive input. This is only resolved by un-maximizing the window.

`hyprmax` solves this by simulating maximize through repositioning and resizing the window to fill the screen.

---

## Features

- Fake maximize that respects `gaps_out` and reserved areas (e.g. Waybar)
- Restores the original window geometry on toggle
- Tiled windows are automatically converted to floating when maximized, and restore their previous floating geometry on unmaximize
- Works with any window switcher (Rofi, Hyprshell, etc.)

---

## Installation

### Script (recommended)

**Dependencies:** `bash`, `jq`, `hyprctl`

```bash
# Install jq if you don't have it
# Arch: sudo pacman -S jq
# Debian/Ubuntu: sudo apt install jq

# Clone the repository
git clone https://github.com/hashylog/hyprmax
cd hyprmax

# Copy the script
mkdir -p ~/.config/hypr/scripts
cp scripts/hyprmax.sh ~/.config/hypr/scripts/
chmod +x ~/.config/hypr/scripts/hyprmax.sh
```

Then add the keybind to your `hyprland.conf`:

```ini
bind = $mainMod, F, exec, ~/.config/hypr/scripts/hyprmax.sh
```

Reload Hyprland:

```bash
hyprctl reload
```

### Plugin (coming soon)

A native Hyprland plugin is planned and will be available once multi-distro build support is implemented. It will be installable via `hyprpm`:

```bash
# Not available yet
hyprpm add https://github.com/hashylog/hyprmax
hyprpm enable hyprmax
```

---

## Known Issues / Limitations

- **`GAPS_OUT` is hardcoded** to `20` in the script. If you use a different value in your config, edit line `GAPS_OUT=20` in `fakemaximize.sh` to match.
- **Floating geometry is only saved when going through the script.** If a window was tiled before ever being fake maximized, it will be unmaximized to floating without a saved position.
- **The plugin version is not yet available.** The script relies on temporary files in `/tmp` for state, which is cleared on reboot.
- **Multi-monitor support** has not been thoroughly tested.
