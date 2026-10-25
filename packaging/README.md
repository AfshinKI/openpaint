# Linux and macOS packages

Packaging produces review artifacts. It never publishes releases or installs into
the host system. Release approval and exact-byte clean-machine tests follow the
[release contract](../docs/RELEASES.md).

## AppImage (x86_64)

Build on Ubuntu 24.04 with the [Qt 6.12.0 toolchain](../docs/BUILD.md), Python 3.11+,
CMake 3.28+, Ninja, GCC 13, the native build dependencies and OpenGL development
libraries. Install the runtime prerequisites listed in the `appimage` CI job,
including D-Bus, EGL, font and Wayland libraries even in a headless build container.
Enable signed Ubuntu `deb-src` indexes for the installed packages before
collecting corresponding sources. The CI workflow is an executable example.

```sh
python3 tools/package_appimage.py --qt build/qt/6.12.0/gcc_64 --collect-system-sources
```

Use a fresh `--work` directory for every run. `--suffix dev` is the default;
release preparation supplies the reviewed prerelease suffix. Do not assign a
release version before the milestone gates pass. `--jobs` controls compilation.
The default output is `build/appimage/`: AppImage, SHA256SUMS, dependency inventory,
CycloneDX 1.6 bill of materials and third-party source archive. Omitting `--collect-system-sources` is allowed for
local preflight only; the inventory records this omission and such a build must
not be published. The matching OpenPaint source archive is produced separately
from the reviewed commit during release preparation.

The recipe verifies every downloaded tool/runtime/source against the checked-in
SHA-256 manifests, including cached files. The upstream `continuous` URLs are
mutable: a changed digest fails the build and requires a reviewed manifest update;
it never silently trusts new bytes. `appimage-tools.json` records source commits.
Qt source digests are checked against its official release checksum endpoints.
ICU and runtime source pins were recorded from their official HTTPS release URLs.
The upstream runtime pins libfuse and squashfuse source checksums in its build recipe.

The bundle includes X11, Wayland, desktop-portal, SVG, image-codec and OpenSSL TLS plugins. The OpenSSL runtime libraries
are explicitly bundled for release checks, and the host supplies trusted CA
certificates (the DEB requires ca-certificates). The
vendor-neutral OpenGL dispatch library is explicitly included because a minimal
Fedora GNOME installation does not necessarily provide it. Graphics drivers,
glibc and the desktop/session services remain owned by the host. Do not bundle
GPU-specific drivers. Qt's plugin loader chooses the session backend. The launcher requests the desktop
portal theme unless the user explicitly selected another theme. A working portal
backend is part of the desktop prerequisite (GNOME 50 delegates file selection
to Nautilus). No broad filesystem grants are added.

System library identity and corresponding source versions come from `dpkg-query`;
`apt-get source --download-only` uses authenticated Ubuntu indexes. The inventory
lists actual bundled file hashes, upstream source pins and package versions. It
is a distribution inventory, not a claim that every static upstream subcomponent
is independently vulnerability-certified. The CycloneDX summary identifies the application, Qt, ICU, portable runtime and
bundled Ubuntu packages; the detailed inventory supplies per-file hashes. Qt source archives retain their
third-party attribution files. License files are copied without extracting links
or arbitrary archive paths. Debian copyright files and referenced common license
texts are included, as are the upstream runtime and its permissive dependencies'
notices. The runtime source includes its libfuse patch and build instructions;
the runtime links libfuse, squashfuse, zstd, zlib, mimalloc and musl.

Before release, inspect the inventory and source bundle, run a secret/history
scan, and test the exact final artifact on the supported clean systems. Compiler
prefix mapping and the artifact audit reject private developer workspace/home
paths. Generic controlled build locations such as `/src` and the root account
directory are not personal paths; also audit the staged output from the outer
workspace before publication. Source
bundle tar headers omit local owner/group information. The AppImage is not signed
by an OpenPaint signing key; checksums detect changed bytes but are not an update
authentication mechanism. Verified portable self-updates remain M5.

## Runtime notice sources

