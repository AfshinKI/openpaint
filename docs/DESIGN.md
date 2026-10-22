# Interface design

Pending alpha corrections are specified in the [eight-step UI polish plan](plans/0028-alpha-ui-polish.md).
It supersedes the earlier shared custom-palette, Swap-button and left-rail targets;
current shipped behavior remains unchanged until implementation is verified.

Status: design specification, not an implemented or pixel-verified replica.
Goal: someone familiar with modern Paint should find the same everyday workflows
quickly, with Linux window conventions and original OpenPaint assets.

The detailed [menu and control specification](MENUS.md) defines group order,
menu entries, brush inventory, settings and feature discovery. Read it before
adding or rearranging visible controls; planned entries are not shipped features.

## Window anatomy

```text
┌────────────────────────────────────────────────────────────────────────┐
│ OpenPaint     Untitled •                              window controls   │
├────────────────────────────────────────────────────────────────────────┤
│ File Edit View │ Save Undo Redo                                      │
├────────────────────────────────────────────────────────────────────────┤
│ Selection │ Image │ Tools │ Brushes │ Shapes │ Colors │ AI* │ Layers   │
├───────────┬───────────────────────────────────────────────┬────────────┤
│ Tool      │                                               │ Layers     │
│ options   │          canvas on a neutral workspace         │            │
│           │                                               │ Drawing    │
│ Size      │        transparency checkerboard in image      │ Background │
│ Opacity   │        bounds only; resize handles at edges    │            │
│           │                                               │ +  …       │
├───────────┴───────────────────────────────────────────────┴────────────┤
│ Cursor x,y    Selection w×h    Image w×h       Fit    −  100%  +         │
└────────────────────────────────────────────────────────────────────────┘
```

The optional intelligent-tools group remains deferred until its feature gate.

Desktop decorations follow the window manager. The product must not draw fake
Windows title-bar buttons. The first launch opens a blank white 1152×648 canvas
at a fitted zoom, pencil selected, black primary and white secondary colors.
White is a real initial background, not a substitution for transparency.

One horizontal command area groups familiar tools; avoid scattering primary
editing controls across unrelated menus. Tool options show only relevant fields.
The Layers panel can be toggled and is hidden by default for a new single-layer
image. Generative features live in a later optional panel, not a compulsory start
screen. Files, settings, and update controls are available without an account.

## Visual system

Use original vector icons on a 24-unit grid, with a consistent 1.6-unit stroke,
rounded line endings/joins and 20px rendered toolbar glyphs, with accessible text labels and
tooltips. System UI font; no bundled proprietary fonts. Keep controls compact
without making them difficult to hit. Spacing uses 4, 8, 12, 16, 24, and 32 logical
pixels. Standard targets are at least 32×32; dense color palettes may use 24×24
with keyboard focus and tooltips. Touch mode targets 44×44 where possible.
Panels use 8-unit corner radii; avoid decorative shadows inside the editor.

| Token | Light | Dark |
| --- | --- | --- |
| Window | `#F5F6F8` | `#181B20` |
| Panel | `#FFFFFF` | `#22262D` |
| Workspace | `#E2E5EA` | `#111418` |
| Text | `#18212B` | `#F1F4F8` |
| Secondary text | `#4C5867` | `#ADB8C5` |
| Accent / selected | `#006B58` | `#68DCC2` |
| Text on accent | `#FFFFFF` | `#10211D` |
| Divider | `#C9CED6` | `#454E5A` |

Follow the system theme by default; Settings offers Light, Dark, and System.
These are starting design tokens; verify contrast in the actual controls rather
than treating token selection as accessibility proof. Focus rings must be distinct
from selection. Color selection uses both a visible outline and an accessible name.
Checkerboard contrast changes with theme but exported colors never do.

At 1280×800, show the full toolbar and an optional 200-unit Layers rail. Support
1024×640 logical units without inaccessible actions. Use labeled overflow menus
for toolbar groups while keeping the narrow layer rail beside the canvas at smaller widths. Preserve
the canvas, focus, and selection when panels collapse. Never shrink text to fit.
Support fractional scaling and multiple monitors; test 100%, 125%, 150%, and 200%.

## Interaction contracts

