# Feature: rounded application client corners

Status: implemented; local checks passed; PR deferred
Feature IDs: F01

## Scope and acceptance

The application-owned client surface has 12px rounded transparent corners in normal
windows, including menu/footer backgrounds and modal dimming. Maximize/fullscreen
restores square edges. The input mask follows resize/window state, matching the
visual corners. Opaque appearance still clears the exterior corners. Artwork stays
rectangular and unchanged; native title bars/decorations remain compositor-owned.

- [x] All four captured client corners are transparent in light/dark modes.
- [x] Resize and maximize/restore update mask/radius; controls remain usable.
- [x] Existing appearance/launch tests and document state remain unchanged.

Qt's mask is only an input hint; visuals are cleared by QML backgrounds, not by
assuming the window manager clips them. See [QWindow](https://doc.qt.io/qt-6/qwindow.html#setMask).
Full native-decoration appearance on GNOME/KDE/Wayland requires desktop review.

Dev build, strict QML lint, roundedClientCorners, launchAndResize and
glassAppearanceSettings passed with Qt 6.12/offscreen software. Inspected generated
light/dark captures; pixel assertions verify all four clear corners. Offscreen
reports unsupported native masks, so native input-region enforcement still needs
desktop/package review. Next: later PR validation.

Follow-up: [0050](0050-appearance-refinements.md) limits rounding to Linux bottom
client corners; the top corners remain square.
