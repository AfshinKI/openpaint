# Delivery plan

The current stable release is [v2.0.0](plans/0062-macos-v2-release.md), with bounded
raster layers, OpenRaster projects and Linux/Apple silicon macOS packages.
M1 and M2 are complete for their validated gates. M3 is partially delivered:
recovery, tiling and cross-application interoperability remain open. See the
[build guide](BUILD.md) to run from source.
Milestones are exit gates, not promised dates. Complete thin, usable slices before
adding breadth. [FEATURES.md](FEATURES.md) is the implementation status ledger.

| Gate | Result | Required acceptance evidence |
| --- | --- | --- |
| M0 — foundation | Reviewed docs, license, shared agent rules, templates, working docs/privacy checks | Local checks pass; repository settings verified after publication |
| M1 — first working canvas (complete) | Native window; pencil/eraser; primary/secondary colors; PNG open/save; undo/redo; zoom/pan; dirty-close prompt | Draw → undo → redo → save → reopen preserves pixels; failed save preserves original; clean Linux build; keyboard shell; performance baseline |
| M2 — useful alpha (complete) | Brush options, fill, shapes/text, selections/clipboard, crop/resize/rotate, common image formats; first downloadable alpha | Workflow tests; transparency/lossy export prompts; clean-VM AppImage and test Flatpak install; Updates screen with manual release link |
| M3 — layered beta | Layer operations, .ora projects, tiled storage as needed, bounded history, crash recovery | Cross-tool .ora round-trip; crash/recovery and disk-full tests; large-document budget; full project save fidelity |
| M4 — desktop completeness | Freeform rotation, rulers/grid, stylus/touch, printing, accessibility, adaptive UI and translation infrastructure | GNOME/KDE Wayland and X11 matrix; Orca and keyboard tests; print preview; scale/theme screenshots; input-device tests |
| M5 — future signed updates | Signed portable updates, package-managed update UX, trustworthy downloads, release/support process | Upgrade + failed-upgrade matrix; all core feature gates; licenses/SBOM/source artifacts; no open data-loss/security blockers |
| M6 — optional intelligent tools | Background removal first, object selection, then generative workflows | Separate plans and model-license review; offline/consent tests; memory/latency measurements; cancel/undo and model-deletion checks |

M1 is specified in [the foundation plan](plans/0001-foundation.md). M2 depends on
M1; M3 and release packaging improvements can progress independently once their
interfaces stabilize. M4 depends on editing and document contracts from M2/M3.
These future gates are not prerequisites for the scoped single-layer v1. M6 may be researched earlier, but must not delay core reliability.

## Current handoff toward v2.1.0

