## New in {VERSION}

- Select every exact matching pixel with **Select → By color**, the magic wand.
- Change the font, size or style of selected text, or just what you type next.
- Draw multi-point splines with preview, apply/cancel and undo/redo.
- Clearer eraser modes, tool icons, searchable fonts and selected-tool feedback.
- A one-time welcome and separate **Enable Glass** setup with actual app pictures.
- Opaque fallback when desktop glass is unavailable; setup is hidden when ready.
- Simpler Updates with automatic startup checks and an angled app preview.
- A unified color-dialog surface, including its OK/Cancel footer.

Glass defaults to 40% surface opacity and preserves your saved preferences.
The validated desktop-blur setup is Ubuntu 24.04 / GNOME 46 / X11 with Blur My Shell.
Other or unverified setups use opaque surfaces. OpenPaint never installs or
configures extensions; setup commands run only if you copy and execute them.
Native packages check GitHub Releases at startup and periodically, with cooldown
and backoff. Flatpak remains browser-only. Updates are always installed manually.

OpenPaint is an offline, single-layer Linux paint editor with brushes, fill, shapes,
text, selections, image transformations and undo/redo. Open/export PNG, JPEG, BMP
and WebP; save working drawings as PNG.

Choose one Linux x86_64 download. Native DEB/portable packages require SSSE3,
SSE4.1, SSE4.2 and POPCNT CPU features:

- **Ubuntu 24.04:** download the `.deb`, then install with
  `sudo apt install ./OpenPaint-{VERSION}-ubuntu24.04-amd64.deb`. OpenPaint appears in
  the app menu and image Open With choices. Update by installing a newer DEB;
  remove with `sudo apt remove openpaint`.
- **Other Linux distributions:** install the `.flatpak` using
  `flatpak install --user ./OpenPaint-{VERSION}-x86_64.flatpak`. The KDE runtime is
  downloaded from Flathub. Launch with `flatpak run io.github.AfshinKI.OpenPaint`.
  Replace with a newer bundle manually; this is not a Flathub listing or update feed.
- **Portable:** extract `linux-x86_64.tar.gz` and run `openpaint` inside its folder.
  Keep the whole folder together. Requires Ubuntu 24.04-compatible system libraries.

Download `SHA256SUMS` and verify downloaded files with
`sha256sum --ignore-missing -c SHA256SUMS`. The sources-and-notices archive contains
matching source, dependency licenses and inventories for all formats; it is not an
installer. Checksums are not signing signatures.

Layers, editable project files, crash recovery, printing and automatic update installation are
not included. Save frequently and keep copies of important originals. The latest
stable minor line receives fixes. Broader desktop/input/accessibility certification
remains future work; see the installation guide and release evidence in the source.
