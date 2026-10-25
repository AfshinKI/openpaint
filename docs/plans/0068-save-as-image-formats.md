# Feature: Save As format choices and monochrome BMP

Status: acceptance defined; native validation deferred
Feature IDs: F15
Target: v2.1.0

Save As offers PNG, OpenRaster, JPEG, color BMP, monochrome BMP and WebP through
one options chooser. PNG (flat documents) and OpenRaster save the document.
Other image choices write an explicitly labelled copy through the existing export
pipeline; layered PNG is also a copy. They never mark an editable project saved
or satisfy pending close/new/open guards. Normal Save remains fidelity-preserving.

Monochrome BMP is an actual 1-bit black/white BMP: flatten onto the selected opaque
background, then threshold Qt grayscale at 128 without dithering. Disclose color
and transparency loss before destination selection. No other new codec is implied.

Acceptance: genuine 1-bit BMP header and pixel round-trip; transparent flattening,
odd-width row padding, cancellation and failed writes preserve originals; existing
image formats/ORA remain available; cancel keeps document identity/history; no
suffix is appended outside a portal grant. UI choices disclose copy semantics.
Tests use generated images; native build/execution deferred to final pass.
