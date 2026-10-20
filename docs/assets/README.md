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
The staged region was captured at 1550 × 1140 pixels, then cropped to the app
window at (40, 60, 1489, 1105), yielding 1449 × 1045 pixels. A 22px rounded
alpha mask (4× supersampled) removes the outer corners. No surrounding desktop
border remains. The composed app pixels are unchanged, including the scenery
visible through its glass; rounding is a presentation treatment, not an app UI change.
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

## Blur onboarding design preview

`onboarding-opaque-design.png` was captured on 2026-10-19 with the existing
`build/dev/openpaint_shell_tests` binary using its `websiteReleasePreview` slot,
`QT_QPA_PLATFORM=offscreen`, and `QT_QUICK_BACKEND=software`. The checkout was
`88f0b38`; the existing binary was not rebuilt or asserted to match that commit.
Glass is explicitly disabled by the harness. Temporary generated mountain artwork
and isolated preferences preserve user documents and settings. The capture slot
passed, and the image was visually inspected. This is design evidence, not a new
release screenshot. The generated artwork and capture are GPL-3.0-or-later.

The onboarding mockup pairs this opaque capture with the existing, unmodified
`openpaint-1.2.0-light.png` release capture documented above. CSS clips the upper
left corners and scales them for presentation; screenshot pixels are not repainted.
These captures have different source versions and sample artwork. The glass image
is labeled "Glass appearance": its provenance does not establish native desktop
blur. Updates reuses the same real release image. A matched pair from the final
release build remains required before shipping the onboarding assets.

## v1.3.0 website and README captures

The four `openpaint-1.3.0-*.png` images were captured on 2026-10-20 UTC from source
`158fc1bf08e9a7dd5b9c1382d3f174a23785ef0f`, using a clean Qt 6.12.0 Release build and the
`websiteReleasePreview` UI harness on Ubuntu 24.04 / X11 / OpenGL. These are real
application QML/C++ surfaces in an isolated harness, not installed-executable or
desktop-compositor screenshots. Light and dark both use glass at 40% surface
opacity. The original gradient backing is staged inside the test window; landscape
artwork is generated by QPainter. Neither private desktop content nor user files
are captured. Temporary preferences preserve the user's application settings.

Each 1449 × 1008 capture includes only the app client window. A 22px antialiased
alpha mask, rendered at 4× resolution, rounds the outer corners; interior UI pixels
are unchanged. Light, dark, color editor and resize images were visually reviewed.
PNG metadata contains no text, EXIF or private paths. All artwork, backing and
capture code are GPL-3.0-or-later. Prior v1.2.0 images remain historically labeled.

Reproduce after a Release build (with Qt 6.12.0 and working X11/OpenGL):

```sh
QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=rhi QSG_RHI_BACKEND=opengl \
OPENPAINT_RELEASE_GLASS=1 OPENPAINT_CAPTURE_DIR="$PWD/docs/assets" \
build/release/openpaint_shell_tests websiteReleasePreview
```
