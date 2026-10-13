# Feature: alpha ribbon and color workflow polish

Status: in progress — steps 1–8 implemented; final batch review pending; focused validation recorded below
Feature IDs: F01, F02, F03, F05, F06, F08, F09, F26
Milestone: post-M2 alpha polish, before M3

## Outcome and scope

Make existing editing tools easier to discover in a compact, rounded ribbon.
M2 and 0.1.0-alpha.1 remain completed release checkpoints; this is required work
before leaving the alpha phase. Follow [design](../DESIGN.md) and
[menus](../MENUS.md), with the deliberate changes below taking precedence.

Use one bounded branch, `fix/alpha-ui-polish`, with eight separately reviewable
steps and one final PR/merge after all acceptance cases and CI pass. This grouping
is explicitly requested for this cohesive UI correction. Do not add new editing
features, layers, AI, publishing automation or updater work to this branch.
Do not redistribute external reference images or record their locations.

## Ordered implementation and acceptance checklist

1. **Ribbon groups and split buttons.**
   - [ ] Selection has a main dashed-selection action and a smaller chevron area
     below it. Main activation selects the current selection tool; the chevron
     opens Rectangle, Free-form, Select all, Invert selection, Transparent
     selection and Delete, with separators and correct enabled/check states.
   - [ ] Image uses a compact two-row icon group: Crop, Rotate with an adjacent
     dropdown, Flip with an adjacent dropdown, and Resize/Skew. Rotate offers
     existing 90° clockwise, 90° counterclockwise and 180° commands; Flip offers
     horizontal/vertical. Preserve image/selection scope and preview handling.
   - [ ] Keep bottom group captions, thin dividers and rounded hover/focus states.
     Existing actions stay reachable at the minimum window width.
2. **Quick actions beside View.**
   - [ ] Place Save, Undo and Redo immediately after File/Edit/View, in that order;
     Help and other existing actions must not interrupt this cluster. Preserve
     shortcut bindings, disabled states and unsaved-document safeguards.
3. **Consistent original icons.**
   - [ ] Refine the existing vector family for recognizable, compact tool glyphs
     with consistent strokes and optical alignment. Prefer original SVGs; any
     adopted open-source set requires recorded license/source attribution.
   - [ ] Verify light/dark, disabled, selected, hover and focus appearances; keep
     accessible names/tooltips and current usable hit targets. No copied assets.
4. **Compact shapes gallery.**
   - [ ] Put all 19 existing shapes in a small bordered icon matrix with scrolling
     or overflow access and a visible selected shape. Keep outline/fill controls
     adjacent. Keyboard users can reach every shape; previews and undo stay intact.
5. **Fixed color roles and editable toolbar slots.**
   - [x] Top circle always supplies drawing color; bottom always supplies background/
     fill and right-button drawing. Clicking a circle only selects which color to
     edit. It never swaps colors, changes roles or changes artwork by itself.
   - [x] Basic/filled custom swatches assign the selected circle. Empty toolbar
     slots open a draft. OK applies to the selected circle; Cancel changes neither
     colors nor palettes. Right-click or Shift+F10/Menu offers explicit replacement.
6. **Session-only custom palettes.**
   - [x] Applying a new chooser color inserts it in the first empty ribbon slot
     and first empty editor slot, independently. Ribbon capacity is 10; editor
     capacity is 24. A full palette is skipped; applying the color still succeeds.
     Existing colors do not consume duplicate slots.
   - [x] Both palettes start empty each launch. No palette settings are read or
     written. Legacy files are left untouched and ignored; no migration is needed.
   - [x] Explicit toolbar replacement changes that slot and adds the color to the
     editor if space remains. Dialog Add/Replace/Remove/Clear affect its own list;
     those explicit session edits survive Cancel, while its color draft does not.
     None of these palette actions dirty artwork or enter document undo history.
