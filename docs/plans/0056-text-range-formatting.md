# Feature: font changes within a text box

Status: implemented locally; automated and software-renderer UI checks passed.

## User outcome and behavior

Font family, size and character-style changes apply to the highlighted text. With
no highlight, they set the insertion format for subsequent typing in the same box,
leaving existing characters unchanged. Choosing a font retains the text highlight
through the picker. Canceling the picker changes nothing. New boxes inherit the
current font settings. Alignment, foreground/background color and background mode
remain box-wide in this slice.

## Implementation

C++ merges local character formatting, with a pending format for newly inserted
characters grouped with native keyboard/paste/input-method input in one local undo
edit. Input limits are checked before the native edit. Qt Quick supplies caret/selection state.
The transient text
document remains plain-text input (no HTML/resource loading), at logical canvas
pixel size; zoom scales the view. C++ clones that document for raster preview and
commit, preserving font runs. Saved artwork remains owned by Document and uses the
existing single undoable preview transaction. Headless/plain-text entry retains
its existing rendering path. No new dependency, networking or file format.

## Acceptance cases

- [x] Highlight a substring, choose a family, and only that range changes.
- [x] Choose a font at the caret, type, and only new characters use it.
- [x] Font size/styles follow the same selection/insertion behavior.
- [x] Picker cancel, text undo/redo and focus preserve ranges and content.
- [x] Mixed-format preview, zoom, commit/save and document undo/redo agree.
- [x] Plain-text paste, character limits and existing text workflows still pass.

## Validation and handoff

Targeted text UI workflows passed: selected family and mixed size/style preservation,
caret-only changes, typing inside a word, Unicode input-method commits, plain-text
paste, oversized paste rejection, picker cancel, local undo/redo, zoom invariance,
PNG save and document undo/redo. The generated-artwork screenshot
`build/text-format-evidence/mixed-font-text.png` was visually reviewed at 150% zoom.
It uses Qt's software renderer; no new native compositor or IME-engine certification
is claimed. The existing text workflows also pass. See [ADR 0007](../decisions/0007-inline-text-formats.md).

Final validation: development build succeeded; all 16 CTest suites passed (77.26 seconds), including the new text workflows. Formatting (88 C++/QML files), zero-warning QML lint, repository and whitespace checks passed. Next delivery step: review/commit of this change;
release packaging is outside the request.
