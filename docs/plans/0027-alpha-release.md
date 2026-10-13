# Feature: first downloadable useful alpha

Status: candidate preparation; M2 remains open
Feature IDs: F25 and M2 editing scope
Milestone: M2

## Outcome and scope

Publish `0.1.0-alpha.1` for Linux x86_64 with tested AppImage and Flatpak bundles,
matching source, dependency notices/inventories, checksums and installation notes.
This is an unstable alpha, not complete Paint parity or a stable support promise.
No automatic publishing workflow, signing secrets or self-updater is introduced.

## Acceptance and sequence

- [x] Editing slices and both package recipes merged through protected CI.
- [x] Development packages draw/save through native portals on Ubuntu 24.04
  GNOME/X11 and Fedora 44 GNOME/Wayland; Flatpak grants no network or broad home.
- [x] Full regression: 12/12 development and 12/12 sanitizer suites; formatting
  of 66 C++/QML files, strict QML lint, 15 tooling tests and repository checks.
- [x] Gitleaks 8.30.1 history scan: no leaks in 72 commits at the preflight checkpoint.
- [ ] Merge this version/installation preparation through all required checks.
- [ ] Build final artifacts once from that reviewed commit; record exact hashes.
- [ ] Test those bytes: install/replace, launch, edit, undo/save/reopen and uninstall;
  preserve saved artwork. Inspect Updates identity and native file portals.
- [ ] Audit source, notices, dependency inventories, artifact paths and checksums.
- [ ] Publish a prerelease manually from the reviewed tag with all required assets.
- [ ] Download public bytes, verify hashes and a fresh installation.
- [ ] Close M2 in a separate evidence/documentation PR after publication succeeds.

## Evidence already available

AppImage recipe PR 30 passed all five jobs in run `38331730662`; Flatpak PR 31
passed all six jobs in runs `38332907242` and `38332903243`. Native CI covers
GCC 13, Clang 18 release and Clang 18 sanitizers plus isolated X11 scaling.
The packaging plans record development artifact digests; do not substitute these
for final alpha digests. Independent Pillow reads verified the saved portal PNGs.
The Flatpak overwrite/undo/save preflight restored the original pixel digest.

Local final regression before version assignment: development 12.82 seconds,
sanitizers 15.27 seconds. Gitleaks was obtained from its official v8.30.1 release;
Linux x64 archive SHA-256
`551f6fc83ea457d62a0d98237cbad105af8d557003051f41f3e7ca7b3f2470eb`.
Repeat the history/artifact audit on the release commit. Never publish raw local
logs, VM images, test keys or local test drawings.

## Release limits

The tested desktop pair is an M2 installation gate. Complete GNOME/KDE, Wayland/X11,
hardware acceleration, stylus and accessibility certification remains M4.
No layers, editable project format or crash recovery (M3), printing (M4), AI (M6),
or automatic updates/signing trust mechanism (M5). Flatpak is a test bundle, not a
Flathub listing or OpenPaint update remote. These alpha artifacts are explicitly
unsigned by OpenPaint; checksums are integrity evidence, not signature verification.
M1's PERF-01, UI-01 and PLATFORM-01 follow-ups remain in the roadmap.

Next: merge candidate preparation, build/test the exact alpha files and publish
only after the remaining artifact gates pass. Installation steps are in
[the install guide](../INSTALL.md).
