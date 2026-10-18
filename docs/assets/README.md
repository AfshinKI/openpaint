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