| Action | Behavior |
| --- | --- |
| Draw | Capture pointer, show preview immediately, commit one undo step on release |
| Brushes | Nine named styles with generated previews; floating size and 0–100% stroke opacity; fixed-distance path sampling, one undo step |
| Fill | Left/right press fills with primary/secondary; exact color matching with automatic smooth edges; busy indicator and Cancel/Escape leave original pixels intact |
| Color picker | Select once per press: left sets primary, right sets secondary; exact containing artwork pixel including alpha; stays selected, never paints or changes history |
| Interrupted stroke | On missing initiating button, grab/focus loss, tool or viewport change, finish at the last valid sample; never bridge to a later pointer |
| Escape | Cancel active preview/transform; otherwise dismiss the topmost transient UI |
| Pan / zoom | Space+drag pans outside text entry; Ctrl+wheel zooms around pointer; plain wheel scrolls |
| Selection | Rectangle/free-form marquee, inversion, numeric bounds, select-all/deselect and undoable alpha deletion; all drawing tools clip. Move/paste previews use Enter/Escape, arrows (Shift: 10 px) or numeric position; copy/cut/paste uses PNG. Resize/skew supports selected content or the whole image. |
| Transform | Shift constrains proportions or rotation increments; numeric inputs provide keyboard precision |
| Text | Inline editor and floating formatting toolbar below ribbon; outside-box canvas click or Ctrl+Enter commits, Escape cancels; top color is foreground and bottom is background |
| Shape | 20-item icon palette; Shift constrains geometry, polygon clicks add vertices and Backspace removes the last; drag/release retains preview; numeric start/end and outline/fill options; Enter/Apply commits and Escape/Cancel restores original; new drawing/tool switch commits current preview; Undo cancels the preview |
| Resize image | Pixel/percent inputs, aspect lock, interpolation; one undoable operation |
| Resize canvas | Anchor and dimensions; warn when reducing clips content; undo restores it |
| Save / close | Dirty window offers Save, Discard, Cancel; a failed save leaves the window open |
| Long operation | Progress and cancellation; never leave partially edited pixels after cancellation |

The initiating mouse button owns the gesture. Extra button presses/releases cannot
change its mode/color or end it early. Pan consumes the release position as well
as move events. Stroke starts use continuous half-open image bounds; fractional
positions inside the final pixel are valid and positions just outside are ignored.
Qt logical pointer coordinates are converted once through the canvas transform.

Before any action that would invalidate text/shape/selection previews, resolve
them explicitly. Clicking another canvas location commits shape/text only when
that behavior is specified in its feature plan and tested. Enter in a multiline
text box inserts a newline; it must not trigger a global action.

Open accepts PNG, JPEG, BMP and static WebP. Non-PNG imports show a notice and
Save asks for a PNG destination, preserving the original. The File menu separates
Save, Save As, and Export copy. Export offers lossless PNG/WebP or JPEG/BMP/WebP
interchange with explicit quality and opaque-background choices; it never clears
the working drawing’s unsaved state. Apply/cancel previews before exporting. Show the current file format
in the relevant dialogs. Lossy export and flattening are explicit. Opening a file
with unsupported metadata must not quietly overwrite that original on Save.

### Initial shortcut map

| Shortcut | Action |
| --- | --- |
| Ctrl+N / Ctrl+O | New / Open |
| Ctrl+S / Ctrl+Shift+S | Save / Save As |
| Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z | Undo / Redo / Redo |
| Ctrl+A / Ctrl+D | Select all / Deselect |
| Ctrl+C / Ctrl+X / Ctrl+V | Copy / Cut / Paste |
| Delete | Clear selected pixels on active layer |
| Ctrl+plus / Ctrl+minus / Ctrl+0 | Zoom in / out / fit |
| F11 | Toggle full screen |
| F1 | Help |

Text fields retain normal editing shortcuts. Keyboard activation, accessible
actions, and pointer activation must call the same application command. Define
tool-letter shortcuts later after checking typing and desktop conflicts.

## Settings and Updates

Settings sections: Appearance, Canvas, Files & Recovery, Updates, Privacy, About.
Keep advanced technical information in an expandable diagnostic view.

Updates shows the installed version, concise status, one release action and
attribution beside an angled actual-app corner card. Network-capable builds check
at startup, on opening the panel, and daily while running. Persisted cooldown and
server backoff apply; no check checkbox/button or background daemon exists.

| State | User-facing behavior |
| --- | --- |
| Not checked | “Waiting to check” |
| Checking | “Checking for updates…” |
| Current | “You’re up to date” only after a validated response |
| Available | “An update is ready”, version and Get update browser action |
| Offline/error | “Couldn’t check for updates”; View releases remains available |
| Flatpak | “Latest releases” with a manual browser action |

Installation is manual. Automatic downloading and restart-to-update remain planned.

No forced restarts, silent channel switches, fabricated progress, or update success
messages before verification. Detailed rules are in [RELEASES.md](RELEASES.md).

## Accessibility and design review

Every control needs an accessible name, role, state, visible keyboard focus, and
logical focus order. Announce tool changes and document/save status without reading
every pointer move. Keyboard users can reach dialogs, tool settings, layer actions,
and numeric transform controls; freehand drawing naturally remains pointer-oriented.
Use standard Qt Quick Controls where possible and verify with Orca on Linux.

