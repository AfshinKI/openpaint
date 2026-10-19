# Feature: adjustable glass appearance

Status: merged in PR #44; shipped in v1.1.0
Feature IDs: F01
Milestone: post-v1 appearance

## Behavior and scope

Default-on in-app glass surfaces use translucent tint, a soft vertical highlight,
and fine borders in both themes. Settings → Glass appearance offers a toggle and
40–100% surface opacity (default 85%). Disabled restores opaque flat surfaces.
Only surface backgrounds change opacity: text, controls, canvas pixels and its
checkerboard remain fully independent. No desktop transparency, blur shader,
compositor integration or pixel/document mutation is introduced.

Appearance preferences persist locally in XDG configuration. No network activity.
The setting applies immediately, Close keeps it, and Restore defaults resets it.
Software rendering uses the same rectangles and gradients as accelerated rendering.

## Implementation

A C++ Appearance singleton validates and persists preferences. A shared QML
GlassSurface styles menus, dialogs, ribbon, menu/status bars and floating panels.
Existing native window decorations and light/dark selection remain.

## Acceptance cases

- [x] Toggle/opacity/defaults work with keyboard and persist across instances.
- [x] Disabled and 100% surfaces are opaque; children remain fully opaque.
- [x] Artwork/dirty/history remain unchanged when settings change.
- [x] Light/dark, 1024×640 and 1280×800 captures inspected; controls fit.
- [x] Build, formatting, QML lint and affected tests pass.

## Validation and handoff

Dev build, repository checks, format-check and qml-lint passed. App-info, shell
and shell-more passed offscreen/software; final glassAppearanceSettings capture
run passed. Inspected light/dark 1024px settings and 1280px editor screenshots.
Real-desktop/fractional-scaling certification remains outstanding. Protected PR review completed. Independent of editing fixes.

## Release closure

Merged in [PR #44](https://github.com/AfshinKI/openpaint/pull/44) and included
in [v1.1.0](0038-release-1.1.0.md). The validation limits above remain explicit;
publication does not certify additional desktop environments.

The behavior above records the original v1.1.0 implementation.
[Plan 0039](0039-window-transparency.md) supersedes its appearance/fill controls
for v1.2.0; smooth fill is now automatic and window surfaces support transparency.
