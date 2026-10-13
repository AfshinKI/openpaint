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

At 1280×800, show the full toolbar and an optional 240-unit Layers panel. Support
1024×640 logical units without inaccessible actions. Use labeled overflow menus
for toolbar groups and an overlay drawer for layers at smaller widths. Preserve
the canvas, focus, and selection when panels collapse. Never shrink text to fit.
Support fractional scaling and multiple monitors; test 100%, 125%, 150%, and 200%.

## Interaction contracts

| Action | Behavior |
| --- | --- |
| Draw | Capture pointer, show preview immediately, commit one undo step on release |
| Brushes | Nine named styles with generated previews; floating size and 0–100% stroke opacity; fixed-distance path sampling, one undo step |
| Fill | Left/right press fills with primary/secondary; tolerance 0–255 replaces the size input; busy indicator and Cancel/Escape leave original pixels intact |
| Color picker | Select once per press: left sets primary, right sets secondary; exact containing artwork pixel including alpha; stays selected, never paints or changes history |
| Interrupted stroke | On missing initiating button, grab/focus loss, tool or viewport change, finish at the last valid sample; never bridge to a later pointer |
| Escape | Cancel active preview/transform; otherwise dismiss the topmost transient UI |
| Pan / zoom | Space+drag pans outside text entry; Ctrl+wheel zooms around pointer; plain wheel scrolls |
| Selection | Rectangle/free-form marquee, inversion, numeric bounds, select-all/deselect and undoable alpha deletion; all drawing tools clip. Move/paste previews use Enter/Escape, arrows (Shift: 10 px) or numeric position; copy/cut/paste uses PNG. Resize/skew supports selected content or the whole image. |
| Transform | Shift constrains proportions or rotation increments; numeric inputs provide keyboard precision |
| Text | Inline editor and floating formatting toolbar below ribbon; outside-box canvas click or Ctrl+Enter commits, Escape cancels; top color is foreground and bottom is background |
| Shape | 19-item icon palette; Shift constrains geometry, polygon clicks add vertices and Backspace removes the last; drag/release retains preview; numeric start/end and outline/fill options; Enter/Apply commits and Escape/Cancel restores original; new drawing/tool switch commits current preview; Undo cancels the preview |
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

Updates shows installed version, installation source, channel, last-check time,
and one primary action. Manual checks are always available; automatic checks are
off initially. Offer an opt-in check at most daily, with no background daemon.

| State | User-facing behavior |
| --- | --- |
| Not checked | “Check for updates” |
| Checking | Progress indicator; Cancel |
| Current | “You're up to date” and last successful check |
| Available | Version, release notes, download size, action appropriate to installation |
| Downloading | Bytes/progress, Cancel; editor remains usable |
| Ready (AppImage) | “Restart to update”; resolve all unsaved windows first |
| Managed installation | “Open software manager” or copyable package-manager instructions |
| Offline/error | Plain explanation, Retry; retain installed app and saved artwork |
| Verification failed | Do not install; show failure and link to official releases |

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
