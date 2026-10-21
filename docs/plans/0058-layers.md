# Feature: editable raster layers

Status: implemented; local validation complete; PR #56 open, CI/review pending
Feature IDs: F12, F13, F14
Milestone: M3 bounded layered editing

## User outcome and scope

Add, delete, duplicate, rename, reorder and merge raster layers; control visibility
and opacity; edit the active layer and save/reopen an OpenRaster project. This is
one feature branch at the maintainer's explicit request. Release preparation,
recovery, groups, blend modes and large-document tile storage remain separate.
See [architecture](../ARCHITECTURE.md) and [design](../DESIGN.md).

## Behavior

Start with one white Background layer. New layers are transparent. Selection and
ordinary tools operate on the active layer. Canvas display and export composite
visible layers bottom-to-top with source-over blending. Layer selection does not
dirty content; properties and structural changes each undo as one command.
Layer changes are blocked during active edits/workers. The bottom Background
cannot be deleted or reordered. Merge down requires two visible layers and bakes their opacity.
Save uses .ora when layer structure/properties require it; PNG export is a copy
and never marks the layered document saved. Failed open/save retains artwork.
All editing and project codecs stay offline.

## Implementation

Document owns stable layer IDs and bounded command history. Existing pixel patches
remain targeted to a layer. Structural commands retain implicitly shared snapshots.
The controller exposes actions and immutable snapshots; QML owns no saved pixels.
The panel is toggleable and keyboard-accessible. OpenRaster uses a reviewed ZIP
library, bounded parsing, supported source-over raster stacks and atomic saves.

## Acceptance cases

- [x] Add/delete/duplicate/rename/reorder/merge and visibility/opacity undo/redo.
- [x] Editing changes only the active layer; display/export composite correctly.
- [x] Layer switching cannot retarget a running worker or active preview.
- [x] Whole-document transforms preserve the stack and undo together.
- [x] Save/reopen preserves pixels, order, names, opacity and visibility.
- [x] Unsupported/malformed projects and save failures preserve existing work.
- [x] Saved revisions, redo branching and history budgets remain correct.
- [x] Panel works in both themes and at the minimum supported window size.

## Validation and evidence

Local source validation on Ubuntu 24.04, GCC 13, Qt 6.12.0:

- `cmake --preset dev` and `cmake --build --preset dev --parallel 4`: pass.
  libarchive 3.7.2 runtime was present; development headers came from the matching
  Ubuntu package extracted into temporary local build tooling (no system install).
  Normal builds use the documented `libarchive-dev` dependency.
- `ctest --preset dev --output-on-failure`: all 20 suites passed (81.04 seconds).
- Final focused `openpaint_layers_tests`: 11 Qt cases passed, including init/cleanup;
  stack/history, compositing, limits, save failure/cancellation, malformed projects,
  active-layer fill/selection/picker, whole-stack transforms and round-trip lifecycle.
- Final `openpaint_shell_tests layersWorkflow`: passed, including keyboard add/name
  editing, overlay pointer shielding, both themes and minimum-size layout.
  The same workflow passed isolated X11/software checks at 100%, 125%, 150% and
  200% scale. This does not replace physical-device or compositor certification.
- `cmake --build --preset dev --target format-check qml-lint`: pass, zero QML warnings.
- `python3 tools/check_repo.py`: pass. `python3 -m unittest discover -s tools/tests -v`:
  27 tests passed. `git diff --check`: pass.
- Separate ASan/UBSan build: layers, document, editor, transform and text suites all
  passed with leak detection enabled. The sandbox's ptrace restriction initially
  prevented LeakSanitizer teardown; the same checks passed outside that sandbox.
- Independent Python zipfile/ElementTree/PNG-header checks passed for the synthetic
  project: CRCs, first uncompressed mimetype, layer references, merged dimensions
  and 256×144 thumbnail. This is format validation, not another editor's round-trip.
- Eight 2048×2048 layers: 30 measured CPU composites after five warmups, p50 11.50 ms,
  p95 11.60 ms in the local Debug build. This is a bounded synthetic CPU baseline,
  not physical input latency or the deferred 4096px/eight-layer renderer gate.

Synthetic UI captures: [light 1280](../assets/layers-light-1280.png),
[dark 1024](../assets/layers-dark-1024.png); provenance in [assets](../assets/README.md).

Limits: 32 layers, 128 MiB layer pixels, conservative 256 MiB command history;
flat source-over PNG OpenRaster stacks only. Unsupported groups/blends/off-canvas
content are rejected. GIMP/Krita round-trips, native portal saving, physical desktop
accessibility/scaling, packaged dependency inventory and release validation remain
unverified here. Recovery and large-document tiling are outside this change.

## Progress and handoff

- [x] Read harness and create feat/layers from current origin/main.
- [x] Implement model and tests.
- [x] Integrate tools, panel and project persistence.
- [x] Run local required checks and inspect the diff.
- [x] Publish [PR #56](https://github.com/AfshinKI/openpaint/pull/56) from feat/layers.
- [ ] Complete protected CI/review; no merge or release was performed.
- [ ] Release only in subsequent maintainer-directed work.

## Panel revision: thumbnail rail and fixed background

Requested follow-up on the same branch: replace the form-like layer controls with
an uncluttered right-side thumbnail rail. The first opening on a flat document
adds one undoable transparent layer above the original raster. Toggling the rail
again must not add layers. Keep the original/bottom raster in a separate fixed
background tile below a divider; it cannot be dragged or deleted. Other layers
reorder by dragging, with keyboard/context-menu alternatives. Visibility uses an
original eye icon at the top-left on hover/focus; a small cross marks hidden layers.
Additional layer commands belong to context menus, not persistent text buttons.
Keep safe save/reopen, hidden data, undo/redo, busy guards and existing artwork.

Acceptance for this revision:

- [x] First opening adds exactly one transparent layer; reopening adds none.
- [x] Background stays at the bottom through reorder/delete/undo/save/reopen.
- [x] Hover/focus eye toggles the intended layer without requiring preselection.
- [x] Pointer dragging reorders ordinary layers once on drop; cancel changes nothing.
- [x] Context menus and keyboard retain add/duplicate/delete/rename/opacity/merge.
- [x] Light/dark, minimum-size and scaled UI checks pass; captures are refreshed.

Revision evidence (2026-10-21): the focused layers suite passed all 12 Qt cases
(including initialization/cleanup). The revised shell workflow passed actual
pointer reorder, undo/redo, held-pointer Escape cancellation, background pinning,
hover visibility without selection changes, context menu opening, F2 properties,
keyboard layer creation and panel input shielding. It also passed X11/software
at 100%, 125%, 150% and 200%. Refreshed both synthetic development screenshots
and visually inspected them. All 20 CTest suites passed (83.39 seconds). Build,
formatting, zero-warning QML lint, repository checks, 27 tooling tests and
`git diff --check` passed. No release or merge was performed; protected CI/review
is the next unfinished step on the same PR.

UI reference: Microsoft's [layer announcement](https://blogs.windows.com/windows-insider/2023/09/18/paint-app-update-adding-support-for-layers-and-transparency-begins-rolling-out-to-windows-insiders/)
and [background tile update](https://blogs.windows.com/windows-insider/2024/03/13/paint-update-begins-rolling-out-to-windows-insiders/).
The fixed original raster and automatic initial transparent layer follow the
maintainer's requested behavior; no exact version-wide Paint parity is claimed.
All icons are original project vectors.
