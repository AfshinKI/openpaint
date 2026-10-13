# Feature: explicit image export copies

Status: implemented; protected CI required before merge
Feature IDs: F15, F16, F26
Milestone: M2

## Outcome

File → Export copy offers PNG, JPEG, BMP and WebP. PNG and WebP preserve alpha by
default; WebP defaults to lossless. JPEG exposes quality and always warns of loss;
BMP/JPEG require an opaque background. The dialog offers white, black or the opaque
secondary color. Optional flattening is explicit for alpha-capable formats too.
WebP lossy quality is 1–99; quality 100 invokes Qt's lossless codec mode.

Options are a local draft; Cancel changes nothing. Accept closes before opening a
native destination dialog with the matching filter/suffix and overwrite prompt.
Active previews must be applied/cancelled first. A worker exports an immutable
snapshot, strips metadata and atomically replaces only the chosen destination.
Export never changes document pixels, history, backing URL or saved revision.
Editing may continue while encoding; cancel/failure preserves the destination.
No network or restart behavior is added.

## Acceptance

- [x] Lossless alpha round trips and opaque compositing match expected pixels.
- [x] Lossy output, quality choices, metadata removal and file suffix checks work.
- [x] Failed writes/commits and cancellation preserve existing destination bytes.
- [x] Export preserves dirty/saved state, undo and the PNG backing URL.
- [x] Keyboard options, cancellation, focus handoff and both themes are checked.
- [x] Development/sanitizer, format/QML and privacy checks pass.

## Implementation and evidence

ImageIo owns ExportOptions and atomic serialization. Editor routes export results
separately from Save, so export cannot resume a pending Save/Close flow. ExportDialog
uses shared rounded controls.

Qt 6.8.3/Linux x86_64: all 11 development and ASan/UBSan suites pass, including
alpha/opaque/lossy output, metadata, injected short writes and commit failures,
cancellation after encoding/write, and edits made while export runs. The shell
workflow exercises JPEG quality by keyboard, WebP defaults, Cancel and native-dialog
fallback handoff to a PNG copy, asserting that dirty state remains unchanged.
Isolated X11 export workflow passes (3 test results); formatting, strict QML lint,
repository checks and seven tooling tests pass.

Reviewed generated minimum-size captures: [JPEG light](../assets/export-jpeg-light-1024.png)
and [WebP dark](../assets/export-webp-dark-1024.png). Their only PNG metadata is DPI.
The visual review caught a content-width overflow; the layout and a width assertion
now prevent that regression. Cancellation is checked around Qt encoding and before
atomic replacement; it cannot interrupt the codec internally.

Next: maintained release toolchain, installation identity/Updates, then separately
reviewed AppImage and Flatpak builds and the real alpha release gate.
