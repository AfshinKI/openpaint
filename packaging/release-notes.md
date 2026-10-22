## New in {VERSION}

- Work with raster layers: add, reorder, hide, duplicate, rename and merge down.
- Save editable OpenRaster (.ora) projects; export flattened PNG, JPEG, BMP or WebP.
- A rounded glass Layers panel, thumbnail previews and a fixed background layer.
- Invert image colors, use the footer zoom slider, and right-drag with a grab cursor to pan.
- Refined tool icons and simpler Linux glass setup in one user-run terminal block.
- **macOS on Apple silicon:** an app bundle with native glass and Command shortcuts.
- New light/dark website and README pictures, with glass at 40% surface opacity.

## Downloads

- **macOS 14.4+ / Apple silicon:** open `macos-arm64.dmg` and drag OpenPaint to
  Applications. This build is ad-hoc signed, **not Apple notarized or Developer ID
  signed**. See the installation guide for macOS first-launch approval. Intel
  Macs are not included. Native glass works without a setup command and respects
  Reduce Transparency.
- **Ubuntu 24.04 / amd64:** install the `.deb` using
  `sudo apt install ./OpenPaint-{VERSION}-ubuntu24.04-amd64.deb`.
- **Other Linux desktops / x86_64:** install the `.flatpak` with
  `flatpak install --user ./OpenPaint-{VERSION}-x86_64.flatpak`.
  The KDE runtime is downloaded from Flathub; this is not a Flathub listing.
- **Portable Linux:** extract `linux-x86_64.tar.gz` and run `openpaint` inside.
  Requires Ubuntu 24.04-compatible libraries. Keep the whole folder together.

Linux native packages require SSSE3, SSE4.1, SSE4.2 and POPCNT CPU features.
Download `SHA256SUMS` from this release. On Linux use
`sha256sum --ignore-missing -c SHA256SUMS`; on macOS compare `shasum -a 256`
for your downloaded DMG against its checksum entry. Checksums are not signatures.
The sources-and-notices archive contains matching source, licenses and inventories
for all formats; it is not an installer.

Glass defaults to 40% surface opacity and preserves existing preferences. Linux
live blur is validated on Ubuntu 24.04 / GNOME 46 / X11 with Blur My Shell;
unsupported configurations use opaque surfaces. OpenPaint does not execute the
setup command. Native packages check GitHub Releases automatically; Flatpak uses
the browser. Updates are installed manually on every platform.

Editing works offline, with no account or telemetry. Projects support up to 32
layers within the bounded image-memory budget and a supported OpenRaster subset.
Crash recovery, printing, AI tools and automatic update installation are not
included. Save frequently and keep copies of important originals. See the
installation guide and source release evidence for validation limits.
