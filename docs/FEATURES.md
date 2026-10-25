# Feature checklist and reference audit

Current release: [v2.0.0](https://github.com/AfshinKI/openpaint/releases/tag/v2.0.0),
published 2026-10-22 UTC from merged PR #56. [Plan 0062](plans/0062-macos-v2-release.md)
records release/platform evidence and remaining limits. Raster layers and
OpenRaster shipped in v2; this does not complete the recovery, tiling or
cross-application acceptance gates of M3. See [release progression](ROADMAP.md#release-progression)
for changes introduced between the alpha and v2.

Reference audit date: 2026-10-12. The reference audit is historical; current
OpenPaint implementation status is recorded below. M1/M2 closure evidence remains
in [M1 closure](plans/0009-m1-closure.md) and [the alpha plan](plans/0027-alpha-release.md).
The reference is modern Windows 11 Paint, including separately identified preview
features. This is a research baseline, not an exhaustive test of today's installed
Paint. No Windows binary was available for a hands-on comparison. The
[menu/control specification](MENUS.md) records the visual target separately from
verified interaction behavior.

Reference codes link to [REFERENCES.md](REFERENCES.md). M-numbers link conceptually
to delivery gates in [ROADMAP.md](ROADMAP.md). “Product choice” identifies an
OpenPaint requirement, not a claim about Microsoft behavior.

## Scope and delivery

| ID | Capability | Evidence / distinction | Target | Status |
| --- | --- | --- | --- | --- |
| F01 | Familiar toolbar, light/dark, responsive canvas | R1; visual layout in the menu/control specification; interaction audit pending | M1–M4 | In progress: original icon toolbar/menus, borderless idle ribbon controls, session-aware selection/transform icons, shared rounded surfaces, narrow left-side size/opacity controls, canvas and themes implemented; exact parity audit pending. See [M1 evidence](plans/0001-foundation.md#evidence-and-handoff). |
| F02 | Pencil, eraser, basic brush, sizes | R1 | M1 | Implemented: round pencil, opaque/transparent eraser, 1–128 px size; mouse gesture/edge regression coverage in [plan 0006](plans/0006-canvas-gesture-lifecycle.md); additional styles in F03. |
| F03 | Additional brush styles, stroke opacity | R1, R2; nine-style inventory in the menu/control specification | M2 | Implemented: nine original deterministic raster styles, generated previews and per-stroke opacity; [plan 0012](plans/0012-brush-styles.md). Proprietary simulation parity and pressure remain unverified. |
| F04 | Fill with smooth edges | R1 | M2 | Implemented: cancellable contiguous and global matching-color modes, selection clipping, alpha/opacity and inferred edge recoloring; [fill modes](plans/0078-fill-modes.md), [edge limits](plans/0075-fill-edge-background.md). |
| F05 | Primary/secondary colors, picker, HEX/RGB/HSV | R1 | M1–M2 | Implemented: primary/secondary rectangular spectrum, brightness, HEX/RGB/HSV, 24 session custom colors. Alpha polish adds a fixed foreground/background pair with selectable editing target, automatic 10/24 session palette insertion, transparent-background choice with checkerboard swatches, and removes Swap; see [plan 0028](plans/0028-alpha-ui-polish.md). Behavioral and visual evidence in the [reference layout plan](plans/0005-reference-color-layout.md); pointer-attached hover swatch and canvas eyedropper implemented with alpha-preserving sampling; see [plan 0010](plans/0010-canvas-eyedropper.md). |
| F06 | Lines, rectangles, ellipses, shape palette | R1; partial visible shape inventory specified, full palette still unverified | M2 | Implemented: 20-shape 7×3 ribbon gallery with separate Outline and Fill menus, retained previews, outline/fill, numeric geometry, curve bends, polygon and spline entry and Shift constraints; [basic previews](plans/0013-basic-shapes.md) and [palette](plans/0014-shape-palette.md). Exact full-reference inventory remains unverified. |
| F07 | Text, font/style/size/alignment and background | R1; detailed control behavior needs observation | M2 | Implemented: inline plain text, system font/style/alignment, optional background, floating formatting toolbar and outside-click/Ctrl+Enter raster commit; [plan 0015](plans/0015-inline-text.md). |
| F08 | Rectangle/free-form/color selection; copy/cut/paste | R1 | M2 | Implemented: rectangle/free-form selection, inversion, color-key transparency, numeric bounds, selection clearing on tool changes and undoable background-color deletion; [plan 0016](plans/0016-rectangular-selection.md). Move and PNG clipboard: [plan 0017](plans/0017-selection-clipboard.md); free-form and transparent selection: [plan 0018](plans/0018-freeform-selection.md). |
| F09 | Crop, resize/skew, flip, right-angle rotate, invert colors | R1 supports crop/resize/flip/rotate; skew needs observation | M2 | Implemented: crop, flips and right-angle rotations for image/selection, with dimension-aware undo; [plan 0019](plans/0019-crop-rotate-flip.md). Canvas crop/expand and active selection, shape and text corner handles: [alpha polish](plans/0028-alpha-ui-polish.md). Active-layer color inversion shipped in v2; see [plan 0059](plans/0059-invert-colors.md). Pixel/percent resize, resampling and bounded skew: [plan 0020](plans/0020-resize-skew.md). |
| F10 | Freeform rotation of active content | R4: Paint 11.2601.391.0 preview | M4 | Planned |
| F11 | Zoom, pan, status, rulers, grid | R1; exact parity not claimed | M1–M4 | Implemented: zoom/pan, region zoom, presets, synchronized 1–3200% slider, Fit, status and persistent pixel rulers; [rulers](plans/0069-canvas-rulers.md), [region zoom](plans/0079-region-zoom-tool.md). Grid and thumbnail remain planned. |
| F12 | Add/delete/rename/reorder/duplicate/merge layers | R1 covers layers/reorder/merge; other operations are product choices pending audit | M3 | Shipped in v2: bounded raster layers, undoable stack operations and OpenRaster persistence; [plan 0058](plans/0058-layers.md), [release evidence and platform limits](plans/0062-macos-v2-release.md). |
| F13 | Transparent canvas, layer visibility/opacity | R1 plus OpenPaint contract; detailed parity needs observation | M3 | Shipped in v2: bounded raster layers, undoable stack operations and OpenRaster persistence; [plan 0058](plans/0058-layers.md), [release evidence and platform limits](plans/0062-macos-v2-release.md). |
| F14 | Editable layered project save/reopen | R2; OpenPaint uses .ora initially | M3 | Shipped in v2: bounded raster layers, undoable stack operations and OpenRaster persistence; [plan 0058](plans/0058-layers.md), [release evidence and platform limits](plans/0062-macos-v2-release.md). |
| F15 | PNG, JPEG, BMP, WebP interchange; static GIF import | R1 documents some formats; exact supported subset is our choice | M1–M4 | In progress: static PNG open/save, EXIF orientation normalization and failure checks implemented; see [PNG validation](plans/0007-png-validation.md); JPEG/BMP/WebP import: [plan 0021](plans/0021-image-import.md). Explicit export copies: [plan 0022](plans/0022-image-export.md). Static GIF import remains M4. |
| F16 | Undo/redo, crash recovery, atomic saves | Product choice; reference workflow requires hands-on audit | M1–M3 | In progress: stroke history and atomic PNG saves tested; crash recovery pending. |
| F17 | Stylus pressure, touch and fractional scaling | R1 confirms stylus; precise pressure behavior needs observation | M4 | Planned |
| F18 | One-click background removal | R1 | M6 | Planned |
| F19 | Object selection | R5, hardware-dependent in reference | M6 | Planned |
| F20 | Generative erase/fill | R6; separate model/license feasibility gate | M6 | Planned |
| F21 | Text-to-image and sketch-guided generation | R1, R6; independently implemented providers | M6 | Planned |
| F22 | Sticker generation | R5 | M6 | Planned |
| F23 | Coloring-page generation | R3 preview, hardware/account restrictions in reference | M6 | Planned |
| F24 | Collapsible command toolbar | Persistent/auto-hide modes specified visually; timing and focus behavior need validation | M4 | Planned |
| F25 | Linux/macOS installers, portable downloads and Updates screen | Platform product choice | M2–M5 | Updates screen, bounded manual and default-on startup/daily release checks ([plan 0037](plans/0037-release-notifications.md)) and install metadata implemented in [plan 0024](plans/0024-install-updates.md); AppImage recipe in [plan 0025](plans/0025-appimage-package.md); Flatpak recipe in [plan 0026](plans/0026-flatpak-package.md); alpha downloads and validation in [plan 0027](plans/0027-alpha-release.md); Ubuntu DEB installer in [plan 0030](plans/0030-debian-installer.md); v1.0.0 publication and Ubuntu 24.04 GNOME/X11 evidence in [plan 0031](plans/0031-stable-release.md); Apple silicon DMG and native glass shipped in [v2](plans/0062-macos-v2-release.md); signed automatic installation remains M5. |
| F26 | Privacy controls, metadata removal, accessible keyboard UI | Product choice | M1–M5 | In progress: offline PNG metadata removal, keyboard lifecycle, accessible names/roles and GNOME portal workflow tested; see [desktop workflow](plans/0008-keyboard-desktop-workflow.md); Orca usability certification remains M4. |
| F27 | Printing through the Linux print dialog | Product choice; reference details need observation | M4 | Planned |

No percentage-complete claim until each row has a finite acceptance checklist.
Move through Planned → In progress → Implemented → Verified, linking the feature
plan, implementation PR, tests, and manual evidence when status changes.
“Verified” is for a named release/platform and does not imply perpetual parity.

## Explicit gaps

1. An exact Microsoft Paint build/channel, full shape inventory, hidden submenus,
   shortcut conflicts, print behavior and edge cases still need a hands-on audit.
   The menu/control specification establishes visual inventory, not tested behavior.
   Record the version and observation date; use synthetic artwork. Keep copyrighted
   reference material outside distributed assets.
2. `.paint` import/export is deferred pending format documentation, safe parser design,
   test fixtures with permission to redistribute, and compatibility tests. `.ora`
   support is not `.paint` compatibility.
3. Microsoft accounts, Copilot service access, Microsoft models, proprietary icons,
   and identical AI outputs are not part of this implementation. Equivalent creative
   workflows require separate openly usable models or explicit provider integrations.
4. Version 1.0 targets the non-generative desktop editor. Background removal and
   generative capabilities follow in M6; do not advertise full modern Paint parity
   at 1.0. Persistent vector text/shapes, animation editing, CMYK/HDR, PSD editing,
   and third-party binary plugins are outside that release.

## Reference audit procedure

For each feature: record reference version/channel and availability, reproduce a
small task, list controls/defaults/shortcuts, test cancel/undo/save, and distinguish
observed behavior from our improvements. Add acceptance cases to a feature plan.
Re-audit before a parity-focused release; do not silently expand a milestone when
Microsoft ships another preview. Record the gap and prioritize it explicitly.

Implemented subset evidence: [canvas](plans/0002-drawing-canvas.md) and
[PNG lifecycle](plans/0003-png-file-workflow.md).

Portal save filename safety is tracked in [plan 0032](plans/0032-portal-save-filenames.md).
Missing format extensions are rejected without reporting a successful save.

Bounded GitHub release notifications are tracked in [plan 0037](plans/0037-release-notifications.md). Automatic installation remains planned.

Adjustable glass appearance is tracked in [plan 0036](plans/0036-glass-appearance.md).

Same-color fill fringe cleanup is tracked in [plan 0035](plans/0035-fill-fringe.md).

Canvas edge handles and border removal are tracked in [plan 0034](plans/0034-canvas-edges.md).
Selection dismissal and canvas Select All are tracked in [plan 0033](plans/0033-selection-dismissal.md).

## Frosted appearance

Glass appearance uses a continuous translucent window base with separate tinted
bars. Floating controls blur lower app layers on shader-capable renderers; menus
and dialogs also include the fixed bars and floating controls beneath them.
Popup backdrops are flattened to prevent sharp content leaking through the blur.
Software rendering uses solid popups. The paper/checkerboard, text, and icons keep
their own opacity; appearance changes do not affect artwork or undo history.

Floating menus retain at least 72% neutral tint for readable labels over strong
background colors, while the interface opacity setting remains 40% by default. The shape gallery and footer buttons
have transparent idle backgrounds when glass is enabled, with hover and focus
feedback preserved. Disabling glass or choosing 100% restores opaque surfaces.
The startup client size is 1449 × 1008; the minimum remains 1024 × 640.

Desktop blur is supplied by the compositor, not by screen capture. GNOME requires
an extension such as Blur My Shell; it is not installed or configured by OpenPaint.
Without desktop blur, transparency alone remains visible. This is frosted glass,
not optical refraction. See [implementation and validation](plans/0039-window-transparency.md)
and [rendering decision](decisions/0005-local-panel-blur.md).

Theme selection (System/Light/Dark), glass enabled state and surface opacity persist
across restarts. Inline text displays a blinking caret at the scaled font height.
The Updates dialog uses explicit theme text colors and brighter dark-mode buttons.

Current default: glass starts at 40% opacity; saved user choices are retained.
The font picker replaces the native dialog with searchable family previews;
see [plan 0043](plans/0043-font-picker.md).
Text editing uses a dashed black/white box outline and a blinking
black caret immediately in an empty box at the scaled text height; neither overlay is saved in the artwork.
The startup release-check policy is recorded in [plan 0045](plans/0045-startup-updates.md).
Linux windows use 12px rounded bottom client corners and matching input masks;
top corners stay square, and other platforms receive no custom corner mask.
maximized/fullscreen edges are square. Native decorations remain desktop-owned.
Glass selection uses stronger neutral frost and light outlines instead
of green fills. Software rendering uses tint/outline; desktop blur stays compositor-owned.
Developer attribution now belongs in About; its removal from Updates is recorded
in [plan 0067](plans/0067-about-project-details.md).


Current refinements retain the 40–100% glass range with a 40% default and no
separate tint control. Flat toolbar buttons stay clear when idle; selected controls
use stronger frost and light outlines. Checkmarks use neutral frosted colors, and
selected color swatches use a bright outer ring without recoloring the swatch.
The font picker retains its searchable per-family previews.

Pencil, brush, eraser and eyedropper cursors identify the active tool over the canvas,
with white contrast edges and tip-aligned hotspots. Right-button/Space/middle-button panning
uses a hand and restores the tool cursor afterward. Shape controls show a solid
filled drop when solid fill is active, and a small × on the outline icon when
outline is off. See [refinement acceptance](plans/0050-appearance-refinements.md).

Eraser modes are Paper white (opaque white, independent of the background/fill
swatch) and Transparent. A small dropdown beside the eraser replaces the separate
ribbon options group. The eraser cursor's + marks the actual erase point. Left-button
erasing honors the chosen mode; right-drag pans in v2; strokes retain undo/redo and cancellation.
See [eraser acceptance](plans/0051-eraser-target.md).

The eraser button/cursor shows a small filled patch in Paper white mode; Transparent
uses the hollow eraser. Both menu choices carry the corresponding eraser icon.

The selected original Open palette application icon has light and dark variants.
The running window and Updates dialog follow Light/Dark/System immediately. The
installed launcher icon is the light variant; shell-controlled launcher/dock icons
may not follow an application's theme preference.

Spline is a separate open multi-point shape, alongside the two-bend Curve. Click or
drag the first segment, then click to add points. Backspace removes the last point;
Enter applies and Escape cancels. It interpolates the clicked points smoothly,
uses outline color/width, and ignores fill. Endpoints and overall bounds remain
editable; undo/redo and PNG saving use the existing document pipeline.
See [icon and spline acceptance](plans/0052-icon-and-spline.md).

The [blur welcome and Updates](plans/0054-blur-onboarding-design.md):
real app previews, local capability checks, optional setup command copying,
a separate one-time welcome with a contribution link, and silent opaque fallback.
Disabled glass controls link to the on-demand Enable Glass panel. Initial setup scope is Ubuntu
24.04 / GNOME 46 / X11; broader desktop support is not claimed. Setup now offers
one copyable, user-run block to install/configure Blur My Shell, followed by
Check again ([plan 0060](plans/0060-zoom-and-blur-setup.md)).

Updates refinement: automatic startup/open checks with cooldown/backoff, concise status, and an angled app-corner card. Legacy automatic-check preference is superseded; installation remains manual. Validation is recorded in plan 0054.

Select by color adds an exact RGBA, image-wide selection mode with a magic-wand icon. Implementation and validation are tracked in [plan 0055](plans/0055-color-selection.md).

Text font controls support selected ranges and insertion formatting in the same box; mixed font runs survive raster commit and saving. Evidence: [plan 0056](plans/0056-text-range-formatting.md).

## macOS platform

Apple silicon/macOS 14.4+ build and DMG packaging are implemented, with native
AppKit glass and Command shortcuts. Validation is tracked in [plan 0062](plans/0062-macos-v2-release.md);
physical Mac visual review remains pending. Intel packages and Apple notarization
are not part of v2.0.0.

Development opacity controls now cover pencil, fill and shapes as well as brushes
([plan 0073](plans/0073-drawing-tool-opacity.md)).

Development shape previews use a floating Options / Apply / Cancel toolbar
([plan 0072](plans/0072-floating-shape-toolbar.md)), retaining preview undo/cancel semantics.

Development Updates can remember a skipped release across launches while still
notifying for later versions ([plan 0071](plans/0071-skip-update-version.md)).

Optional canvas-edge/center smart guides and preview move grips are implemented
for v2.1 development, with local combined validation ([plan 0070](plans/0070-smart-guides.md)).
Automatic painted-object alignment and equal-spacing guides remain outside scope.

Development v2.1 adds top/left pixel rulers with adaptive ticks and pointer
markers, toggled together from View → Rulers (Ctrl+R / Command+R). They follow
canvas pan/zoom and do not modify artwork. Alignment snapping remains separate.
[Acceptance and pending validation](plans/0069-canvas-rulers.md).

Development Save As offers PNG/OpenRaster document saving and explicit image-copy
choices (JPEG, BMP, monochrome 1-bit BMP, WebP, and layered PNG). Copies retain
the editable document's saved state. [Acceptance](plans/0068-save-as-image-formats.md).

Development fix for [issue #57](plans/0065-glass-schema-compatibility.md): copied
glass setup detects optional extension redraw keys before use. v2.1 planning is
tracked in [the request plan](plans/0064-v2.1-roadmap.md); it is not released.
