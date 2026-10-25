# Feature: fill disconnected matching colors

Status: implemented; combined-preview validation passed
Feature IDs: F04
Target: v2.1.0

## User outcome and behavior

Fill retains contiguous fill by default and offers Replace matching color from a
16-unit dropdown. The selected mode changes the original toolbar icon. Matching
is exact, with existing smooth boundary treatment; tolerance remains out of scope.
Both modes use the active layer and selection, primary/secondary mouse-button
colors, and the existing cancellable worker and undo command. No network access.
Mode is session-only. Busy and active text editing disable the controls.

## Implementation

An optional allMatching argument seeds all matching islands before the existing
bounded fill traversal. Worker captures the mode at launch. Document mutation
still occurs through the existing revision-checked fill completion command.
The new fill-all vector is original geometric OpenPaint artwork, GPL-3.0-or-later.

## Acceptance cases

- Contiguous fill stops at barriers; global fill reaches disconnected matches.
- Selection and transparency are preserved; undo/redo restore the whole edit.
- Cancel/no-op does not modify the document.
- Menu switches modes and icons; keyboard opening remains available.

## Validation and handoff

Combined preview: build, format-check, zero-warning QML lint and repository
checks passed. Full document/editor suites passed; fillModeMenu, minimum-width
and keyboard-control workflows passed (5 Qt cases including harness). New
matchingFillMode covers disconnected areas, transparency, selection, undo/redo,
cancellation and no-op. Next: review the scoped branch and CI. Opacity is supplied by the separate v2.1
opacity change in the combined preview.
