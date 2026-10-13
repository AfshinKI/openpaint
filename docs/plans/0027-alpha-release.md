# Feature: first downloadable useful alpha

Status: complete — alpha published and public downloads verified
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
- [x] Merge this version/installation preparation through all required checks.
- [x] Build final artifacts once from that reviewed commit; record exact hashes.
- [x] Test those bytes: install/replace, launch, edit, undo/save/reopen and uninstall;
  preserve saved artwork. Inspect Updates identity and native file portals.
- [x] Audit source, notices, dependency inventories, artifact paths and checksums.
- [x] Publish a prerelease manually from the reviewed tag with all required assets.
- [x] Download public bytes, verify hashes and a fresh installation.
- [x] Close M2 in a separate evidence/documentation PR after publication succeeds.

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

## Final release evidence

[OpenPaint 0.1.0-alpha.1](https://github.com/AfshinKI/openpaint/releases/tag/v0.1.0-alpha.1)
was published as a prerelease on 2026-10-13 UTC from
`98f9fd62a55cd4c58698e7335bef4e86c7ec04e0`. PR 32 passed all six CI jobs;
the merged commit and tag also passed all six jobs (runs `38333949186` and
`38334806091`). No protection or required check was bypassed.

| Final artifact | SHA-256 |
| --- | --- |
| AppImage x86_64 | `35170a2e492b9eef6b4c71900627984398d7e15f64c02a8991c3a4923bb23ef3` |
| Flatpak x86_64 | `73df1700ec8c4dad00aa82aa8245e16cab3c57259eb4702abb21b28375aebacb` |
| Matching source tar | `a6a907318db9d41114427f61385d58d999cbabf3c8eb89a56977be1272bd7586` |

The exact final binaries passed launch, edit, native save, overwrite and undo/save
pixel equality on Ubuntu 24.04 GNOME/X11 and Fedora 44 GNOME/Wayland. Flatpak native
Open reopened generated drawings; AppImage opened them by launch argument and
created pixel-identical native Save As copies. Flatpak replacement of development
bundles passed. Uninstall/removal left artwork intact. AppImage FUSE launch and
extraction fallback passed; Updates showed correct alpha/source instructions.

Tests used fresh cloud-image-derived desktop VMs and software rendering. Ubuntu
also hosted the isolated Flatpak SDK build; installed apps use the separate
runtime. No native development Qt supplied AppImage dependencies. Fedora's GNOME
50 portal required its normal Nautilus component. This does not close the broader
M4 desktop/hardware matrix or M1 performance follow-ups.

All 13 uploaded assets matched GitHub's reported digests. Every asset was then
downloaded anonymously from the public release and matched the local tested bytes.
Fresh Flatpak installs and AppImage version checks from those public downloads
passed on both systems. The release includes installation instructions, application
source, third-party source/notices, inventories, a CycloneDX SBOM, license, checksums
and detailed pre-publication evidence. No local logs, keys or VM data were uploaded.

Gitleaks scanned 76 commits at the release source with no leaks. All 1,920 staged
AppImage files passed the outer-workspace private-path audit; 116 source archive
entries have normalized ownership. CycloneDX 1.6 validation passed. All 41 existing
repository screenshots had no text/EXIF chunks. The new README image was generated
by the existing isolated X11 `themeActions` test (3 passed) and visually inspected;
it contains only the app's blank canvas and original controls.

## Next unfinished milestone

M2 is closed. Complete the [alpha UI polish checklist](0028-alpha-ui-polish.md)
before starting M3. M3 starts with a bounded layer/document-model plan, `.ora` persistence,
and crash recovery, with PERF-01 considered before expanding document memory.
Do not add those features to the release or closure branches. The current alpha
continues to use manual updates; signed portable updates remain M5.
