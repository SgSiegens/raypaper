# Raypaper
This is a fork of raypaper by [danihek](https://github.com/danihek), 
a wallpaper picker written in C using Raylib for Linux and macOS.
Unlike the original single-file implementation, this version features
a multi-file architecture and an integrated caching mechanism for 
faster thumbnail retrieval.

## "Features"

*   **Multiple Layouts**: Switch between four different animated layouts (Grid, Horizontal River, Vertical River, Wave).
*   **Live Search**: Filter wallpapers instantly by typing.
*   **Full-Screen Preview**: Isolate and preview a single wallpaper.
*   **GPU-Accelerated Effects**: Add optional, configurable visual effects for startup, key presses, and exit.
*   **Total Customization**: Every color, animation speed, and behavior can be configured in an external text **.conf** file.
* **Thumbnail Caching**: Store pre-processed images locally for faster
 wallpaper retrieval.

## Dependencies

*   **raylib**

## Building

Just run: 

```bash
make
```
This creates `raypaper` executable. If you run `make install`, it will be installed in `/usr/local/bin`

## Usage

The basic command runs the picker, pointing it to a directory of images. Upon selection, it prints the full path of the chosen wallpaper to standard output.

```bash
./raypaper [OPTIONS] [PATH_TO_WALLPAPERS]
```

If no path is provided, it defaults to `~/Pictures/`.

### Options

| Flag                | Argument | Description                                                                     |
| ------------------- | -------- | ------------------------------------------------------------------------------- |
| `--help`              |          | Show the help message and exit.                                               |
| `--filename`          |          | Print only the filename to `stdout` instead of the full path.                 |
| `--clear-cache`       |          | Wipe the on-disk thumbnail cache before starting.                             |
| `--width `          | `<pixels>`  | Set the initial window width.                                                |
| `--height`          | `<pixels>`  | Set the initial window height.                                               |
| `--startup-effect`  | `<name>` | Override the configured startup animation.                                      |
| `--keypress-effect` | `<name>` | Override the configured key press animation.                                    |
| `--exit-effect`     | `<name>` | Override the configured exit animation.                                         |

### Keybindings

| Key(s)                           | Action                                                                  |
| -------------------------------- | ----------------------------------------------------------------------- |
| **Mouse**                        | Hover over thumbnails to highlight them.                                |
| **Mouse Wheel**                  | Scroll through the wallpaper list.                                      |
| **Ctrl + Mouse Wheel**           | Zoom in/out, scaling the thumbnails.                                    |
| **LMB (Left Click)**             | Select the highlighted wallpaper and exit.                              |
| **RMB (Right Click)**            | Show a full-screen preview of the highlighted wallpaper.                |
| **ESC**                          | Exit the program (or close the preview/search).                         |
| `h` / `l` / **Left/Right Arrows**  | Highlight the previous/next wallpaper.                                  |
| `k` / `j` / **Up/Down Arrows**     | Highlight the wallpaper above/below (Grid) or previous/next (other modes). |
| `1`, `2`, `3`, `4`               | Switch between layout modes (Grid, H-River, V-River, Wave).             |
| `/`                              | Enter search mode. Press Enter or ESC to exit search.                   |
| **Enter**                        | Select the keyboard-highlighted wallpaper and exit.                     |
| **Left Shift**                   | Show a full-screen preview of the keyboard-highlighted wallpaper.       |

## Configuration

Raypaper is fully configurable via a plain text file located at:
`~/.config/raypaper/raypaper.conf`

The application will create the directory on first run if it doesn't exist. If the config file is not found, default values will be used.

Here is a template `raypaper.conf` with all available options:

```ini
# Raypaper Configuration File
# Lines starting with # or ; are comments.

[Theme]
# Colors are defined as R, G, B, A (0-255)
bg = 10, 10, 15, 255
idle = 30, 30, 46, 255
hover = 49, 50, 68, 255
border = 203, 166, 247, 255
ripple = 245, 194, 231, 255
overlay = 10, 10, 15, 200
text = 202, 212, 241, 255

[Settings]
# Width
width = 1280
# Height
height = 720
# The maximum number of wallpapers to load from the directory.
max_wallpapers = 512
# The base size of the square thumbnail images.
base_thumb_size = 150
# The base padding between thumbnails.
base_padding = 15
# The thickness of the glowing border on hover.
border_thickness_bloom = 3.0
# The number of threads to use for loading thumbnail images.
max_threads = 8
# The speed of all layout and hover animations. Higher is faster.
anim_speed = 20.0
# The number of particles to emit on selection.
particle_count = 50
# The duration of the Ken Burns (pan/zoom) effect in preview mode.
ken_burns_duration = 15.0
# The maximum frames per second.
max_fps = 200

[Effects]
# Available effects: none, glitch, blur, pixelate, shake, collapse, reveal
startup_effect = blur
keypress_effect = none
exit_effect = glitch
```

## Integration Examples

You can pipe the output of `raypaper` directly into your favorite wallpaper setting command.

### Wayland Compositors

```bash
# Select a wallpaper and immediately set it with swaybg
swaybg -i "$(./raypaper ~/Wallpapers)" -m fill
```

```bash
# With swww
swww img "$(./raypaper ~/Pictures)"
```

### For X11 (using feh)

```bash
# Select a wallpaper and immediately set it with feh
feh --bg-fill "$(./raypaper_x11 ~/Wallpapers)"
```

### In a Shell Script

You can create a simple script to make this even easier.

**`setwall.sh`**
```bash
#!/usr/bin/env sh

WALLPAPER_DIR=~/Pictures/
SELECTED_WALL=$(./raypaper "$WALLPAPER_DIR")

# Exit if no wallpaper was selected (e.g., user pressed ESC)
if [ -z "$SELECTED_WALL" ]; then
    echo "No wallpaper selected."
    exit 1
fi

swww img "$SELECTED_WALL"
```
