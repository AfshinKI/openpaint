# Feature: safe PNG files and a usable basic editor

Status: implemented; CI/merge evidence is tracked in the implementation PR
Feature IDs: F15, F16, F26
Milestone: M1 Step 4 and basic usability checkpoint

## Outcome and scope

A user can create a drawing, save PNG, reopen it, and close without silently losing
edits. File/New/Open/Save/Save As and their standard shortcuts are live. This is a
source-run development editor, not stable v1 or a downloadable packaged release.
Advanced tools, layers, recovery and the updater remain separate roadmap work.

## Behavior and implementation

Editor owns one document and serializes file operations. QtConcurrent receives
immutable images and local paths; it never touches QML or the live document.
Opening freezes editing until decode completes. Saving permits further strokes;
only the captured revision becomes saved. New/Open/Close ask Save/Discard/Cancel
when dirty. Save failure/cancel cancels the pending action. If new edits arrive
during a requested save-and-close, the window stays open with the dirty marker.
Closing during active IO is blocked; the status bar shows progress and Cancel.

QSaveFile uses sibling temporary files with direct-write fallback disabled. PNG
encoding failure or failed replacement preserves the original. Cancel is cooperative
at codec boundaries and before commit, so a codec currently running may take time
to return. Once replacement has committed, the save completes successfully even
if a cancellation request races with it. No partial file is reported as saved.

Only local static PNG is supported. Limits: 128 MiB encoded, 16,384 per dimension,
32 MP total, plus Qt's decoder allocation limit. Tagged colors convert to sRGB;
untagged images are assumed sRGB. Internal pixels are 8-bit premultiplied RGBA.
Text metadata chunks are removed before decoding and the user is informed when
metadata was discarded. Saved images contain pixels and sRGB only, with alpha
preserved. Animated PNG remains rejected. EXIF orientation is now normalized and EXIF is
removed before decoding; see [PNG validation](0007-png-validation.md) for the
bounded parser and failure coverage.

Qt Quick FileDialog requests platform-native dialogs when available and falls back
to Qt's dialog. This slice does not establish Flatpak portal replacement guarantees.
All work remains offline; no file paths, image contents or diagnostics are uploaded.

## Acceptance and evidence

- [x] Generated alpha image round-trips pixel-exactly in premultiplied space.
- [x] Text metadata is removed; imported metadata produces a visible notice.
- [x] Truncated, malformed and oversized PNG inputs are rejected.
- [x] Injected commit failure, missing parent and invalid destination preserve data.
- [x] Cancelled codec calls preserve the original file.
- [x] Editing during save preserves the snapshot and leaves new edits dirty.
- [x] Failed open/save preserves pixels, filename and dirty state.
- [x] UI New cancellation, Save-and-close, failed-save-close and file-dialog cancel pass.
- [ ] Portal behavior, disk-full device injection, Wayland and accessibility matrix.

Debug and ASan/UBSan CTest passed all five suites with leak detection enabled:
document, PNG IO, controller lifecycle, QML UI and CLI version. Formatting,
zero-warning QML lint, repository/privacy checks and seven tooling tests passed.
Light/dark screenshots were reviewed at 1024×640 and 1280×800; color targets are
32 px with a regression check for minimum-width clipping. UI tests also pass at
150% scaling (the preceding canvas slice additionally tested 125%).
The X11 desktop/default renderer passed the UI workflow, including actual dialog
open/cancel. No human visual inspection of a portal chooser is claimed.
The canvas's additional default-renderer probe passed at p50 16.73 ms/p95 18.28 ms
(60 samples after warm-up) on the setup in [the canvas plan](0002-drawing-canvas.md).
This is not a physical input-latency or cross-machine smoothness guarantee.

## Handoff

The usable basic source-build checkpoint is delivered when this PR's full CI passes
and it merges. The broader M1 platform/quality gate remains open for the unchecked
cases above and the canvas plan's device/accessibility work. Next feature branches
should add individual M2 tools and packaging; do not extend this branch into them.
The maintainer permits checked PR merges through stable v1, with no bypasses.
