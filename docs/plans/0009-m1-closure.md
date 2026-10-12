# M1 performance baseline and closure

Status: M1 acceptance complete; closure takes effect after protected CI/merge
Feature IDs: F01, F02, F11, F15, F16, F26
Milestone: M1

## Outcome and scope

Close the first working canvas checkpoint using reproducible measurements and
links to implemented acceptance evidence. No new product tools, packaging or
renderer rewrite. Performance targets are measured honestly; misses produce a
specific renderer follow-up as required by the foundation plan.

## Acceptance

- [x] Release startup: five warm-up processes and 30 measured launches.
- [x] 2048px strokes and 4096px single-layer viewport: at least 60 samples after
      warm-up; distinguish frame notifications from physical input latency.
- [x] Worker-save responsiveness and peak RSS/history budget recorded on generated
      content; demonstrate editing during saving and lossless reopen.
- [x] Fresh minimum/typical light/dark captures inspected; source-build limits clear.
- [x] All M1 evidence reconciled with the roadmap; later gates remain explicit.
- [x] Local checks and protected CI pass before merging closure.

## Measured evidence

2026-10-12, Ubuntu 24.04, GCC 13.3 Release, Qt 6.8.3, GNOME Shell 46.0 on X11.
Intel Core i9-12900KS, 64 GiB RAM, NVIDIA RTX 3070, NVIDIA 580.178.04/OpenGL 4.6,
2560×1440 output at 59.95 Hz, 1× Qt scale, 1280×800 test window. This is the
available measurement host, faster than the aspirational 4-core/8 GiB/integrated-GPU
baseline. These results do not certify that lower-end configuration.

Run after building Release, without concurrent builds:

```sh
QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 python3 tools/benchmark_m1.py
```

Startup uses five warm-up processes and 30 measured processes. Timing begins at
entry to the test executable and ends after QML loads, the window is exposed,
and a rendered frame is grabbed. It includes test initialization and temporary
settings setup, but excludes OS loader time. Rendering uses five warm-up samples
and 60 measured updates per scenario. Reported render values below are from the
final standalone `renderTiming` run after adding maximum-delay reporting.

| Measurement | p50 | p95 | Maximum / additional evidence |
| --- | --- | --- | --- |
| Warm process entry to editable rendered window | 758.245 ms | 832.059 ms | 30 launches; below 1 s on this host |
| 2048×2048 stroke update → next frame notification | 16.688 ms | 17.008 ms | 17.075 ms |
| Stroke frame-notification interval | 16.691 ms | 17.012 ms | 17.078 ms; misses the strict 16.7 ms target |
| 4096×4096 single-layer zoom update → next frame | 16.633 ms | 17.181 ms | 17.707 ms |
| Viewport frame-notification interval | 16.637 ms | 17.187 ms | 17.711 ms |
| GUI heartbeat during worker save | 10.121 ms | 10.211 ms | 21.305 ms; 400 samples |
| 4096×4096 generated noisy PNG save | — | — | 4072.195 ms; editing continued and saved snapshot reopened exactly |
| Process peak RSS | — | — | 689.125 MiB, including Qt/driver/codec allocations |
| Peak retained undo patches | — | — | 256 MiB; subsequent commands trim safely |

The heartbeat requests 5 ms; event-loop/test scheduling produces about 10 ms
observed spacing. Frame notifications are CPU-observed `frameSwapped` events,
not physical input-to-photon latency. A single layer is not the future eight-layer
scenario. RSS is observed workload memory, not a proof of every maximum-sized
import. No hardware performance threshold is enforced in shared CI.

## Gate evidence

| M1 requirement | Evidence |
| --- | --- |
| Clean native build, basic canvas and history | PRs #3–#5; Debug/Release/sanitizer suites; [foundation](0001-foundation.md) |
| Draw → undo → redo → save → reopen, safe close | [PNG workflow](0003-png-file-workflow.md), [keyboard/portal workflow](0008-keyboard-desktop-workflow.md) |
| Mouse lifetime, clipping and fractional scale | [input validation](0006-canvas-gesture-lifecycle.md); isolated X11 at 1/1.25/1.5/2 |
| Color/alpha/orientation and failure-safe files | [PNG validation](0007-png-validation.md), merged [PR #12](https://github.com/AfshinKI/openpaint/pull/12) |
| Keyboard shell and desktop file flow | [PR #13](https://github.com/AfshinKI/openpaint/pull/13), actual GNOME keyboard tests and portal save/replacement/open/cancel |
| Accessibility review begins in M1 | Qt accessible names/roles/focus checked; full Orca usability certification remains M4 |
| Light/dark and minimum/typical size review | Fresh generated captures inspected at 1024×640 and 1280×800 and byte-matched the existing published images; color dialog fits minimum size |
| Performance baseline and renderer decision | Measurements above; retain bounded QQuickPaintedItem for M1, track PERF-01 below |

Local Debug, Release and ASan/UBSan CTest, formatting, zero-warning QML lint,
repository checks and seven tooling tests pass. The opt-in large-document probe
also passes under ASan/UBSan with leak detection enabled (offscreen/software; those
timings are not used as the desktop baseline). Final CI is recorded in the closure PR. Only synthetic test-window images are published; no desktop captures,
user artwork or machine-specific paths are included.

## Scope reconciliation and remaining work

M1 closes the original roadmap gate: a native basic PNG editor, reliable history
and file lifecycle, keyboard shell, clean build, and measured performance baseline.
The detailed foundation plan also listed exploratory pen and desktop matrix work;
those are retained under the roadmap's existing M4 gate, not marked as tested.
A measured frame-budget miss is explicitly allowed to become a renderer follow-up
by the M1 foundation plan. It is not relabeled as a passing performance result.

- **PERF-01 (before M3 renderer expansion):** reproduce the 16.7 ms p95 miss on
  integrated graphics/8 GiB and GNOME/KDE Wayland/X11; capture presentation/input
  traces, compare dirty-tile scene-graph rendering, and keep CPU pixels/history
  authoritative. Acceptance: recorded frame and physical-input budgets, bounded
  memory under multilayer load, and no save/history regressions.
- **UI-01 (M4 polish):** rapid popup handoffs during close animations can disturb
  Qt fallback focus. The validated workflow waits for actual closure. Audit queued
  keyboard actions and nested dialogs rather than relying on timing delays.
- **PLATFORM-01 (M2/M4/M5):** packaged Flatpak grants/replacement, lower-end hardware,
  Wayland/KDE, physical pen/touch and human Orca usability remain unverified.

No packaged alpha, crash recovery, layers, selections, updater or stable-release
claim is made. Next implementation work is a fresh, bounded M2 feature plan/PR.
