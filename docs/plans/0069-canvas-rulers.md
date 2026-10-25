# Feature: top and left pixel rulers

Status: implemented; combined Linux validation passed; protected review pending
Feature IDs: F11
Target: v2.1.0

Add optional top/left rulers in image pixels, enabled by default for the session.
View → Rulers (Ctrl+R, Command+R on macOS) toggles both. Adaptive tick spacing and
labels follow the canvas origin, zoom and dimensions, with pointer markers only
while the pointer is over the image. Keep the Layers rail and controls reachable
at minimum size. Rulers do not intercept drawing, enter history, or appear in
saved images. Pan/zoom and toggling preserve artwork and dirty state.

Smart alignment/snapping is a separate follow-up: temporary edge/center guides
for editable content with an optional snap toggle and modifier bypass. This slice
provides rulers/pointer guides only; do not advertise alignment snapping yet.

Acceptance: coordinates remain synchronized at fit, 1%, 3200%, panned origins,
fractional scale and canvas resize; ticks are bounded to the visible image range;
labels remain readable in both themes; toggle/keyboard and minimum-size layout
work without mutating pixels/history. Native build/UI checks deferred until the
requested final validation pass.

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

## Visual revision

User review requests the same transparency/material rules as the app, a small
corner radius and subtle elevation. Rulers now use inset GlassSurface panels
with 3px corners and a soft one-pixel shadow. Tick canvases remain transparent;
text/markers keep their own opacity. Glass-off/100% uses the standard opaque
fallback. Revalidate themes, 40% opacity and disabled glass before approval.

## Revised preview validation — 2026-10-25

The compact icons, glass rulers, Updates cleanup and optional smart guides were
rebuilt together with all v2.1 slices in the isolated combined checkout. All 20
CTest suites pass across the full run and editor/text rerun; a font-dependent
smart-guide test was moved into the existing GUI text suite. All 44 tooling tests,
formatting and zero-warning QML lint pass. Focused X11/Mesa checks at 125% pass
(10 Qt cases); offscreen checks at 200% pass (6 cases). Synthetic light/dark
captures were inspected. These results supersede earlier visual-preview evidence.
Required hosted review/checks and release/package validation remain outstanding.

## Canvas-length revision

Keep rulers at their original fixed top and left workspace locations, with their
original 24px thickness. Each length follows the displayed canvas and extends
12px beyond each end. Panning changes the longitudinal alignment only; offscreen
segments are clipped. Tick offsets account for each ruler’s local origin, and
adaptive zoom behavior remains unchanged.
Acceptance: fit lengths, fixed top/left positions during pan and zoom, correct
zero alignment, bounded rendering at 1–3200% and unchanged pixels.

Final combined Qt 6.12.0 build, formatting and zero-warning QML lint pass.
Ruler/Save As/export workflows pass offscreen at 100% and 200% (5 Qt cases each),
and with the floating-glass check on X11/Mesa at 125% (6 cases). All six shell
CTest suites passed before the final fixed-edge clarification; its geometry was
then rechecked by the targeted cases above. Final light/dark captures inspected.
Required protected review and release/package checks remain outstanding.
