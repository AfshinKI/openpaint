# Feature: a responsive single-layer drawing canvas

Status: merged in [PR #5](https://github.com/AfshinKI/openpaint/pull/5)
Feature IDs: F01, F02, F05, F11, F16
Milestone: M1 Step 3

## Outcome and behavior

Draw continuous round strokes and erase with the secondary color or explicit
transparent mode. Select size/color in a compact top command strip. Right-button
drawing uses the secondary color; Swap exchanges colors. The bottom bar shows
image/cursor dimensions and zoom. Fit, pointer-anchored Ctrl+wheel, ordinary wheel
pan, Space+drag and middle-button drag share a C++ coordinate transform.

Each drag commits once. Escape restores its original pixels; focus loss, grab
loss, tool change and viewport changes finish at the last received sample. Segments
crossing image bounds clip to the image; re-entry continues from the last sample.
The initial image is white 1152×648. No pixels, input or diagnostics leave the device.
This slice has no file UI; dirty close offers Discard/Cancel until Step 4 lands.

## Implementation

Editor owns the document. CanvasItem renders it with QQuickPaintedItem during Qt's
GUI-blocked synchronization phase and requests dirty-rectangle updates during drags.
It does not retain a QImage snapshot per event (which would force full-image copies).
QML holds controls only. The document has no QML dependency. QtQuick is an explicit
QML module dependency for static type registration and linting.

The layout follows [DESIGN.md](../DESIGN.md). Original text controls are used;
finished iconography, additional toolbar groups, pressure/touch, rulers and exact
Windows reference-build parity remain later work. No Microsoft assets are copied.

## Acceptance and evidence

- [x] Pointer-driven draw → undo → redo restores exact pixels.
- [x] Escape cancels; focus interruption commits without reconnecting on release.
- [x] Right-button secondary drawing and fractional zoom alignment are tested.
- [x] Zoom leaves pixels unchanged and preserves the anchor's document coordinate.
- [x] Minimum/typical light/dark captures reviewed; palette fits at 1024×640.
- [x] Offscreen tests pass at 1× and 1.25×; X11/software tests pass at 1×.
- [ ] Wayland, actual pen input, Orca and GPU performance matrix remain unverified.

Local setup is the [build baseline](../BUILD.md). Debug CTest, formatting and
zero-warning lint and ASan/UBSan with leak detection pass. Synthetic screenshots in `docs/assets` contain only the
app. `OPENPAINT_BENCHMARK=1` runs a 2048×2048 sustained-stroke render probe in the
shell suite, after five warm-up frames, with 60 samples. X11/software, Debug,
Qt 6.8.3, Intel i9-12900KS, 64 GiB RAM, 1× scale, 1280×800 window: sample-to-frame p50 5.42 ms, p95 17.18 ms. This measures
controller-update to frameSwapped, not physical stylus-to-photon latency or a
60 Hz frame-cadence guarantee. The 32 ms latency target passes this narrow probe;
GPU/Wayland and the full 16.7 ms frame budget still need separate measurements.

## Handoff

[CI run 38291376923](https://github.com/AfshinKI/openpaint/actions/runs/38291376923)
passed all four jobs before this slice merged. Start `feat/png-file-workflow`
from merged main. That slice must finish PNG worker snapshots, atomic replacement,
and Save/Discard/Cancel before calling this a usable basic editor.