Target 4.5:1 text contrast and 3:1 essential control boundaries; support high contrast,
reduced motion, translated strings, and right-to-left UI without mirroring artwork.
Color must never be the sole indication of state. Tooltips show names and shortcuts.

For each visible feature, submit synthetic light/dark screenshots at the minimum
and typical window size, keyboard checks, and failure/disabled states. Compare
workflow placement with the reference audit; do not claim pixel parity without
an observed, versioned comparison. The layout above is the initial design contract.

## Required polish before additional tools

The shared UI foundation uses original
20-unit toolbar icons inside 32 px buttons, hover tooltips with shortcut labels, and a common rounded
control family: 6 px buttons/inputs, 8 px toolbar/menu surfaces, 12 px dialogs.
Toolbar swatches use 16 px circles inside 24 px hit areas. Rounded menu rows are inset;
focus must remain visible independently of hover or selection. Native window
frames and file dialogs follow the desktop. The artwork itself remains rectangular.

Use the app-owned color chooser with a rectangular hue/saturation spectrum,
vertical brightness slider and color preview, HEX input and switchable RGB/HSV
fields, basic/custom grids and explicit accept/cancel. Both primary and secondary
colors are editable. The top circle always controls foreground drawing color;
the bottom always controls background/fill and right-button drawing. Selecting
a circle changes only the color-editing target. Applying a new chooser color
fills the first empty slot in each custom palette: 10 on the ribbon, 24 in the
editor. Skip full palettes and existing duplicates. Both palettes are session-only
and start empty on launch. Explicit editor Add/Replace/Remove/Clear affects its
own list immediately for this session; Cancel discards only the pending color
draft. Palette changes never dirty artwork or enter its undo history. See the
[current alpha plan](plans/0028-alpha-ui-polish.md) for the superseding contract.

The [reference layout refinement](plans/0005-reference-color-layout.md) supersedes
the earlier inferred hue-ring design. Toolbar height is 112 logical pixels;
two fixed-role color circles stack beside two rows of basic colors and a
third row of ten independently editable toolbar slots. Empty slots remain visible as hollow circles and open the
editor. The color editor always exposes all 24 custom slots. Plus/empty-slot
activation appends the draft to the next free slot; Manage custom colors offers
replace/remove/clear. Removal compacts the slots. Native window frames remain
under desktop control. User-provided reference screenshots are not redistributed.

M1 keyboard evidence: [desktop workflow](plans/0008-keyboard-desktop-workflow.md).
Dialogs take keyboard focus; Escape cancels unsaved changes without mutating pixels.
Ctrl+Q is an explicit Quit binding, independent of the desktop theme’s defaults.

### Image transform controls

Save/Undo/Redo share the menu row as compact quick actions. The toolbar starts
with Selection and Image groups so commands fit the minimum window. Image actions
name image/selection scope explicitly. Crop uses selected bounds and clears
unselected interior pixels; rotations and flips preserve alpha. Selected-content
transforms retain a movable Apply/Cancel preview, while whole-image commands commit
one undo step and refit when dimensions change. Resize/skew offers pixel/percent
size, aspect lock, smooth/nearest resampling and bounded horizontal/vertical skew.
Its local dialog draft is accepted after closing; Cancel preserves all pixels.

### Alpha floating controls and menu icons

Size/opacity moved out of the ribbon into a left-side floating rail. Tool-specific
visibility, numeric limits and input isolation are defined in [MENUS.md](MENUS.md).
The rail overlays workspace without changing canvas coordinates. Shared menu rows
use original 18px icons, theme-aware disabled colors, shortcut hints, check
indicators and submenu arrows. Brush choices retain generated stroke previews.

