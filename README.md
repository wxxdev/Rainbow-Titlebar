# Rainbow Titlebar

Animated RGB colors for Windows title bars using Windhawk.

![Preview](screenshots/preview.png)

## Features

- Smooth RGB animation
- Automatic color cycling
- Lightweight
- Restores default colors when disabled

## Installation

1. Install [Windhawk](https://windhawk.net/)
2. Create a new mod
3. Copy `src/rainbow-titlebar.cpp`
4. Paste it into the Windhawk editor
5. Compile and enable the mod

## Speed

Edit this line in `rainbow-titlebar.cpp`:

```cpp
int p = (int)((time / 2) % 1536);

Lower value = faster animation.

## Compatibility
Windows 10 / 11.

Some applications may not support custom title bar colors.