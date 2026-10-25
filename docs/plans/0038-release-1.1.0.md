# Feature: stable v1.1.0 release

Status: complete — stable release published; public downloads and checksums verified
Feature IDs: F25

## User outcome and scope

Publish the merged selection, canvas-edge, fill, glass and release-notification
changes as v1.1.0 using the existing protected release workflow. The maintainer
requested publication after merging PRs #41–#45. No new editor behavior is added.

## Acceptance cases

- [x] Source, AppStream metadata, release notes, tag and packages identify 1.1.0.
- [x] Release preparation passes protected PR checks before merging.
- [x] An explicit main-branch dispatch passes all validation before publication.
- [x] Public assets and checksums match the release and reviewed source.

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

## Publication and public-download closure

[PR #46](https://github.com/AfshinKI/openpaint/pull/46) merged as
`4c150eb1d63583f1b3374a2a214a60bca12918cd` after all nine validation jobs passed in
[PR run 38985166161](https://github.com/AfshinKI/openpaint/actions/runs/38985166161).
The publishing job was correctly skipped on the PR.

[Release run 38985890186](https://github.com/AfshinKI/openpaint/actions/runs/38985890186)
was explicitly dispatched on main at that same commit. All ten jobs passed,
including release identity, native builds/sanitizers, X11 scaling, packaging,
Flatpak sandbox, Ubuntu package lifecycle and publication.
[Stable v1.1.0](https://github.com/AfshinKI/openpaint/releases/tag/v1.1.0)
was published at `2026-10-17T00:34:35Z`; the tag resolves to the source commit above.

Closure verification downloaded every public asset, streamed SHA-256 over each
package/source archive, and matched both the published `SHA256SUMS` entries and
GitHub asset digests. The checksum file itself also matched its API digest.
These are integrity checks, not signatures or a new installed-desktop certification.

| Public asset | Verified SHA-256 |
| --- | --- |
| `OpenPaint-1.1.0-linux-x86_64.tar.gz` | `4a3dd36afcafd14380c77529fc1bb4062852d5b56370e60170a5d2d00322c49d` |
| `OpenPaint-1.1.0-sources-and-notices.tar` | `0d2ee5a4e4f7f847eee83836703029bd8b9328fea5d451e9609213817e7e095c` |
| `OpenPaint-1.1.0-ubuntu24.04-amd64.deb` | `d959732931e4c5bd1373b8861ffee53f96e3af210ab2623381dc7a99a1a96942` |
| `OpenPaint-1.1.0-x86_64.flatpak` | `019e7661c73c76ff885b3f7c28d50c07d72f57a3b965ce5947b9b54b7c6c970a` |
| `SHA256SUMS` | `c2448afaf573876473dc0d193868c9d0f705af505ab6923dd2d93bc2167ff619` |

## Handoff

Release preparation, publication and public-download verification are complete.
Do not redispatch or replace this published version. Remaining platform/input/
accessibility limits above are carried forward, not silently closed. The next
feature planning step is recorded in [the roadmap](../ROADMAP.md#current-handoff-after-v2).
