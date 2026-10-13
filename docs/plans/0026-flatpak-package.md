# Feature: sandboxed Flatpak test bundle

Status: complete; merged after protected CI
Feature ID: F25
Milestone: M2

## Outcome and boundaries

Build an x86_64 alpha-channel test bundle with the maintained KDE 6.11 runtime,
Flatpak installation identity, and native file portals. Record exact SDK/runtime
commits and source identity. No Flathub submission, update remote, signing keys or
publication in this slice. Final release-byte testing and publication follow.

## Acceptance cases

- [x] Build from a committed source archive with matching runtime/SDK pins.
- [x] Reject dependency drift and unexpected effective permissions.
- [x] No network, broad home access or inherited KDE configuration/bus grants.
- [x] Portal Save, overwrite and undo/save preserve expected pixels.
- [x] Bundle, source archive, dependency inventory and checksums are produced.
- [x] Tooling tests and protected CI pass before merge.

## Implementation and validation

Use a JSON manifest and Python orchestration around installed Flatpak tools.
Build and packaging remain explicit developer operations; no runtime network or
package-manager process is added to the application. Flatpak owns the runtime;
the bundle contains OpenPaint only. Dependency updates require reviewed lock edits.

A disposable Ubuntu 24.04 GNOME/X11 preflight built using Qt 6.11.2 in KDE 6.11.
Native portal save and overwrite worked; undo followed by save restored the exact
original pixel digest. Removing inherited permissions leaves display, IPC and
render-device access only. These preflight results are not final release evidence.

The committed recipe produced development bundle SHA-256
`6b94359a80d3d67d2fbf2b2a62ae265140c9db9f69b19502a6d8e25980b4013d`.
All four checksums passed. Installation, version and host-home sentinel isolation
passed. The restricted package launched and saved a 1152 × 648 PNG with 231 colors
through the native portal. All 15 tooling tests and repository checks pass.

This recipe is merged. The first alpha and public-download verification are
recorded in [release closure](0027-alpha-release.md).
