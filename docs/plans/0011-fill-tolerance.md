# Feature: contiguous fill and tolerance

Status: implemented
Feature IDs: F04
Milestone: M2

## Outcome and behavior

Fill a four-connected region from a left/right press with primary/secondary color.
Tolerance is 0–255 (default 0), the maximum absolute difference in any premultiplied
sRGB RGBA byte from the seed. Transparent hidden RGB is therefore ignored. Fill
replaces matching pixels, including alpha; it does not blend repeatedly. Matching
always uses the original snapshot, never the growing output. Disconnected islands
remain untouched. No selection exists yet; selection clipping joins this contract
when selections land. No merged-layer sampling or network access.

One fill is one undo command. Dragging does not repeat fills. Busy fill blocks
editing and file actions, while navigation remains available. A visible busy
indicator and Cancel action are available immediately; Escape cancels too. Cancel,
failure, stale revision and history-budget rejection leave artwork unchanged.
Changing tools cancels a pending fill. No-op fills preserve history and dirty state.

## Implementation

A bounded worker consumes an immutable image and returns a candidate plus its
changed rectangle. The GUI controller checks cancellation and source revision
before applying one document patch. Only one background operation runs at once.
Document patch application rejects invalid rectangles, format/size mismatches,
active strokes and stale revisions. Maximum input remains 32 MP; temporary image,
visited bytes and integer work queue are bounded by that limit. History is 256 MiB.
No new dependency. Existing file-operation cancellation also cancels fill.

## Acceptance

- [x] Exact/tolerant/maximum fills respect four-connectivity and alpha.
- [x] No-op, invalid seed/tolerance and cancellation preserve input.
- [x] Patch undo/redo, saved revision, stale result and budget rejection are tested.
- [x] Controller cancellation, tool switch and save/reopen preserve pixels.
- [x] Keyboard tolerance, toolbar layout and busy cancellation are checked.
- [x] Development, sanitizer, formatting, QML and privacy checks pass.

## Evidence and handoff

Qt 6.8.3 on Linux x86_64: all seven dev and ASan/UBSan/LeakSanitizer suites
passed. Isolated X11 shell: 18 passing results, including tolerance keyboard
activation. Optional portal/performance routines were inactive. Formatting, QML
lint and repository checks passed. Controller tests cover cancel before delivery,
tool-switch cancellation, undo/redo and saved PNG equivalence. Document tests cover
exact/tolerant/full fills, diagonal islands, alpha, no-op, invalid input, stale
revision and undo-budget rejection. Light/dark minimum-window captures reviewed:
[light](../assets/fill-light-1024.png), [dark](../assets/fill-dark-1024.png).

Next M2 feature after protected merge: brush styles and opacity. Selection-mask
integration remains part of the selection slice. Full Orca/device certification
remains a later gate.