Active shapes, text boxes and selections have four square corner resize handles.
Their hit areas remain fixed at screen scale. Resizing preserves the active
preview until committed; cancellation restores the original artwork. See the
[control specification](MENUS.md#workspace-and-status-bar). Canvas corners separately crop or extend the image without resampling,
using an undoable worker transform.

Glass appearance adds translucent gradients and fine highlights to shared UI surfaces. Settings → Glass appearance can disable it or set 40–100% surface opacity (default 40%). It affects backgrounds only; artwork, text and controls retain their own opacity. This supersedes the flat-surface target; the frosted behavior is described below.

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

Development text editing uses a dashed black/white box outline and a blinking
black caret immediately in an empty box at the scaled text height; neither overlay is saved in the artwork.
Development Linux windows use 12px rounded bottom client corners and matching input masks;
top corners stay square, and other platforms receive no custom corner mask.
maximized/fullscreen edges are square. Native decorations remain desktop-owned.
Development glass selection uses stronger neutral frost and light outlines instead
of green fills. Software rendering uses tint/outline; desktop blur stays compositor-owned.


Development refinements retain the 40–100% glass range with a 40% default and no
separate tint control. Flat toolbar buttons stay clear when idle; selected controls
use stronger frost and light outlines. Checkmarks use neutral frosted colors, and
selected color swatches use a bright outer ring without recoloring the swatch.
The font picker retains its searchable per-family previews.

Pencil, brush, eraser and eyedropper cursors identify the active tool over the canvas,
with white contrast edges and tip-aligned hotspots. Space/middle-button panning
uses a hand and restores the tool cursor afterward. Shape controls show a solid
filled drop when solid fill is active, and a small × on the outline icon when
outline is off. See [refinement acceptance](plans/0050-appearance-refinements.md).

Eraser modes are Paper white (opaque white, independent of the background/fill
swatch) and Transparent. A small dropdown beside the eraser replaces the separate
ribbon options group. The eraser cursor's + marks the actual erase point. Both
mouse buttons honor the chosen mode; strokes retain undo/redo and cancellation.
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

## Development blur welcome panel

[Blur onboarding and Updates](plans/0054-blur-onboarding-design.md) implements
real app corner previews, optional local setup guidance,
clipboard copying and a Check again action. A separate Welcome to OpenPaint
appears once, includes a side picture and invites contributions. Updates places text on the left,
a real app picture on the right, and Enable Glass in its footer only while glass is unavailable. The original [interactive design](designs/blur-onboarding.html)
remains a design artifact rather than executable application behavior.

Current capability evidence is limited to Ubuntu 24.04 / GNOME 46 / X11. Without
verified configuration, the effective appearance is opaque while saved glass
preferences are retained. The glass checkbox is unchecked and disabled until requirements are met.
Enable Glass opens setup on demand, even on an unverified desktop; commands are
shown only for a verified recipe. Unavailable setup stays opaque without a package
warning. This supersedes
the earlier transparency-only fallback described above. No setup command runs
inside OpenPaint, and the welcome panel waits until drawing and other popups finish.

App preview images have 18px rounded corners and a restrained shadow, rendered
with QPainter so software rendering needs no optional effects. Setup uses short
copy and a selectable 11px monospace command below a Terminal label.

Updates uses a compact status/action column and a softly elevated, perspective-angled actual-app corner preview. It checks on startup and opening without a check button or checkbox; failures never imply the installed version is current.

The Select menu includes By color with an original magic-wand icon. Each click selects all exact-color pixels across the image; selection feedback and subsequent editing reuse the existing mask workflow. See [plan 0055](plans/0055-color-selection.md).

## Development layer panel

The Layers ribbon control has a separate divider and a 48×56 target with a 30-unit
icon. It and View → Layers toggle a 200-unit vertical thumbnail
rail that reserves space on the right of the canvas at every supported width.
Like the main toolbar, its glass surface has 8-unit rounded corners and a 4-unit
outer inset. A 4-unit gap separates it from the canvas.
It starts hidden. First opening on a flat document creates one undoable transparent
layer above the existing raster; subsequent toggles do not create more layers.

The original bottom raster stays in a pinned Background tile beneath a divider.
It cannot be dragged or deleted. Ordinary layer thumbnails appear topmost first
in the scrollable region above it. Drag a thumbnail to reorder; a drag ghost and
insertion line preview the destination. Drop commits once, Escape cancels, and
auto-scroll reaches offscreen layers. The background is never a drop target.

A top-right eye appears on hover or keyboard focus. Clicking it toggles that layer
without selecting it; hidden layers retain a crossed-eye indication. Names and
opacity appear in tooltips rather than permanent form controls. Right-click a
thumbnail or empty rail for icon-labelled layer operations. A circular + control
at the top-right of the rail or Ctrl+Shift+N creates a layer;
Shift+F10 opens the context menu, F2 opens Properties, and Alt+Up/Down reorders.
Properties contains the existing name and opacity fields. Merge down remains
available for adjacent visible layers, including merging into the fixed background.

The rail, tiles and circular add control follow the existing glass/blur appearance
and surface opacity, with opaque styling when glass is disabled. Backdrop sources
exclude the rail itself to prevent rendering feedback.

The side-panel and background references are Microsoft's original
[layer announcement](https://blogs.windows.com/windows-insider/2023/09/18/paint-app-update-adding-support-for-layers-and-transparency-begins-rolling-out-to-windows-insiders/)
and [background update](https://blogs.windows.com/windows-insider/2024/03/13/paint-update-begins-rolling-out-to-windows-insiders/).
The automatic first transparent layer and pinned original raster follow the
maintainer's requested OpenPaint behavior; exact Microsoft build parity is not claimed.
All icons and captured artwork remain original OpenPaint assets.

The Image ribbon group includes an Invert colors contrast-circle icon. It inverts
the active layer's RGB, or selected pixels only, preserving transparency and other
layers. Selection inversion remains a separate selection-menu command.
