# Feature: floating shape actions

Status: implemented; local validation passed; protected review pending
Feature IDs: F01
Target: v2.1.0

## Outcome and acceptance

While a new shape preview is active, show a compact glass toolbar below the ribbon
(and below rulers when enabled), matching the text toolbar. Move Options, Apply
and Cancel out of the footer for shapes. Keep selection and text workflows intact.
The toolbar disappears after commit/cancel; existing Enter/Escape shortcuts and
undo/redo behavior remain. No pixel mutations move into QML and no networking is added.

Acceptance: only shape previews show the toolbar; it clears the ruler, supports
keyboard Options and mouse Apply/Cancel, and preserves undo/redo/cancel pixels.

## Validation

Combined Qt 6.12.0 Debug build, formatting and zero-warning QML lint pass.
The floatingShapeToolbar and shapeOptionsKeyboard workflows pass (4 Qt cases).
Shell-editing, shell-text and shell-controls suites pass (17.73 seconds), covering
selection/text regressions as well as shape editing. The generated light-theme
capture was inspected. Tests verify toolbar visibility, ruler clearance, mouse
Apply/Cancel, undo/redo and keyboard Options. Repository checks pass.
Next: protected PR review and required hosted checks.
