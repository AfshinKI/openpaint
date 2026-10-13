# Feature: bounded common-image import

Status: implemented; protected CI required before merge
Feature IDs: F15, F26
Milestone: M2

## Outcome and boundary

Open accepts static PNG, JPEG, BMP and WebP by file signature. Decoding uses only
those named Qt codecs, runs on a worker, and retains the existing 128 MiB encoded,
16,384-pixel side and 32-million-pixel limits. Animated WebP is rejected. Imports
normalize orientation and tagged color to sRGB and copy pixels into a fresh image
without text, EXIF or other source metadata. Untagged input is assumed sRGB.

PNG retains its safe Save workflow. Other formats open as imported drawings;
Save asks for a PNG destination, preserving the imported original. A notice explains
this behavior. Export format/options are a separate next PR. No network access.

## Acceptance

- [x] Generated JPEG/BMP/WebP files open, regardless of filename extension.
- [x] Dimensions/encoded size, damaged containers, animations and unsupported
      formats are rejected without replacing the drawing.
- [x] Alpha, orientation, color and metadata handling are exercised with fixtures.
- [x] Cancel preserves the drawing; imported files never become PNG Save targets.
- [x] Existing PNG and keyboard file workflows, format/lint and sanitizers pass.

## Implementation and evidence

ImageIo selects the codec from bytes and validates container boundaries before
QImageReader. PngIo retains its stricter chunk reader and shares raster sanitation.
Editor chooses a backing URL only for PNG content with a PNG extension. Open's
native file filter lists all supported formats. No fixture files are redistributed.

Qt 6.8.3/Linux x86_64: all 11 development and ASan/UBSan suites pass;
`image-io` covers signature selection, source preservation, cancellation,
metadata removal, P3 conversion, JPEG/WebP orientation, WebP alpha and malformed
containers/limits. The tiny-WebP fixture also covers Qt 6.8's minimum probe-size
workaround (a legal ignored RIFF chunk in the private decode buffer).
CI exposed a race in the earlier transform-cancel shell test: its keyboard helper
pumped events before Escape, allowing a tiny operation to finish. A semaphore
now holds the worker until Escape is delivered, preserving the original assertion.
Existing X11 keyboard file/cancel workflows pass (4 test results). Strict QML
lint, C++/QML formatting, repository checks and seven tooling tests pass.

BMP V4/V5 color headers are deliberately rejected because the Qt BMP codec does
not normalize those color descriptions. Full decoder fuzzing remains a stable
release requirement. No downloadable package is claimed by this change.

Next step: explicit lossy/alpha export options, then packaging.
