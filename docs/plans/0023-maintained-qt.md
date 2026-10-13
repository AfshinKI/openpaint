# Feature: maintained release toolchain

Status: implemented; protected CI required before merge
Feature IDs: F25, F26
Milestone: M2

## Outcome and boundary

Pin source CI and the forthcoming AppImage to Qt 6.12.0, replacing the development
checkpoint's Qt 6.8.3. Qt 6.8 remains the API floor; its prior validation remains
historical evidence. Flatpak will use its maintained KDE runtime and record the
runtime/SDK commits separately (currently Qt 6.11.2 in the 6.11 branch).

Fix new strict QML lint findings without suppressing warnings: disambiguate IDs
and provide generated resource mappings so the linter sees the singleton module.
GNU/Clang builds map source/build prefixes out of compiled file/debug references.
Installed artifact privacy still requires a separate binary audit and stripping.
No automatic dependency downloads, runtime networking or publishing is introduced.

## Acceptance

- [x] Qt 6.12 development and sanitizer builds pass all application suites.
- [x] Strict lint/formatting pass with resource-aware module resolution.
- [x] Existing keyboard/file/clipboard/tool workflows pass isolated X11 scaling.
- [x] Compiler prefix mapping is present; repository/tooling checks pass.
- [ ] CI pins the maintained runtime and all protected jobs pass before merge.

## Evidence and handoff

Qt 6.12.0/GCC 13.3.0 on Linux x86_64: all 11 development and ASan/UBSan
suites pass. The isolated X11 shell at 1.5× passes 28 results; optional portal and
performance hooks are not enabled in that run. CI repeats X11 at 1, 1.25, 1.5 and
2× and builds Clang 18 release/sanitizer variants.

Strict QML lint and formatting pass. Qt's linter prints two informational resource
lookup diagnostics for its internal Basic controls; no warnings are suppressed.
All 73 compile commands contain source-prefix mapping. Repository checks and all
seven tooling tests pass. Clang 18 is not installed locally; its validation is CI.

Installation identity and manual Updates UI are next; this
change alone does not provide downloadable binaries or close the M2 release gate.
