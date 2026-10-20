# Feature: stable v1.3.0 release

Status: preparation; protected release PR and publication pending.
Feature IDs: F25

## Outcome

Publish the maintainer-requested editor refinements as v1.3.0, including exact-color
selection, mixed-font text, spline drawing, opaque glass fallback and simpler
welcome/setup/update flows. No additional editing behavior is introduced here.

## Acceptance cases

- [x] Version, AppStream metadata and release notes identify 1.3.0.
- [x] New light/dark and changed-control images show the release UI at 40% opacity.
- [x] Website build, desktop/mobile visual review and local release checks pass.
- [ ] Protected release-preparation PR passes all checks before merge.
- [ ] Explicit main dispatch passes every gate and publishes the five assets.
- [ ] Public asset hashes, release identity and deployed website are verified.

## Evidence and limitations

Editor PR #52 merged as `d9d27cb9e4121240d181a0bd9dfa72a7280a94d3`
after all nine checks passed in run 39365124692. Runtime prerequisite PR #53
merged as `aae20760a61285a77ac19410f2d9add2b0ec96b6` after nine passing
checks in run 39362535496. The runtime binary and matching source were verified;
checksum enforcement remains mandatory.

Local editor validation: 19 development suites passed (79.38s), 19 sanitizer
suites passed (87.78s), and all 61 native UI cases passed at each scale 1, 1.25,
1.5 and 2. All 55 scheduled UI workflows are retained across smaller 30-second
groups, avoiding sanitizer group timeouts without dropping tests. A fractional-DPR
caret sampling error was reproduced and corrected without changing its black and
blinking assertions. The earlier stale Qt 6.8 build cache was replaced with a
clean Qt 6.12 build; only pinned-toolchain results are release evidence.

Qt 6.12.0 remains the latest entry in the [official 6.12 download directory](https://download.qt.io/official_releases/qt/6.12/), checked 2026-10-20.
The listed October patches target Qt5Compat, which this app neither imports nor
requests in its SDK modules. The [Qt security feed](https://www.qt.io/blog/tag/security/page/0)
was reviewed; this is not a general claim of vulnerability-free dependencies.

Fresh screenshots use Release source `158fc1bf08e9a7dd5b9c1382d3f174a23785ef0f` with Qt 6.12.0
on X11/OpenGL. They are isolated real-UI harness captures with a staged original
backdrop, not installed-package or compositor-blur certification. All four were
visually reviewed; provenance is in [assets](../assets/README.md).

Read-only real desktop probing returned glass ready on the configured Ubuntu
24.04 / GNOME 46 / X11 system. Native glass, color dialog, color selection and text
workflows passed. No desktop extension or user setting was changed. Clean extension
installation/recheck is not newly certified. Unknown environments use opaque
fallback. Prior desktop package install/draw/save/reopen evidence remains in
[the stable-release plan](0031-stable-release.md); current CI repeats clean package
install/upgrade/remove, and text tests compare rendered preview, saved PNG and undo.
Broader KDE/Wayland, IME-engine, stylus and accessibility certification remains open.

## Next step

Validate and merge this preparation, dispatch 1.3.0 on main, then verify the
published downloads and GitHub Pages deployment. Do not overwrite a published tag.

Release preparation validation: clean Qt 6.12 Release build and all 19 CTest suites passed (77.75s); all 27 tooling tests, formatting, zero-warning QML lint, repository and whitespace checks passed. Website desktop and 390px-wide mobile layout were reviewed; no horizontal overflow or broken loaded images. All four PNGs have the expected dimensions, transparent corners and DPI-only metadata.
