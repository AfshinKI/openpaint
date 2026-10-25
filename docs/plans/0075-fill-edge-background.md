# Fix: recolor antialiased background edges

Status: implemented; local validation passed; protected review pending
Feature IDs: F04
Target: v2.1.0

Fill previously cleaned fringes only when the replacement matched the line color.
For a black pencil line on white, red fill left gray/white edge pixels behind.
Now infer a local ink/background coverage blend from stronger neighboring ink
within two pixels, and replace only the background contribution. Only the existing
one-pixel flood boundary is adjusted: inferred pixels never expand the flood.
Preserve line cores, unrelated colors, selection limits, cancellation and undo.

Raster images lack original coverage metadata. This conservative local inference
handles ordinary antialiased lines; it cannot perfectly reconstruct arbitrary
painted gradients, textures or every thin/translucent edge. Unrecognized edges
remain unchanged. Same-color cleanup and exact flood connectivity remain intact.

Regression covers black pencil on white/transparent backgrounds, red fill and
repeat blue fill, line coverage within two 8-bit rounding levels, no gray halo,
closed boundaries, unrelated colors, selection clipping and undo/redo. Targeted
fill tests pass in the combined Qt preview. The full document/editor CTest suites,
formatting, zero-warning QML lint and repository checks pass. Protected review
and required hosted checks remain.
