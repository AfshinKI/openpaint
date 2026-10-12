# Feature: crop, right-angle rotation and flips

Status: implemented; protected CI required before merge
Feature IDs: F09
Milestone: M2

## Outcome

Image commands rotate right/left 90° or 180°, and flip horizontally/vertically.
With a selection they transform selected pixels into a floating preview at the
selection origin. Enter/Apply commits, Escape cancels; its transformed irregular
mask moves with it. Without a selection they transform the whole image as one
undo command. Crop requires a selection and replaces the image with its bounds;
nonselected pixels inside an irregular crop become transparent.

Workers process immutable snapshots. Busy input is blocked, Escape/Cancel discards
results, and revision checks reject stale work. All operations preserve alpha.
Whole-image commands clear transient selection. Undo/redo restores dimensions and
pixels and refits the canvas when its size changes. Commands exceeding the history
budget are refused, never applied without undo.

Save/Undo/Redo move into the menu row to make room for a compact Image group at
1024×640. Menu actions identify image versus selection scope. No resize/skew or
arbitrary-angle controls are advertised in this slice.

## Acceptance

- [x] Exact pixel mappings for both flips and three rotations; alpha/mask fidelity.
- [x] Crop and whole-image history restore dimensions, pixels, dirty state and redo branches.
- [x] Selected transforms preview/cancel/commit correctly, including irregular masks.
- [x] Invalid/stale/over-budget/cancelled operations preserve the document.
- [x] Image commands and retained quick actions pass keyboard and minimum theme checks.
- [x] Development/sanitizer, formatting/QML/privacy and protected CI pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all ten development and sanitizer suites pass; targeted
transform/shell sanitizer checks also pass after the final no-op crop regression.
Pixel mappings and masks are checked for all five flip/rotation operations.
History tests cover dimensions, saved revision, redo branching, stale results,
budget refusal and cancellation. Crop PNG round-trip preserves exact pixels.
Isolated X11 menu/keyboard tests and strict QML/format/privacy checks pass, as do
seven tooling tests. Synthetic captures: [minimum light](../assets/image-controls-light-1024.png),
[dark Image menu](../assets/image-menu-dark.png).

Next: resize/skew, common image formats, install identity/Updates and packages.
