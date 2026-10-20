# Embedded appearance previews

These are unmodified real OpenPaint PNG captures (GPL-3.0-or-later).
`glass.png` is `docs/assets/openpaint-1.2.0-light.png`, the historical installed
v1.2.0 capture. `opaque.png` is `docs/assets/onboarding-opaque-design.png`, an
opaque development-harness capture. See [provenance](../../docs/assets/README.md).
ScreenshotPreview crops the corner at display time and clips it to rounded
corners with a soft shadow. It does not repaint captured controls. QPainter
provides this presentation on both software and GPU renderers.
The historical glass image demonstrates transparency, not independently verified
compositor blur. These are appearance examples, not screenshots of this release.
