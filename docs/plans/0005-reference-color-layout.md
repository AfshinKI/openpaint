# Feature: Reference color layout and compact toolbar

Status: implemented; merge requires all CI jobs to pass
Feature IDs: F01, F05
Milestone: M1 visual refinement

## User outcome and scope

Follow the two maintainer-provided Paint screenshots: rectangular hue/saturation
spectrum, vertical brightness control and preview, HEX plus an RGB/HSV mode
selector, basic/custom color grids, and a compact toolbar with empty custom slots.
These observations supersede the inferred hue-ring layout in plan 0004. Reference
images are not redistributed or bundled; all controls and icons remain original.

## Behavior

The dialog keeps drawing colors as a draft until OK; Cancel/Escape preserve them.
The spectrum spans hue horizontally and saturation vertically (white at bottom).
Brightness is separate. Numeric mode switching preserves color. Basic swatches
select a draft. Twenty-four custom colors persist locally; existing ten-color
palettes load unchanged. The toolbar shows the first ten slots, including empty
outlines. An empty toolbar slot opens the editor without changing drawing color.
The dialog shows all 24 slots; + or an empty slot appends the draft to the next
free slot. Removal compacts slots. Explicit palette changes persist immediately.

Toolbar height becomes 112 logical pixels, icons 20 inside 32px targets, palette
swatches 16 inside 24px targets. Primary/secondary are stacked circles. Dense
palettes retain keyboard focus, accessible names and tooltips. Dialog targets are
24–32px; minimum supported window stays 1024×640. Keep rounded surfaces and themes.
No networking, artwork mutation, new drawing tools or release changes.

## Implementation

Replace ColorWheel with ColorSpectrum using the existing ColorSelection model.
QML owns the brightness slider, numeric mode and grids. Increase ColorPalette's
bound to 24 with the existing version-1 format; keep bounded reads and atomic writes.
Reuse shared controls and original icons. One scoped PR on feat/reference-color-layout.

## Acceptance cases

- [x] Spectrum, brightness, HEX and both numeric modes agree; mode switch preserves draft.
- [x] Empty toolbar slots are visible and open editing without changing artwork/color.
- [x] Basic swatches select drafts; custom add/restart/remove/limit/error behavior passes.
- [x] OK/Cancel and independent primary/secondary editing remain correct.
- [x] Light/dark minimum/typical captures show compact controls with no clipping.
- [x] Keyboard, native fractional-scale, lint, formatting, unit/shell and privacy checks pass.

## Validation and handoff

Debug and ASan/UBSan CTest pass all six suites, including leak detection. All 13
native X11/default-renderer shell cases pass at 125% scale in the final run.
Formatting and zero-warning QML lint pass. Color tests cover 24-color bounds,
version-1 ten-color migration, reload, replacement/removal and storage failures.
Shell tests cover empty toolbar/dialog slots, typed HEX/HSV, spectrum mapping,
brightness Home/End, popup keyboard selection, basic swatches, immediate custom
persistence despite Cancel, independent secondary acceptance and a full palette.

Synthetic light/dark captures at 1024×640 and 1280×800 were reviewed, including
empty/full palettes and the mode popup. Screenshots are project-generated; the
supplied references are not in the repository. PNG metadata was inspected.

A preliminary native run intermittently failed an existing drawing pixel assertion;
subsequent runs, including the final suite, passed. No canvas code or stroke
assertion was changed. Follow up with isolated-display input testing during M1
platform validation. Exact Windows-build parity and Orca/Wayland remain unverified.
The next unfinished step is that M1 validation, not another drawing feature.
