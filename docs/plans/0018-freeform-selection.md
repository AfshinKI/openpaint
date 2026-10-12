# Feature: free-form and transparent selection

Status: implemented; protected CI required before merge
Feature IDs: F08
Milestone: M2

## Outcome

Selection offers Rectangle and Free-form modes. Drag a lasso; release closes it
with odd-even filling on whole pixels. During input, show the path without
rasterizing its mask for every pointer event. Limit paths to 4,096 distinct
consecutive pixel positions; excessive or degenerate paths restore the prior
selection. A 4-million segment-row estimate and 16,384-region-rectangle ceiling
bound mask construction and outline complexity; larger paths are rejected without
changing the prior selection. Escape cancels. Invert selects the image minus the current region;
inverting a full-image selection deselects. Neither operation dirties artwork.
Numeric selection bounds deliberately replace an irregular mask with a rectangle.

All existing clipping, copy/cut/delete/move behavior works with irregular masks.
The marquee shows only the boundary, without internal scanline rectangles.
Transparent selection is off by default. When enabled, exact premultiplied RGBA
matches to the secondary color become transparent in copied or floating content.
Other colors and partial alpha remain unchanged. This is a color key, not fuzzy
background removal. Moving/cutting still clears the entire original selected
region to transparency; Delete is unaffected by the color key.

Changing the toggle or secondary color updates a pending move/paste without
accumulating raster damage. Switching the toggle off restores the original
floating pixels. Clipboard encoders capture the chosen key when Copy/Cut begins.

## Acceptance

- [x] Lasso, reverse/self-crossing paths, degenerate/oversized rejection and cancel preserve state.
- [x] Inversion is bounded and transient; irregular holes survive copy/move/delete and undo.
- [x] Color key affects only exact matches and can be toggled during a preview.
- [x] Marquee/menu and all prior keyboard workflows fit both minimum-size themes.
- [x] Development/sanitizer, QML/format/privacy and protected CI pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: nine development suites and the complete ASan/UBSan
rerun pass. The first sanitizer process was terminated during its shell test;
no checks were disabled. Tests cover lasso cancellation/degeneracy/point limits,
odd-even paths, inversion, irregular move holes, exact color/alpha keying and
reversible preview toggling. Isolated X11 menu/drag tests pass. Formatting,
strict QML lint, privacy/link checks and seven tooling tests pass. Reviewed
synthetic captures: [light lasso](../assets/lasso-light-1024.png),
[dark menu](../assets/lasso-menu-dark.png).

Next: crop/resize/skew/flip/right-angle rotation, image formats and packages.
