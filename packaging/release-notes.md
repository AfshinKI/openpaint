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

Layers, editable project files, crash recovery, printing and automatic updates are
not included. Save frequently and keep copies of important originals. The latest
stable minor line receives fixes. Broader desktop/input/accessibility certification
remains future work; see the installation guide and release evidence in the source.
