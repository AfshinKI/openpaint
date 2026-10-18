# Documentation image provenance

`openpaint-frost-light.png` is a capture of the real OpenPaint Qt Quick UI at
1449 × 1008 in light mode with glass enabled at 54%. The landscape is procedural
project-authored QPainter artwork. A generated multicolor window backing demonstrates
alpha surfaces without capturing the desktop; it is not a photograph or a claim
of native compositor blur. The Brush menu uses the actual app-layer blur shader.

The image and generator are GPL-3.0-or-later, like the project. No third-party assets,
user documents, private desktop content or embedded author/EXIF metadata are used.

Regenerate after building the dev preset (requires a working GPU/X11 session):

```sh
QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=rhi QSG_RHI_BACKEND=opengl \
OPENPAINT_CAPTURE_DIR="$PWD/docs/assets" \
build/dev/openpaint_shell_tests readmeFrostPreview
```

Inspect the result before publishing. This explicit documentation capture is not
part of CTest; it creates only temporary demo artwork and isolated preferences.
Other historical test-window captures are documented in [the build guide](../BUILD.md).

## v1.2.0 website and README captures

The four `openpaint-1.2.0-*.png` files were captured on 2026-10-18 from the
installed OpenPaint DEB `1.2.0-1`, corresponding to release source commit
`4fb7f036987dd80a9e485ae9aee2ed8536a6f510`. These are installed-application
captures, not source-harness renders. The executable was launched in a separate
instance with isolated XDG preferences and generated `Mountain study.png` artwork.

Glass is enabled; **Surface opacity is 40%** in both themes, verified in the
appearance dialog. The setting describes opacity, not a 40% transparency value.
GNOME/X11 supplies the desktop composition. An original illustrated sunset and
mountain backdrop was displayed in a temporary window behind the app. This is
actual composed window transparency, not a backdrop composited into screenshots.
The app, its shadows and the staged region were captured at 1550 × 1140 pixels.
No private windows, icons or personal wallpaper are included. The user's original
OpenPaint instance, artwork, settings and wallpaper were left untouched; the
capture instance and backdrop window were closed afterward.

The light/dark images show the same generated landscape. The colors image shows
the dark dialog, and resize shows the light dialog. All four images were visually
reviewed. PNG metadata is inspected before publication. Artwork and backdrop can
be regenerated with `python3 tools/generate_website_art.py`; Pillow is required.
The illustrations, generator and screenshots are GPL-3.0-or-later.

For future captures, follow the website checklist in [releases](../RELEASES.md#website-and-readme-release-checklist).
Verify the installed version, use isolated settings, stage only redistributable
content, and record theme, opacity, capture method and environment. Inspect every
image and update `website/release.json`, HTML references and README together.
The optional `websiteReleasePreview` source-harness slot is separate test tooling;
its opaque software-rendered output must not replace these installed glass images
without clearly changing the provenance and captions.
