# Reference material

Reviewed 2026-10-12. Primary sources inform the plan; design choices belong to
OpenPaint. Marketing pages and preview announcements do not prove identical
availability on every Windows machine. No claim is made that this list captures
every Paint update through the review date.

## Paint scope

| ID | Source | What it establishes |
| --- | --- | --- |
| R1 | [Microsoft Paint product page](https://www.microsoft.com/en-us/windows/paint) | Core tools, layers, background removal, image generation, image manipulation, stylus and color controls |
| R2 | [Windows experiences, 2025-10-16](https://blogs.windows.com/windowsexperience/2025/10/16/new-experiences-currently-rolling-out-for-windows-11/) | Editable .paint project files and pencil/brush opacity in a general rollout announcement |
| R3 | [Paint update, 2026-01-21](https://blogs.windows.com/windows-insider/2026/01/21/notepad-and-paint-updates-begin-rolling-out-to-windows-insiders/) | Coloring-book generation and fill tolerance; announced to Insider Canary/Dev |
| R4 | [Paint in Insider build, 2026-02-27](https://blogs.windows.com/windows-insider/2026/02/27/announcing-windows-11-insider-preview-build-26220-7934-beta-channel/) | Freeform rotation of active shape/text/selection content in Paint 11.2601.391.0 |
| R5 | [Windows AI experiences, 2025-07-22](https://blogs.windows.com/windowsexperience/2025/07/22/windows-11-is-the-home-for-ai-on-the-pc-with-even-more-experiences-available-today/) | Sticker generator and object select, with reference hardware restrictions |
| R6 | [Paint AI update, 2024-11-06](https://blogs.windows.com/windows-insider/2024/11/06/new-ai-experiences-for-paint-and-notepad-begin-rolling-out-to-windows-insiders/) | Generative fill/erase and Cocreator direction; availability differs by feature |

The next audit requires a real reference installation with build/channel recorded.
Link public source material instead of redistributing Microsoft screenshots or assets.

## Engineering

- [Qt licensing](https://doc.qt.io/qt-6/licensing.html): module-specific license choices.
- [Qt QImage](https://doc.qt.io/qt-6/qimage.html): raster formats and image handling.
- [Qt Quick scene graph](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html): rendering architecture and thread boundaries.
- [QQuickPaintedItem](https://doc.qt.io/qt-6/qquickpainteditem.html): initial canvas adapter and rendering limitations.
- [QSaveFile](https://doc.qt.io/qt-6/qsavefile.html): safe file replacement and fallback considerations.
- [Qt Quick accessibility](https://doc.qt.io/qt-6/accessible-qtquick.html): standard controls and accessible metadata.
- [OpenRaster specification](https://www.openraster.org/): layered raster interchange; verify supported subset during M3.
- [Flatpak desktop integration](https://docs.flatpak.org/en/latest/desktop-integration.html): portals and desktop integration.
- [Flatpak usage](https://docs.flatpak.org/en/latest/using-flatpak.html): installation and package-managed updates.
- [AppImage updates](https://docs.appimage.org/packaging-guide/optional/updates.html) and [signatures](https://docs.appimage.org/packaging-guide/optional/signatures.html): transfer and signing facilities; not a substitute for our updater verification tests.
- [GitHub Actions security](https://docs.github.com/en/actions/reference/security/secure-use): workflow permissions and dependency pinning.
- [Codex AGENTS.md](https://developers.openai.com/codex/guides/agents-md) and [Claude Code memory/imports](https://code.claude.com/docs/en/memory): shared repository instructions.
- [GNU GPL version 3](https://www.gnu.org/licenses/gpl-3.0.html): project license text; a copy is included at the repository root.

These links are reference evidence, not a dependency lockfile. Pin versions and
hashes in the actual build/packaging configuration when those components land.
