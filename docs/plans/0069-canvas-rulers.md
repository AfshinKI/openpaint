# Feature: top and left pixel rulers

Status: acceptance defined; native validation deferred
Feature IDs: F11
Target: v2.1.0

Add optional top/left rulers in image pixels, enabled by default for the session.
View → Rulers (Ctrl+R, Command+R on macOS) toggles both. Adaptive tick spacing and
labels follow the canvas origin, zoom and dimensions, with pointer markers only
while the pointer is over the image. Keep the Layers rail and controls reachable
at minimum size. Rulers do not intercept drawing, enter history, or appear in
saved images. Pan/zoom and toggling preserve artwork and dirty state.

Smart alignment/snapping is a separate follow-up: temporary edge/center guides
for editable content with an optional snap toggle and modifier bypass. This slice
provides rulers/pointer guides only; do not advertise alignment snapping yet.

Acceptance: coordinates remain synchronized at fit, 1%, 3200%, panned origins,
fractional scale and canvas resize; ticks are bounded to the visible image range;
labels remain readable in both themes; toggle/keyboard and minimum-size layout
work without mutating pixels/history. Native build/UI checks deferred until the
requested final validation pass.
