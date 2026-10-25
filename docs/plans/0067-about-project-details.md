# Feature: current About information and public contact

Status: implemented; combined Linux validation passed; protected review pending
Target: v2.1.0

Show the build's actual version, Linux/macOS description, approved public credit
“Afshin Kashani, PhD, P.Eng”, GPL license and independent-project disclosure.
Provide explicit Website and Contact / report an issue buttons using the public
project site and GitHub Issues. No email address is inferred or published.

Acceptance: opening About sends no request; only deliberate button activation
opens a fixed HTTPS destination. Browser launch failure leaves a readable URL.
F1, keyboard focus, Escape/OK, light/dark and minimum-size layout remain usable.
Document pixels/history are unaffected. Extend the URL-handler test without
making real browser/network requests; defer native execution/build to final pass.

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

## Attribution placement revision

Remove the designer credit from Updates at the maintainer’s request. Keep public
developer details in About. The Updates workflow asserts the old credit is absent;
this revision awaits the final combined rebuild and test pass.

## Revised preview validation — 2026-10-25

The compact icons, glass rulers, Updates cleanup and optional smart guides were
rebuilt together with all v2.1 slices in the isolated combined checkout. All 20
CTest suites pass across the full run and editor/text rerun; a font-dependent
smart-guide test was moved into the existing GUI text suite. All 44 tooling tests,
formatting and zero-warning QML lint pass. Focused X11/Mesa checks at 125% pass
(10 Qt cases); offscreen checks at 200% pass (6 cases). Synthetic light/dark
captures were inspected. These results supersede earlier visual-preview evidence.
Required hosted review/checks and release/package validation remain outstanding.
