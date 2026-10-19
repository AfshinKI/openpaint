# Feature: frosted window and layered surfaces

Status: merged in PR #47; shipped in v1.2.0
Feature IDs: F01

## Outcome and behavior

Provide a translucent desktop window and frosted app surfaces using the existing
persistent glass toggle and 40–100% opacity setting. A continuous base fills the
entire client area, including toolbar margins. Text, icons, paper and the image
transparency checkerboard stay opaque. No saved pixel, history, network or file
format changes are introduced.

Floating controls blur explicitly ordered lower app layers. Menus and dialogs
include fixed bars and floating controls, flattening their blur source to prevent
sharp originals leaking through alpha. Menus use less white tint in light mode;
shape-gallery and footer control wells become transparent when glass is enabled.
Hover, pressed, selected and keyboard-focus feedback remain visible.

The initial client size is 1449 × 1008, matching the maintainer-approved window.
Minimum size remains 1024 × 640. Appearance settings retain the existing defaults
(enabled, 85%); disabled glass and 100% restore opaque backgrounds.

## Implementation and limits

Qt Quick alpha buffer plus Qt Quick Effects MultiEffect and bounded,
half-resolution ShaderEffectSource captures. The dependency graph excludes each
surface's own foreground, preventing recursive feedback. See [ADR 0005](../decisions/0005-local-panel-blur.md).

Desktop blur is external: verified locally with GNOME 46/X11 and Blur My Shell
v75. OpenPaint never installs the extension, captures the desktop or changes its
settings. Without compositor blur, transparent areas remain unblurred. Native
window decoration is desktop-controlled. Per-bar desktop blur, optical refraction,
general recursive popup stacking and a cross-desktop capability detector are not
implemented. Software rendering uses solid menus/dialogs and tinted other surfaces.

## Acceptance and evidence

- [x] Continuous base covers toolbar gaps; off/100% are opaque.
- [x] Actual Brush menu blurs translucent toolbar content in light and dark themes.
- [x] Actual size and Fill panels blur generated canvas artwork on the GPU.
- [x] Foreground controls, saved artwork, dirty state and undo history remain unchanged.
- [x] Shape-gallery and footer keyboard/accessibility checks pass.
- [x] Maintainer reviewed the local appearance and requested publication.
- [x] Final full validation after startup-size and README capture changes.
- [x] PR checks and packaged validation before release (run `39156488388`).

Final validation passed with Qt 6.12: all 15 CTest suites, 26 tooling tests,
format/QML lint, repository checks, and seven focused X11/OpenGL test results.
The README capture was visually inspected at 1449 × 1008.

Earlier targeted validation passed with Qt 6.12: dev builds, all three shell
suites, format/QML lint, repository checks, plus X11/OpenGL visual tests on an
NVIDIA RTX 3070. GPU offscreen runs were inconsistent, so direct X11/OpenGL tests
were used for visual evidence. No broad compositor/Wayland support is claimed.

## Documentation image

The README image is rendered by `readmeFrostPreview` using project-authored
procedural artwork and a generated multicolor window backing. It captures only the test window;
no private desktop, user drawing, external asset or personal metadata is included.
See [asset provenance](../assets/README.md) for the regeneration command and license.

## Next step

PR #47 merged after all checks passed and shipped in
[stable v1.2.0](0040-release-1.2.0.md). Release closure is recorded there;
broader compositor/Wayland certification remains future work.

## Review follow-up

Theme choice now persists alongside glass settings. Updates text uses explicit theme
colors, dark buttons are brighter, and dark popup tint has a readability floor over
white artwork. Text uses a blinking caret at the displayed font height. Fill always
uses zero tolerance and smooth edges, with no options panel. The README preview
opens the Brush menu over a generated multicolor backing.

Acceptance: restart persistence, caret blinking, exact smooth fill with undo,
minimum-size dialog layout, and the revised screenshot are covered by targeted
tests and visual inspection. Final suite results are recorded with the PR.