7. **Floating size and opacity controls.**
   - [ ] Place two narrow rounded vertical panels on the LEFT of the workspace,
     following the requested placement. Replace ribbon size/opacity controls.
     Preserve existing tool-specific limits and 0–100% opacity; expose numeric
     values in tooltips, accessible labels and keyboard adjustment.
   - [ ] Hide/disable irrelevant controls without changing stored tool settings.
     Fill tolerance remains a separately labeled option. Controls must not steal
     canvas gestures, hide required actions, or alter document coordinates when
     shown/resized. Check minimum size and fractional scaling.
8. **Icons in menus and dropdowns.**
   - [ ] Apply a shared icon/check column, aligned text, shortcut hints, separators
     and submenu chevrons to existing action menus. Selection is the first review
     case. Checked, disabled and focused rows must remain distinct and readable.
   - [ ] Escape dismisses and restores focus; keyboard navigation activates the
     same commands as pointer input. Avoid placeholder actions for future features.

## Implementation contracts

QML owns layout and transient UI state; existing C++ Editor/Document commands own
pixel mutations. Use Editor for the selected color-editing target and ColorPalette for bounded
session lists; do not duplicate these models in QML. Preserve alpha
when assigning colors. No document format changes or new networking are needed.
Tool-setting changes affect subsequent gestures, not a partially committed stroke.
Resolve active previews through the existing guards before invoking transforms.

Update DESIGN/MENUS and the feature ledger in each implementation step, marking
only verified behavior implemented. This plan supersedes the older shared custom
palette, explicit Swap button and left-side rail targets. Those remain historical
implementation evidence until the new behavior passes acceptance.

## Validation and evidence

For each step, record targeted behavioral tests and generated OpenPaint captures.
Check light/dark at 1024×640 and 1280×800, plus 100%, 125%, 150% and 200% scaling.
Test keyboard focus, menus, disabled actions and relevant preview/cancel/undo paths.
Color tests cover fixed-role mapping across tools, transparent colors, independent capacity,
empty palettes on launch, Cancel and explicit slot replacement. Slider tests cover
bounds, keyboard input and unchanged artwork/history before drawing.

Run repository/tooling checks, format/QML lint and applicable native/sanitizer
regressions; all protected CI jobs must pass before the single final merge.
Do not claim full accessibility or pixel parity based on screenshots alone.
A later alpha release needs fresh artifact validation; do not replace alpha.1.

## Progress and next action

Step 1 replaces the generic Image menu with separate Rotate and Flip dropdowns,
retains direct Crop/Resize, and gives Selection a smaller chevron below its icon
and a group caption. Existing selection bounds/deselect entries remain available.
Two original vector glyphs identify Rotate and Flip. Existing command guards,
preview handling and keyboard shortcuts are retained.

Next: final batch review, including scale/theme, sanitizer and protected CI checks. Broader theme/scale and full regression
checks are deferred until all eight steps are implemented, as requested.

### Step 1 focused evidence

Fresh dev configuration with Qt 6.12.0 and the full dev build succeeded. The
executable is `build/dev/openpaint`; desktop `--version` succeeds. Focused shell
cases `launchAndResize`, `imageTransformWorkflow` and `freeFormSelectionWorkflow`
passed (5 including setup/cleanup), with offscreen software rendering and no QML
warnings. Repository and diff whitespace checks passed. This is not a full visual
or accessibility certification; the batch review remains pending.

### Steps 2 and 3

Save/Undo/Redo now immediately follow File/Edit/View; Image/Help remain on the
right. Existing actions and shortcut bindings are reused. Toolbar SVGs use one
24-unit grid and 1.6-unit rounded strokes, with original independently authored
glyphs and SPDX attribution. The app logo and multicolor chooser glyph are unchanged. Existing theme tinting,
accessible labels, hit targets and focus/selected states remain in place.

The dev executable was rebuilt. Focused launch/layout, quick-action placement,
theme capture and keyboard Help checks are the validation scope for this step;
full scale/state and accessibility review remains part of the final batch check.

