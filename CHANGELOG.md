# Changelog

## Unreleased — toward 2.1.0

- Keep Layers visible at minimum window width and fit artwork clear of floating tool controls.

## 2.0.0 — 2026-10-21

- Add an Apple silicon macOS app bundle, native glass and platform keyboard shortcuts.
- Build, test and audit the macOS DMG alongside Linux release packages.
- Refresh website and README images with the layered v2 UI at 40% glass opacity.

- Pan the canvas with right-button hold-and-drag and a closed-hand cursor,
  replacing canvas right-button drawing/sampling.

- Use live blur with full desktop redraws in the manual glass setup recipe and keep setup accessible
  for repairing flicker on an already-enabled desktop.

- Restore the prior Image icons, add a footer zoom slider, and simplify blur setup
  to one user-run terminal block with verified extension installation/configuration.

- Restore the monochrome Fill icon; its drop becomes solid when Fill is selected.

- Center the Text tool and refresh original colored ribbon icons, with fixed
  fill and brush artwork colors independent of the selected paint color.

- Add an Image-ribbon Invert colors command for the active layer or selection,
  preserving transparency and supporting undo/redo.

- Refine layer controls with a larger separated ribbon button, circular add
  button, right-side eyes, context-menu icons and glass appearance.
- Add a thumbnail layer rail with hover visibility icons, drag reordering and a
  fixed background; first opening adds a transparent drawing layer.
- Add undoable raster layers with thumbnails, names, reordering, duplication,
  merge down, visibility and opacity controls.
- Save/reopen bounded OpenRaster projects; PNG and other exports flatten visible
  layers without changing the project's saved state.
- Keep tools and selected-content edits on the active layer; transform the whole
  stack together and optionally sample or copy visible layers.

## 1.3.0 — 2026-10-20

- Select all exact matching pixels with the new By color magic wand.
- Apply font family, size and styles to selected text or subsequent typing within a box.
- Add a once-only welcome and a separate desktop glass setup panel, with opaque fallback.
- Simplify Updates, check automatically at startup, and hide Enable Glass when ready.
- Blend the color editor button footer into its dialog surface.

- Swap color picker and eraser positions, keeping the eraser dropdown attached.
- Strengthen light floating-menu tint for readability over strong background colors.

- Distinguish paper-white and transparent eraser modes with matching button/menu icons.
- Adopt the Open palette application icon with live light/dark/system variants.
- Add an open multi-point Spline with preview, Backspace, apply/cancel and undo/redo.

- Add an eraser crosshair and compact Paper white/Transparent dropdown; erasing
  no longer uses the selected background/fill color.

- Default glass opacity to 40%, preserving saved preferences.
- Use a consistent searchable font picker with family previews.
- Use a dashed text-box outline and immediately blinking black caret in empty boxes.
- Default release checks on at startup and daily, retaining bounded cooldown and backoff.
  Flatpak retains browser-only checking.
- Round only Linux bottom client corners, retaining square maximized edges.
- Show selected tools with neutral frosted surfaces and light outlines in glass mode.
- Clarify pencil/eyedropper icons and show tool-shaped canvas cursors.
- Indicate solid shape fill and disabled outline directly on their toolbar icons.
- Use neutral frosted checkmarks and bright color-selection rings; keep flat idle
  controls transparent and retain the 40% glass default without a tint control.
- Credit designer Afshin Kashani, PhD, P.Eng in Updates.

## 1.2.0 — 2026-10-18

- Persist theme selection, improve dark Updates controls, show a blinking text caret,
  and simplify Fill to automatic smooth edges with zero tolerance.
- Refresh the frosted preview with the Brush menu and a multicolor demo backdrop.

- Add translucent window backgrounds and layered frosted app surfaces, with lighter
  menus and transparent shape/footer control wells. Desktop blur requires compositor support.
- Use a 1449 × 1008 initial client window and refresh the README appearance preview.

## 1.1.0 — 2026-10-16

- Add adjustable glass-style surfaces in Settings, independent of artwork opacity.
- Dismiss selections and commit shapes on outside clicks without starting another gesture.
- Make Ctrl+A activate Selection and select the entire canvas.
- Clean same-color antialias fringes during fill, with an exact-match option.
- Add single-axis canvas edge resize handles and remove the decorative border.
- Add manual and opt-in daily GitHub stable-release notifications. Installation
  remains manual; Flatpak retains its no-network sandbox and browser links.

User-visible changes are recorded here. See [release validation](docs/plans/0031-stable-release.md)
for artifact hashes, tested desktops and known limits.

## 1.0.0 — 2026-10-16

- Preserve portal-selected filenames on save/export; reject missing extensions
  instead of reporting success for a hidden, unauthorized sibling file.
- Freeze the implemented single-layer editing scope for v1; layers, recovery and
  automatic updates remain future work.
- Publish stable DEB, Flatpak and portable downloads only from explicit, fully
  checked release runs, with version-only titles and matching source/checksums.
- Add an Ubuntu 24.04 amd64 DEB recipe with application-menu integration, private
  Qt runtime, generated system dependencies and isolated installation checks.

## 0.1.0-alpha.1 — 2026-10-13

- Add a restricted Flatpak test-bundle recipe with pinned runtime/SDK commits,
  source identity, permission auditing and CI review artifacts.
- Add an audited AppImage build recipe and CI review artifacts with dependency
  licenses, corresponding sources, checksums and a software bill of materials.

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
