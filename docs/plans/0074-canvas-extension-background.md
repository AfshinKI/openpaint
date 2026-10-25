# Fix: canvas expansion background in layered drawings

Status: implemented; local validation passed; protected review pending
Feature IDs: F13
Target: v2.1.0

Corner/edge canvas expansion must use the selected background/fill color,
including its alpha. Single-layer resize already did this; the layered path
incorrectly forced transparent expansion on every layer. Fill new space only on
the bottom background layer, leaving drawing-layer extensions transparent and
preserving existing pixels, layer visibility/opacity and undo history.

Acceptance: opaque white/color, partial alpha and transparent backgrounds work
for both single-layer and layered drawings; shifted original pixels are preserved
and undo/redo restores exact images. Combined Qt 6.12.0 build, formatting and
zero-warning QML lint pass. Layers, transform and shell-editing CTest suites pass.
The new regression covers all background alpha cases in single-layer and layered
drawings. The prior layered-resize expectation was corrected to the documented
background-color rule. Repository checks pass. Next: protected PR review.
