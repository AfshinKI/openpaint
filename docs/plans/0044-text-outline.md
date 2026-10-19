# Feature: dashed text outline and black caret

Status: implemented; local checks passed; PR deferred
Feature IDs: F07

## Acceptance

The active text box uses a one-screen-pixel black dashed outline over white,
remaining visible over dark and light artwork. The black caret blinks every 500 ms,
resets when moved, scales to the text height, and hides with focus loss/selection.
These overlays never enter saved artwork or undo history. Existing resize handles,
inline text, commit, cancel and undo remain intact.

## Evidence

Qt 6.12 dev build, strict QML lint, repository checks, inlineTextWorkflow and
cornerResizeHandles passed offscreen/software. Inspected the generated text capture:
black dashed outline and caret are visible; commit/undo and resize tests pass.
Next: later PR/package validation.