[PR #56](https://github.com/AfshinKI/openpaint/pull/56) merged and v2.0.0 was
published on 2026-10-22 UTC. [Plan 0062](plans/0062-macos-v2-release.md) records
publication evidence separately from physical Mac, public-download and deployed
website verification. The release label does not close those unverified gates.
The requested v2.1.0 editing and UI slices are now integrated for release validation.
[Plan 0080](plans/0080-release-2.1.0.md) tracks protected merges, exact package
evidence, fresh screenshots, publication and cleanup. Publication is still gated.

## Release progression

| Release | Delivered changes and decision trail |
| --- | --- |
| Alpha / v1.0 | M1/M2 editor and Linux packages; [alpha polish](plans/0028-alpha-ui-polish.md) replaced shared saved palettes, Swap and earlier layout targets; [stable scope](plans/0031-stable-release.md). |
| v1.1 | Canvas edges, selection dismissal, fill fringe cleanup, glass and release notifications ([plans 0033–0038](plans/0038-release-1.1.0.md)). |
| v1.2 | Frosted window surfaces, persistent theme, blinking text caret and exact-match fill with automatic smoothing ([release plan](plans/0040-release-1.2.0.md)); the original adjustable fill tolerance was superseded. |
| v1.3 | Searchable fonts, text range formatting, color selection, spline, eraser modes/cursors, 40% glass, menu readability and onboarding ([release plan](plans/0057-release-1.3.0.md), [plans 0042–0056](plans/0054-blur-onboarding-design.md)). Updates became automatic on startup/open with cooldown; the old opt-out/check controls were removed. |
| v2 | [Layers and rail revisions](plans/0058-layers.md), [inversion and final icon choices](plans/0059-invert-colors.md), [zoom and final live-blur setup](plans/0060-zoom-and-blur-setup.md), [right-drag pan](plans/0061-right-drag-pan.md), [macOS and release](plans/0062-macos-v2-release.md). Right-button canvas painting/picking was replaced by panning. Live blur with full redraw superseded the intermediate wallpaper recipe. |

Plans retain dated implementation evidence and intermediate experiments. Their
latest revision and release-status note govern current behavior; earlier “no
merge/release” statements describe that checkpoint, not today's release status.
M3–M6 remain later gates; neither v1 nor v2 implies their completion.

Keep alpha/stable channels distinct. Nightly artifacts, if added, are development
outputs with no automatic stable-update eligibility. Publish an actual support
matrix based on installed-artifact tests, not only the distribution used to compile.

## First implementation sequence

1. Establish CMake presets and Qt dependency versions; prove a native window in CI
   and a real Wayland/X11 session.
2. Implement a headless document model and reversible stroke commands.
3. Connect a canvas view with precise input/zoom mapping and undo.
4. Implement PNG open/save and data-loss prevention.
5. Measure, document, and close M1 acceptance cases before adding M2 tools.

## M2 implementation sequence

Each item is a bounded outcome delivered through a fresh branch and checked PR.
Split larger items into independently usable slices with acceptance plans before
implementation; do not accumulate all M2 work on one branch.

1. Canvas eyedropper ([plan 0010](plans/0010-canvas-eyedropper.md)).
2. Contiguous fill with tolerance, worker cancellation and undoable patches.
3. Brush styles and stroke opacity, with deterministic saved raster output.
4. Lines/rectangles/ellipses, then the remaining scoped shape palette; previews,
   outline/fill options and commit/cancel behavior.
5. Raster text with font/style/alignment/background controls and explicit commit.
6. Rectangular selection and clipboard, followed by free-form selection; bounded
   import, transparent pixels and one undo step per committed edit.
7. Crop, resize/skew, flip and right-angle rotation, with clipping/quality choices.
8. JPEG/BMP/WebP interchange and lossy/alpha export choices, preserving PNG safety.
9. Install identity, manual Updates/release link screen, then AppImage and Flatpak
   builds in separate packaging PRs. No automatic install or restart in M2.
10. Alpha closure: complete workflow tests and clean-VM installation evidence,
    publish source/licenses/checksums alongside downloads, and only then assign
    `0.1.0-alpha.1`. Record incomplete desktop certification as explicit limits.

The M2 gate passed with `0.1.0-alpha.1`; [closure evidence](plans/0027-alpha-release.md)
records exact binaries, public-download verification and remaining limits.

## Risks to retire early

| Risk | Mitigation / decision point |
| --- | --- |
| Ambiguous “latest Paint” parity | Freeze an observed reference build per audit; maintain explicit gaps |
| Slow full-image uploads or undo memory growth | Benchmark M1; dirty regions first, tiled storage/renderer by M3 if needed |
| C++ memory safety and malformed image files | Sanitizers, decode limits, fuzzing for parsers before stable |
| Package-manager and self-updater conflicts | Build-time installation identity; separate backends and install tests |
| Inaccessible custom controls | Standard controls first; keyboard/Orca review starts in M1 |
| AI model distribution and hardware requirements | Defer runtime choice until measured proof, license and download UX exist |
| Privacy leaks in contributions or binaries | Review staged content/metadata; CI pattern checks; release privacy audit |

## Definition of 1.0

The implemented M1/M2 editor and scoped installation/release acceptance cases pass
on the published support matrix. Layers, project files, recovery, printing, full
accessibility/input certification and signed automatic updates are deferred. No known
reproducible document-loss defect, unverified update installation path, or critical
security issue remains open. Downloads, corresponding source, license notices,
release notes, install/update/uninstall instructions, and limitations are available.
The README advertises only shipped capabilities. AI and exact Microsoft-format
compatibility remain clearly identified future work.

## Follow-ups from M1

[Closure evidence](plans/0009-m1-closure.md#scope-reconciliation-and-remaining-work)
tracks PERF-01 (renderer/lower-end performance before M3), UI-01 (rapid dialog
handoffs in M4), and PLATFORM-01 (packaged portals, full desktop/input/accessibility
matrix). M1 completion does not close these later gates.
