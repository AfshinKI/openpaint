# Feature: Save As format choices and monochrome BMP

Status: implemented; combined Linux validation passed; protected review pending
Feature IDs: F15
Target: v2.1.0

Save As offers PNG, OpenRaster, JPEG, color BMP, monochrome BMP and WebP through
one options chooser. PNG (flat documents) and OpenRaster save the document.
Other image choices write an explicitly labelled copy through the existing export
pipeline; layered PNG is also a copy. They never mark an editable project saved
or satisfy pending close/new/open guards. Normal Save remains fidelity-preserving.

Monochrome BMP is an actual 1-bit black/white BMP: flatten onto the selected opaque
background, then threshold Qt grayscale at 128 without dithering. Disclose color
and transparency loss before destination selection. No other new codec is implied.

Acceptance: genuine 1-bit BMP header and pixel round-trip; transparent flattening,
odd-width row padding, cancellation and failed writes preserve originals; existing
image formats/ORA remain available; cancel keeps document identity/history; no
suffix is appended outside a portal grant. UI choices disclose copy semantics.
Tests use generated images; native build/execution deferred to final pass.

## Final local validation — 2026-10-25

Validated together with the other independent v2.1 slices in a temporary combined
checkout based on v2.0.0 (PRs #59–#63), without changing the installed app or
user settings. Qt 6.12.0 / GCC 13 Debug build passes. All 20 CTest suites pass
(71.56 seconds); all 44 Python tooling tests pass. Formatting (99 C++/QML files),
zero-warning QML lint, repository checks and diff hygiene pass.

Focused ribbon, rulers, About, Save As and export workflows pass offscreen at
200% scale (7 Qt cases including harness setup/cleanup). The same workflows plus
floating-panel blur pass on isolated X11/Mesa OpenGL at 125% scale (8 cases).
Synthetic light/dark ribbon, ruler and About captures were visually inspected.
The initial sandbox X11 socket restriction was resolved by running the isolated
Xvfb test outside the sandbox. No live-desktop input or glass setup was performed.

This is local combined-source validation, not package or physical Mac certification.
Next: required hosted checks and protected review/merge of the individual PR;
then v2.1 release metadata, fresh release screenshots and package validation.

## Save As visual revision

Use the shared glass dialog/menu surfaces and a full-width, left-aligned format
picker with extension text. The format menu matches the picker width and marks
the active choice. Concise format help and aligned option rows reduce clutter.
The primary action reads “Choose location…” before opening the system file picker.
Save/copy semantics are unchanged. Validate light/dark, keyboard format selection,
minimum window width and both save/export flows after rebuilding.

The revised combined preview builds with Qt 6.12.0; formatting and zero-warning
QML lint pass. All six shell CTest suites pass. Final ruler/Save As/export workflows
pass offscreen at 100% and 200% (5 Qt cases each), and with the floating-glass
check on isolated X11/Mesa at 125% (6 cases). Light/dark PNG, monochrome BMP,
JPEG and WebP dialog captures were inspected. Keyboard destination selection
and export preserve their existing behavior. Protected review remains pending.
