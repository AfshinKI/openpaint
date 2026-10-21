# Feature: editable raster layers

Status: implemented; local validation complete; PR/CI pending
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
Layer changes are blocked during active edits/workers. The last layer cannot be
deleted. Merge down requires two visible layers and bakes their opacity.
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
- [ ] Publish a focused PR and complete protected CI/review.
- [ ] Release only in subsequent maintainer-directed work.
