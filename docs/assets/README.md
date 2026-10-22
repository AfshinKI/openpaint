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

## Layer feature development evidence

`layers-light-1280.png` and `layers-dark-1024.png` were captured on 2026-10-21
from the plan 0059 colored-ribbon follow-up revision using `openpaint_shell_tests layersWorkflow`,
Qt 6.12.0 offscreen/software, at 1280×800 and 1024×640. They show a generated
orange stroke at 65% layer opacity, an empty transparent layer, and the pinned
white background below its divider. The right rail uses top-right hover/focus eye controls, a circular add button
and contextual commands, with glass enabled at 40% surface opacity. These are development
UI harness captures, not release-build or native desktop-blur evidence. Artwork
and icons are project-authored (GPL-3.0-or-later). Settings are isolated by the
existing shell harness; no user artwork or desktop backdrop was captured.

`layers-context-menu.png` uses the same synthetic harness and source revision at
1450×1008. It records the original context-action icons and circular add control;
it is also development evidence, not release/compositor certification.

`invert-colors-ribbon.png` was generated by `openpaint_shell_tests
invertColorsWorkflow` on 2026-10-21 using Qt 6.12.0 offscreen/software at
1024×640. It shows a generated white canvas inverted to black and the original
contrast icon in the Image group. The isolated settings and development-only
evidence limitations above also apply.

`colored-ribbon-light.png` and `colored-ribbon-dark.png` were captured by
`openpaint_shell_tests coloredRibbonWorkflow` on 2026-10-21 from the colored-icon
follow-up, Qt 6.12.0 offscreen/software at 1024×640. The blank white canvas is
generated; red foreground and blue background selections demonstrate the live
fill/brush paint regions. All vectors are original project artwork. Settings are
isolated; these are development UI captures, not release/compositor evidence.
The layer and inversion captures above were refreshed from the same source.

## Zoom and simplified blur setup development evidence

`zoom-slider-1024.png` and `glass-one-block-setup.png` were captured on
2026-10-21 from plan 0060 using the isolated `zoomSliderWorkflow` and
`glassSetupWorkflow` harnesses, Qt 6.12.0 offscreen/software at 1024×640.
They show generated blank artwork and mocked desktop capability results.
These are development UI evidence, not actual extension-installation or
compositor certification. No private artwork/settings were used. Existing
plan 0059 captures above retain their historical revision and icon styling.

## v2.0.0 website and README captures

The four `openpaint-2.0.0-*.png` images were freshly captured on 2026-10-21
from application source `1b5dfefa03566c90a4498dd861306e2db38c3265`, Qt 6.12.0,
CMake Release configuration, using `websiteReleasePreview` in an isolated
X11/OpenGL session. Both themes have glass enabled at **40% surface opacity**.
The project-authored landscape is a real four-layer OpenRaster document; the
Layers panel is open in each capture. The gradient backdrop is generated by
the harness. These are actual app UI captures with staged backing, not evidence
of desktop compositor blur or native macOS glass. No user artwork or settings
were changed; all demo artwork, icons and backing are GPL-3.0-or-later.

The 1449 × 1008 app-only images retain their UI pixels, with a 22px antialiased
transparent outer-corner mask. Light/dark, color-editor and resize captures
replace the v1.3 images on the website and README; historical files remain
unchanged. Reproduce from the release build with `OPENPAINT_RELEASE_GLASS=1`,
`OPENPAINT_CAPTURE_DIR`, and the `websiteReleasePreview` shell-test function.
