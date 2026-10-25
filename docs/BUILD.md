# Build OpenPaint

The v2 editor supports drawing tools, selections, transforms, raster layers,
OpenRaster projects, image import/export, undo/redo and System/Light/Dark themes. Packaged downloads are described in [INSTALL.md](INSTALL.md). This guide builds
from source and does not install the app system-wide.

## Tested baseline

Ubuntu 24.04 x86_64; GCC 13.3.0; Qt 6.12.0; CMake 3.30.0; Ninja 1.12.1;
Python 3.12.3. CMake requires 3.28+, Python tooling requires 3.11+, and the Qt API
floor is 6.8. CI also builds with Ubuntu's Clang 18 package. Qt 6.12.0 is a pinned
native toolchain. Historical M1/early M2 evidence used Qt 6.8.3; see
[the migration plan](plans/0023-maintained-qt.md). Recheck available security/patch
updates when preparing a binary release.

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
  libarchive-dev zlib1g-dev libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0 libxkbcommon-x11-0
```

Install development tools and Qt in the ignored build directory:

```sh
python3 -m venv build/tooling
build/tooling/bin/python -m pip install -r tools/requirements-dev.txt
build/tooling/bin/python -m aqt install-qt linux desktop 6.12.0 linux_gcc_64 \
  --outputdir build/qt --modules qtimageformats --archives icu qtbase qtdeclarative qtsvg qttools qtwayland qtimageformats
export PATH="$PWD/build/tooling/bin:$PATH"
export CMAKE_PREFIX_PATH="$PWD/build/qt/6.12.0/gcc_64"
```

When changing Qt versions in an existing build tree, use `cmake --fresh --preset dev`
(and the matching release/asan preset) after setting the new `CMAKE_PREFIX_PATH`.
This prevents cached Qt module paths from mixing SDK versions.

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

Use View → Theme to select System, Light, or Dark. The choice is saved between launches.
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
build/qt/6.12.0/gcc_64/bin/qmlformat -i qml/Main.qml qml/Theme.qml
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
OPENPAINT_CAPTURE_DIR="$PWD/build/evidence" ctest --preset dev -R '^shell'
```

The capture workflows cover light/dark and multiple window sizes with generated
artwork only, excluding personal file paths or information. Inspect them before attaching them to a PR.
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
button holds and drags to pan with a closed-hand cursor. Choose a palette color or click Primary for a
custom color. Click the secondary swatch to edit it independently. The editor has a
rectangular hue/saturation spectrum, brightness slider and HEX/RGB/HSV fields.
Choose RGB or HSV from the mode menu; changing modes preserves the color. OK applies the draft;
Cancel/Escape discard it. Add/Replace/Remove/Clear palette actions save up to 24
custom colors locally immediately, independently of OK/Cancel. Saved swatches
appear on restart. The toolbar shows the first ten, including hollow empty slots;
all 24 are available in the editor. Empty toolbar slots open the editor, while +
or an empty dialog slot saves the draft in the next free slot. The ⋯ menu offers
replace/remove/clear; removal compacts the palette. Eraser offers Paper white
and Transparent modes. Each drag is one undo step; Escape cancels it.

Ctrl+Z undoes; Ctrl+Y/Ctrl+Shift+Z redo. Ctrl+wheel zooms around the pointer;
Right-drag, Space+drag or middle-drag pans; Ctrl+0 fits. Ctrl+N creates a blank image, Ctrl+O
opens supported images/projects, Ctrl+S saves and Ctrl+Shift+S chooses another destination.
On macOS, use the platform Command shortcuts. Unsaved
New/Open/Close offers Save, Discard and Cancel. Failed saves keep the drawing open.

Static PNG, JPEG, BMP and WebP imports have a 32 MP limit. Source metadata is
removed, colors normalize to sRGB and JPEG/PNG/WebP EXIF orientation is applied.
Animated PNG/WebP and BMP V4/V5 color headers are rejected. Flat drawings save as
PNG; documents requiring layer structure/properties save as OpenRaster (.ora).
Non-PNG image imports ask for a new destination; bounded .ora projects reopen
with their layers intact. File → Export copy writes PNG/JPEG/BMP/WebP
with explicit quality/background options and leaves the drawing’s saved state intact. Recovery after a
crash remains planned. Current implemented tools and layer limits
are tracked in [the feature checklist](FEATURES.md).

Controller/codec tests verify snapshot saves, errors and lossless reopen. After a
Release build, measure startup, strokes, viewport updates, worker-save responsiveness
and memory:

```sh
QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 python3 tools/benchmark_m1.py
```

The probe opens short-lived test windows and uses temporary generated artwork. See [M1 closure](plans/0009-m1-closure.md)
for the measured baseline, target misses and remaining later gates.

## Staged installation

`cmake --install build/dev --prefix /usr --strip` installs the executable, desktop
entry, original icon, AppStream metadata, license and documentation. Use `DESTDIR`
for staging rather than installing development builds over a system package.
Packaging builds may set `BUILD_TESTING=OFF`; developer formatter/Python tooling
is then unnecessary. Set `OPENPAINT_INSTALL_KIND` to `appimage`, `flatpak` or
`system` only in the corresponding package recipe (default `source`).
`OPENPAINT_VERSION_SUFFIX` defaults to `dev`; a prerelease recipe supplies e.g.
`alpha.1`, while an empty suffix identifies a stable build. The generated identity
is shown by `--version` and Help → Updates.

`openpaint [image]` opens one local image in a new window/process. More than one
argument is rejected. Save and export retain the same safety rules as File → Open.

Development layer support: use the Layers ribbon button or View → Layers. Tools
edit the selected layer. Save layered drawings as `.ora`; use Export copy for
flattened images. The build now also requires `libarchive-dev`. See
[layer scope and current validation](plans/0058-layers.md).

## macOS release build

Use an Apple silicon Mac with Xcode 16 or newer and macOS 14.4+. The CI runner
installs Qt 6.12.0 (`aqt install-qt mac desktop 6.12.0 clang_64`) with qtbase,
qtdeclarative, qtsvg, qttools and qtimageformats. Install development tooling from
`tools/requirements-dev.txt`, CMake 3.30.5, Ninja and Homebrew libarchive headers.
The package links Apple's system libarchive, not the Homebrew runtime.

Set `CMAKE_PREFIX_PATH` to the installed Qt `macos` directory, then configure:

```sh
cmake --preset release -DOPENPAINT_INSTALL_KIND=macos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4 \
  -DLibArchive_INCLUDE_DIR="$(brew --prefix libarchive)/include" \
  -DLibArchive_LIBRARY="$(xcrun --show-sdk-path)/usr/lib/libarchive.tbd"
cmake --build --preset release
ctest --preset release
python3 tools/package_macos.py --qt "$CMAKE_PREFIX_PATH" --version 2.0.0
```

The package tool runs macdeployqt, generates the original app icon, signs ad-hoc,
audits runtime paths and builds a DMG plus matching source/license inventory.
Developer ID signing and notarization are not configured. The Cocoa smoke tests
need a logged-in graphical session; offscreen tests do not establish native glass
appearance. CI runs both editor tests and selected native Cocoa workflows.
