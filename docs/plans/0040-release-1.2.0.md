# Feature: stable v1.2.0 release

Status: complete — stable release published; public downloads and checksums verified
Feature IDs: F25

## Outcome and scope

Publish the maintainer-approved frosted appearance and editing-control refinements
as stable v1.2.0 through the existing protected release workflow. This preparation
updates version metadata and release notes; it adds no editor behavior.

## Acceptance cases

- [x] Source, AppStream metadata, release notes, tag and packages identify 1.2.0.
- [x] Release preparation passes all protected PR checks before merging.
- [x] Explicit main-branch dispatch passes every gate before publication.
- [x] Public assets and checksums match the tested source and release identity.

## Validation and limitations

Feature PR #47 passed all nine validation jobs in run `39156488388` and
merged as `d9cbdd93afca14d4b1b5f635865fcc6165040345`. It includes persistence, fill/undo, text-caret and popup-rendering tests.
Local X11/OpenGL captures cover light/dark menus, the Updates dialog and floating
controls. The maintainer reviewed the frosted app and explicitly requested release.
The fractional-scale screenshot test samples the actual captured pixel dimensions;
its opaque foreground assertion remains exact. Resize bounds wait for layout.

Prior Ubuntu GNOME/X11 package install, draw, save and reopen acceptance is recorded
in [plan 0031](0031-stable-release.md). Current CI tests package installation,
replacement, launch and removal, plus native, sanitizer, X11 scaling and Flatpak
sandbox behavior. This release does not claim a fresh manual certification of all
desktops. KDE/Wayland, stylus and broader accessibility coverage remain outstanding.
Desktop blur is compositor-provided; local visual acceptance used GNOME 46/X11
with Blur My Shell. OpenPaint does not configure desktop extensions. Downloads
remain unsigned; checksums detect corruption and installation remains manual.

## Publication and public-download closure

[PR #48](https://github.com/AfshinKI/openpaint/pull/48) merged as
`4fb7f036987dd80a9e485ae9aee2ed8536a6f510` after all nine validation jobs passed in
[PR run 39157273448](https://github.com/AfshinKI/openpaint/actions/runs/39157273448).
The publishing job was correctly skipped on the PR.

[Release run 39157968275](https://github.com/AfshinKI/openpaint/actions/runs/39157968275)
was explicitly dispatched on main at that same commit. All ten jobs passed,
including release identity, native builds/sanitizers, X11 scaling, packaging,
Flatpak sandbox, Ubuntu package lifecycle and publication.
[Stable v1.2.0](https://github.com/AfshinKI/openpaint/releases/tag/v1.2.0)
was published at `2026-10-18T19:30:28Z`; the tag resolves to the source commit above.

Closure verification downloaded every public asset, streamed SHA-256 over each
package/source archive, and matched both the published `SHA256SUMS` entries and
GitHub asset digests. The checksum file itself also matched its API digest.
These are integrity checks, not signatures or a new installed-desktop certification.

| Public asset | Verified SHA-256 |
| --- | --- |
| `OpenPaint-1.2.0-linux-x86_64.tar.gz` | `8f671c8018e6f083f5825f821a25a3c8dd61199243e4b0a3f53322a8407dd706` |
| `OpenPaint-1.2.0-sources-and-notices.tar` | `dbbb0714fec3b465928886eb324e525ee491efd096243df69891e195b69f3c5b` |
| `OpenPaint-1.2.0-ubuntu24.04-amd64.deb` | `27943950e80ebd422fdaa60fb5dad3a24410084b786abb5cb8cb8a54ab0f1021` |
| `OpenPaint-1.2.0-x86_64.flatpak` | `81851eadf45fe13c42cd6b8501046b25b60e2b8da4e8d0065c4cacaa31ffa0cf` |
| `SHA256SUMS` | `0bd8d95ff17d0d113c9cd258726d7e5622013ab2a5cb514c10916aa03360323a` |

## Handoff

Release preparation, publication and public-download verification are complete.
Do not redispatch or replace this published version. Remaining platform/input/
accessibility limits above are carried forward, not silently closed. The next
feature planning step is recorded in [the roadmap](../ROADMAP.md#current-handoff-toward-v210).
