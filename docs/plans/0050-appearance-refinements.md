# Feature follow-up: clearer tool feedback and restrained glass

Status: implemented; local checks passed; PR/release deferred
Feature IDs: F01, F07, F25

## User outcome and scope

Refine the local v2 UI batch from plan 0049 using maintainer screenshot feedback.
Retain the original 40–100% opacity range and 40% default. Remove the experimental
0–100% tint control and restore the prior searchable, per-family font previews.
Flat idle toolbar buttons must not become bright tiles. No application icon,
package installation, release dispatch or PR is included.

## Acceptance

- [x] Saved glass values survive; reset uses 40%, the minimum is 40%, no tint UI.
- [x] Flat idle buttons stay clear; selected/hover/focus surfaces use neutral frost.
- [x] Linux bottom client corners alone are rounded; maximized/fullscreen square.
- [x] An empty text box renders a black caret immediately and visibly blinks before
  typing; selection, editing, undo/redo and commit/cancel remain intact.
- [x] Font previews use the previous layout; keyboard Apply/Cancel remains working.
- [x] Pencil, brush, eraser and eyedropper show distinct native canvas cursors with
  matching tip hotspots and white contrast edges. Hand panning restores the tool.
- [x] Solid fill shows a filled drop; disabled shape outline shows a small × badge.
- [x] Checkmarks are frosted neutral colors; selected color swatches get bright outer
  rings without changing their actual colors or creating document/history edits.

## Implementation and privacy

Shared QML controls own visual feedback; CanvasItem selects cached native cursors
on the GUI thread using existing original SVG resources. No new dependency,
document format, saved-pixel ownership or network behavior is introduced.
The clearer pencil/eyedropper SVGs retain the 24px/1.6px rounded-stroke family.
The caret is an explicit TextEdit child so Qt's empty cursor-delegate wrapper
cannot hide it. Bottom input masks apply only under Q_OS_LINUX.

## Validation and evidence

- Dev build, format-check and strict QML lint passed on Qt 6.12.
- Repository checks passed (303 files); all 27 tooling tests passed.
- Focused software UI run passed all 8 cases including init/cleanup.
- Hidden-window OpenGL run passed all 14 cases with no skips: cursors/indicators,
  fonts, empty text caret, corners, glass settings, selected frost, shape menus,
  color editor, brush menu, floating blur and light/dark menu blur.
- A separate 2× scale cursor/indicator run passed all 3 cases including init/cleanup.
  It checks distinct bitmap cursors, hotspots, Space/middle-button hand restoration,
  bright swatch rings, idle button transparency, and unchanged artwork/history.
- Synthetic captures in ignored build/evidence-v2-final and build/evidence-v2-hidpi
  were inspected for clear idle shapes, state icons, rings and restored font previews.
- `ctest --preset dev --parallel 4`: all 15 suites passed (24.78 seconds).
  An earlier sequential run was interrupted; this completed run is the final evidence.

Tests use synthetic artwork; no personal files or desktop captures are committed.
Earlier superseded tint/compact-font runs are not evidence for this final behavior.

## Handoff

Next: the new application icon, then maintainer-directed PR/release-v2 checks.
Installed package and Wayland validation remain pending. Native title-bar corners
and desktop blur remain compositor-owned. No release or PR was created.