Focused checks passed: 6 including setup/cleanup, no QML warnings. Light/dark
1280px captures were inspected; the chooser retains its multicolor glyph for
contrast. Repository and diff whitespace checks passed.

### Steps 4 and 5

The ribbon now contains a four-column, three-visible-row shape gallery, with
scrolling and arrow-key access to all 19 shapes. The full chooser remains next to
it, alongside outline/fill toggles. Dense shape targets are 24px; tooltips and
keyboard focus identify each shape. The minimum-width toolbar remains reachable.

The later color correction supersedes the initial role-switching/persistence
implementation: top is always foreground, bottom is always background/fill.
Selection only chooses an editing target. Applying a chooser color fills each
palette's first available slot independently, skipping full lists. Custom colors
last only for the app session. There are no settings migrations or palette writes.

Focused validation: dev build succeeded; shell launch/minimum layout, full shape
chooser, shape options, color editor, exact-slot/role workflow and theme captures
passed (8 including setup/cleanup). Color unit suite passed (7); active-role
drawing/undo and alpha-preservation test passed. Generated dark UI inspected.
Broader scale/accessibility checks and the full regression remain deferred.

Additional focused checks passed: direct gallery keyboard navigation, control
accessibility smoke checks, active-role fill/undo and GUI text color mapping.
Text coverage runs in the GUI shell suite because the headless editor unit suite
has no font database. This does not claim full Orca verification.

### Color correction and step 6

The fixed-role/selected-target split is owned by Editor; palette insertion and
capacity limits are owned by ColorPalette. All pixel commands retain their
original foreground/background meaning, regardless of the selected circle.
The 10/24 session lists replace the earlier persistent stores. Existing legacy
settings are neither read nor deleted. Full palettes do not block color acceptance.
Focused validation covers fixed-role drawing/fill/shape/text, both capacity limits,
duplicates, explicit replacement, cancel, and fresh-instance empty palettes.

Correction validation: dev build succeeded; fixed-color-role engine case passed
(3 including setup/cleanup), color model suite passed (5), focused shell launch,
color editor and toolbar workflow passed (5). Repository and diff checks passed.
Earlier step 5 test counts above describe the superseded implementation only.

### Steps 7 and 8

Floating left-side Size and Opacity panels replace their ribbon controls. Size
is 1–128 px for pencil/brush/eraser/shapes; brush opacity is 0–100%. Original size/opacity icons accompany narrow 36px vertical panels; numeric
values are shown in tooltips, which support keyboard adjustment.
Fill tolerance remains in the ribbon, and irrelevant controls hide without losing
settings. The panels consume input in their margins and do not resize the canvas.

Action-backed menus share icons with their commands. Selection, shape styles,
color management, units, alignment and export choices have original vector glyphs;
brushes retain generated sample previews. Qt supplies command shortcuts and submenu navigation. Shared rows render
aligned icons/checkmarks and native-text shortcut hints with live theme colors. Disabled rows use muted icons/text. No new
editing commands, network behavior or external asset dependency is introduced.

The build and focused slider/menu tests are recorded below. Full batch review and
all protected CI jobs remain required before the single final merge.

Step 7/8 evidence: dev rebuild passed. Focused final shell checks for theme
changes, free-form selection menu, floating sliders and brush menu passed
(6 including setup/cleanup); keyboard/control checks also passed. The slider test
verifies numeric changes, hidden irrelevant controls, retained settings, unchanged
canvas geometry and no painting through panel margins. Runtime shortcut-hint
colors have a dark-theme regression assertion. Light/dark generated menu captures
and minimum-size floating controls were visually inspected. Strict QML lint,
repository and diff checks passed. Broader batch review remains pending.

### Follow-up: narrow left rail and contextual text toolbar

