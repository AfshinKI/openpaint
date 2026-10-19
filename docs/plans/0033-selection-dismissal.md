# Feature: consume outside-click selection dismissal

Status: merged in PR #41; shipped in v1.1.0
Feature IDs: F06, F08
Milestone: post-v1 fixes

## User outcome and behavior

An outside click clears an existing selection without beginning another selection.
Moved/pasted pixels commit first. An outside click commits a retained non-polygon
shape without starting another shape. Clicks inside its bounds preserve the preview;
resize handles remain usable. Polygon clicks continue vertex entry. Pan is unchanged.
Ctrl+A commits non-text previews, activates Selection and selects the whole canvas.
Text fields retain native Select All. Busy operations block selection commands.
No data leaves the device. Selection bounds alone do not dirty pixels.

## Implementation and acceptance

- [x] Controller and canvas tests cover consumed click/drag, workspace clicks,
  unchanged pixels, subsequent selection, and shape undo/redo.
- [x] Shell keyboard test covers Ctrl+A from Pencil and a retained shape.
- [x] Existing editing and shell tests pass.

## Validation and handoff

Build, format-check, qml-lint and repository checks passed. CTest canvas, editor,
clipboard, text, transform, shell and shell-more passed (offscreen/software).
Existing tests were updated for the deliberate dismissal/Select All behavior.
Real desktop input validation remains unverified. Protected PR review completed.
Independent follow-ups delivered in PRs #42–#45:
glass settings, fill antialias fringe handling, canvas edge handles/border, and
GitHub release notifications; each is a separate outcome and PR.

## Release closure

Merged in [PR #41](https://github.com/AfshinKI/openpaint/pull/41) and included
in [v1.1.0](0038-release-1.1.0.md). The validation limits above remain explicit;
publication does not certify additional desktop environments.
