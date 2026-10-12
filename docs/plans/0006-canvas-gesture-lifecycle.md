# Feature: predictable canvas gesture lifetime

Status: implemented; protected merge requires all CI jobs to pass
Feature IDs: F02, F11, F16, F26
Milestone: M1 input reliability

## Outcome and scope

One initiating mouse button owns a drawing/pan gesture until its release or an
explicit interruption. Additional button presses/releases cannot switch its mode,
color or commit it early. A lost initiating button commits at the last valid
sample without drawing a bridge to a later pointer location. Pan applies its final
release position. Escape cancels the active stroke; focus/grab/settings/viewport
changes commit it. Rebinding an editor finishes the old document before disconnect.

Qt mouse positions are logical coordinates; the canvas transform converts once to
document pixels. Stroke starts use continuous half-open image bounds, including
fractional positions just inside the right/bottom edges. Invalid zoom anchors are
ignored. Resizing the viewport ends the gesture even after manual pan/zoom.
Pressure/tilt, new tools, UI redesign and file-format work are outside this slice.

## Implementation

Track the initiating button in CanvasItem alongside drawing/panning state. Reuse
Editor/Document commands for all pixels and history. Add event-level Qt tests for
lifetime and coordinate boundaries, retaining the existing real-window shell
checks. Exercise isolated X11/software input at multiple device scales, keeping it
separate from physical-device, compositor and performance evidence.

## Acceptance cases

- [x] Additional buttons cannot switch drawing to pan, change color or end a stroke.
- [x] Missing release, focus/grab loss, tool change and viewport change finish once;
      later moves/releases cannot append pixels. Escape restores the original.
- [x] Editor replacement leaves neither document with a stranded transaction.
- [x] Pan includes the release endpoint without dirtying artwork.
- [x] Fractional edge starts and zoom/coordinate round-trips are correct; invalid
      anchors cannot poison later drawing.
- [x] Out-of-bounds exit/re-entry clips deterministically and undoes in one step.
- [x] Targeted tests demonstrate regressions before the fix and pass afterward;
      full suites, sanitizer, lint and repository checks pass.

## Evidence and handoff

The new event-level canvas suite initially failed 12 cases on the pre-fix code.
After implementation it passes all 21 Qt Test results (including setup/cleanup).
All seven Debug and ASan/UBSan CTest suites pass, with leak detection enabled.
Formatting, zero-warning QML lint, repository checks and seven tooling tests pass.

The shell suite passes all 14 results at 100%, 125%, 150% and 200% on isolated
X11/software (Xvfb, Qt 6.8.3, Ubuntu 24.04). Two additional 125% runs also pass.
Tests include real-window mouse chords and Space+drag pan alongside event-level
coverage. The GCC Debug CI lane now runs the same four-scale matrix. No display
server TCP listener or application network operation is required.

Isolation exposed an additional test-readiness bug: waiting for a new frame can
time out if the required state already rendered. The shell harness now synchronizes
window events and grabs a non-null rendered window before coordinate actions.
Stroke pixel assertions remain intact. This evidence does not prove the cause of
every prior live-desktop failure or replace compositor/device testing.

Next: remaining M1 codec/orientation and save-failure validation, followed by the
real desktop accessibility/portal matrix. Wayland, physical pen, Orca and portal
behavior remain unverified. No M2 tools or release publication are included here.
