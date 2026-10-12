# Quality and validation

Current checks cover repository content, document/history, PNG codecs, controller
lifecycle and the native editor UI. Updater suites become required when it lands.

## Check selection

| Change | Required evidence |
| --- | --- |
| Documentation/templates | Repository checks; inspect rendering and factual/status claims |
| Tooling/check scripts | Focused positive and negative tests; repo checks |
| Document/tool/command | Qt unit tests: normal, boundary, undo/redo, cancellation |
| File/project codec | Round-trip and malformed/oversized input; original survives failure |
| UI/input | Relevant Qt Quick tests, manual keyboard checks, synthetic screenshots |
| Worker/render code | Revision/race/lifetime tests, sanitizers, relevant benchmarks |
| Network/updater | Fake transport/clock/filesystem, adversarial cases in RELEASES.md |
| Packaging | Install, launch, save, upgrade, uninstall on clean target environments |

Run targeted checks first, then the affected full suite. Record what actually ran;
don't repeat unrelated suites without a reason. Never accept empty test discovery
as evidence that application behavior passes.

## Fixture and comparison rules

Generate small deterministic images (gradients, alpha edges, shapes, noise) with a
fixed seed. Store provenance beside any third-party fixture. No real user artwork
or private photos. Pixel-exact comparisons apply to deterministic core operations;
fonts, platform rasterization, and scaling use documented tolerances and separate
manual review. Fix locale/font/test environment before comparing UI captures.

Every durable editing command gets apply → undo → redo equivalence checks. Test
dirty state at the saved revision, history trimming, and branches after undo. File
tests include truncated files, bad dimensions, unexpected metadata, failed writes,
disk full, invalid color profiles, and import/export alpha conversion.

Headless tests use an offscreen Qt backend where appropriate. They do not replace
real Wayland/X11, clipboard/portal, GPU fallback, accessibility, stylus, or print
tests. Prefer public control interfaces and stable test IDs to coordinate clicking.

## Initial limits and performance targets

These are targets. [M1 measurements and limitations](plans/0009-m1-closure.md)
record actual results, including the strict frame-budget miss. At M1, record a reproducible machine
profile (CPU, RAM, GPU/driver, compositor, Qt version, scale) without serials,
private names, or paths. Baseline: contemporary 4-core x86_64, 8 GiB RAM, integrated
graphics, SSD, 1920×1080 display at 60 Hz. Name exact hardware when measured.

| Scenario | Initial target |
| --- | --- |
| Warm start to editable blank document | < 1 second on recorded reference machine |
| 2048×2048 brush drag, one layer | p95 frame time ≤ 16.7 ms; input-to-visible p95 ≤ 32 ms |
| Pan/zoom a 4096×4096 document with eight raster layers | p95 frame time ≤ 33 ms |
| Long fill/load/save | GUI responsive; progress visible if operation exceeds 250 ms |
| Undo memory | Default history budget 256 MiB; trim oldest commands safely |

Use monotonic instrumentation and at least 30 measured repetitions after warm-up
for discrete operations. Record frame/input traces for sustained strokes; report
percentiles and environment. Document regressions, rather than tuning tests to pass.

Initial import limits: 16,384 pixels per dimension, 32 million pixels total in M1 (100 million is a later target), and a
1 GiB estimated document working-set budget including decoded layers. Estimate
before allocation, account for temporary buffers and undo, and allow stricter
limits on low-memory systems. A single operation larger than the undo budget
must warn before committing or refuse; it must not silently become non-undoable.
Revisit limits with measurements and a documented decision, not arbitrary removal.

## CI progression

Today: Python content checks and native shell builds/tests run on every PR/push,
with read-only tokens. CI covers GCC 13 Debug, Clang 18 Release and ASan/UBSan with
Qt 6.8.3, formatting and QML lint. Dependabot checks workflow dependencies weekly.
The GCC Debug lane additionally runs the shell suite on isolated X11/software at
100%, 125%, 150% and 200% scale. Xvfb prevents live-desktop pointer interference;
it is not compositor, GPU or physical-device validation. Tests synchronize and
grab the rendered window before coordinate actions instead of requiring an extra
frame when the requested state may already have rendered.
M3 adds parser fuzz targets with bounded corpora. M4 adds representative UI smoke
tests. M5 adds artifact and updater gates. Sanitizers run in separate compatible
jobs; known external-library exceptions must be narrow and documented.

## Manual release walkthrough

Start clean; create a drawing, resize, select/copy/paste, add text and shapes, add
layers, undo/redo, export PNG/JPEG, save/reopen `.ora`, and recover after a deliberate
test crash. Check light/dark, scaling, keyboard/Orca, pressure pen, clipboard between
apps, file portals, printing, and offline behavior. Then test upgrade and failed
upgrade with unsaved artwork. Use generated files only and record actual results.

Review image metadata, diagnostics, binary paths, license inventory, known bugs,
and all advertised features before a stable release. See [ROADMAP.md](ROADMAP.md)
for the gate; a checklist with unchecked items is not a release approval.
