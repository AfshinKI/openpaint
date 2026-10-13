# Build OpenPaint

The current development app supports pencil/eraser, colors and size, undo/redo,
zoom/pan, PNG open/save, guarded close and System/Light/Dark themes. There are no packaged
downloads. This guide builds from source and does not install the app system-wide.

## Tested baseline

Ubuntu 24.04 x86_64; GCC 13.3.0; Qt 6.8.3; CMake 3.30.0; Ninja 1.12.1;
Python 3.12.3. CMake requires 3.28+, Python tooling requires 3.11+, and the Qt API
floor is 6.8. CI also builds with Ubuntu's Clang 18 package. Qt 6.8.3 is a pinned
development baseline, not a claim that it is the newest Qt release.

The setup downloads Qt from its distribution mirrors using aqtinstall. CMake never
downloads dependencies. The top-level Python tools are pinned in
[requirements-dev.txt](../tools/requirements-dev.txt); transitive Python packages
and Ubuntu package patch versions are resolved at setup time. This is not yet a
bit-for-bit reproducible release toolchain.

## Ubuntu setup

From the repository root, install system prerequisites:

```sh
sudo apt-get update
sudo apt-get install -y g++-13 cmake ninja-build python3-venv \
  zlib1g-dev libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0 libxkbcommon-x11-0
```

Install development tools and Qt in the ignored build directory:

```sh
python3 -m venv build/tooling
build/tooling/bin/python -m pip install -r tools/requirements-dev.txt
build/tooling/bin/python -m aqt install-qt linux desktop 6.8.3 linux_gcc_64 \
  --outputdir build/qt --modules qtimageformats --archives icu qtbase qtdeclarative qtsvg qttools qtwayland qtimageformats
export PATH="$PWD/build/tooling/bin:$PATH"
export CMAKE_PREFIX_PATH="$PWD/build/qt/6.8.3/gcc_64"
```

ICU is required by the downloaded Qt binaries even if another ICU version is
already installed on the system. An existing compatible Qt installation can be
used instead by setting `CMAKE_PREFIX_PATH`; keep private paths in your shell or
ignored `CMakeUserPresets.json`, never in the shared presets.

## Build, run, and check

```sh
cmake --preset dev
cmake --build --preset dev --parallel 4
./build/dev/openpaint
```

Use View → Theme to select System, Light, or Dark. The choice is session-only.
F1 opens About; Escape closes it. File → Quit closes the window. Window decorations
belong to the desktop. `openpaint --help` and `openpaint --version` are available.

```sh
cmake --build --preset dev --target format-check qml-lint
ctest --preset dev
python3 tools/check_repo.py
python3 -m unittest discover -s tools/tests -v
```

CTest runs offscreen with software rendering by default and errors on empty test
discovery. The shell suite checks launch/rendering, resizing, mutually exclusive
theme actions, keyboard About/Escape, and Quit. The CLI test checks the built
executable's version. QML warnings fail the shell test; `qml-lint` fails on lint
warnings as well as errors. `format-check` compares output without editing files.

To apply the pinned formatters:

```sh
clang-format -i src/app/main.cpp tests/ui/tst_shell.cpp
build/qt/6.8.3/gcc_64/bin/qmlformat -i qml/Main.qml qml/Theme.qml
```

`release` and `asan` presets use separate directories and the same configure/build/
test sequence. `asan` enables address and undefined-behavior sanitizers, including
leak detection. Run it outside a debugger/process-tracing sandbox: LeakSanitizer
cannot operate under ptrace. Do not turn off leak checks to hide that environment
limitation. To select another compiler, set `CXX=clang++-18` before a **fresh**
configure; do not change the compiler in an existing CMake build directory.

## Native desktop and visual checks

To validate the real GNOME file portal, install `python3-gi` and
`gir1.2-atspi-2.0`, then run `python3 tools/validate_portal.py` in an active
desktop session. It launches a separate test process, handles only uniquely named
test dialogs, and removes generated artwork afterward. It tests an unpackaged
application; Flatpak document grants still require packaged-app validation.


From an actual desktop session, run either applicable command:

```sh
QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=software ./build/dev/openpaint_shell_tests
QT_QPA_PLATFORM=wayland ./build/dev/openpaint_shell_tests
```

X11 with the default renderer at 125% scale and offscreen/software have been
checked locally. These
tests do not establish Wayland, Orca or packaged-app support. The
[M1 baseline](plans/0009-m1-closure.md) additionally records actual GNOME/X11
OpenGL, keyboard and unpackaged portal validation on one machine.

