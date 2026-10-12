# Feature checklist and reference audit

Audit date: 2026-10-12. **The first single-layer drawing canvas is implemented.**
The M1 source-editor checkpoint is complete; see [validation and limits](plans/0009-m1-closure.md).
Safe PNG files are implemented; remaining capabilities stay planned.
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
| F01 | Familiar toolbar, light/dark, responsive canvas | R1; visual layout in the menu/control specification; interaction audit pending | M1–M4 | In progress: original icon toolbar, shared rounded surfaces, canvas and themes implemented; exact parity audit pending. See [M1 evidence](plans/0001-foundation.md#evidence-and-handoff). |
| F02 | Pencil, eraser, basic brush, sizes | R1 | M1 | Implemented: round pencil, opaque/transparent eraser, 1–128 px size; mouse gesture/edge regression coverage in [plan 0006](plans/0006-canvas-gesture-lifecycle.md); advanced brushes pending. |
| F03 | Additional brush styles, stroke opacity | R1, R2; nine-style inventory in the menu/control specification | M2 | Planned |
| F04 | Fill and adjustable tolerance | R1; tolerance announced as preview in R3 | M2 | Planned |
| F05 | Primary/secondary colors, picker, HEX/RGB/HSV | R1 | M1–M2 | Implemented: primary/secondary rectangular spectrum, brightness, HEX/RGB/HSV, 24 saved custom colors, swap and right-button secondary drawing. Behavioral and visual evidence in the [reference layout plan](plans/0005-reference-color-layout.md); canvas eyedropper remains planned. |
| F06 | Lines, rectangles, ellipses, shape palette | R1; partial visible shape inventory specified, full palette still unverified | M2 | Planned |
| F07 | Text, font/style/size/alignment and background | R1; detailed control behavior needs observation | M2 | Planned |
| F08 | Rectangle/free-form selection; copy/cut/paste | R1 | M2 | Planned |
| F09 | Crop, resize/skew, flip, right-angle rotate | R1 supports crop/resize/flip/rotate; skew needs observation | M2 | Planned |
| F10 | Freeform rotation of active content | R4: Paint 11.2601.391.0 preview | M4 | Planned |
| F11 | Zoom, pan, status, rulers, grid | R1; exact bounds and ruler behavior need observation | M1–M4 | In progress: zoom/pan and basic status implemented; rulers, grid, thumbnail and expanded zoom controls planned. |
| F12 | Add/delete/rename/reorder/duplicate/merge layers | R1 covers layers/reorder/merge; other operations are product choices pending audit | M3 | Planned |
| F13 | Transparent canvas, layer visibility/opacity | R1 plus OpenPaint contract; detailed parity needs observation | M3 | Planned |
| F14 | Editable layered project save/reopen | R2; OpenPaint uses .ora initially | M3 | Planned |
| F15 | PNG, JPEG, BMP, WebP interchange; static GIF import | R1 documents some formats; exact supported subset is our choice | M1–M4 | In progress: static PNG open/save, EXIF orientation normalization and failure checks implemented; see [PNG validation](plans/0007-png-validation.md); other formats pending. |
| F16 | Undo/redo, crash recovery, atomic saves | Product choice; reference workflow requires hands-on audit | M1–M3 | In progress: stroke history and atomic PNG saves tested; crash recovery pending. |
| F17 | Stylus pressure, touch and fractional scaling | R1 confirms stylus; precise pressure behavior needs observation | M4 | Planned |
| F18 | One-click background removal | R1 | M6 | Planned |
| F19 | Object selection | R5, hardware-dependent in reference | M6 | Planned |
| F20 | Generative erase/fill | R6; separate model/license feasibility gate | M6 | Planned |
| F21 | Text-to-image and sketch-guided generation | R1, R6; independently implemented providers | M6 | Planned |
| F22 | Sticker generation | R5 | M6 | Planned |
| F23 | Coloring-page generation | R3 preview, hardware/account restrictions in reference | M6 | Planned |
| F24 | Collapsible command toolbar | Persistent/auto-hide modes specified visually; timing and focus behavior need validation | M4 | Planned |
| F25 | Flatpak/AppImage downloads and Updates screen | Linux-specific product choice | M2–M5 | Planned |
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