The slider rail moved left and narrowed to 36px. Original stroke-size and half-full
droplet vectors replace text/readouts; hover/drag tooltips and accessible slider
values preserve precision. No external images were copied. Image/Help menus are
removed; existing image commands remain in the ribbon and File hosts Updates/About.

Text now exposes a floating non-modal formatting row below the ribbon as soon as
Text is selected. Font/style/alignment/background controls preserve the draft.
A left click on canvas/workspace outside a ready text box commits through the
existing C++ command, consumes the click and does not start a second box. Panning
and inside-box clicks do not commit. Escape and Ctrl+Enter retain their existing
semantics. Text has no Apply/Options footer buttons; guarded New/Open/Close flows
still protect an active draft. Full batch review is still pending.

Follow-up validation: dev build and strict QML lint passed. Focused launch/layout,
floating controls, existing text workflow, outside-click commit/undo and File →
Updates workflow passed (7 including setup/cleanup). Text toolbar and narrow
left rail were inspected in generated 1024px captures. Tab/Shift+Tab now move
from the inline text editor to controls; Ctrl+Enter and Escape remain available.
Repository and whitespace checks passed. Changes remain on the same polish branch.

## Follow-up requests

Items 1–7 are implemented below; items 8–11 remain pending before the final polish
review. Earlier implementation evidence does not verify these requests.

1. Add small square corner resize handles to active shapes, text boxes and
   selections. Committed raster marks require selecting their pixels before
   resizing; retaining separately editable drawn objects would need a separate
   document-model decision. Specify preview, cancel and one-step undo behavior.
2. Remove idle button outlines in Image, Tools, Brushes and Shapes. Preserve
   visible hover, selected and keyboard-focus states.
3. Change the Selection button icon to match Rectangle or Free-form selection.
4. Remember the last horizontal/vertical Flip choice and show the matching icon;
   the main button repeats that operation. Session state is sufficient unless
   persistence is explicitly requested later.
5. Give rotation choices distinct original direction/angle icons. Show the last
   chosen rotation on the main button so its next action is clear.
6. While Color picker is selected, show a floating sampled-color square attached
   to the eyedropper tip, following the pointer. Define out-of-canvas and alpha
   presentation; hovering must not change colors, artwork or undo history.
7. Clear the active selection when switching to another tool. This deliberately
   changes the current cross-tool selection-clipping behavior: reconcile active
   move/paste previews and document the new command/undo semantics before coding.
8. Add corner handles for canvas resizing, with explicit clipping behavior and
   undo. Keep canvas resizing distinct from resampling selected content.
9. Make Edit colors a larger circular color icon without a square outline.
10. Make the Brushes icon larger and centered above its caption. Main activation
    selects the current brush for painting; a separate small arrow below opens
    the brush-style menu. Pencil continues to draw a solid line independently of
    brush style. Selecting Brush shows opacity below thickness in the left rail.
11. Enlarge the visible shape gallery to six columns by three rows or larger so
    every shape is available without scrolling. There are currently 19 shapes,
    so use at least 19 visible cells (for example 7×3), while preserving minimum-
    window usability and keyboard access.

Use original assets. Define acceptance cases for interaction changes before
implementation. Resume deliberately in bounded scope; do not merge the polish
branch until the requested review and required checks pass.

### Item 1 acceptance — active corner resize handles

Scope: four corner handles for a ready text box, an active shape preview, or a
selection (including floating pasted/moved content). Committed raster marks must
first be selected. Canvas resizing is item 8 and is outside this change.

- Drag shows bounds; release resizes the existing preview, without flattening it.
- Opposite corner stays fixed; bounds stop at one pixel and image limits.
- Text reflows without changing font size; shape geometry scales, including all
  polygon vertices; selection pixels/mask resize through the bounded worker.
- Repeated selection resizing stays in one preview; Apply/Enter commits one undo
  step. Escape during a handle drag discards only that drag. Existing preview
  Cancel restores original pixels; undo/redo after commit preserves the result.
