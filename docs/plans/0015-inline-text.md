# Feature: inline raster text

Status: implemented; protected CI required before merge
Feature IDs: F07
Milestone: M2

## Behavior

Choose Text and drag a box, or click for a default 300×120-pixel box clipped to the
image. Type plain text inline. Text options select a system font, pixel size
(1–512), bold/italic/underline/strikeout, left/center/right alignment, geometry and
an optional secondary-color background. Primary color draws text; a right-button
start swaps colors. Text wraps inside the box; content outside it is clipped.
No rich HTML, external fonts or remote resources are loaded. Limit: 10,000 UTF-16
code units per box. Nothing is sent over the network.

Ctrl+Enter or Apply commits one raster undo command. Escape/Cancel restores the
original. Normal typing, selection, clipboard and local text undo stay with the
inline editor. Tools are disabled until Apply/Cancel; New/Open/Close asks to apply,
discard or keep editing the text before its normal unsaved-document guard. Save
explicitly commits current text before writing. Formatting dialog Close preserves
the draft. After commit, text is raster content and cannot be re-edited as text.

## Implementation

C++ owns draft text, bounds and formatting and renders a bounded QTextDocument
patch through the existing preview transaction. QML TextEdit supplies inline input;
while editing the canvas displays the immutable base so text is not drawn twice.
The view scales the editor with canvas coordinates. A finished drag requests text
focus, avoiding focus loss prematurely ending box placement. System FontDialog is
user initiated. No new dependency or file format.

## Acceptance

- [x] Plain/unicode/multiline text renders within bounds with formatting/background.
- [x] Draft edits, cancel, one-step undo/redo and PNG round-trip are correct.
- [x] Invalid size/oversized text is rejected without changing content.
- [x] Inline keyboard typing/undo, Ctrl+Enter/Escape and tool/file guards work.
- [x] Two-row tools and text controls fit minimum light/dark layouts.
- [x] Development, sanitizer, QML/format/privacy and protected CI checks pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all eight development and sanitizer suites pass.
The isolated X11 text workflow covers actual pointer placement, typing, local
undo/redo, Ctrl+Enter/Escape and New transitions (Apply/Discard/Cancel). An X11
Ctrl+Y discrepancy was reproduced and fixed with an explicit inline redo handler.
Unit tests cover Unicode/plain text, formatting, clipping, rejected limits,
undo/redo and PNG round-trip. Format/QML/privacy checks and seven tooling tests
pass. Native font-dialog and IME platform certification remain part of M4.
Synthetic captures reviewed: [minimum light](../assets/text-light-1024.png),
[light options](../assets/text-options-light.png),
[dark options](../assets/text-options-dark.png).

Next M2 slices: selection/clipboard, geometry transforms, image formats,
then packages and release validation. Text is not a persistent vector object.
