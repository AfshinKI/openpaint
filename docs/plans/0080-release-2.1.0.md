# Release: v2.1.0

Status: published; public assets and website verified
Date: 2026-10-25

## Scope and authorization

The maintainer authorized release, protected merges and removal of superseded
branches. Includes PRs 59–81: glass schema fix (#57), ribbon refinement, About,
Save As/mono BMP, persistent rulers/guides, skipped updates, floating shape toolbar,
drawing opacity, background extension, fill-edge correction and global matching,
region zoom/presets, tool previews/cursors, original icons and rounded glass edges.
The native desktop frame may still have a square outline; replacing the system
title bar was explicitly left out. No copied Microsoft artwork or fonts are used.

## Acceptance checklist

- [x] Preserve all local refinements and audit the assembled code against the tested preview.
- [x] Merge documentation PR 58 through protection; retain focused PRs for the remaining slices.
- [x] Feature PRs #59–#81 consolidated in #82 and closed at maintainer request.
- [x] Consolidated release PR passes required checks and merges through protection.
- [x] Release build, full native/tooling tests, format/QML checks and scaled UI validation pass.
- [x] Fresh light/dark release captures use original artwork/backdrop at 40% surface opacity.
- [x] README, website, notes, version and metadata are reconciled and visually reviewed.
- [x] Package install/upgrade/edit/save/reopen/remove evidence is recorded.
- [x] Protected release-preparation PR merges, then main workflow dispatch promotes 2.1.0.
- [x] Six public assets and checksums/source identities are verified; public website deployed.
- [x] Superseded/merged branches cleaned up after preserving recoverable history.

## Evidence and limitations

Earlier combined development preview passed all 20 suites; later feature-specific
checks are recorded in their plans. Release evidence must be recorded below and
must not be inferred from earlier builds. Physical Mac visual validation and
notarization remain unavailable; hosted native Cocoa validation and ad-hoc signing
are the supported release lane. GNOME/KDE/Wayland coverage is reported exactly.

### Local release validation (2026-10-25)

Qt 6.12.0/GCC 13 Release build completed. All 20 CTest suites pass (78.06 s),
including engine, persistence, updater and shell workflows. All 44 Python tooling
tests pass. C++/QML formatting, zero-warning QML lint, repository links/privacy
checks and website staging pass. Two stale UI regressions were updated in their
owning PRs: cursor hotspots now use the approved 5,5 crosshair, and eraser
selection is verified on the shared split-button surface, including opaque mode.
No behavior checks were removed or disabled. The complete shell suite also passes
under isolated X11/Mesa OpenGL at 125% scale: 74 cases, zero failures/skips
(58.16 s).

Seven fresh captures are recorded in `website/release.json` and the asset
provenance notes. Light/dark and Save As/fill/zoom menus were visually inspected.
The staged website was reviewed in Chrome at desktop size and a 390×844 phone
viewport (375px content width with scrollbar); no horizontal overflow or broken
images was found. The external-link SVG renders as a monochrome vector on mobile.
This is browser viewport validation, not a physical-device certification.

The maintainer accepted the interactive development UI before requesting release.
Native system-frame outline limitations remain accepted; no custom title bar was
introduced. Installed-package and hosted macOS validation remain separate gates.

### Historical branch disposition

Merged historical branch heads and their ancestors were removed after preserving
a verified recovery bundle locally. Two obsolete documentation-only branches were
archived in the same bundle: generic DEB troubleshooting is retained in RELEASES;
the unconfirmed v1.3 “Waiting to check” report remains a follow-up below. All superseded v2.1 branch names were removed after #82 merged; detached
worktree directories and ignored builds were retained to preserve local work.

The historical v1.3 waiting-state report was not reproduced against an installed
v2.1 package. Unit tests cover release-check states; they do not prove the cause
of that report. If reported again, record package/session/network conditions and
confirm startup/open transitions to checking, cooldown, success or error. Do not
claim this historical report fixed solely from the current test suite.

### Consolidation authorized by maintainer

To avoid duplicate builds and serial squash-merge conflicts, the maintainer
explicitly authorized using the release PR containing all changes and closing
superseded feature PRs. PR #82 now targets main; #59–#81 are closed. All feature
implementations are in the release history. Eight independent formatting commits
are not ancestors, but their before/after contents differ only in whitespace;
the final release passes the same formatting gate. No feature code was omitted.
The superseded workflow runs are no longer active. The complete validation and
packaging workflow still gates #82; no required check or protection was removed.

### arm64 numeric boundary regression

The first consolidated CI run passed all Linux/native/package/lifecycle checks
and 82 of 83 macOS tests. The canvas-fit assertion rejected an arm64 fused
multiply-add result of 71.999999999999972 for the exact 72px inset. The same
arithmetic was reproduced locally with `fma`; no visible overlap or application
behavior change was involved. The regression now allows 1e-9 screen pixels of
rounding at both fit boundaries, while retaining the free-zoom assertion and
pixel-preservation check. The corrected exact-head CI run passed all jobs, including macOS, before merge.

## Publication and public verification (2026-10-25)

- [PR #82](https://github.com/AfshinKI/openpaint/pull/82) merged through protection
  after [exact-head CI](https://github.com/AfshinKI/openpaint/actions/runs/40231838013)
  passed. Published source: `db70f7566b8f277ef4f4ab341f2bbc9baf6decf1`.
- [Stable publication run](https://github.com/AfshinKI/openpaint/actions/runs/40233032339)
  passed all jobs: documentation, identity, GCC, Clang Release/ASan, scaled X11,
  DEB, portable, Flatpak, Ubuntu package lifecycle, macOS Cocoa/bundle audit and
  publication. No required check was bypassed.
- [v2.1.0](https://github.com/AfshinKI/openpaint/releases/tag/v2.1.0) became the
  latest stable release at 21:10:37 UTC. Its tag resolves to the tested source.
  All six public assets were downloaded and independently matched GitHub's
  SHA-256 digests; all five payloads also passed the published `SHA256SUMS`.
  Inventories inside the source archive identify version 2.1.0 and the same
  source commit for portable, DEB, Flatpak and macOS.
- The public portable and extracted DEB executables report `OpenPaint 2.1.0`.
  Both passed isolated five-second offscreen startup checks without a QML load
  failure. User settings and installed applications were not changed.
- [Pages deployment](https://github.com/AfshinKI/openpaint/actions/runs/40233016329)
  succeeded. The [public website](https://afshinki.github.io/openpaint/) was
  visually reviewed at desktop and 390×844 mobile viewport sizes: all seven new
  screenshots load, four external-link icons use original SVG, and the mobile
  layout has no horizontal overflow. Download links target the latest release.
- PRs #59–#81 were closed as superseded, not individually merged. Their changes
  are included in #82. Recovery bundles preserve prior branch history; remote
  feature branches and their local names were removed. Redundant CI runs were
  cancelled after the authoritative runs were established.

### Public asset digests

| Asset | SHA-256 |
| --- | --- |
| OpenPaint-2.1.0-linux-x86_64.tar.gz | `99b9bbf2d4796e529fc59db9ba66e365f93411ab3dc9f2b09953c6995700d046` |
| OpenPaint-2.1.0-macos-arm64.dmg | `7285062d966d5aa448dee2e979a260a4e2fbc952b032130774e488683586f0fd` |
| OpenPaint-2.1.0-sources-and-notices.tar | `adeee7d83e2bc70028a2e19f9a0180bb1c501ff1515ff3605a2ad102500bf0d2` |
| OpenPaint-2.1.0-ubuntu24.04-amd64.deb | `474c020c2e71a44ca24725f72b15615cb169354b321225aed316b22515d32dc9` |
| OpenPaint-2.1.0-x86_64.flatpak | `6195e77c14423e17fcd40a0c4a60b7e7f24ce444b640c2df0f99e4c0030b3900` |
| SHA256SUMS | `bfd5bfd8af503888d6919da03cc6301827e0436367d0a6c31b347190fe41d701` |

### Remaining acceptance limits

Hosted Ubuntu install/replace/remove tests and automated editor edit/save/reopen
coverage passed. Prior maintainer desktop acceptance is retained separately;
this session did not perform a new manual installed-package drawing workflow or
an upgrade of the user's installed app from the public download. Extracted-package
startup is not proof of those manual gates. Hosted macOS native tests passed;
physical Mac visual checks, notarization, and broad GNOME/KDE/Wayland/input-device
certification remain open. The historical updater report above remains a follow-up.

## Next step

Collect the remaining manual installed-package and physical-device acceptance
when those environments are available. Track new regressions as scoped follow-up
PRs; do not modify the immutable v2.1.0 release assets.
