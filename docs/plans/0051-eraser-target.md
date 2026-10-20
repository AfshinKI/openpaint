# Feature: precise eraser target and local mode menu

Status: implemented; local checks passed; PR/release deferred
Feature IDs: F02, F01

## Scope and acceptance

- The eraser cursor has a small contrast-edged + centered on its actual hotspot.
- Paper mode erases to opaque white, independently of the secondary/background
  drawing color and mouse button. Transparent mode clears alpha.
- Both modes remain undoable, with cancellation preserving the original pixels.
- A small dropdown beside the eraser selects Paper white or Transparent. Remove
  the separate eraser ribbon group; keyboard access and selected mode remain clear.
- Preserve the previous batch's glass, font, caret and Linux corner behavior.

CanvasItem owns cursor rendering; Editor chooses the stroke color through the
existing undoable document stroke. QML owns only mode selection. No persistence,
network or format changes. Paper is white in the current single-layer editor;
this is independent of the user-selected background/fill swatch.

## Validation and handoff

Dev build, format-check, strict QML lint and repository checks passed.
`eraserUsesPaperOrTransparency` verifies opaque white and alpha clearing with a
green secondary swatch, both mouse buttons, untouched neighbors, undo/redo and
cancellation. UI checks verify keyboard mode changes, reselecting the eraser,
unchanged artwork/history, cursor hotspots and restored hand/tool behavior.

All 15 CTest suites passed (`ctest --preset dev --parallel 4`, 24.06 seconds).
Focused UI checks passed again after preserving selected state on repeated eraser
clicks. The 2× hidden OpenGL run passed all 4 cases including init/cleanup.
Synthetic dropdown/cursor captures in build/evidence-eraser were visually reviewed.

Two original SVG icon concepts, each with light/dark variants, were prepared and
visually reviewed in build/icon-concepts. They are proposals, not installed assets.
The maintainer selected Open palette; implementation and eraser state glyphs are
recorded in [0052](0052-icon-and-spline.md). PR/release remain deferred.
