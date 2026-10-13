# Feature: alpha ribbon and color workflow polish

Status: planned — no implementation claimed
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
5. **Active color roles and toolbar slots.**
   - [ ] Clicking either large circle makes that role active for color assignment;
     the selected role becomes the main drawing color, the other is background/
     fill. Show a clear active outline without requiring a Swap button. Switching
     roles must preserve both stored colors. Define left/right drawing and shape/
     text fill mapping in tests so all tools follow the same active-role contract.
   - [ ] Clicking an empty toolbar slot opens the chooser for that exact slot;
     OK stores and selects its color as main, Cancel leaves the slot and colors
     unchanged. Clicking a filled slot selects it as main; right-click opens an
     explicit replacement action. Provide equivalent keyboard/context-menu access.
6. **Independent persistent custom palette.**
   - [ ] Keep ten toolbar slots separate from the chooser's 24 saved custom colors.
     Adding/replacing/removing in one must not silently change the other. Preserve
     both across restart; palette edits do not dirty artwork or enter undo history.
   - [ ] Migrate existing saved colors without loss: preserve all 24 in the dialog
     library and seed the toolbar's ten slots from the previous visible entries
     once. Version settings and test repeated startup, gaps and malformed settings.
   - [ ] Dialog library Add/Replace remains an explicit immediate save; Cancel
     discards the drawing-color/toolbar-slot draft, not explicit library changes.
7. **Floating size and opacity controls.**
   - [ ] Place two narrow rounded vertical panels on the RIGHT of the workspace,
     following the requested placement. Replace ribbon size/opacity controls.
     Preserve existing tool-specific limits and 0–100% opacity; expose numeric
     values, accessible labels, keyboard adjustment and tooltips.
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
pixel mutations. Extend ColorSelection/ColorPalette for active roles, separate
slot stores and migration; do not duplicate these models in QML. Preserve alpha
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
Color tests cover active-role mapping across tools, transparent colors, independent
persistence, migration, Cancel and exact-slot replacement. Slider tests cover
bounds, keyboard input and unchanged artwork/history before drawing.

Run repository/tooling checks, format/QML lint and applicable native/sanitizer
regressions; all protected CI jobs must pass before the single final merge.
Do not claim full accessibility or pixel parity based on screenshots alone.
A later alpha release needs fresh artifact validation; do not replace alpha.1.

## Progress and next action

Planning only. Next: implement and validate step 1, then review each numbered
step in order on this branch. No application behavior has changed yet.
