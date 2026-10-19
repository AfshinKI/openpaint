# Menu and control specification

Pending alpha corrections are specified in the [eight-step UI polish plan](plans/0028-alpha-ui-polish.md).
It supersedes the earlier shared custom-palette, Swap-button and left-rail targets;
current shipped behavior remains unchanged until implementation is verified.

This is the visual and interaction contract for future implementation. It does not
mark features as shipped; [FEATURES.md](FEATURES.md) remains the status ledger.
Static layouts establish placement and visible choices, not runtime behavior,
exact dimensions, hidden submenu contents or keyboard semantics. Items below
remain planned unless the ledger says otherwise.

## Shared layout

Use a compact menu row above the grouped command toolbar. File, Edit and View sit
on the left, followed immediately by quick Save, Undo and Redo actions with a
small separator. The Image and Help menus are removed; image commands remain in the ribbon and
Updates/About are in File. Share is
deferred until desktop sharing is implemented. Place What's new and Settings at the far right. F1 opens About for keyboard access. Window controls remain
native Linux decorations, separate from application actions. Core editing needs
no account; an account/profile button is not an OpenPaint requirement.

The command groups run left to right: Selection, Image, Tools, Brushes, Shapes,
Colors, optional intelligent tools, Layers. Use thin vertical dividers and short
captions below groups. Dense icon grids replace rows of text buttons. A selected
tool has a rounded inset background and accent outline. Small chevrons identify
menus; a toolbar-collapse chevron sits at the far right.

Keep the current 112px toolbar, 20px icons in 32px buttons and 24px dense color
targets as OpenPaint sizing tokens, not measurements of another application.
Future groups must fit through deliberate overflow/collapse behavior rather than
shrinking labels. Do not expose decorative buttons for unavailable features.

Menus have rounded dark/light surfaces, a consistent left icon/check column,
left-aligned labels, right-aligned shortcut hints, and submenu chevrons at the
right edge. Separators divide related commands. Highlighted rows are rounded and
inset from the panel border. Disabled actions retain their place with muted text
and icons; they must not look selected. Keyboard focus remains visible.

## File

Preserve this grouping and order when the corresponding capabilities exist:

| Group | Entries, in order | Layout and implementation notes |
| --- | --- | --- |
| Documents | New; Open; Import to canvas; Recent; Save; Save as; Save as project | Import, Recent and Save as are submenu entries. Use familiar document/folder/save icons. Recent is local and clearable. Import modifies the current document through undoable commands. |
| Output | Print; Share | Print has a submenu affordance; its exact entries are not established. Linux output/sharing requires an explicit action and platform integration. |
| Desktop | Set as desktop background | A separate, availability-dependent submenu. Defer until Linux desktop integration is scoped; do not add a dead placeholder. |
| Information | Image properties | Separate from output commands. Ctrl+E is the target shortcut; dialog contents need a feature specification. |
| Application | Exit | Bottom row with a close icon; uses the unsaved-document guard. |

New/Open/Save use Ctrl+N/Ctrl+O/Ctrl+S. Current OpenPaint Ctrl+Shift+S remains Save
As. The layered-project workflow must resolve its shortcut deliberately before
adding Save as project: do not silently reassign the current shortcut or bind two
commands to it. Editable project storage uses the architecture's .ora contract;
this menu is not a promise of proprietary project-format support.

Save-as formats, recent-file presentation, import choices, print options and
properties fields still need individual behavior plans. Failed import/save must
preserve artwork, and New/Open/Exit retain Save/Discard/Cancel handling.

## Edit

The compact clipboard menu orders Cut (Ctrl+X), Copy (Ctrl+C), Paste (Ctrl+V),
then a separator and Copy visible layers (Ctrl+Shift+C). Each row has its own
clipboard-related icon. Undo/Redo remain quick actions in the top row; retaining
accessible menu equivalents in OpenPaint is allowed.

Cut/Paste are undoable edits. Copy visible layers requires a defined compositing
and selection contract in M3. Empty selection/clipboard states disable affected
actions without hiding them. Normal text editing shortcuts continue to work in
text fields. Clipboard ownership, transparency and external-app interchange need
behavioral tests; menu labels alone do not establish those semantics.

## View

| Order | Entry | Presentation |
| --- | --- | --- |
| 1 | Zoom | Magnifier and submenu chevron; exact preset list remains unspecified. |
| 2 | Rulers | Ctrl+R hint; toggle with visible state. |
| 3 | Gridlines | Ctrl+G hint; toggle with visible state. |
| 4 | Status bar | Checkmark when enabled. |
| 5 | Always show toolbar | Checked persistent-toolbar mode, separated from display aids. |
| 6 | Automatically hide toolbar | Alternative to persistent-toolbar mode. |
| 7 | Full screen | Expansion icon and F11 hint, after a separator. |
| 8 | Thumbnail | Small image icon; preview contents, docking and navigation need specification. |

