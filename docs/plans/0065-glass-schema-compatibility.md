# Fix: glass setup across extension schema revisions

Status: implemented; Python regression checks pass; native build deferred
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
