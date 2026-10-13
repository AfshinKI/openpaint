# Feature: audited AppImage recipe

Status: implemented; protected CI/merge pending
Feature ID: F25
Milestone: M2

## Outcome

Produce an x86_64 portable package from a reviewed source tree on Ubuntu 24.04,
with Qt 6.12.0, immutable tool digests, AppImage installation identity, desktop
portal and Wayland/X11 support. Ship notices, source references and an inventory.
CI builds and checks an artifact without publishing a release.

## Acceptance

- [x] Pinned tools and runtime fail closed on digest mismatch.
- [x] Release build installs, bundles required plugins and launches without the SDK.
- [x] Ubuntu 24.04 and Fedora 44 preflight launches succeed with no development Qt.
- [x] Dependency inventory, license texts and corresponding source archives accompany the package.
- [x] Artifact audit rejects local workspace/home paths and missing required files.
- [x] Recipe/tooling tests pass; protected CI must pass before merge.

## Boundaries

No updater, signing keys, Flatpak manifest or publication in this slice. Alpha
version assignment, exact release-byte edit/save/uninstall tests and public
publication remain the closure gate. GNOME/KDE and graphics certification remains
M4; do not infer it from an offscreen launch.

## Evidence

The full recipe completed in a fresh Ubuntu 24.04 container based on image digest
`534baea6a22c03a63003dbc8dbe78fe34bc0d7e595d9a9dc9834884ff530eb55`.
Its 46,090,744-byte development AppImage has SHA-256
`f1761943c76d8ff720109a421088957ce0fded19c332ed440d3e5435afc0fb33`.
All four output checksum entries pass; the CycloneDX 1.6 schema validates. The
inventory identifies 43 Ubuntu packages, and the source archive has 116 members
with normalized owner/group headers. An outer-workspace scan of all 1,920 staged
files found no local workspace/home paths. Repository checks and all 12 tooling
tests pass. These are development artifacts, not the final alpha release bytes.

Preflight packages launched visibly on Ubuntu 24.04 GNOME/X11 and Fedora 44
GNOME/Wayland without a development Qt installation. Drawing and native portal
PNG saving worked on both; independent Pillow reads verified 1152 × 648 images
with painted pixels. Fedora's minimal desktop needed its normal Nautilus component
for GNOME 50's file-chooser portal. The first preflight exposed missing OpenGL and
Wayland graphics helper libraries; the recipe now bundles and audits them.
Generic controlled build directories are not personal paths; the privacy audit
is additionally run outside the container with the actual workspace prefixes.

The first CI run rejected a stale preinstalled PNG package whose exact source
revision had left the current indexes. The packaging job now uses the same fresh,
pinned Ubuntu base container as local validation; source matching remains strict.

Next: merge after all CI jobs pass, then deliver the separate Flatpak recipe.
Exact final-artifact installation, editing, uninstall and publication remain M2's
release closure gate.
