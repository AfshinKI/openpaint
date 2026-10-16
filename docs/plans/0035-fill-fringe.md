# Feature: fill same-color antialias fringes

Status: implemented; PR review pending
Feature IDs: F04
Milestone: post-v1 fixes

## Behavior and implementation

Fill defaults to Smooth edges. Along the eight-neighbor fringe of the four-connected flood boundary it also
replaces a one-pixel coverage blend between the seed color and chosen fill color,
with one-channel-unit rounding tolerance in premultiplied RGBA. These fringe pixels
never expand the flood, so the option cannot traverse the fringe to another region.
This fixes filling black around an antialiased black line without raising tolerance
and crossing unrelated color boundaries. Disable Smooth edges for exact matching.
It is a raster heuristic: matching edge pixels of a gradient may also be affected;
it does not reconstruct arbitrary foreground colors or eliminate every mixed-color
halo. Selection clipping, cancellation, revisions and one-command undo remain.
The setting is session-only. Fill options move into a floating panel so the palette
remains accessible at 1024px. No data leaves the device.

## Acceptance cases

- [x] Generated antialiased black line on white becomes uniformly black on fill.
- [x] Exact mode retains coverage pixels; unrelated colors and enclosed regions stay.
- [x] Selection clipping, cancellation, undo/redo and existing tolerance tests pass.
- [x] Fill controls fit the minimum window and have accessible names.

## Validation and handoff

Dev build, repository checks, formatting and QML lint passed. Document/editor
tests and shell-more passed; fillOptionsLayout was rerun after moving the controls.
Inspected light/dark 1024px captures; palette and fill options fit. Offscreen/software
evidence only; real-desktop validation remains outstanding. Next: protected PR review.

Integration CI exposed the expanded shell-more group reaching its 30-second budget
under ASan after its assertions had passed. The control tests now run as a separate
shell-controls suite, preserving every case and the same 30-second per-suite limit.
