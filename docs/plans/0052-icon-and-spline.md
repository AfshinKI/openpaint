# Feature batch: eraser states, themed palette icon and spline

Status: implemented; local checks passed; PR/release deferred
Feature IDs: F01, F02, F06

## Acceptance sequence

1. Paper-white eraser has a filled patch; transparent eraser stays hollow. Both
   dropdown choices show their corresponding eraser icon. Cursor retains its +.
2. Install the selected original Open palette concept. Running window and Updates
   icon follow Light/Dark/System immediately; desktop launchers keep the installed
   light fallback where the shell does not honor window icons. No theme-file rewriting.
3. Add a separate open multi-point Spline beside the existing two-bend Curve.
   Click/drag the first segment and add points; Enter applies, Backspace removes the
   last point, Escape cancels. Smooth interpolation passes through chosen points;
   previews, undo/redo, selection clipping and safe PNG persistence stay intact.

## Implementation and evidence

Original SVG resources; appearance uses Qt style hints for system theme. Native
icon updates stay on the GUI thread. Spline paths are rendered in the existing C++
shape pipeline and committed by the existing preview command, never QML pixels.
No external dependencies or network behavior.

## Validation

- Dev build, formatting and strict QML lint passed.
- Repository checks passed (307 files), and all 27 tooling tests passed.
- All 15 CTest suites passed in 24.56 seconds.
- Eraser UI checks cover menu glyphs, button state and distinct native cursor images.
- Theme checks compare actual light/dark application and window icon pixels, and
  inject Qt system-theme notifications to verify automatic updates. The headless
  backend cannot change a real OS theme; packaged desktop testing remains separate.
- Spline checks cover click-only and drag entry, interpolation through points,
  Backspace, cancellation, preview commit, undo/redo, selection clipping and PNG
  save/reopen. Keyboard navigation reaches the twentieth shape in the ribbon.
- A 2× hidden-window OpenGL run passed all 7 cases including init/cleanup.
  Synthetic spline, eraser-menu and light/dark Updates captures were reviewed in
  build/evidence-icon-spline and build/evidence-icon-spline-gl.

Next: maintainer local review, then scoped PR/CI and v2 release planning. No release
version was bumped and no PR or release dispatch occurred.

PR/release remain deferred; installed desktop/Wayland behavior needs package QA.
