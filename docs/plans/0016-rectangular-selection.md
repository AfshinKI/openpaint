# Feature: rectangular selection and drawing boundary

Status: implemented; protected CI required before merge
Feature IDs: F08, F04
Milestone: M2

## Outcome

Drag a rectangular selection, select all, enter numeric bounds, or deselect. A
black/white marquee identifies the selected pixels. Selection is transient UI
state: creating, replacing or clearing it does not change pixels or history.
Escape cancels an unfinished selection; otherwise it cancels a tool preview or
clears the selection. New/open resets selection. Rectangle edges use whole pixels.

Pencil, eraser, all brushes, fill, shapes and text respect the same selection.
Fill receives an immutable mask and cannot traverse outside it. Delete clears
selected pixels to transparency in one undo command. No selection means unrestricted
drawing, and Delete is disabled. Selection changes are disabled while loading,
filling or a retained tool preview is active. Undo/redo preserves the boundary.

This first slice uses a binary QRegion mask; feathering is not part of M2. Numeric
bounds replace the rectangle. Clipboard, movable content and free-form selection
are separate follow-up PRs; no nonfunctional menu items advertise those yet.

## Implementation and acceptance

Document owns the clipping mask and rejects changes during pixel transactions.
QPainter clips strokes/preview patches; procedural brushes test mask membership.
The worker gets a copied region. Text uses the clipped raster preview beneath a
transparent inline input surface when a selection is active.

- [x] Rectangle creation/cancel/bounds/select-all/deselect never change history.
- [x] All tools clip; fill cannot escape or bridge through unselected pixels.
- [x] Delete preserves outside pixels and alpha, with undo/redo and no-op cases.
- [x] Busy/preview guards and new/open reset prevent stale selection state.
- [x] Pointer/keyboard workflow and minimum light/dark layout pass.
- [x] Development/sanitizer, formatting/QML/privacy and protected CI pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all eight development and ASan/UBSan suites pass.
Tests cover reverse drags, selection cancellation, rejected bounds, busy/preview
guards, all nine brush styles, standard strokes, preview clipping, selected text,
disconnected fill regions, deletion alpha and undo/redo. Isolated X11 pointer
and keyboard selection/Delete/Undo/Select-all/Deselect pass. Format, strict QML
lint, privacy/link checks and seven tooling tests pass. Reviewed synthetic
minimum-window captures: [light menu](../assets/selection-menu-light.png),
[dark bounds](../assets/selection-bounds-dark.png).

Next: move/copy/cut/paste with bounded clipboard import; free-form selection,
then transforms, common formats and downloadable alpha validation.
