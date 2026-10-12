# Feature: Paint-inspired visual foundation and custom colors

Status: in progress, before further drawing tools
Feature IDs: F01, F05, F26

## Requirements

Replace the temporary text-heavy toolbar with original 24-unit line icons. Keep
visible group captions, named accessibility actions, tooltips with shortcuts,
keyboard focus, disabled states and clear selection. Reuse existing document
commands; appearance changes must not dirty artwork or change shortcuts.

Rounded geometry is a requirement, not optional decoration: 6 px control corners,
8 px toolbar/menu surfaces, 12 px dialogs, and circular color swatches. Menus use
rounded hover/selection rows inset from their rounded surface. Native window
frames and native file choosers remain under desktop control. Keep the actual
image rectangle square: rounding artwork would hide pixels.

Use consistent light/dark tokens and subtle borders, at least 32 px targets, and
comfortable spacing at 1024×640 and 1280×800. Do not copy Windows icons/fonts or
fake Windows window controls. These are independent Paint-inspired requirements;
exact parity needs a versioned Windows reference audit.

The color editor must be app-owned and consistent across desktops: circular hue
ring, saturation/value area, numeric H/S/V and R/G/B, editable HEX, original/current
preview, and explicit OK/Cancel. Color edits are a draft until accepted; Escape
and Cancel preserve the prior drawing color. Hue remains meaningful for grayscale.
Primary and secondary colors are independently editable. Store up to ten custom
swatches locally, with add/replace/clear controls and graceful persistence errors.
No artwork or filenames belong in the palette settings; no network is involved.

## Scoped delivery

1. `feat/icon-toolbar`: shared rounded controls/menu/dialog surfaces, original
   icons, toolbar integration, screenshots and regression checks.
2. `feat/custom-colors`: dedicated hue/HSV/RGB/HEX editor and locally persisted
   custom swatches, with behavioral/persistence tests and updated screenshots.

Merge each PR after all CI jobs pass before starting the dependent slice. Do not
add fill, shapes, selection or other drawing tools to either branch.

## Acceptance

- [x] Existing actions use icons with tooltips/accessibility names and retain shortcuts.
- [x] Rounded controls, menus and dialogs work in light/dark at both window sizes.
- [x] Focus, hover, disabled and selected states are distinct; no toolbar clipping.
- [ ] Color ring, SV area and numeric/HEX fields agree on the selected color.
- [ ] OK applies to the chosen color; Cancel/Escape do not change it or artwork.
- [ ] Custom colors survive restart; limits, replacement, removal and errors are tested.
- [ ] Formatting, QML lint, relevant CTest, privacy checks and screenshots reviewed.

## Evidence and handoff

Record implementation and validation below. Native compositor rounding, Orca and
Windows-build pixel parity are separate platform/reference checks, not implied by
a passing offscreen suite.

Slice 1: original SVG icons and shared PaintButton/IconButton/PaintMenu/
PaintMenuItem/PaintDialog/PaintSpinBox/PaintCheckBox components are implemented.
Debug CTest (five suites), formatting and zero-warning lint pass. Synthetic
light/dark screenshots at 1024×640 and 1280×800 plus menu/dialog captures were
reviewed. A menu-width regression assertion prevents collapsed popup surfaces.
Icons in `qml/icons` are original project-authored GPL-3.0-or-later artwork.
Slice 2 remains next after this branch passes CI and merges.
