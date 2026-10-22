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
bundle dependencies and record source/inventory/checksums. Signing choice is
pending maintainer response; never claim ad-hoc signing is Developer ID/notarization.

## Acceptance cases

- [ ] Existing Linux application and package gates pass.
- [ ] macOS builds, automated editor tests and Cocoa input/render tests pass.
- [ ] macOS bundle has no non-system external runtime dependencies; fresh-copy
  launch/edit/save/reopen and blur behavior are validated with stated limits.
- [ ] Platform-specific glass instructions never suggest GNOME commands on Mac.
- [ ] Version 2.0.0 source, notes, checksums, artifacts and website are consistent.
- [ ] Fresh release-build screenshots use generated artwork/backdrop, 40% glass,
  transparent rounded corners and recorded provenance.
- [ ] Protected PR merge, release publication, public downloads and Pages verified.

## Validation and progress

Configured Mac host is currently unreachable. GitHub hosted macOS runner will
supply automated build/test evidence; physical Mac visual validation is pending.
Developer ID signing availability is pending maintainer response.
Linux packaging currently fails on an inherited systemd library/source version
mismatch. Upgrade the container package set before collecting exact sources;
do not weaken provenance checks. The platform bridge and package lane are implemented. Hosted macOS compilation
now passes after using installed libarchive headers with the Apple system library.
Next: finish Mac runtime/package validation, protected merge and release promotion.

Local evidence: Release build and formatting/QML lint pass; 39 Python tooling tests
and repository/website checks pass. Initial full CTest passed 19/20 groups and
caught an offscreen Quit-key regression; explicit Ctrl+Q restores the tested
keyboard save/discard/quit workflow. The affected group now passes (17 cases); all 20 local Release groups have passing evidence. Fresh
1449×1008 light/dark/color/resize captures show the real four-layer artwork at
40% glass opacity. Desktop and 390px website layouts were inspected in-browser.

First hosted Mac runtime evidence: document, layer/OpenRaster, image I/O, clipboard,
text, canvas, updater and app-info tests passed. The native glass test revealed an
AppKit frame-hierarchy warning and a Reduce Transparency assumption; use an owned
content container and assert availability against the actual accessibility state.
Several offscreen UI groups timed out. Move Mac UI validation to Cocoa/Metal,
keep the shared menu strip and make all controls Tab-reachable. Do not waive the
tests. Rerun the complete lane before package promotion.

Packaging also launches a fresh copy of the deployed app via Cocoa/Metal and
rejects early exit or known runtime/import failures. This supplements, rather
than replaces, the editor save/reopen tests and physical visual-review limit.

After the menu/focus changes, a fresh complete Linux Release run passes all 20
CTest groups (87.62 seconds). The Mac test now uses an Objective-C pointer
comparison compatible with ARC; the application bridge itself compiled cleanly.

Cocoa executes early UI cases successfully but exceeds the old 30-second grouped
process budget before later cases run. The Mac lane now registers every existing
UI behavior separately, each retaining a 30-second limit and all assertions.
Glass attachment is deferred until after Qt exposes its native window to avoid
re-entering its initial visibility transition and replacing the new container.
