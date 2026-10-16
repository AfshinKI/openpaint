# Feature: Ubuntu desktop installer

Status: in progress
Feature IDs: F25

## User outcome and scope

Install OpenPaint on Ubuntu 24.04 amd64 through APT, launch from the application
menu or an image's Open With menu, and remove it through the package manager.
This slice adds packaging and CI validation; stable versioning/publication follows
in a separate PR. Other distributions retain Flatpak and portable options.

## Behavior

The package registers the existing desktop file, icon and AppStream metadata.
It does not change default image associations, open drawings, or user settings.
Updates are manual package replacement; the app identifies as a distribution
package and never writes package-owned files or runs an elevated installer.
Installation may download declared dependencies through APT. Editing stays offline.

## Implementation

Reuse the audited bundled Qt recipe with system installation identity. Install its
runtime privately under /usr/lib/openpaint and a launcher in /usr/bin. Generate
host dependencies from every ELF binary/library/plugin using dpkg-shlibdeps;
bundled SONAMEs resolve to this package, never to invented system Qt packages.
See [the packaging decision](../decisions/0003-private-debian-runtime.md).

## Acceptance cases

- [x] Package metadata, permissions, private runtime and desktop registration pass.
- [x] Missing host libraries fail dependency generation instead of being ignored.
- [x] Clean Ubuntu install launches without a development Qt installation.
- [x] Replacement and removal preserve a generated user artwork fixture.
- [ ] Desktop launch, image opening and draw/save/reopen pass on a desktop VM.
- [ ] Tooling tests and all required CI jobs pass before protected merge.

## Validation and evidence

Local preflight built a 36 MB DEB using the pinned Ubuntu 24.04 container and
Qt 6.12 with corresponding dependency sources. A separate clean Ubuntu 24.04
container without a Qt SDK passed APT install, upgrade from a lower-version
transaction fixture, offscreen version check, Xvfb graphical launch, reinstall,
and removal. Desktop-file validation and MIME cache registration passed; generated
PNG bytes survived upgrade/reinstall/removal. The transaction fixture has the same
app payload, so this is not historical application-version compatibility evidence.

All 21 repository-tool tests pass, including an actual ELF linked to an absent
library that must fail dependency generation. Repository/privacy checks, whitespace
checks, shell syntax and workflow YAML parsing pass. The packaged GUI stayed alive
under Xvfb; no interactive GNOME/KDE menu, Wayland or draw/save check is claimed.
Hosted CI and desktop release acceptance remain pending.

## Progress and handoff

Next: submit the installer PR and complete hosted checks and desktop acceptance. After protected merge,
prepare stable release automation and reconcile the roadmap with the maintainer's
chosen single-layer v1 scope. No stable package has been published by this plan.
