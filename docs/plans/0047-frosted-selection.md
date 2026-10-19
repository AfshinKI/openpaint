# Feature: frosted selected controls

Status: implemented; local checks passed; PR deferred
Feature IDs: F01

## Acceptance and behavior

Selected/checked buttons in glass mode use a stronger neutral frost tint, stronger
in-app blur and a light outline. This includes Pencil/Eraser/Text, shapes, formatting,
and Selection/Brush split controls. Hover/press and keyboard focus remain distinct.
Disabling glass retains the existing green selection style. Opaque 100% surfaces
remain opaque. No tool semantics, artwork or undo changes.

Blur samples only the base window and canvas, never the selected control or its
parent toolbar/popup. Desktop blur intensity is still compositor-owned; software
rendering uses the neutral frost tint and outline without shaders.

- [x] Selected state follows tool changes and keyboard navigation.
- [x] Light/dark minimum-size captures and normal non-glass behavior pass.
- [x] No recursive capture warnings or document/history mutations.

Dev build, strict QML lint, repository checks, frostedToolSelection,
ribbonShapeKeyboard and brushMainActivation passed offscreen/software.
X11/OpenGL frostedToolSelection and glassBackdropRendering passed, with synthetic
light/dark captures inspected. No additional desktop certification is implied.
The eraser-specific ribbon still overflows the palette at minimum width; this
existing layout limitation is separate from the selection material change.
Next: later PR/package review.
