# Feature: sandboxed Flatpak test bundle

Status: implementation in progress
Feature ID: F25
Milestone: M2

## Outcome and boundaries

Build an x86_64 alpha-channel test bundle with the maintained KDE 6.11 runtime,
Flatpak installation identity, and native file portals. Record exact SDK/runtime
commits and source identity. No Flathub submission, update remote, signing keys or
publication in this slice. Final release-byte testing and publication follow.

## Acceptance cases

- [ ] Build from a committed source archive with matching runtime/SDK pins.
- [ ] Reject dependency drift and unexpected effective permissions.
- [ ] No network, broad home access or inherited KDE configuration/bus grants.
- [ ] Portal Save, overwrite and undo/save preserve expected pixels.
- [ ] Bundle, source archive, dependency inventory and checksums are produced.
- [ ] Tooling tests and protected CI pass before merge.

## Implementation and validation

Use a JSON manifest and Python orchestration around installed Flatpak tools.
Build and packaging remain explicit developer operations; no runtime network or
package-manager process is added to the application. Flatpak owns the runtime;
the bundle contains OpenPaint only. Dependency updates require reviewed lock edits.

A disposable Ubuntu 24.04 GNOME/X11 preflight built using Qt 6.11.2 in KDE 6.11.
Native portal save and overwrite worked; undo followed by save restored the exact
original pixel digest. Removing inherited permissions leaves display, IPC and
render-device access only. These preflight results are not final release evidence.

Next: implement recipe, validate its exact output and CI, then merge separately
before alpha preparation. M2 remains open until public downloads are verified.
