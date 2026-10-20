# Feature: stable v1.3.0 release

Status: published and publicly verified on 2026-10-20 UTC.
Feature IDs: F25

## Outcome

Publish the maintainer-requested editor refinements as v1.3.0, including exact-color
selection, mixed-font text, spline drawing, opaque glass fallback and simpler
welcome/setup/update flows. No additional editing behavior is introduced here.

## Acceptance cases

- [x] Version, AppStream metadata and release notes identify 1.3.0.
- [x] New light/dark and changed-control images show the release UI at 40% opacity.
- [x] Website build, desktop/mobile visual review and local release checks pass.
- [x] Protected release-preparation PR passes all checks before merge.
- [x] Explicit main dispatch passes every gate and publishes the five assets.
- [x] Public asset hashes, release identity and deployed website are verified.

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

Release work is complete. Broader desktop/input/accessibility certification remains
follow-up work; do not overwrite the published tag or assets.

Release preparation validation: clean Qt 6.12 Release build and all 19 CTest suites passed (77.75s); all 27 tooling tests, formatting, zero-warning QML lint, repository and whitespace checks passed. Website desktop and 390px-wide mobile layout were reviewed; no horizontal overflow or broken loaded images. All four PNGs have the expected dimensions, transparent corners and DPI-only metadata.

## Protected preparation and deployment

Release PR #54 passed all nine jobs in run 39366258536 and merged as
`a20a261b47ce4dca82c5acd849ffe8804ca7231d`. Stable publication was explicitly
dispatched with version 1.3.0 on that exact main commit in run 39367261582.
The main-branch validation run precedes it under the workflow concurrency policy.

GitHub Pages run 39367229165 succeeded on the same commit. The public
[website](https://afshinki.github.io/openpaint/) shows v1.3.0; all four served PNG
SHA-256 hashes match the reviewed repository files. The live page and its release
download link were inspected.

## Public release verification

[Stable publishing run 39367261582](https://github.com/AfshinKI/openpaint/actions/runs/39367261582)
passed all ten jobs, including the nine validation gates and publication.
[v1.3.0](https://github.com/AfshinKI/openpaint/releases/tag/v1.3.0) was published
at 2026-10-20T04:20:03Z and is the latest stable release, targeting the exact
approved commit above. The public release page confirms the tag and commit.

All five public assets were downloaded. Their sizes and SHA-256 hashes match
GitHub's published asset digests; all four payloads also match `SHA256SUMS`.
The downloaded DEB reports version `1.3.0-1`; the extracted portable launcher
runs and reports `OpenPaint 1.3.0` with the offscreen platform. No system package
was installed and no user artwork or preferences were changed for this check.

| Asset | SHA-256 |
| --- | --- |
| Portable tar.gz | `82af14a35edf3a8685e748ec3457d1fb6f4bd53c36e67f469bc4ef7eeb4a54ad` |
| Ubuntu DEB | `34607521472190808c2c2f3709f143cb61f6d61402d05f84e1c9c478e72e8de2` |
| Flatpak | `8a2567adb51c1e3df703ced93847909c01b78160453112051a59c924c2594c27` |
| Sources and notices | `d709314a4b4c7d73b1c443f34d3d9b4d961ee25c18bd9a9cf6e6c615b2e2a082` |
| SHA256SUMS | `c172c6d09b0a1f1f04e7d087bd8c3a161321937de4053282a7bc69e0eabdda23` |
