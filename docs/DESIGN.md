# Interface design

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
│ File Edit View │ Save Share Undo Redo             What’s new Settings  │
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

Use original, consistent 20/24-unit vector icons with accessible text labels and
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
| Interrupted stroke | On missing initiating button, grab/focus loss, tool or viewport change, finish at the last valid sample; never bridge to a later pointer |
| Escape | Cancel active preview/transform; otherwise dismiss the topmost transient UI |
| Pan / zoom | Space+drag pans outside text entry; Ctrl+wheel zooms around pointer; plain wheel scrolls |
| Selection | Visible marquee, handles, numeric bounds, active-layer scope; dragging previews until commit |
| Transform | Shift constrains proportions or rotation increments; numeric inputs provide keyboard precision |
| Text | Inline editor with font, size, alignment, style, foreground and optional background; Ctrl+Enter commits, Escape cancels |
| Shape | Adjustable before commit; outline/fill choices explicit; Enter commits and Escape cancels |
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

The File menu separates Save, Save As, and Export. Show the current file format
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
colors are editable. Twenty-four local custom
swatches support add, replace, remove and clear. These explicit palette actions save
immediately, even if the drawing-color draft is later canceled; the dialog explains
this distinction. Palette changes are settings, not undoable
artwork edits, and never dirty the document. See the [UI polish plan](plans/0004-ui-polish.md)
for the two focused implementation slices and acceptance cases.

The [reference layout refinement](plans/0005-reference-color-layout.md) supersedes
the earlier inferred hue-ring design. Toolbar height is 112 logical pixels;
primary/secondary circles stack beside two rows of basic colors and a third row
of ten custom slots. Empty slots remain visible as hollow circles and open the
editor. The color editor always exposes all 24 custom slots. Plus/empty-slot
activation appends the draft to the next free slot; Manage custom colors offers
replace/remove/clear. Removal compacts the slots. Native window frames remain
under desktop control. User-provided reference screenshots are not redistributed.

M1 keyboard evidence: [desktop workflow](plans/0008-keyboard-desktop-workflow.md).
Dialogs take keyboard focus; Escape cancels unsaved changes without mutating pixels.
Ctrl+Q is an explicit Quit binding, independent of the desktop theme’s defaults.
