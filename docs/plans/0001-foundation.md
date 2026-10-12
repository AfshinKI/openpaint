# M1: first working canvas

Status: ready for implementation; not started
Feature IDs: F01, F02, F05, F11, F15, F16, F26 (initial subsets)
Dependencies: M0 documentation and repository tooling

## Outcome

A Linux user can launch a native window, draw and erase, undo/redo, open/save a
PNG, and close the app without accidentally losing unsaved work. This is an
engineering checkpoint, not a public stable release or a complete Paint clone.

Contracts: [architecture](../ARCHITECTURE.md), [design](../DESIGN.md), and
[quality](../QUALITY.md). M1 omits layers UI, advanced brushes, selections, text,
AI, project files, packaging publication, and automatic updates.

## Step 1 — reproducible native shell

- [ ] Add a root CMake project, explicit C++20/Qt requirements, and `dev`, `release`,
  `asan` configure/build/test presets with out-of-source output.
- [ ] Pin tested toolchain/Qt versions and provide installation instructions for
  one development environment; establish an additional compiler lane in CI.
- [ ] Add the app entry point, QML window, light/dark tokens, application actions,
  system font, and translated labels. Do not draw fake functioning editor controls.
- [ ] Add clang-format, QML lint/format checks, and a real CTest smoke test.
- [ ] Verify launch/close offscreen in CI and in a real Linux desktop session.

Acceptance: a clean checkout builds with the documented commands, launches a
window, and closes successfully. Test discovery is nonempty and CI fails on an
intentional broken smoke assertion. Record exact environment and commands below.

## Step 2 — headless document and history

- [ ] Introduce single-layer Document, raster surface abstraction, stable revision,
  saved revision, and undo stack with a memory budget.
- [ ] Implement stroke pixel patches and clear-alpha eraser commands; opaque
  eraser mode is explicit. Deterministically generate fixtures in tests.
- [ ] Test apply/undo/redo equivalence, redo invalidation, history trimming, saved
  revision tracking, and invalid image dimensions/overflow.

Acceptance: engine tests run without loading QML or making network requests.
Each stroke yields one history step; undo to the saved state clears the dirty flag.

## Step 3 — canvas and input

- [ ] Implement the initial QQuickPaintedItem adapter over document snapshots.
- [ ] Normalize pointer coordinates/pressure and centralize viewport transforms.
- [ ] Connect pencil, eraser, size, primary/secondary color, zoom/pan and status UI.
- [ ] Cover Escape cancellation, pointer capture/loss, out-of-bounds movement,
  tool changes, keyboard focus, and fractional device scaling.
- [ ] Record stroke/viewport timing and memory on the quality baseline; log a
  renderer follow-up if the adapter cannot meet the target.

Acceptance: pointer marks align with the visible canvas at multiple zoom and scale
factors; a stroke crossing outside then returning has defined behavior. Undo/redo
works from menu and keyboard. No QML pixel buffer owns the saved image.

## Step 4 — trustworthy PNG lifecycle

- [ ] Decode/encode PNG with size limits, orientation/color-space normalization,
  metadata stripping, and alpha preservation. Use a worker snapshot for saving.
- [ ] Add native/portal Open, Save and Save As, current filename, and dirty marker.
- [ ] Implement New/Close Save–Discard–Cancel, and preserve originals on failure.
- [ ] Test unwritable target, failed commit, malformed/oversized PNG, cancel, and
  editing while a snapshot is saving. Test portal behavior on a real desktop.

Acceptance: draw → save → reopen is pixel-equivalent for a lossless sRGB fixture.
A failed save keeps both the original file and unsaved document intact. A save of
an earlier revision does not clear edits made while it ran.

## Step 5 — close the gate

- [ ] Run documented build, unit/integration, formatting, sanitizer, and repo checks.
- [ ] Complete a keyboard-only shell/file workflow and light/dark size review.
- [ ] Record real desktop smoke tests and performance measurements.
- [ ] Update feature rows only for the implemented subsets; publish honest developer
  build instructions, screenshots, and limitations. Keep later gates open.

Acceptance: every required M1 case has evidence and no reproducible data-loss bug
remains. Code is reviewable without private files or machine-specific settings.

## Evidence and handoff

No application validation yet. Next action: Step 1.
Record commands/results, environmental limitations, key decisions, and the next
unfinished checkbox here as implementation proceeds. Do not copy private terminal
paths or complete agent conversations into this log.
