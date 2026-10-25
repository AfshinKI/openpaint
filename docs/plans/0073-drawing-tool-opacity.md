# Feature: opacity for pencil, fill and shapes

Status: implemented; local validation passed; protected review pending
Feature IDs: F02, F03, F04
Target: v2.1.0

## Outcome and acceptance

Expose the existing 0–100% Opacity slider for Pencil, Fill and Shapes as well as
Brush. The shared session value starts at 100%. Eraser/text behavior is unchanged.
Shape changes update the active preview; pencil snapshots opacity at stroke start;
fill captures it before the worker starts. Zero opacity changes no pixels, and
100% retains the existing rendering. Selection clipping, cancel and undo/redo remain.

Operation opacity interpolates premultiplied result pixels with the original.
Pencil retains a full-strength stroke surface for the gesture so overlap does not
reapply opacity. Shape outline/fill are blended together once. Fill still selects
the same connected region and smoothed fringe before opacity is applied.
On opaque backgrounds lower opacity mixes colors; on transparent layers it keeps
partial alpha. No networking or format changes.

## Validation

Acceptance cases cover 0/50/100%, transparent backgrounds, stable pencil overlaps,
live shape preview changes, selection bounds, fill cancellation and undo/redo.
Combined Qt 6.12.0 build, formatting and zero-warning QML lint pass. Document,
editor, canvas, shell-editing and shell-controls CTest suites pass (15.15 seconds).
After normalizing slider geometry, shell-more also passes (6.86 seconds), including
keyboard opacity for every added tool and equal panel/track sizes. The expanded
keyboard test’s final expected value was corrected for its three extra decrements.
Next: protected review and required hosted checks.

The Size and Opacity panels use identical explicit icon, track and handle insets,
removing native-layout height differences. The UI workflow checks equal panel
and slider heights and top offsets.