Toolbar modes are mutually exclusive. Auto-hide must retain menu/keyboard access,
restore focus predictably and avoid moving the image in document coordinates.
Do not infer auto-hide timing or thumbnail behavior until tested and specified.

## Selection and image groups

Selection uses a prominent dashed-rectangle icon with a dropdown below it. The
menu order is Rectangle, Free-form, separator, Select all (Ctrl+A), Invert
selection, Transparent selection, then Delete (Delete). Rectangle has a rounded
selected row. Invert/Delete may be disabled when no suitable selection exists.
Transparent selection must expose its on/off state when implemented; its pixel
and background-color rules need explicit tests.

The Image group uses two rows: Crop and Rotate above Resize/Skew and Flip.
Rotate and Flip each have a direct default action and an adjacent dropdown.
Rotation offers right 90°, left 90° and 180°; Flip offers horizontal and vertical. Plan Crop, Rotate, Flip, Resize/Skew
and background removal against their existing feature IDs. Unlabeled glyphs do
not prove exact commands, angle lists or enabled-state rules. Specify those in the
feature plan before wiring an icon to a command. Background removal stays in M6.

## Tools, brushes and shapes

The Tools group is a two-row, three-column icon grid: Pencil, Fill, Text on the
first row; Eraser, Color picker, Magnifier on the second. Use original symbols and
accessible names/tooltips. Tool options should not force the whole toolbar to
expand in height.

Brushes use one brush icon and a dropdown. The menu is a tall rounded list with
names at left and original sample strokes at right, in this order:

1. Brush
2. Calligraphy brush
3. Calligraphy pen
4. Airbrush
5. Oil brush
6. Crayon
7. Marker
8. Natural pencil
9. Watercolor brush

Samples communicate stroke character and share a common preview width. They must
be generated by OpenPaint or independently authored. This list establishes UI
inventory, not pressure response, blending equations, texture fidelity or default
sizes. Implement each brush with its own input/undo/performance acceptance cases.

Shapes occupy a compact outlined-icon matrix with small overflow/scroll cues and
adjacent outline/fill/style controls. The visible vocabulary includes straight
and curved lines, oval, rectangle, rounded rectangle, polygonal/triangular shapes,
diamond, pentagon, hexagon, directional arrows, stars and speech bubbles. This is
not an exhaustive shape catalog; partially visible items and unlabeled secondary
controls need confirmation. Shape defaults, constrained drawing, stroke/fill
options and preview commit/cancel belong in the implementation plan.

## Colors

Stack two larger color circles to the left of two rows of ten basic
swatches. A third row always shows ten custom slots, with hollow circles for
empty positions. A colorful edit-colors action sits beside the grid; the Colors
caption is below. A filled swatch and an empty slot are distinct states, not
missing UI. Clicking either large circle selects it for color editing: top always
means drawing color, bottom always means background/fill. It does not swap roles.
Basic and filled custom swatches assign the selected circle. Empty slots open a
chooser draft. OK applies and adds a new color to the first empty slot in both
palettes; skip each full palette independently. Right-click or Shift+F10/Menu
provides explicit slot replacement. No Swap button is needed.

The rounded Edit colors dialog has a large square hue/saturation spectrum on the
left, a narrow tall selected-color preview, a vertical brightness slider, then a
HEX field, RGB/HSV selector and three numeric channels with labels to their right.
Below are a 12-by-4 Basic colors grid and a 6-by-4 Custom colors grid with an add
button. A separate footer contains two broad side-by-side OK and Cancel buttons.
Keep the dialog usable at the minimum window size without clipping its footer.

OpenPaint offers a custom-color management menu and an explicit session-lifetime
hint. The ribbon holds 10 colors and the editor 24. Both start empty on launch;
no palette settings are read or written. Existing colors do not consume another
slot on acceptance. Explicit editor Add/Replace/Remove/Clear acts immediately
within this session; Cancel discards the draft but not these explicit actions.

## Workspace and status bar

Active shapes, ready text boxes and selections expose four small square corner
handles with fixed 16px hit targets at every zoom level. Drag previews bounds;
release resizes the active preview. Text reflows at its existing font size.
Selection pixels and masks resample in the bounded worker. Repeated resizing
commits as one undoable edit. Escape during dragging discards that drag; preview
Cancel restores the original artwork. Focused handles support arrow keys (Shift
for 10px). Committed raster marks must first be selected.

