# Fix: glass setup across extension schema revisions

Status: implemented; combined Linux validation passed; protected review pending
Target: v2.1.0; issue #57

## Outcome

The user-run glass setup supports both the older extension redraw key and the
pinned Blur My Shell 76 schema, which omits it. The app still only copies the
script; no setup is executed by OpenPaint or by this change.

## Acceptance cases

- [x] Inspect schema keys before reading or writing optional `hacks-level`.
- [x] Where present, preserve the original value in backups and set mode 2.
- [x] Where absent, configure live blur without reading/writing the missing key
  or inventing a backup value; preserve any older backup value.
- [x] Unknown present values or failed schema inspection abort before mutations.
- [x] Both schemas preserve unrelated settings and repeat-run backup originals.
- [x] Output describes whether the redraw override was available.

## Evidence and handoff

The official version_tag 76770 archive matches the existing SHA-256
`0dfde0d3575b4c7e32647e94010fa37f15ac933f4fe61c46bfe200e772114d85`.
Its root schema lacks `hacks-level`; application blur keys remain available.
Source: [official extension](https://extensions.gnome.org/extension/3193/blur-my-shell/).
No extension was installed and no desktop settings were modified.

Application builds are deferred to the final v2.1 validation pass by request.
Next: implement and run mocked Python regression tests, then protected review.

All 11 mocked blur-setup tests pass, including absent-key, existing-key, old-backup
preservation and failed schema inspection cases. No real desktop setup was run.
Native build and UI validation remain deferred to the final pass.

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
