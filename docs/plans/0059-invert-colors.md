# Feature: invert active-layer colors

Status: implemented; local validation complete; PR #56 review pending
Feature IDs: F09 (image editing extension)
Milestone: M3 development polish

## User outcome and scope

Add an Invert colors control with an original contrast icon in the Image ribbon
group. Continue on the maintainer-requested feature branch. Selection inversion
remains a separate operation. No new format, dependency or networking is needed.

## Behavior

Invert RGB on the active visible layer, clipped to any selection, preserving alpha,
selection geometry and other layers. One undoable edit. Disable during workers or
active previews. Cancellation and failed history commits preserve original pixels.

## Implementation

Use the existing transform worker and revision guard, adding a color-only mode
that commits an active-layer patch instead of transforming the layer stack.
Premultiplied components invert as alpha minus component; alpha remains unchanged.

## Acceptance cases

- [x] Invert opaque/partial-alpha/transparent pixels, preserving alpha and metadata.
- [x] Selection holes and other layers remain unchanged; undo/redo restore pixels.
- [x] Hidden layers, active previews and cancellation cannot mutate pixels.
- [x] Ribbon control is keyboard accessible and fits at minimum window size.

## Validation and evidence

Qt 6.12.0 / GCC 13 development build, format-check and zero-warning QML lint pass.
All 15 transform cases pass, including alpha/mask inversion, active-layer isolation,
hidden/active-edit guards, cancellation, undo/redo, history-budget failure and PNG
save/reopen. Focused keyboard/UI and existing Layers workflows pass offscreen and
X11/Mesa OpenGL at 100% and 200%; the minimum-size ribbon remains within bounds.
Repository checks and diff hygiene pass. All 20 CTest suites passed.

Synthetic [ribbon capture](../assets/invert-colors-ribbon.png) and refreshed layer
captures were visually inspected; provenance is in [assets](../assets/README.md).

## Progress and handoff

Implemented on feat/layers per the ongoing one-branch request. Published to existing
PR #56; protected CI/review is the next unfinished step. No merge or release.
