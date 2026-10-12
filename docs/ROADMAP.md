# Delivery plan

A usable basic source editor supports drawing, history and safe PNG files.
M1 is complete for the validated source-build checkpoint; M2 tools and packages come next.
See [M1 closure evidence and limits](plans/0009-m1-closure.md). See [the build guide](BUILD.md) to run the preview.
Milestones are exit gates, not promised dates. Complete thin, usable slices before
adding breadth. [FEATURES.md](FEATURES.md) is the implementation status ledger.

| Gate | Result | Required acceptance evidence |
| --- | --- | --- |
| M0 — foundation | Reviewed docs, license, shared agent rules, templates, working docs/privacy checks | Local checks pass; repository settings verified after publication |
| M1 — first working canvas (complete) | Native window; pencil/eraser; primary/secondary colors; PNG open/save; undo/redo; zoom/pan; dirty-close prompt | Draw → undo → redo → save → reopen preserves pixels; failed save preserves original; clean Linux build; keyboard shell; performance baseline |
| M2 — useful alpha | Brush options, fill, shapes/text, selections/clipboard, crop/resize/rotate, common image formats; first downloadable alpha | Workflow tests; transparency/lossy export prompts; clean-VM AppImage and test Flatpak install; Updates screen with manual release link |
| M3 — layered beta | Layer operations, .ora projects, tiled storage as needed, bounded history, crash recovery | Cross-tool .ora round-trip; crash/recovery and disk-full tests; large-document budget; full project save fidelity |
| M4 — desktop completeness | Freeform rotation, rulers/grid, stylus/touch, printing, accessibility, adaptive UI and translation infrastructure | GNOME/KDE Wayland and X11 matrix; Orca and keyboard tests; print preview; scale/theme screenshots; input-device tests |
| M5 — 1.0 release quality | Signed portable updates, package-managed update UX, trustworthy downloads, release/support process | Upgrade + failed-upgrade matrix; all core feature gates; licenses/SBOM/source artifacts; no open data-loss/security blockers |
| M6 — optional intelligent tools | Background removal first, object selection, then generative workflows | Separate plans and model-license review; offline/consent tests; memory/latency measurements; cancel/undo and model-deletion checks |

M1 is specified in [the foundation plan](plans/0001-foundation.md). M2 depends on
M1; M3 and release packaging improvements can progress independently once their
interfaces stabilize. M4 depends on editing and document contracts from M2/M3.
M5 requires M1–M4. M6 may be researched earlier, but must not delay core reliability.

## Release progression

M1 is a development checkpoint. M2 produces an explicitly unstable `0.1.0-alpha.1`
when its gate passes. M3/M4 may produce further alpha/beta releases. M5 produces
`1.0.0` only after the non-generative scope passes. Exact later version numbers
are assigned at release time; milestone numbers are not version numbers.

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

All M1–M5 acceptance cases pass on the published support matrix. No known
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
