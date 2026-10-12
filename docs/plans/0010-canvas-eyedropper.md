# Feature: canvas eyedropper

Status: implemented
Feature IDs: F05
Milestone: M2

## User outcome and scope

Pick an existing artwork color for subsequent drawing. This first M2 slice adds
one original icon to Tools; fill and additional drawing tools remain separate PRs.
See [design](../DESIGN.md) and [architecture](../ARCHITECTURE.md).

## Behavior

Color picker stays selected until another tool is chosen. A left press samples
primary, a right press samples secondary, once per gesture. Dragging does not
paint or resample. Middle/Space drag still pans. Outside/nonfinite positions and
input during open are ignored. Sampling uses the containing image pixel (floor),
not the scaled preview or checkerboard, including its unpremultiplied alpha.
Fully transparent pixels sample transparent black. Color choice does not dirty
artwork, consume undo or invalidate redo. There is no pending picker transaction
to cancel. Switching tools finishes an existing stroke once. No data leaves the
device; palette storage is unchanged. Tool selection is keyboard reachable with
an accessible name and selected state; no letter shortcuts are introduced.

## Implementation

Use an explicit Pencil/Eraser/ColorPicker controller enum to prevent conflicting
tool flags. The controller reads its owned raster on the GUI thread. Canvas keeps
one gesture owner even when sampling changes color settings synchronously. QML
only selects tools and displays colors. No dependencies or format changes.

## Acceptance cases

- [x] Primary/secondary sample exact pixels, including partial/zero alpha.
- [x] Zoomed fractional positions and image edges select the containing pixel.
- [x] Outside/nonfinite/loading input leaves colors and pixels unchanged.
- [x] Picker dragging/button chords/pan never paint or unexpectedly resample.
- [x] Selection and sampling preserve dirty state and undo/redo history.
- [x] Switching from a stroke commits it once; drawing resumes with picked color.
- [x] Keyboard activation and compact light/dark layouts pass shell checks.

## Validation and evidence

Validated on Linux x86_64, Qt 6.8.3, GCC 13:

- `cmake --build --preset dev` and `ctest --preset dev`: all 7 suites passed.
- Controller tests cover partial/zero alpha, dirty/clean and redo preservation,
  bounds/nonfinite positions and loading rejection. Canvas test covers fractional
  zoom, button chords, drag, pan, switching and picked-color drawing.
- Shell keyboard test activates the picker and checks its accessible name and
  selected state, including repeated activation. Isolated X11 shell: 17 results
  passed (optional portal/performance routines inactive for this feature).
- `format-check qml-lint`, repository/privacy checks and 7 Python tests passed.
- `ctest --preset asan`: all 7 suites passed with leak detection enabled.

Synthetic test-window captures visually reviewed at 1024×640 and 1280×800:
[light minimum](../assets/eyedropper-shell-light-1024.png),
[dark minimum](../assets/eyedropper-shell-dark-1024.png),
[light typical](../assets/eyedropper-shell-light-1280.png),
[dark typical](../assets/eyedropper-shell-dark-1280.png).
The three implemented tools temporarily occupy one compact row; the full two-row
Tools inventory follows as its remaining tools land. These checks do not certify
Orca, physical stylus/touch or full reference parity.

## Progress and handoff

Implementation and local validation complete. Merge requires all protected CI
checks. Next M2 slice: contiguous fill and tolerance, with asynchronous cancellation and one
undoable patch. The M2 sequence is tracked in the roadmap.
