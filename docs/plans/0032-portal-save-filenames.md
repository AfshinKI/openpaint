# Feature: preserve the portal-selected save filename

Status: in progress
Feature IDs: F11, F12

## User outcome and scope

Saving/exporting through a sandbox portal must write exactly the authorized name.
Desktop testing found that Qt Quick's defaultSuffix appends an extension after
portal selection. An extensionless name therefore wrote a hidden portal temporary
file while the UI reported success. Fix this independently of release automation.

## Behavior and implementation

Remove post-selection suffix rewriting from Save and Export dialogs. Users choose
a filename with the format's extension; existing C++ validation rejects missing or
wrong extensions and leaves the drawing dirty/unchanged. Dialog titles indicate the
required extension. Never bypass the portal or widen sandbox permissions. Existing
atomic replacement and original-file preservation remain in place.

## Acceptance cases

- [x] Extensionless portal-style URLs remain exact and are rejected without saving.
- [x] Save/export failures preserve pixels, dirty state and existing destinations.
- [ ] Explicit PNG extension saves and reopens in a restricted Flatpak desktop.
- [ ] Existing keyboard save/export and native CI checks pass.

## Validation and evidence

Reproduced with a current Flatpak on Ubuntu 24.04.5 GNOME/X11: portal granted
`flatdrawing`, Qt returned `flatdrawing.png`, and host output remained hidden.
The native DEB saved and reopened normally. An explicit `.png` name saved visibly
through Flatpak and decoded independently as 1152×648 RGBA. No personal artwork was
used. The new saveDialogsPreserveAuthorizedFilename regression and existing keyboard
save/export workflows pass locally on Qt 6.12. Full checks and fixed-package desktop
acceptance remain pending.

## Progress and handoff

Next: targeted regression tests, restricted Flatpak desktop acceptance and protected
PR checks. Stable publication remains pending until this defect is resolved.
