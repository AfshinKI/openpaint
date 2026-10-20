# Tool placement and floating menu readability

Status: implemented; local checks passed; PR/release deferred

## Acceptance

- Color picker occupies the former eraser position; the eraser and its mode dropdown
  move together to the former picker position. Both remain keyboard accessible.
- Internal floating menus retain neutral tint of at least 72% in both themes at
  default glass opacity. Light menu tint increases smoothly to opaque at 100%.
- Preserve the global 40% default, frosted selection appearance and actual backdrop
  blur. No tint preference or document mutation is introduced.

## Validation

- Dev build, formatting and strict QML lint passed.
- All 15 CTest suites passed (24.67 seconds); all 27 tooling tests passed.
- Repository checks and diff whitespace checks passed.
- Tool-position assertions and the complete eraser keyboard/menu workflow passed.
- Desktop OpenGL menu checks passed in both themes, including the existing pixel
  blur comparison; the light-menu capture was visually reviewed in
  build/evidence-menu-tint-desktop.
- Offscreen OpenGL produced incomplete black frames and failed the pixel checks;
  it is not accepted as rendering evidence. Desktop OpenGL passed unchanged checks.
- Shared menu material keeps tint at least 72%; the global preference stays 40%.

Next: push this bounded follow-up to the existing review branch; maintainer local
review precedes PR/CI and v2 release work.
