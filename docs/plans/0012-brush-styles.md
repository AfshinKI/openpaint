# Feature: brush styles and opacity

Status: implemented
Feature IDs: F03
Milestone: M2

## Outcome and behavior

A Brushes dropdown selects nine original raster styles: round Brush, diagonal
Calligraphy brush, narrower Calligraphy pen, soft Airbrush, striated Oil brush,
grainy Crayon, square Marker, textured Natural pencil and soft Watercolor brush.
Menu samples come from the same renderer as artwork. These are independent
implementations, not a claim of matching proprietary paint simulation.

Brush size remains 1–128 pixels. Opacity is 0–100 percent (default 100), multiplied
by the chosen color alpha, and applies once per stroke coverage rather than
accumulating on every pointer event. Repeated distinct strokes can build opacity.
Right mouse uses secondary color. Escape restores the complete prior image;
release or a tool/settings switch commits once. Pencil/eraser behavior is unchanged.
All work stays offline and uses deterministic procedural texture, with no assets
or dependencies obtained from a third party. Stylus pressure remains M4.

## Implementation

A C++ brush rasterizer stamps at fixed distance intervals along the input path,
retaining spacing across events. A bounded one-byte-per-pixel coverage mask uses
maximum coverage, then composites against the pre-stroke snapshot. Off-canvas
segments are clipped before stamp iteration. Existing document patch history
owns commit/cancel. The same renderer generates QML menu previews.

## Acceptance

- [x] Each of nine styles produces a distinct, bounded, repeatable stroke.
- [x] Splitting a straight path into input events does not change its pixels.
- [x] Opacity/alpha, zero opacity, overlap, primary/secondary, cancel and undo pass.
- [x] Out-of-bounds paths do not iterate over unbounded off-canvas distances.
- [x] Dropdown samples, selected state, keyboard and min-size layouts work.
- [x] Development, sanitizer, QML/format and privacy checks pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all seven development and sanitizer suites passed.
The final dropdown changes passed focused X11 keyboard checks and QML lint; the
complete X11 shell run passed 19 results before the final indicator adjustment.
Document tests compare all nine styles, split/unsplit paths, opacity/alpha, zero
opacity, distant clipping, cancel and undo/redo. Controller coverage includes
secondary-color opacity and PNG round-trip. Formatting and privacy checks passed.
Synthetic menus reviewed at minimum and typical sizes:
[light minimum](../assets/brush-menu-light.png),
[dark minimum](../assets/brush-menu-dark.png),
[light typical](../assets/brush-menu-light-1280.png),
[dark typical](../assets/brush-menu-dark-1280.png).

These are procedural brush approximations. Physical pressure, proprietary texture
parity and lower-end latency certification remain unverified; the existing PERF-01
follow-up still applies. Next bounded M2 slice: basic shape previews and commit/cancel.
