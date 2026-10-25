# Feature: macOS port and v2 release

Status: v2.0.0 published; remaining manual validation listed below
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

- [x] Existing Linux application and package gates pass.
- [x] macOS builds, automated editor tests and Cocoa input/render tests pass.
- [x] macOS bundle has no non-system external runtime dependencies and launches
  from a fresh copy; native editor save/reopen and blur checks pass with stated limits.
- [x] Platform-specific glass instructions never suggest GNOME commands on Mac.
- [ ] Version 2.0.0 source, notes, checksums, artifacts and website are consistent.
- [x] Fresh release-build screenshots use generated artwork/backdrop, 40% glass,
  transparent rounded corners and recorded provenance.
- [ ] Protected PR merge, release publication, public downloads and Pages verified.

## Validation and progress

Local Release build, formatting and zero-warning QML lint pass. All 20 CTest
suites passed (87.62 seconds), as did 42 Python tooling tests and repository/site
checks. The split keyboard accessibility cases also passed on isolated X11/OpenGL
(4 QtTest cases, 4.67 seconds), retaining every original assertion.

Hosted run [261](https://github.com/AfshinKI/openpaint/actions/runs/39711611154)
passes all 75 Mac tests and every Linux gate: GCC development, Clang Release,
ASan, X11 scaling, Flatpak, portable/Ubuntu packaging and Ubuntu
install/replace/remove checks. The native suite covers glass attachment,
Reduce Transparency, window title/resize lifecycle, layers/OpenRaster,
keyboard accessibility, rendered menu blur, editing and save-failure protection.

That run also passes bundle signing, the Cocoa version check, the complete
runtime/private-path audit and fresh-copy Cocoa/Metal startup. It creates the
DMG successfully. The remaining failure was an uncreated source-download cache;
the source collector now creates its directories. A generated local source archive
test verifies first-run cache creation, source digests and retained license notices.
Final required checks must pass on the reviewed commit before promotion.

The macOS bridge preserves Qt's content view and installs its non-interactive
glass sibling after window exposure. Each Mac UI behavior runs separately on
Cocoa/Metal with its assertions and 30-second deadline. The keyboard harness
avoids a duplicate native-window wait for each Tab. Tool and control accessibility
are separate cases to keep execution comfortably within the deadline.

Packaging selects GUI/image/network plugins plus self-contained SQLite, avoiding
unused external SQL clients. It uses Cocoa for the deployed app, handles repeated
universal-binary architecture headings in `otool`, and retains strict external
runtime/private-path rejection. Fresh-copy startup uses isolated preferences.
Packaging may diagnose independent failures after a failed test, but any failed
test still fails the job and blocks publication.

Fresh 1449×1008 light/dark/color/resize captures show the four-layer generated
artwork at 40% surface opacity. Source and rendering provenance are recorded in
[the asset notes](../assets/README.md) and `website/release.json`. Desktop and
390px website layouts were inspected in-browser. The pictures are actual app UI
with a staged backdrop, not physical Mac/compositor certification.

The configured physical Mac is unreachable. Hosted native automation supplies
build/input/render evidence; physical Mac visual review and Gatekeeper/notarized
installation remain unverified. Notes and downloads state Apple silicon,
macOS 14.4+ and ad-hoc signing. Intel packaging is deferred.

Next: complete source/notices packaging and all required checks, merge
[PR #56](https://github.com/AfshinKI/openpaint/pull/56) through protection, then
dispatch the explicit v2.0.0 release run. Record promotion, public-download and
Pages verification on the linked PR, which remains the live release record after
the immutable release source is tagged. No release is published at this commit.

## Publication reconciliation — 2026-10-25

The pre-publication handoff above is historical. GitHub records
[PR #56](https://github.com/AfshinKI/openpaint/pull/56) merged on 2026-10-22 at
02:57:16 UTC as `71c4e80fbc895c5699375e6fbdb4be96f3a8014f`; the `v2.0.0` tag
resolves to that commit. [v2.0.0](https://github.com/AfshinKI/openpaint/releases/tag/v2.0.0)
was published at 03:15:19 UTC. The changelog's October 21 date is the local release
date; this record uses UTC.

The public release lists the Linux portable archive, Ubuntu 24.04 amd64 DEB,
x86_64 Flatpak, Apple silicon DMG, sources-and-notices archive and SHA256SUMS.
[Release workflow](https://github.com/AfshinKI/openpaint/actions/runs/39714535405)
and [main checks](https://github.com/AfshinKI/openpaint/actions/runs/39714184866)
completed successfully. [Website deployment](https://github.com/AfshinKI/openpaint/actions/runs/39714184902)
also completed successfully. This reconciliation checked public API metadata and
local tag identity; it did not download/re-hash the binaries or visually inspect
the deployed page, and does not promote those acceptance cases to verified.

Next unfinished steps: independently verify public artifact checksums and the
deployed page, and record physical Mac visual/first-launch evidence when available.
Cross-editor OpenRaster round-trips, crash recovery, tiling, Intel packages and
Apple notarization remain outside the shipped scope. Earlier source-cache and
PR-review handoffs are superseded by the successful release run above.