For repeatable X11 input checks without live-desktop pointer interference, install
`xvfb` and `xauth` using the distribution package manager, then run:

```sh
for scale in 1 1.25 1.5 2; do
  QT_SCALE_FACTOR="$scale" xvfb-run -a -s '-screen 0 3200x2000x24 -nolisten tcp' \
    env QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=software ./build/dev/openpaint_shell_tests
done
```

The GCC Debug CI lane uses this same matrix. It checks Qt's X11 input/rendering
path in isolation, not desktop integration or physical input latency. The `canvas`
CTest suite separately covers event-level gesture lifetime and edge coordinates.

Generate synthetic captures of just the test window, excluding the desktop:

```sh
OPENPAINT_CAPTURE_DIR="$PWD/build/evidence" ctest --preset dev -R '^shell$'
```

The captures cover light/dark at 1024×640 and 1280×800. They contain no artwork,
file paths, or personal information. Inspect them before attaching them to a PR.
Regenerate captures after UI changes rather than treating old images as evidence.

Reviewed canvas captures (generated by `tests/ui/tst_shell.cpp`, project-authored,
GPL-3.0-or-later; no external assets or embedded text/EXIF metadata):

| Size | Light | Dark |
| --- | --- | --- |
| 1024×640 | [Light preview](assets/shell-light-1024.png) | [Dark preview](assets/shell-dark-1024.png) |
| 1280×800 | [Light preview](assets/shell-light-1280.png) | [Dark preview](assets/shell-dark-1280.png) |

Color editor: [Light preview](assets/color-editor-light.png),
[Dark minimum-size preview](assets/color-editor-dark-1024.png), and
[Toolbar custom slots](assets/custom-palette-dark-1024.png),
[Full custom editor](assets/color-editor-full-dark-1024.png), and
[Color model menu](assets/color-model-menu-dark.png).

## Use the basic editor

Run `./build/dev/openpaint` after building. Draw with the left button; the right
button uses the secondary color. Choose a palette color or click Primary for a
custom color. Click the secondary swatch to edit it independently. The editor has a
rectangular hue/saturation spectrum, brightness slider and HEX/RGB/HSV fields.
Choose RGB or HSV from the mode menu; changing modes preserves the color. OK applies the draft;
Cancel/Escape discard it. Add/Replace/Remove/Clear palette actions save up to 24
custom colors locally immediately, independently of OK/Cancel. Saved swatches
appear on restart. The toolbar shows the first ten, including hollow empty slots;
all 24 are available in the editor. Empty toolbar slots open the editor, while +
or an empty dialog slot saves the draft in the next free slot. The ⋯ menu offers
replace/remove/clear; removal compacts the palette. Swap exchanges primary/secondary. Eraser uses the secondary color
unless Transparent is checked. Each drag is one undo step; Escape cancels it.

Ctrl+Z undoes; Ctrl+Y/Ctrl+Shift+Z redo. Ctrl+wheel zooms around the pointer;
Space+drag or middle-drag pans; Ctrl+0 fits. Ctrl+N creates a blank image, Ctrl+O
opens supported images, Ctrl+S saves and Ctrl+Shift+S chooses another PNG filename. Unsaved
New/Open/Close offers Save, Discard and Cancel. Failed saves keep the drawing open.

Static PNG, JPEG, BMP and WebP imports have a 32 MP limit. Source metadata is
removed, colors normalize to sRGB and JPEG/PNG/WebP EXIF orientation is applied.
Animated PNG/WebP and BMP V4/V5 color headers are rejected. Save writes PNG;
non-PNG imports ask for a PNG destination. File → Export copy writes PNG/JPEG/BMP/WebP
with explicit quality/background options and leaves the drawing’s saved state intact. Recovery after a
crash, layers, shapes, text, selection, packaged downloads and updates are not yet
available. The developer build is useful for basic drawings, not a stable release.

Controller/codec tests verify snapshot saves, errors and lossless reopen. After a
Release build, measure startup, strokes, viewport updates, worker-save responsiveness
and memory:

```sh
QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 python3 tools/benchmark_m1.py
```

The probe opens short-lived test windows and uses temporary generated artwork. See [M1 closure](plans/0009-m1-closure.md)
for the measured baseline, target misses and remaining later gates.
