# Feature: optional canvas alignment guides

Status: implemented; local combined validation passed; protected review pending
Feature IDs: F01
Target: v2.1.0

## User outcome and scope

View → Smart guides enables temporary magenta alignment lines and snapping while
moving a selection, pasted image, shape preview or text preview. It starts off and remembers the user’s choice
across application sessions. Targets are canvas edges and center. Automatic detection
of objects painted into a layer, spacing guides and ruler-dragged guides are outside
this slice. See DESIGN.md, MENUS.md and ARCHITECTURE.md.

## Behavior

Snap within six logical screen pixels, independently per axis. Ctrl bypasses snapping.
Only exact integer-pixel placements snap; parity-mismatched centers remain unsnapped.
A small move grip above active bounds allows moving shapes/text without taking over
text selection. Arrow keys nudge one pixel; Shift nudges ten, without snapping.
Existing selection dragging also supports guides. Release, cancel, commit and zoom
clear the lines. Escape cancels the existing preview. All pixel edits retain the
existing preview/commit/undo commands; guides never enter artwork or exports.
Everything operates offline. Rulers remain independently toggleable.

## Implementation

Editor owns snapping geometry and moves the existing previews. CanvasItem draws
transient lines and converts screen tolerance into image coordinates. QML only
routes input and exposes the optional toggle and accessible move grip.

## Acceptance cases

- Default-off movement stays exact; enabled movement snaps closest edges/centers.
- Both axes work, Ctrl bypass works, screen tolerance follows zoom.
- Text and shape moves preserve dimensions; selection commit can be undone.
- Cancel restores pixels; release/disable clears guides without changing pixels.
- View toggle and move-grip keyboard operation are accessible.

## Validation and handoff

Qt 6.12.0/GCC Debug combined preview build passes with PRs #59–#63.
All 20 CTest suites pass across the full run and targeted editor/text rerun.
The first run exposed a font test placed in the headless editor harness; moving
that case to the existing GUI text suite fixed the harness error. Runtime code
was unchanged. All 44 combined tooling tests pass, as do format-check and
zero-warning QML lint. The standalone feature branch has 42 tooling tests.

Cases: smartGuideMoves (paste/selection, edges/center, bypass, parity, undo/cancel),
smartGuideShapeMoves, smartGuideTextMoves, smartGuideZoomAndBypass (50/100/200%
zoom), and smartGuidesWorkflow (View toggle, real grip drag, keyboard and Escape).
Focused combined shell checks pass on isolated X11/Mesa at 125% scale (10 Qt
cases) and offscreen at 200% (6 cases). Synthetic captures were inspected in both
themes. No live desktop input, artwork or user settings were touched.

Next: required hosted checks and protected review/merge; v2.1 release preparation
and package/physical Mac validation remain separate.

View → Smart guides now persists its state in local settings independently of
glass preferences. A temporary-settings regression verifies both values across
fresh application-settings instances.
