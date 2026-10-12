# Feature: editable line, rectangle and ellipse previews

Status: implemented
Feature IDs: F06
Milestone: M2

## Behavior and scope

Choose Line, Rectangle or Ellipse from Shapes. Drag establishes the geometry;
release retains a preview. Enter or Apply commits one undo step; Escape or Cancel
restores the original. Clicking a new canvas position or switching tools commits
the previous shape. Save/New/Open/Close resolve the preview using the existing
commit-and-dirty-guard path. A grab/focus/viewport change ends the drag but retains
the preview, allowing toolbar options to change it.

Outline uses primary color and fill secondary; right-button drag swaps them.
Default outline on, fill off, current stroke width. The options dialog changes
start/end coordinates and outline/fill while previewing. Coordinates are bounded
and clipped to the image. No-fill and no-outline is a reversible no-op. Shape
settings never dirty artwork when no preview exists. No network access.

This slice excludes additional palette geometry, curved-line control points,
polygon point entry and constraint modifiers, which follow in the palette slice.
It does not introduce persistent vector objects.

## Implementation

Document exposes a bounded preview transaction: immutable original plus replacement
patches, committed by existing history. Shape rasterization returns only the
clipped patch. Old preview pixels are restored before each new patch, avoiding
stroke accumulation. Controller owns pending shape state; QML displays options.
Canvas distinguishes ending pointer input from committing a retained preview.

## Acceptance

- [x] All three geometries have expected outline/fill and clipping behavior.
- [x] Preview updates remove old pixels; cancel and one-step undo/redo are exact.
- [x] Release/focus/zoom retain preview; Enter/Escape and switching resolve it.
- [x] Numeric options and colors update the preview without duplicate history.
- [x] Save/reopen, no-op and history-budget failures preserve correct pixels.
- [x] Keyboard, minimum-size themes, sanitizers and repository checks pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all seven development suites passed, with document
preview replacement/cancel/history/budget cases, controller PNG round-trip and
canvas release/focus/zoom/Enter/Escape/switch coverage. The isolated X11 shell
passed 20 results, including keyboard shape selection, numeric options, Apply and
Escape. Formatting, QML lint and repository/privacy checks passed.
Minimum-window synthetic captures reviewed:
[light](../assets/shape-options-light.png), [dark](../assets/shape-options-dark.png).
Sanitizer validation is recorded with the PR checks before merge.

Next slice extends the shape palette and constrained input. Exact reference
handle placement and physical input/accessibility certification remain unverified.
