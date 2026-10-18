# Feature: stable v1.2.0 release

Status: release preparation; publication pending
Feature IDs: F25

## Outcome and scope

Publish the maintainer-approved frosted appearance and editing-control refinements
as stable v1.2.0 through the existing protected release workflow. This preparation
updates version metadata and release notes; it adds no editor behavior.

## Acceptance cases

- [ ] Source, AppStream metadata, release notes, tag and packages identify 1.2.0.
- [ ] Release preparation passes all protected PR checks before merging.
- [ ] Explicit main-branch dispatch passes every gate before publication.
- [ ] Public assets and checksums match the tested source and release identity.

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

## Next step

Merge after all required checks, dispatch stable 1.2.0 on main, verify the public
release and asset digests, and record the run and exact source identity. Never
replace previously published assets or bypass branch/release protections.
