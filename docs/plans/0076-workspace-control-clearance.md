# Fix: keep ribbon controls and fitted artwork visible

Status: implemented; combined-preview validation passed
Feature IDs: F01, F11
Target: v2.1.0

Minimum window width follows the ribbon’s measured content width plus its insets,
with a 1024px floor. This accommodates font/control sizing without losing Layers.
Fit reserves the floating size/opacity rail’s right edge plus the normal canvas
padding before scaling and centering artwork. Zoom and pan remain unrestricted;
changing tools does not force a refit. No pixels/history or persistence change.

Acceptance: Layers stays inside the window at its minimum width; Fit keeps the
artwork beyond the visible tool rail, and manual zoom can cross that boundary.
Combined build, formatting, QML lint and all 20 CTest suites passed. Focused
minimum-width/Fit checks also passed on isolated X11 at 125% scaling.
Next: protected branch review and release integration.
