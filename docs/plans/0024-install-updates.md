# Feature: installation identity and manual Updates

Status: implemented; protected CI/merge pending
Feature IDs: F25, F26
Milestone: M2

## Outcome

Help → Updates shows the compiled version, release channel and installation kind.
A View releases button explicitly opens the fixed public GitHub Releases page in
the user's browser. The screen explains source, AppImage, Flatpak test-bundle or
system-package replacement. It never claims to be up to date, checks automatically,
installs updates or restarts. It preserves unsaved documents. Browser failure has a
visible fallback URL and explanation; the editor stays usable offline.

Install rules provide the executable, original SVG icon, desktop entry, AppStream
metadata and license. The desktop entry opens a single local image argument in a
new process/window; command-line parsing rejects extra arguments. This slice adds
no downloadable package or automatic publishing.

## Implementation

CMake validates trusted version suffix/install-kind inputs and generates immutable
AppInfo metadata. The QML singleton has one explicit external action using
QDesktopServices; no HTTP client or writable-path inference is involved. Normal
source builds identify as source; packaging recipes must set their own kind.
Developer lint tools are only required with BUILD_TESTING enabled.

## Acceptance

- [x] Default and alternate package identities/version channels are correct.
- [x] Only explicit View releases invokes the fixed URL; a handler test avoids networking.
- [x] Keyboard access, close and browser handoff preserve unsaved drawing state.
- [x] One local command-line image opens; extra arguments fail without launching.
- [x] Staged install, desktop/AppStream validation, both themes and CLI version pass.
- [x] Application/sanitizer, QML/format and repository checks pass.

## Evidence and next step

Qt 6.12.0 / GCC 13: all 12 development suites pass (13.27 s), all 12
ASan/UBSan suites pass (15.43 s), 66-file formatting and strict QML lint pass.
Repository checks pass (196 files), with all seven tooling tests. Default source,
AppImage/Flatpak prerelease and system/stable CMake identities were checked; invalid
kind and suffix are rejected independently. Staged install and desktop/AppStream
validation pass (AppStream reports two nonblocking pedantic suggestions).

An isolated X11 launch with a generated 47 × 31 PNG showed its filename in the
window title; two image arguments exit 2. The Updates keyboard workflow passes
offscreen and isolated X11 at 1.5×, retains dirty artwork and intercepts the exact
release URL without networking. Both theme captures were visually inspected.

AppImage and Flatpak recipes follow as separate bounded PRs;
then clean-VM artifact testing and publication close M2.