Use a broad neutral workspace around a rectangular, sharp-cornered image. Canvas
resize handles are small squares at all four corners when no selection or editing
preview is active. Dragging previews crop/expansion; releasing commits one undoable
canvas-size change without resampling. The opposite corner stays fixed in document
space. New space uses the background color, retaining existing pixel alpha.
Escape cancels a drag; arrow keys resize by 1px (Shift: 10px). The canvas refits
after commit. Dimensions are limited to 16384 per side and 32MP overall. A left-side floating rail holds narrow 36px rounded Size (1–128 px) and Opacity
(0–100%) slider panels, with original stroke/droplet icons and accent thumbs.
Numeric values appear in tooltips; accessible names retain each role and range. Size appears
for pencil, brush, eraser and shapes; opacity appears for brushes. Both support
keyboard arrows and are disabled while busy. Fill uses exact color matching with automatic smooth edges. The rail consumes pointer input without painting through
its margins and does not resize or reposition the canvas.

The bottom status strip separates cursor/selection information from image size.
The right side groups fit-to-window, a percentage dropdown, zoom-out, a horizontal
zoom slider and zoom-in. Fractional percentage display should remain readable.
The expanded zoom dropdown remains planned. Canvas operations still belong to undoable document commands;
view-only changes never dirty the drawing.

## Settings

Settings is a full application page with a back arrow and heading, preserving the
open document. Use a wide main settings column and a narrower About column on
large windows, stacking them on smaller windows. Appearance contains an expandable
rounded App theme card with Light, Dark and Use system setting radio choices.
Only one choice is active; system mode follows desktop changes.

The About column contains app name/version and links for license, privacy and
third-party acknowledgements, plus an explicit feedback action. OpenPaint does
not display or persist a session identifier. Feedback must not silently send logs,
artwork or diagnostics. The product's Updates section follows [RELEASES.md](RELEASES.md).

A separate generated-content settings card is a later M6 concern. Its intended
role is a clearly explained watermark/export preference with a dropdown; the
complete option list and export effects require a separate contract. Do not add
an inactive AI settings card before the corresponding feature ships. Current
persistent themes are not yet a complete Settings-page implementation.

## What's new

A small announcement/megaphone action near Settings opens an optional, dismissible
feature-discovery surface. Use a rounded large dialog, heading at top left,
close control at top right, a narrow vertical topic list on the left, and a large
preview/detail card on the right. The active topic has a subtle filled row and
accent edge. Each topic has a brief title/description; its detail card combines
original demonstration media, explanation and an optional relevant action.

Populate it only with shipped OpenPaint capabilities and local bundled content.
No sign-in gate, marketing copy or external service dependency is required. AI
creation/erase topics remain deferred; Layers is eligible when implemented.
Do not open this surface automatically or initiate network requests. Treat it as
optional discovery work after core editing/release readiness, not a new M1 gate.

## Implementation review checklist

Before adding a menu or group, record its feature IDs, command ownership, enabled
states, keyboard/accessibility behavior, focus restoration and minimum-size
layout. Test mutations for undo/cancel/save fidelity, use the shared rounded
controls, and generate synthetic light/dark evidence from OpenPaint itself.

Keep the visible inventory separate from assumptions. Unopened submenus, dynamic
interactions, numeric limits, brush rendering, AI availability and exact parity
remain unverified until their own acceptance cases pass. Update the ledger and
this contract together when deliberate Linux-specific choices change the layout.

## Floating text controls

Selecting Text shows a non-modal toolbar immediately below the ribbon: font,
size, bold/italic/underline/strikeout, alignment and opaque background. Formatting
controls do not commit the active text box. Clicking the canvas/workspace outside
the box applies it as one undoable raster edit and consumes that click, without
starting another text box. Clicking inside keeps editing; middle-button/Space pan
does not commit. Escape cancels and Ctrl+Enter remains a keyboard commit shortcut.
The text Apply/Options footer buttons are removed; other tools retain theirs.

### Ribbon action feedback

Image, Tools, Brushes and Shapes controls have transparent, borderless idle
surfaces. Hover, pressed, selected and keyboard-focus states remain visible.
Selection uses the rectangle or lasso icon matching its current mode. Rotate
and Flip retain the last operation for the current session: their main buttons
repeat it, with matching direction/angle icons, tooltips and accessible names.
Their dropdowns continue to expose all operations. Defaults are right 90° and
horizontal flip. These choices do not persist between launches.

Selection is a compact vertical split control with a 26px icon: the icon activates the current
selection mode, and the lower arrow opens its menu. Both share one rounded accent
outline when Selection is active; idle state has no outline. Each half retains
hover feedback, and keyboard focus strengthens the shared outline.

Eyedropper hover shows a 28px color square offset beside the pointer tip, with
checkerboard backing for transparency. It hides outside the image/canvas and
when another tool is selected. Hover does not sample into either working color
or change artwork/history; clicking retains the existing sampling behavior.

