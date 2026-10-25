# Release: v2.1.0

Status: release candidate; publication pending
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
- [ ] Required checks pass and feature PRs merge through protection.
- [ ] Release build, full native/tooling tests, format/QML checks and scaled UI validation pass.
- [ ] Fresh light/dark release captures use original artwork/backdrop at 40% surface opacity.
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

## Next step

Complete exact release-source build and CI, refresh screenshots, then publish
only through the existing gated main-branch release workflow.
