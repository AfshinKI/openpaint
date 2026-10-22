# Feature: right-drag canvas panning

Status: implemented and locally validated
Feature IDs: F11

## User outcome and scope

Hold the right mouse button and drag the canvas with a closed-hand grab cursor.
Continue on the maintainer-requested feature branch; no merge/release.

## Behavior and implementation

CanvasItem routes a right-initiated gesture to existing viewport panning before
tool sampling or drawing. This replaces right-button secondary painting/picking.
Middle-button and Space panning remain. Release includes the final pointer
position; focus/grab loss ends the gesture and restores the tool cursor.
Extra buttons cannot steal an existing stroke. Palette/layer menus are unchanged.
No saved pixels, undo commands, file formats or network behavior are added.

## Acceptance cases

- [x] Right-drag moves the viewport through release and shows a closed hand.
- [x] Release/focus loss/Escape restore the cursor; later hover cannot keep panning.
- [x] Pencil, Fill, Picker and Text do not paint/sample/start previews during pan.
- [x] Pixels, history, existing chord ownership and coordinate mapping stay intact.

## Validation and handoff

Build, formatting, zero-warning QML lint, repository/diff checks pass.
All 20 CTest suites pass (87.85 seconds), including 30 canvas input cases.
The focused interruptedStrokeAndRightPan workflow passes both offscreen/software
and X11/Mesa OpenGL at 100% scaling (3 cases each including setup/cleanup).
Tests verify release position, hand cursor lifecycle, no drawing/sampling/history
changes, retained shape previews and chord ownership.
Next: review on existing PR #56. No merge or release.
