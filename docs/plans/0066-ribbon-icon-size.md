# Feature: larger ribbon tool icons

Status: implemented; combined Linux validation passed; protected review pending
Feature IDs: F01
Target: v2.1.0

## Outcome and acceptance

Increase the visible Image/Tools glyphs to 28 logical pixels (from 20 monochrome
or 24 colored) within their existing 32-pixel targets. Increase Selection to 28
and Brushes to 32 within its larger target. Keep menus/footer/shape gallery sizing
unchanged. Preserve dropdown targets, all keyboard actions, two-row alignment,
minimum 1024×640 bounds and both themes. Artwork/history are unaffected.

Existing coloredRibbonWorkflow and frostedToolSelection cover minimum-size bounds,
keyboard activation, tool colors and theme captures. Extend bounds checks for the
larger content. Application build, QML lint and UI execution are deferred to the
final validation pass per maintainer request; no visual verification claimed yet.

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
