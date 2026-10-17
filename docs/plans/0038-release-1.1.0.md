# Feature: stable v1.1.0 release

Status: release preparation; publication pending
Feature IDs: F25

## User outcome and scope

Publish the merged selection, canvas-edge, fill, glass and release-notification
changes as v1.1.0 using the existing protected release workflow. The maintainer
requested publication after merging PRs #41–#45. No new editor behavior is added.

## Acceptance cases

- [ ] Source, AppStream metadata, release notes, tag and packages identify 1.1.0.
- [ ] Release preparation passes protected PR checks before merging.
- [ ] An explicit main-branch dispatch passes all validation before publication.
- [ ] Public assets and checksums match the release and reviewed source.

## Validation and evidence

PRs #41–#45 passed native dev/release/sanitizer, X11 input/scaling, Flatpak,
portable/DEB packaging and Ubuntu package lifecycle checks before merging.
The combined source passed all local application and 26 tooling tests; the UI
suite was split without removing any of its 40 cases to retain timeout bounds.
Prior Ubuntu GNOME/X11 desktop draw/save/reopen acceptance is recorded in
[plan 0031](0031-stable-release.md). No new manual desktop test details were supplied
with the maintainer's release request. Broader KDE/Wayland, hardware, stylus and
accessibility certification remains outstanding. Downloads remain unsigned;
checksums detect corruption, not publisher authenticity. Installation is manual.

## Progress and handoff

Next: merge the version/notes PR after CI, dispatch stable 1.1.0 on main, and
verify the public release assets. Preserve failed drafts and never overwrite
published assets. No automatic installation or additional network permission
is introduced by this release preparation.
