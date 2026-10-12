# ADR 0001: native Qt application with a C++ raster core

Status: accepted for initial implementation
Date: 2026-10-12

## Context

OpenPaint needs responsive drawing, Linux desktop integration, a familiar modern
toolbar, layered raster editing, accessible controls, and downloadable releases.
Contributors should be able to work with ordinary open-source tooling and any
coding assistant. There is no pre-existing codebase to preserve.

## Decision

Use C++20, Qt 6.8+ APIs, QML/Qt Quick Controls, and CMake/Ninja. Keep document data
and raster operations in C++; keep the interface declarative. Start CPU-first and
measure rendering before adding a complex GPU pipeline. Package as Flatpak and
AppImage. License project-authored work GPL-3.0-or-later.

## Alternatives

| Option | Benefit | Reason not selected initially |
| --- | --- | --- |
| Rust + GTK4 | Memory safety, strong GNOME integration | More work to achieve the desired toolbar and portable desktop appearance; a second ecosystem for custom raster/UI integration |
| Rust + Tauri/web UI | Familiar web tooling, Rust backend | Browser canvas/input/desktop integration adds a second behavior stack to validate |
| Electron + TypeScript | Fast UI iteration and broad contributor pool | Larger runtime and extra care for raster throughput, memory, and native input |
| Python + PySide6 | Fast prototyping with Qt | Likely native acceleration and packaging complexity for the long-term raster core |
| Qt Widgets + C++ | Mature widgets and conventional desktop integration | QML better fits the intended adaptive toolbar, panels, and theme system |

These are project tradeoffs, not claims that alternatives cannot build a good editor.

## Consequences

C++ requires deliberate lifetime/thread ownership, sanitizers, bounded image
decoding, and command-level tests. Custom QML controls require accessibility work.
Qt/module and bundled-library licenses must be inventoried for each release.
Dynamic Qt linking is the default; ship notices and matching source/build material
as required by each dependency. The project license does not relicense dependencies.

No Rust bridge, plugin ABI, browser runtime, cloud backend, or AI dependency is
needed for M1. Revisit the renderer after measured M1/M3 performance and the UI
choice only if concrete input/accessibility tests show a blocker. A new ADR must
include evidence and migration cost before replacing this stack.

Sources: [Qt licensing](https://doc.qt.io/qt-6/licensing.html),
[Qt Quick scene graph](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html),
[Qt Quick accessibility](https://doc.qt.io/qt-6/accessible-qtquick.html).
