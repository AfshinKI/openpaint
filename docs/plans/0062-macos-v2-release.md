# Feature: macOS port and v2 release

Status: implementing
Feature IDs: F25; platform support
Target: 2.0.0

## User outcome and scope

Ship v2 with layers, the current UI refinements, Linux packages and a macOS
download. Update CI runners, release promotion, website/README and versioned
release-build screenshots showing 40% glass in both themes. Continue the existing
maintainer-requested branch through protected review, then release from main.

## Behavior and implementation

Keep document/command code shared. Add a Cocoa NSVisualEffectView behind the Qt
content on macOS, controlled by the existing appearance preference, with no shell
setup or extension. Use platform shortcuts, an app bundle and explicit install
metadata. Respect Reduce Transparency. Build a native arm64 package on macOS 15,
bundle dependencies and record source/inventory/checksums. The first package uses ad-hoc signing; it is not Developer ID signed or notarized.

## Acceptance cases

- [ ] Existing Linux application and package gates pass.
- [x] macOS builds, automated editor tests and Cocoa input/render tests pass.
- [ ] macOS bundle has no non-system external runtime dependencies; fresh-copy
  launch/edit/save/reopen and blur behavior are validated with stated limits.
- [x] Platform-specific glass instructions never suggest GNOME commands on Mac.
- [ ] Version 2.0.0 source, notes, checksums, artifacts and website are consistent.
- [x] Fresh release-build screenshots use generated artwork/backdrop, 40% glass,
  transparent rounded corners and recorded provenance.
- [ ] Protected PR merge, release publication, public downloads and Pages verified.

## Validation and progress

Local Release build, formatting and zero-warning QML lint pass. All 20 CTest
suites passed (87.62 seconds), as did 40 Python tooling tests and repository/site
checks. The affected blur and keyboard tests also passed on isolated X11/OpenGL
(6 QtTest cases, 19.91 seconds) after strengthening the stripe fixture.

Fresh 1449×1008 light/dark/color/resize captures show the four-layer generated
artwork at 40% surface opacity. Source and rendering provenance are recorded in
[the asset notes](../assets/README.md) and `website/release.json`. Desktop and
390px website layouts were inspected in-browser. The pictures are real app UI
with a staged backdrop, not physical Mac/compositor certification.

Hosted run [259](https://github.com/AfshinKI/openpaint/actions/runs/39708901934)
built the native Apple silicon application and passed all 74 Mac checks,
including native glass attachment, accessibility-state handling, window title and
resize lifecycle, layers/OpenRaster, keyboard accessibility, rendered menu blur,
editor operations and save-failure protection. An opaque stripe fixture keeps
the contrast baseline valid beneath the menu tint; its blur threshold is unchanged.
Removing a duplicate native-window wait per Tab event resolves the two keyboard
workflow timeouts without removing keypresses/assertions or extending deadlines.

Run [258](https://github.com/AfshinKI/openpaint/actions/runs/39707687307) passed
all Linux builds, ASan, X11 scaling, Flatpak, portable/Ubuntu packaging and Ubuntu
install/replace/remove checks. Final required checks must pass on the reviewed
commit. Packaging runs after a successful build even if tests fail, to expose
independent problems sooner; failed tests still prevent release promotion.

The macOS bridge preserves Qt's content view and installs its non-interactive
glass sibling only after window exposure. Every Mac UI behavior runs separately
on Cocoa/Metal, retaining all assertions and a 30-second limit. Packaging audits
runtime dependencies and launches a fresh copy of the deployed bundle; this
supplements editor save/reopen tests rather than replacing desktop acceptance.

The first DMG packaging run exposed two deployment issues after all editor tests
passed: the bundle intentionally contains Cocoa rather than the offscreen test
plugin, and Qt's default deployment included unused external SQL clients. The
recipe now selects GUI/image/network plugins plus self-contained SQLite, deploys
their frameworks with macdeployqt, and checks the actual Cocoa bundle. A tooling
test verifies that external SQL drivers/debug/offscreen plugins are excluded
without modifying the Qt SDK. Fresh-copy startup uses isolated preferences.

The configured physical Mac is unreachable. Hosted native automation supplies
build/input/render evidence; physical Mac visual review and Gatekeeper/notarized
installation remain unverified. The release notes and download page state the
Apple silicon/macOS 14.4+ and ad-hoc-signing limits. Intel packaging is deferred.

Next: validate the DMG and every required Linux gate, merge PR #56 through
protection, then dispatch the explicit v2.0.0
release run and verify public assets and Pages. No release is published yet.
