# Feature: resize and skew

Status: implemented; protected CI required before merge
Feature IDs: F09
Milestone: M2

## Outcome

Image → Resize/Skew acts on the selected content, or the entire image when no
selection exists. Choose pixels or percentages, maintain aspect ratio, and smooth
or nearest-neighbor resampling. Horizontal/vertical skew accepts −89° through 89°.
Selection masks follow resampled coverage, including smooth edges; a fully
subpixel mask retains a bounding box for manipulation. Resize happens before skew; the resulting bounds expand to hold the transformed
image, with transparent uncovered areas. Pixel dimensions are rounded to integers.

The dialog edits local values only. Cancel changes nothing. Accept closes the
dialog before starting a cancellable worker. Other document shortcuts are disabled
while it is open. Selected results enter the existing movable Apply/Cancel preview;
whole-image results use one dimension-changing history command. Invalid dimensions,
nonfinite/singular transforms, exceeded output/history limits or cancelled work
leave the image unchanged. No output allocation occurs before bounds validation.

## Acceptance

- [x] Resize dimensions, interpolation differences, skew bounds, alpha and masks are tested.
- [x] Invalid/oversized/singular requests are refused; cancellation preserves original content.
- [x] Image and selection results support one-step commit/undo/redo or preview cancel.
- [x] Pixel/percent, aspect lock, keyboard fields and dialog Cancel/Accept work in both themes.
- [x] Development/sanitizer, QML/format/privacy and protected CI pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all ten development and ASan/UBSan suites pass.
Tests cover nearest exact mapping, smooth differences/edge coverage, mask scaling,
skew bounds/alpha, invalid/oversized/singular requests, cancellation and image/
selection history. Isolated X11 keyboard tests cover aspect lock, pixel/percent
conversion, modal shortcut guards, Cancel and accepted 50% resize with undo.
Strict QML lint, format/privacy/link checks and seven tooling tests pass.
Synthetic minimum-window captures reviewed: [light](../assets/resize-light-1024.png),
[dark](../assets/resize-dark-1024.png).

Next: JPEG/BMP/WebP interchange, install identity/Updates and packages.
