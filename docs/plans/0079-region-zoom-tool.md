# Feature: region zoom tool

Status: implemented; combined-preview validation passed
Feature IDs: F11
Target: v2.1.0

## Behavior

An original magnifier icon in Tools activates Zoom to region. Drag a rectangle
inside the artwork to fit that region into the viewport, using the existing
1–3200% zoom bounds and 24-unit padding. Reverse drags work; outside portions
are clipped to the image. Drags smaller than 4 screen pixels do nothing. Escape,
focus loss and switching tools discard the rubber band. Right/middle/Space pan
continues to work. Slider and percentage stay synchronized. No saved pixels,
history entries or network operations are produced by region zoom.

## Implementation

Editor exposes the tool choice; CanvasItem owns the transient rectangle and
viewport transformation. QML only selects the tool. The combined preview also
respects the separately implemented floating-control inset.

## Acceptance and validation

Combined preview: build, format-check, zero-warning QML lint and repository
checks passed. Full canvas/editor/document suites passed. regionZoomToolWorkflow,
fillModeMenu, fitAndMinimumWidthClearControls and coloredRibbonWorkflow passed
(6 Qt cases with harness). regionZoomIsViewOnly covers reverse drag, Escape,
focus-loss cancellation, click/no-op and unchanged pixels/history.
Next: review the scoped branch and its CI before merging.
