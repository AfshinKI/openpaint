# Feature: movable selection and image clipboard

Status: implemented; protected CI required before merge
Feature IDs: F08
Milestone: M2

## Outcome

Copy/Cut publishes selected pixels as PNG on the explicit clipboard (never the
primary selection). Cut clears alpha only after successful encoding. Paste accepts
PNG clipboard data through the same bounded, metadata-cleaning parser as file open:
128 MiB encoded, 16,384 per side, 32 million pixels. Other MIME formats are rejected
without changing the document; ordinary inline text clipboard remains independent.
No clipboard read happens until Paste. No temporary image file or network is used. PNG decode and copy encoding run on cancellable workers. Clipboard transfer itself
stays on the GUI thread; cancelled Cut never clears pixels or replaces the clipboard.

Paste starts at the image origin as a floating preview. Drag it, use arrows (Shift: 10 pixels) or set numeric X/Y,
then Enter/Apply commits one undo step; Escape/Cancel restores pixels and the prior
selection. Dragging inside an existing rectangle starts a move preview, clearing
its source to transparency and compositing the selected pixels over the destination.
Alpha is preserved with source-over compositing. Partial off-canvas placement is
clipped; a fully invisible placement is rejected. No automatic canvas expansion.
Clicking elsewhere inside the image commits it before starting a new selection.
Changing tools or saving commits; file replacement then uses the unsaved guard.

Rectangle bounds controls do not resize floating content; resize follows separately.
Copy/Cut/Paste are disabled during a retained preview or loading/filling. Input
fields retain their own clipboard shortcuts. This slice does not claim rich-text,
file-URL or arbitrary-MIME clipboard support.

## Acceptance

- [x] Selected alpha and holes survive copy; clipboard contains PNG, no source metadata.
- [x] Valid PNG import shares file validation; corrupt/oversized/unsupported input preserves work.
- [x] Move/paste overlap, partial clipping, cancellation and one-step undo/redo preserve pixels.
- [x] Pointer/keyboard controls, numeric position and file/tool transitions resolve previews.
- [x] Core typing shortcuts still work and both themes fit at minimum size.
- [x] Development, sanitizer, QML/format/privacy and protected CI pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all nine development and ASan/UBSan suites pass.
Clipboard tests cover alpha/holes/metadata, asynchronous copy/cut/paste, cancelled
Cut preserving the prior clipboard, malformed/oversized PNG, cancelled decoding,
overlapping move, clipped paste, cancel/undo/redo and saved pixel equality.
Isolated X11 tests exercise pointer placement, arrow nudging, Ctrl+C/X/V,
Enter/Escape and independent inline-text clipboard shortcuts. Strict QML lint,
format/privacy/link checks and seven tooling tests pass. Synthetic captures:
[minimum light](../assets/clipboard-light-1024.png),
[dark position controls](../assets/clipboard-position-dark.png).
Other applications must offer PNG MIME data; broad desktop/clipboard-manager
interoperability remains part of the platform matrix.

Next: free-form selection, transforms, formats, packages and alpha gate.