- [AppImage runtime](https://github.com/AppImage/type2-runtime/tree/8f39b89e2ac31e1640b3d3f7e9a5108e6ce805fa): MIT; libfuse source patch/build recipe included.
- [musl](https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT?h=v1.2.5): MIT and additional permissive notices.
- [mimalloc](https://github.com/microsoft/mimalloc/blob/v2.1.7/LICENSE): MIT. This is an open-source allocator dependency of the upstream runtime, not Microsoft Paint code or assets.
- [libarchive](https://github.com/libarchive/libarchive/blob/master/COPYING): BSD;
  ZIP project codec, with runtime/transitive attribution collected from system packages.
- [zlib](https://github.com/madler/zlib/blob/v1.3.1/LICENSE): zlib license.
- [zstd](https://github.com/facebook/zstd/blob/v1.5.6/LICENSE): BSD-3-Clause.

These notices preserve upstream copyright attribution. They do not identify a
local developer, computer or account.

## Flatpak test bundle (x86_64)

Install Flatpak 1.14+ and flatpak-builder 1.4+ on a Linux build machine. Add Flathub
as a user remote and install `org.kde.Platform//6.11` and `org.kde.Sdk//6.11`.
The reviewed commits are in [flatpak-runtime.json](flatpak-runtime.json); select
them using `flatpak update --user --commit=COMMIT REF` before building. The recipe
rejects different installed commits instead of silently changing dependencies.
The CI job shows the full setup. Runtime updates require a reviewed lock change.

```sh
python3 tools/package_flatpak.py
```

Commit tracked changes first. The script archives that commit, builds without
source downloads, and writes the bundle, matching application source, generated
manifest, inventory and checksums under a fresh `build/flatpak/` directory.
Use `--work` for another fresh directory and `--suffix` for a reviewed prerelease.
The installed runtime is distributed separately by Flathub; its source/build
recipes are maintained in the [KDE runtime repository](https://invent.kde.org/packaging/flatpak-kde-runtime).
The inventory records the exact runtime/SDK commits. OpenPaint's GPL license and
application metadata are installed inside the bundle.

The manifest disables inherited runtime permissions. It grants Wayland, fallback
X11, shared IPC and rendering devices; it grants neither network nor broad home
access. Native portals grant access to files the user selects. X11 itself offers
weaker isolation than Wayland. The build audits effective metadata, including bus
and environment permissions, before creating the bundle.

For a tested release bundle, installation and removal use:

```sh
flatpak install --user ./OpenPaint-VERSION-x86_64.flatpak
flatpak run io.github.AfshinKI.OpenPaint
flatpak uninstall --user io.github.AfshinKI.OpenPaint
```

The default development suffix produces an `alpha` branch bundle; explicit
release packaging with `--suffix ""` produces the `stable` branch. Neither is a
Flathub listing or an OpenPaint update remote.
Replace it manually by installing the next verified bundle. Uninstall keeps user
settings unless explicitly removed. No OpenPaint signing key authenticates these
bundles; verify downloads and checksums from the release page. Signing and
managed update distribution remain later release gates.

## Ubuntu DEB (amd64)

Use the same pinned Ubuntu container and Qt toolchain as AppImage, plus `dpkg-dev`
and `binutils`. Build with a fresh working directory:

```sh
python3 tools/package_appimage.py --qt build/qt/6.12.0/gcc_64 --work build/deb --kind system --collect-system-sources
```

The public directory contains the DEB, matching source/notices archive and checksums.
The intermediate AppImage is retained for audit only and has system identity.
`package_deb.py` preserves the audited layout privately under `/usr/lib/openpaint`;
only the launcher and desktop registration enter shared system locations. Every
ELF object is scanned for host dependencies; unresolved libraries fail packaging.
No maintainer script deletes user data or changes default image associations.

Run `bash tools/test_deb_install.sh PACKAGE.deb` only in a disposable Ubuntu test
machine with sudo, Xvfb and xauth. It installs/reinstalls/removes the package and
checks launcher survival and generated-file preservation. Desktop menu and actual
draw/save checks remain separate release acceptance cases in
[plan 0030](../docs/plans/0030-debian-installer.md).

## macOS DMG (Apple silicon)

Follow the [macOS build and packaging commands](../docs/BUILD.md#macos-release-build)
with the pinned Qt toolchain, system libarchive and development headers. The
`package_macos.py` recipe bundles Qt, audits runtime paths, signs ad-hoc and emits
a DMG with source/license inventory. It does not provide Developer ID signing,
notarization or Intel support. Native Cocoa tests require a graphical session.
See [v2 platform evidence and limits](../docs/plans/0062-macos-v2-release.md).

## Public release assets

The v2 release publishes Linux portable tar.gz, Ubuntu DEB, Flatpak and macOS DMG,
plus combined source/notices and checksums. AppImage remains a packaging/audit
intermediate and a historical alpha download. Packaging alone does not publish;
the explicit [release workflow](../docs/RELEASES.md#stable-publishing-automation)
promotes checked artifacts. User installation belongs in the
[installation guide](../docs/INSTALL.md).
