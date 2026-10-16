# Feature: preserve the portal-selected save filename

Status: complete — protected PR merged and restricted desktop acceptance passed
Feature IDs: F15, F16

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
- [x] Explicit PNG extension saves and reopens in a restricted Flatpak desktop.
- [x] Existing keyboard save/export and native CI checks pass.

## Validation and evidence

Reproduced with a current Flatpak on Ubuntu 24.04.5 GNOME/X11: portal granted
`flatdrawing`, Qt returned `flatdrawing.png`, and host output remained hidden.
The native DEB saved and reopened normally. An explicit `.png` name saved visibly
through Flatpak and decoded independently as 1152×648 RGBA. No personal artwork was
used. The new saveDialogsPreserveAuthorizedFilename regression and existing keyboard
save/export workflows pass locally on Qt 6.12. All 13 local CTest groups pass, along with format-check and qml-lint.
The fixed restricted Flatpak correctly rejects an extensionless portal destination,
shows the required-extension error, leaves the drawing dirty and creates no host
output. An explicit `fixed.png` saved visibly, decoded independently as nonblank 1152×648
RGBA, and reopened through the portal with the same drawing. PR #38 passed all nine checks on the integrated base in run `38833792664`
and merged as `94746329065874e3acd95e816db67297d70b7256`.
A further stroke saved successfully to the reopened visible PNG.

## Progress and handoff

This defect is resolved. Stable publication and public-download acceptance are
tracked in [plan 0031](0031-stable-release.md).
