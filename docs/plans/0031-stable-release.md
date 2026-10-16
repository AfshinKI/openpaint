# Feature: versioned stable Linux releases

Status: in progress
Feature IDs: F25

## User outcome and scope

The maintainer selected the current single-layer editor for v1.0.0. A deliberate
workflow run on reviewed main builds and tests one version, then publishes a
release named exactly v1.0.0 with Ubuntu DEB, Flatpak and portable downloads.
Layers, recovery, printing and automatic signed updates remain future work.

## Behavior and implementation

Ordinary pushes and PRs validate development packages without publishing releases.
A workflow dispatch accepts an exact version matching CMake. Stable packages use
no prerelease suffix. All native and packaging checks gate publication. Verify
checksums before promoting the same artifacts; combine compliance material for
all formats. Upload to a draft first and never replace published release assets.
Release jobs alone receive write permission. Editing and document storage do not
change. Downloads and dependency installation use GitHub/Flathub/Ubuntu servers.

## Acceptance cases

- [ ] A release request must be on main and match the source version.
- [ ] Stable version, tag, title and all three package filenames agree.
- [ ] Missing, mismatched or tampered package material prevents publication.
- [ ] All CI checks pass before promotion; published assets remain immutable.
- [ ] Desktop integration and draw/save/reopen evidence is recorded honestly.

## Validation and evidence

Installer PR #36 passed all eight checks and merged through the protected flow.
An initial VM menu test launched a previously installed alpha Flatpak instead of
the DEB. Its save left a hidden portal temporary file; that run is not DEB evidence.
The VM is being retested with host CPU features and the old alpha removed.
25 tooling tests pass, including invalid release requests, tampered/missing assets,
wrong versions/source commits and compliance material retention.

## Progress and handoff

Next: implement version selection and promotion, run tooling/native/package checks,
submit the scoped PR, then validate the release workflow. No v1 release is yet
published. Support covers tested environments only; checksums are not signatures.
