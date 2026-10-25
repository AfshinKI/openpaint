# Feature: invert active-layer colors

Status: shipped in v2.0.0 via merged PR #56
Feature IDs: F09 (image editing extension)
Milestone: M3 development polish


Release reconciliation (2026-10-25): PR #56 merged on 2026-10-22 UTC and this
feature shipped in v2.0.0. [Plan 0062](0062-macos-v2-release.md) records publication
and remaining validation. Below, review/no-release handoffs are historical
checkpoints; their implementation evidence and stated manual limits are retained.

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

## Ribbon artwork follow-up

Acceptance: center Text like Invert colors; use original colored pencil/picker and
blue image icons; color only the fill liquid/drop and brush tip from the selected
color; retain readable light/dark, disabled and minimum-width controls. Preserve
keyboard activation and icon meanings. Implemented with original SVG vectors and
separate tint masks. Rendered-pixel assertions verify red/blue selected paint in
both Fill and Brush, foreground/background switching, transparent overlay hiding,
Text centering and keyboard activation. The workflow passes offscreen/software
and X11/Mesa OpenGL at 100% and 200%. Light/dark captures were visually inspected
and refreshed. Build, formatting, zero-warning QML lint and repository/diff checks
pass. All six shell regression suites passed (55.65 seconds). Protected CI/review remains next; no release.

## Fixed tool artwork refinement

Acceptance: Fill and Brush retain their original fixed artwork colors when the
foreground/background paint changes or becomes transparent. Remove dynamic tint
masks and bindings; preserve the remaining colored icons and Text alignment.
The existing ribbon workflow now checks fixed rendered colors in both themes and
after foreground/background/transparency changes. Build and the focused offscreen
ribbon workflow pass (3 cases including initialization/cleanup), as do formatting,
zero-warning QML lint, repository and diff checks. Next: review on existing PR #56.

## Monochrome Fill refinement

Acceptance: restore the original monochrome bucket, outline its drop when inactive
and fill the drop with the theme foreground when Fill is selected. Selected paint
color must not tint it. Brush retains its fixed colored artwork. Build and focused
ribbon UI checks pass (3 cases including initialization/cleanup), along with
formatting, zero-warning QML lint and repository/diff checks. Next: PR review.
