# Feature: shape palette, curves and polygons

Status: implemented
Feature IDs: F06
Milestone: M2

## Behavior

Extend the three basic shapes to a 19-item original icon palette: line, rectangle,
ellipse, rounded rectangle, triangle, diamond, pentagon, hexagon, four directional
arrows, five-point star, rectangular speech bubble, curve, polygon, four-point star,
six-point star and oval speech bubble. This finite alpha inventory does not claim
an exhaustive match to every reference build.

Shift constrains line/curve/polygon segments to 45-degree increments and other
shapes to equal width/height while dragging. Numeric coordinates remain explicit.
Curves have two perpendicular bend controls (−200% to 200% of chord length), default
+30%/−30%, editable in Shape options. Polygons start with a drag for the first edge;
subsequent clicks/drags add vertices. Backspace removes the last vertex; Enter
closes and commits with at least three vertices, otherwise cancels the incomplete
polygon. Escape cancels; tool/file transitions follow the basic-shape contract.
Undo cancels a pending shape without undoing earlier artwork; redo stays disabled
until the preview is resolved. Up to 4096 vertices are accepted. All styles reuse outline/fill and undo behavior.

## Implementation and acceptance

Use QPainterPath geometry in the existing bounded preview renderer. Palette icons
are generated from that renderer; no external assets. Extend controller point
state and normalized modifier input, not document ownership or persistence.

- [x] Every palette shape renders within a bounded patch and has a distinct preview.
- [x] Curve controls affect pixels; invalid coordinates/point counts are rejected.
- [x] Polygon vertex addition/removal, minimum count, cancel and undo are tested.
- [x] Shift constraints preserve gesture ownership and input mapping.
- [x] Icon palette is keyboard reachable and fits both themes at minimum size.
- [x] Existing shape/save tests, sanitizers, QML and privacy checks pass.

## Evidence and next step

Qt 6.8.3/Linux x86_64: all seven development suites passed; X11 shell passed
21 results. Geometry tests compare all 19 outputs, bounded rectangles, curve
changes and rejected oversized point lists. Controller tests cover polygon
minimum/add/remove/commit/undo and Shift geometry; canvas tests apply Shift at
fractional zoom. Palette arrows/activation pass the keyboard shell test. Format,
QML and privacy checks passed. The first sanitizer invocation was terminated
before its shell result; the focused shell rerun and subsequent complete seven-suite
sanitizer run passed. Protected CI remains required before merge.
Synthetic minimum-window captures reviewed:
[light](../assets/shape-palette-light.png), [dark](../assets/shape-palette-dark.png).
The same shell run checks the typical window and existing file workflows.

Next M2 slice: inline raster text and formatting. Shape objects rasterize on
commit; persistent vector editing and full reference-build parity remain outside
this alpha feature claim.
