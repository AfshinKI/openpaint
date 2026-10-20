# Feature: select by color

Status: implemented locally; automated and software-renderer UI checks passed.

## User outcome and scope

Select → By color uses a magic-wand icon. Each canvas click replaces the selection
with all pixels matching the sampled color, including disconnected areas. Matching
is exact in the document's premultiplied RGBA representation, including alpha.
Tolerance, contiguous-only mode and additive selection are outside this slice.

## Behavior and implementation

The existing Select tool gains a color mode. Every valid click samples again,
including clicks inside the previous selection; dragging does not paint or move.
Existing keyboard nudging, clipboard, inversion, deletion and resize actions use
the resulting mask. Pending moved content commits before sampling. Outside-canvas
clicks and right clicks do not replace the selection. Other selection modes retain
their existing gestures. Selection itself does not dirty pixels or enter content
undo history; subsequent pixel edits remain undoable.

A cancellable C++ worker scans an immutable image snapshot into bounded horizontal
runs, merging repeated rows. Results apply on the GUI thread only at the captured
revision. Busy state blocks conflicting edits; Escape cancels and retains the prior
selection. More than 16,384 region rectangles fails with a short message, retaining
the prior mask. No new dependency, networking or persistent setting is introduced.

## Acceptance cases

- [x] Menu and toolbar expose the wand with an accessible By color label.
- [x] Click selects separated exact-color pixels and excludes near colors/alpha.
- [x] A second click replaces the selection, even within prior bounds; drag is inert.
- [x] Selecting preserves pixels/dirty state; delete/move undo restores pixels.
- [x] Cancellation, invalid seed, complexity and stale result preserve prior state.
- [x] Rectangle/free-form behavior, clipboard and keyboard interaction still pass.

## Validation and handoff

Development build succeeded. All 16 CTest suites passed (77.08 seconds), including
new clipboard/controller cases for exact matches, repeated clicks, alpha, cancellation,
stale revisions, complexity limits, merged bands, copying and undoable moves/deletes.
The shell workflow covers keyboard menu activation and real canvas clicks; existing
rectangle/free-form workflows also pass. Formatting (88 C++/QML files), zero-warning
QML lint, repository and whitespace checks passed.

The generated-artwork capture `build/color-selection-evidence/color-selection-menu.png`
was visually reviewed at 1024×640 using Qt's offscreen/software renderer. The wand
is original SVG artwork; no third-party image source was used. Hardware-renderer
and manual desktop confirmation remain unverified. The implementation is ready for
review; commit/push and release packaging are the next unfinished delivery steps.
