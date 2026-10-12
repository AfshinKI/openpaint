# M1: first working canvas

Status: in progress; basic PNG editor and mouse input reliability implemented; remaining platform/codec validation stays open
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

- [x] Add a root CMake project, explicit C++20/Qt requirements, and `dev`, `release`,
  `asan` configure/build/test presets with out-of-source output.
- [x] Pin tested toolchain/Qt versions and provide installation instructions for
  one development environment; establish an additional compiler lane in CI.
- [x] Add the app entry point, QML window, light/dark tokens, application actions,
  system font, and translated labels. Do not draw fake functioning editor controls.
- [x] Add clang-format, QML lint/format checks, and a real CTest smoke test.
- [x] Verify launch/close offscreen in CI and in a real Linux desktop session.
  Local offscreen/X11 and the GitHub compiler matrix passed; see evidence below.

Acceptance: a clean checkout builds with the documented commands, launches a
window, and closes successfully. Test discovery is nonempty and CI fails on an
intentional broken smoke assertion. Record exact environment and commands below.

## Step 2 — headless document and history

- [x] Introduce single-layer Document, raster surface abstraction, stable revision,
  saved revision, and undo stack with a memory budget.
- [x] Implement stroke pixel patches and clear-alpha eraser commands; opaque
  eraser mode is explicit. Deterministically generate fixtures in tests.
- [x] Test apply/undo/redo equivalence, redo invalidation, history trimming, saved
  revision tracking, and invalid image dimensions/overflow.

Acceptance: engine tests run without loading QML or making network requests.
Each stroke yields one history step; undo to the saved state clears the dirty flag.

## Step 3 — canvas and input

- [x] Implement the initial QQuickPaintedItem adapter over document snapshots.
- [x] Centralize mouse viewport transforms using Qt logical coordinates, including
  fractional zoom and continuous image-edge bounds.
- [ ] Validate physical pen pressure/device behavior separately; full support is M4.
- [x] Connect pencil, eraser, size, primary/secondary color, zoom/pan and status UI.
- [x] Cover mouse Escape cancellation, pointer capture/loss, out-of-bounds movement,
  tool changes, keyboard focus, and fractional device scaling in isolated X11.
  Evidence: [gesture lifecycle plan](0006-canvas-gesture-lifecycle.md).
- [ ] Record stroke/viewport timing and memory on the quality baseline; log a
  renderer follow-up if the adapter cannot meet the target.

Acceptance: pointer marks align with the visible canvas at multiple zoom and scale
factors; a stroke crossing outside then returning has defined behavior. Undo/redo
works from menu and keyboard. No QML pixel buffer owns the saved image.

## Step 4 — trustworthy PNG lifecycle

- [x] Decode/encode PNG with size limits, orientation/color-space normalization,
  metadata stripping, and alpha preservation. Use a worker snapshot for saving.
  Evidence: [PNG validation](0007-png-validation.md).
- [x] Add native/Qt Open, Save and Save As, current filename, and dirty marker.
  GNOME portal save/reopen/replacement/cancel is covered by [plan 0008](0008-keyboard-desktop-workflow.md); sandbox grants remain a packaging gate.
- [x] Implement New/Close Save–Discard–Cancel, and preserve originals on failure.
- [x] Test unwritable target, failed commit, malformed/oversized PNG, cancel, and
  editing while a snapshot is saving. Test portal behavior on a real desktop.
  Evidence: plans [0007](0007-png-validation.md) and [0008](0008-keyboard-desktop-workflow.md).

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

Step 1 local evidence, 2026-10-12: Ubuntu 24.04 x86_64, GCC 13.3.0, Qt 6.8.3,
CMake 3.30.0, Ninja 1.12.1, Python 3.12.3. [BUILD.md](../BUILD.md) gives reproducible
setup and commands. The branch is `build/qt-cmake-shell` and contains no drawing
engine or document operations.

- Debug and Release configure/build/CTest passed (shell suite plus CLI version).
- Shell suite checks actual QML load/render, resize, theme actions, F1/Escape,
  and Quit; all four behavioral cases passed locally offscreen and on X11/software.
- ASan/UBSan tests passed with leak detection enabled outside the process-tracing
  sandbox. Inside it, LeakSanitizer reports its ptrace limitation.
- Temporarily replacing a title assertion with an intentionally wrong expectation
  made CTest fail; the original assertion was restored and the suite passed again.
- Formatting, zero-warning QML lint, repository checks, and all seven Python
  tooling tests passed. [GitHub run 38288884601](https://github.com/AfshinKI/openpaint/actions/runs/38288884601)
  passed all jobs for implementation commit `1913821`: GCC 13 Debug, Clang 18
  Release, Clang 18 ASan/UBSan, and repository checks from clean checkouts.
- Synthetic light/dark captures at minimum and typical sizes were reviewed. They
  contain only the app's own preview content; no user documents or desktop capture.

[PR #3](https://github.com/AfshinKI/openpaint/pull/3) is merged. Wayland, GPU
rendering, Orca, and packaging remain unverified.

Step 2: the headless QImage document supports round antialiased stroke segments,
alpha erasing, cancel, bounded patch history, saved revisions and redo branching.
M1 temporarily caps images at 32 MP so full-image transaction and codec snapshots
fit the working-set contract. Operations larger than the history budget roll back.
Document Qt tests cover pixel equivalence, cancellation, alpha, branching,
trimming and invalid dimensions. Debug CTest passed locally on the Step 1 setup.
Next: merge this slice after CI, then implement canvas/input on a new branch.

## Usable basic editor delivery

The maintainer authorizes checked auto-merge through stable v1. Each step remains
a focused PR; all CI jobs must pass and required reviews/protection still apply.
After document/history, deliver drawing-canvas and PNG-file-workflow separately.
The basic usability gate is draw/erase → undo/redo → save/reopen → safe close.

Paint-like feel is an explicit acceptance target: compact grouped toolbar above
a neutral workspace, visible color/size choices, bottom status/zoom, interpolated
continuous strokes and pointer-anchored zoom. Record actual performance separately
from subjective smoothness. Exact reference-build layout/behavior and GPU/Wayland
parity remain later audit work; do not label the app an identical replica.

Step 3 behavior and measured evidence: [drawing canvas plan](0002-drawing-canvas.md).

Step 4 implemented subset and remaining codec/platform checks:
[PNG lifecycle plan](0003-png-file-workflow.md). The basic source editor is usable;
the full M1 validation gate is not declared complete while unchecked cases remain.

Mouse input hardening: [plan 0006](0006-canvas-gesture-lifecycle.md) covers initiating
button ownership, interruption, viewport/editor changes, fractional edge starts
and isolated X11 scaling. These automated checks do not close physical-device,
Wayland, Orca, file-portal or remaining codec validation gates.
