# Feature: balanced ribbon tool icons

Status: implemented; combined Linux validation passed; protected review pending
Feature IDs: F01
Target: v2.1.0

## Outcome and acceptance

Use 20px Image/Tools glyphs and 24px Selection/Brush glyphs in their existing
hit targets, following the compact reference supplied after preview review.
Keep menus/footer/shape gallery sizing unchanged. Preserve dropdown targets,
keyboard actions, two-row alignment, minimum 1024×640 bounds and both themes.
Artwork/history are unaffected. Original OpenPaint SVGs remain in use.

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

## Reference-driven revision

User rejected the oversized preview and supplied a compact Paint ribbon reference.
Use 20px Image/Tools glyphs and 24px Selection/Brush glyphs, retaining 32px hit
areas and separate dropdown targets. Original OpenPaint SVGs remain in use;
reference artwork is not copied into the repository. The prior 28/32px target
above is superseded. Validate light/dark and minimum width after rebuilding.

## Revised preview validation — 2026-10-25

The compact icons, glass rulers, Updates cleanup and optional smart guides were
rebuilt together with all v2.1 slices in the isolated combined checkout. All 20
CTest suites pass across the full run and editor/text rerun; a font-dependent
smart-guide test was moved into the existing GUI text suite. All 44 tooling tests,
formatting and zero-warning QML lint pass. Focused X11/Mesa checks at 125% pass
(10 Qt cases); offscreen checks at 200% pass (6 cases). Synthetic light/dark
captures were inspected. These results supersede earlier visual-preview evidence.
Required hosted review/checks and release/package validation remain outstanding.