Switching tools clears selection bounds. Pending moved or pasted content is
committed first as one undoable edit; clearing bounds adds no history entry.
Busy transforms block tool changes. An explicit selection created while a tool
is already active may still clip its edits until the next tool switch.

Edit colors uses a 26px multicolor circular icon with no idle square
outline. A checkerboard option directly below selects transparent background
color and activates the background editing target; foreground is unchanged.
Transparent/partially transparent swatches show checkerboard backing, distinct
from empty custom slots. Choosing another color restores an opaque background.
This sets the working background/fill color, including new canvas space; it does
not erase existing pixels or add a document history entry.

Cut/Delete and the source area vacated by moved or transformed selections use the
current background color, including its alpha. Irregular masks preserve pixels
outside the selection. Undo restores the original pixels. The two color-option
icons sit between the three palette rows; the transparency icon is 22px, with separate 28px circular controls.

Brushes has a centered 28px icon: main activation selects the retained brush;
the separate lower arrow opens brush styles. Pencil remains independent. Brush
shows opacity below size in the left rail. The ribbon shape gallery is seven
columns by three rows, exposing all 19 shapes without scrolling.

The menu row ends with Check for updates and Settings icon buttons. Settings
exposes System/Light/Dark appearance. The update shortcut opens the existing
Updates screen (release guidance; automatic checking/installing remains planned).

Brushes mirrors Selection: the icon and lower arrow share one rounded accent
outline when active and a stronger shared keyboard-focus outline. Idle has no
outline; each half retains hover/pressed feedback and its separate action.

Menu-row Save, Undo and Redo use flat idle surfaces with no outlines. Undo/Redo
use mirrored original curved-arrow icons; hover and keyboard focus remain visible.

The full-shape popup button is removed from the ribbon because all shapes are
visible in the gallery. Separate icon-and-chevron controls open Outline and Fill
menus. Outline offers None/Solid outline; Fill offers None/Solid fill. These
reflect the supported renderer styles and update active shape previews.

Outline and Fill choices are exclusive: activating the already selected entry
keeps its checkmark. Ribbon group captions share a common bottom baseline,
including Selection and Brushes.

## Save/export filename safety

Save requires a `.png` filename; Export requires the selected format's extension.
The dialogs preserve the exact filename authorized by the desktop portal. They do
not append extensions after selection. A missing/wrong extension shows an error
and preserves the current drawing and saved state; choose again with the extension.

Updates provides Check now, Cancel check, a default-on startup/daily-check option and View releases. A newer stable release produces a dismissible in-app notice; it never interrupts or restarts editing. Flatpak retains manual browser links.

Settings includes Glass appearance with a persistent toggle, 40–100% surface opacity slider, Restore defaults and Close. Defaults are enabled/40%; disabling restores opaque surfaces. No artwork edit or history entry occurs.

Fill always cleans a one-pixel same-color antialias fringe without expanding flood connectivity. Color tolerance is fixed at zero.

Fill has no floating options panel.

Canvas midpoint handles resize one axis: left/right change width, top/bottom change height. Corners retain two-axis resizing. The decorative image outline is removed; handle and preview feedback remains.
## Selection dismissal

An outside click dismisses existing selection bounds and consumes the gesture;
pending moved/pasted pixels commit first. A subsequent gesture creates a new
selection. Outside clicks commit retained non-polygon shapes without creating
another shape. Polygon vertex entry is unchanged. Ctrl+A activates Selection and
selects the entire canvas after committing non-text previews; focused text fields
retain their own Select All. Busy operations block this action.

## Frosted appearance

Glass appearance uses a continuous translucent window base with separate tinted
bars. Floating controls blur lower app layers on shader-capable renderers; menus
and dialogs also include the fixed bars and floating controls beneath them.
Popup backdrops are flattened to prevent sharp content leaking through the blur.
Software rendering uses solid popups. The paper/checkerboard, text, and icons keep
their own opacity; appearance changes do not affect artwork or undo history.

Light menus use very little white tint. The shape gallery and footer buttons
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

Development font picker: a shared searchable dialog previews installed families.
Arrow keys browse, Space selects, Enter applies, and Escape cancels. Applying a
family preserves size/style and returns to the active text preview.
Development text editing uses a dashed black/white box outline and a blinking
black caret immediately in an empty box at the scaled text height; neither overlay is saved in the artwork.
Development Linux windows use 12px rounded bottom client corners and matching input masks;
top corners stay square, and other platforms receive no custom corner mask.
maximized/fullscreen edges are square. Native decorations remain desktop-owned.
Development glass selection uses stronger neutral frost and light outlines instead
of green fills. Software rendering uses tint/outline; desktop blur stays compositor-owned.
Updates identifies the designer as Afshin Kashani, PhD, P.Eng beneath the version.


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
