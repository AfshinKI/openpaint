# Feature: canvas edge resizing and border removal

Status: merged in PR #42; shipped in v1.1.0
Feature IDs: F09, F01
Milestone: post-v1 fixes

## Behavior and implementation

Add four midpoint handles to the idle canvas. Left/right change width only;
top/bottom change height only. The opposite edge stays fixed. Retain corner
resizing, 16px targets, keyboard arrows/Shift, preview cancellation, worker bounds
and one undo step. Active shapes/text/selections retain their four corners.
Remove the decorative canvas outline without changing saved pixels or checkerboard.
No data leaves the device. QML delegates resize through existing C++ commands.

## Acceptance cases

- [x] All four edges constrain the perpendicular axis and undo/redo pixel-exactly.
- [x] Keyboard resizing and existing corner cancellation pass.
- [x] Rendering leaves pixels outside the image transparent.
- [x] Build, lint, formatting, targeted tests and synthetic screenshot inspected.

## Validation and handoff

Dev build, repository checks, formatting, QML lint, canvas/transform/shell tests
passed. Inspected generated canvas-edge-handles.png (offscreen/software, 1280×800).
Real desktop validation remains outstanding. Protected PR review completed. No dependency on selection dismissal,
glass, fill, or update-notification changes.

## Release closure

Merged in [PR #42](https://github.com/AfshinKI/openpaint/pull/42) and included
in [v1.1.0](0038-release-1.1.0.md). The validation limits above remain explicit;
publication does not certify additional desktop environments.
