# Changelog

User-visible changes will be recorded here. There are no releases yet.

## Unreleased

- Add Help → Updates with build identity and explicit release-page browsing, plus
  desktop integration, an original application icon and single-image launch arguments.

- Export PNG/JPEG/BMP/WebP copies with explicit quality, lossless WebP and opaque
  background choices, without changing the working drawing’s saved state.

- Open static JPEG, BMP and WebP images with bounded worker decoding, orientation
  normalization and source-metadata removal. Save imports as PNG copies.

- Add pixel/percentage resize, aspect lock, smooth/nearest resampling and bounded
  horizontal/vertical skew for images and selected content.

- Add crop, flips and right-angle rotation with dimension-aware undo and selected
  content previews; keep quick Save/Undo/Redo beside the menus.

- Add free-form lasso selection, inversion and exact secondary-color transparency
  for copied/floating content, with reversible preview toggling.

- Add PNG copy/cut/paste and movable selection previews with drag, numeric position,
  arrow nudging, explicit Apply/Cancel and one-step undo.

- Add rectangular selection, numeric bounds and select-all/deselect, with clipping
  across all drawing tools and undoable deletion to transparency.

- Add inline text boxes with system fonts, style/alignment/background controls,
  local text undo, explicit raster commit and guarded file transitions.

- Expand Shapes to a 19-icon palette with curve bends, polygon point entry and
  Shift constraints. Undo cancels a pending shape without undoing earlier artwork.

- Add editable line, rectangle and ellipse previews with numeric geometry,
  outline/fill options and explicit Apply/Cancel.

- Add nine original brush styles, generated stroke previews and per-stroke opacity.

- Add cancellable contiguous fill with adjustable tolerance and one-step undo.

- Add a canvas color picker for primary/secondary colors, including transparency,
  without modifying artwork or undo history.

- Complete the M1 source-editor checkpoint with reproducible startup/render/save/memory measurements and documented platform limits.

- Give dialogs keyboard focus, provide a consistent Ctrl+Q shortcut, and correct PNG CLI help.

- Normalize PNG EXIF orientation while stripping metadata; reject corrupt chunk checksums and structure.

- Keep each canvas drag owned by its initiating button; prevent extra buttons and
  interrupted drags from switching modes or appending stray pixels. Include the
  final pan position and correct fractional image-edge hit testing.

- Refine the color editor from supplied visual references: rectangular spectrum,
  brightness slider, RGB/HSV mode selector, basic grid and 24 custom slots. Show
  ten hollow custom slots in the compact toolbar, with smaller icons and stacked
  primary/secondary circles. Existing saved palettes remain compatible.

- Replace temporary text toolbar actions with original icons and shared rounded
  buttons, inputs, menus, panels and dialogs; establish the custom-color UI gate.

- Add the native Qt/QML shell, session theme controls, About/Quit actions, CMake
  presets, shell tests, and GCC/Clang CI. The basic canvas and safe PNG lifecycle are now implemented.
- Establish the product scope, interface design, native architecture, and roadmap.
- Define release/update behavior and privacy requirements.
- Add shared contributor/agent instructions and repository validation tooling.
