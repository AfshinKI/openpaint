# Release: v2.1.0

Status: consolidated release candidate in PR #82; publication pending
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
- [ ] Consolidated release PR passes required checks and merges through protection.
- [x] Release build, full native/tooling tests, format/QML checks and scaled UI validation pass.
- [x] Fresh light/dark release captures use original artwork/backdrop at 40% surface opacity.
- [ ] README, website, notes, version and metadata are reconciled and visually reviewed.
- [ ] Package install/upgrade/edit/save/reopen/remove evidence is recorded.
- [ ] Protected release-preparation PR merges, then main workflow dispatch promotes 2.1.0.
- [ ] Six public assets and checksums/source identities are verified; public website deployed.
- [ ] Superseded/merged branches cleaned up after preserving recoverable history.

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
the unconfirmed v1.3 “Waiting to check” report remains a follow-up below. Active
v2.1 branches are retained until the consolidated release PR merges.

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

## Next step

Complete exact release-source build and CI, refresh screenshots, then publish
only through the existing gated main-branch release workflow.