- Handles keep constant screen size across zoom, sit above inline text input, and
  support keyboard arrows. Busy state, viewport changes and lost grabs must not
  leave a stale drag or paint through a handle.

### Item 1 implementation and evidence

Implemented four corner handles with 8px squares and 16px hit targets. Text
reflows, shape vertices scale, and selected/pasted pixels resize asynchronously.
Repeated selection resizing retains the original preview base for one-step
commit/undo and cancellation. Keyboard arrows resize by 1px (Shift: 10px).

Validation: Qt 6.12 dev build passed; `./build/dev/openpaint` is rebuilt. All nine
transform cases, the polygon geometry case and four focused shell cases passed
(including corner drag/Escape, constant-size zoom targets, text editing and image
transforms). Formatting, strict QML lint and repository checks passed. Inspected
the generated text-handle capture. UI checks used offscreen software rendering;
full desktop/platform and batch regression checks remain pending. At that checkpoint the next request was item 2. Canvas resize remains item 8.

### Items 2–5 acceptance — ribbon action clarity

- Image, Tools, Brushes and Shapes buttons have no idle outline/background;
  hover, pressed, selected and keyboard focus remain visible.
- Selection shows rectangle or lasso to match its selected mode.
- Flip and Rotate remember their last session choice. Main activation repeats
  that action; icon, tooltip and accessible label agree with it. Menus retain
  every operation with distinct original direction/angle icons.
- Existing busy/preview guards and undoable transforms remain unchanged.

### Items 2–5 implementation and evidence

Implemented scoped flat ribbon buttons while retaining selected/hover/focus
feedback. Rectangle/lasso selection follows the active mode. Flip and rotation
main buttons retain their last session action and its label/icon; dropdowns use
original horizontal/vertical and clockwise/counterclockwise 90°/180° artwork.
Rotation angle numerals are vector paths, with no font dependency.

Qt 6.12 dev build, focused shell action-memory and image-transform workflows,
formatting, strict QML lint and repository checks passed. The action-memory case
exercises all five operations, Undo and main-button repetition, plus both selection
icons. Inspected the generated ribbon capture. Tests used offscreen software
rendering; the final desktop/platform regression review remains pending. Launch
with `./build/dev/openpaint`. Next unfinished request: item 6, eyedropper preview.

### Selection split-control appearance follow-up

Selection now uses a compact 40×56 vertical split control with one rounded accent
outline around both halves when active, no idle outline, per-half hover feedback
and a stronger shared keyboard-focus outline. Rectangle/lasso mode indication
and separate main/dropdown actions remain intact. Dev build, three focused shell
workflows (free-form, rectangle and ribbon action memory), formatting, strict QML
lint and repository checks passed. Inspected the generated active-selection
capture at minimum window width; full desktop review remains pending.

### Items 6–7 acceptance

Eyedropper hover shows a pointer-attached square with checkerboard alpha backing;
leaving the image/canvas or changing tools hides it. Hover changes neither colors
nor document/history. Tool changes commit any active move/paste preview once,
then clear selection without adding a history entry; Undo restores edited pixels.
Busy transforms must finish before switching tools. Explicit selections created
while already using another tool can still clip that tool until the next switch.

### Items 6–7 implementation and evidence

Implemented a 28px pointer-attached hover swatch with checkerboard alpha backing,
image-bound checks and hover-leave cleanup. Hover sampling is read-only. Tool
changes commit pending move/paste content then clear selection without another
undo entry; busy transforms block tool switching. Explicit clipping selections
created after choosing a tool remain supported by the engine.

Dev build passed. Editor suite (12 cases), canvas suite (24 cases/data rows), and
both selection shell workflows passed with offscreen Qt. Coverage includes hover
without document/color/history mutation, preview hiding, selection clearing,
paste commit and Undo. Formatting, strict QML lint and repository checks passed.
Full desktop/platform review remains pending. Next unfinished request: item 8,
canvas corner resizing. Binary: `./build/dev/openpaint`.
