# Feature: Paint-inspired visual foundation and custom colors

Status: merged in PRs #7 and #8; reference layout superseded by [plan 0005](0005-reference-color-layout.md)
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
- [x] Color ring, SV area and numeric/HEX fields agree on the selected color.
- [x] OK applies to the chosen color; Cancel/Escape do not change it or artwork.
- [x] Custom colors survive restart; limits, replacement, removal and errors are tested.
- [x] Formatting, QML lint, relevant CTest, privacy checks and screenshots reviewed.

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
Slice 1 merged as [PR #7](https://github.com/AfshinKI/openpaint/pull/7) after all four CI jobs passed.

The X11/default-renderer suite also passes at 125% scale. Test startup now waits
for window activation and a rendered frame, preventing late native pointer events
from contaminating synthetic strokes. Pixel assertions and production input code
remain unchanged. Offscreen checks pass with the same readiness requirement.

Slice 2: ColorSelection keeps hue/HSV/RGB/HEX synchronized, ColorWheel handles the
ring and SV surface, and ColorPalette atomically persists ten opaque colors. QML
owns the draft/accept/cancel interaction. Explicit palette actions persist
immediately; Cancel discards only the drawing-color draft. The UI states this.

Debug and ASan/UBSan CTest pass all six suites with leak detection enabled.
Color tests cover conversion, invalid input, grayscale hue, palette restart,
bounds, replacement/removal, corrupt data and failed writes. UI tests cover typed
HEX, disabled acceptance for incomplete input, ring/SV clicks, Escape, independent
secondary acceptance, no artwork mutation, and a full ten-swatch toolbar at
1024×640. Formatting and zero-warning QML lint pass. Updated synthetic light/dark
captures are in `docs/assets`; no private images or desktop captures are included.

Next unfinished step: complete M1 platform/accessibility and reference comparison
gates from the foundation plan before expanding drawing tools. Wayland/Orca and
versioned Windows comparison are still unverified; this change does not claim
exact Windows parity or packaged releases.

Native X11/default-renderer checks pass at 125% scale (13 cases). Tests synchronize
window-system events after activation and wait for popup/resize frames before
clicking; this also keeps screenshot-enabled and ordinary runs consistent.
