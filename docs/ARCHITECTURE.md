# Architecture

Status: native shell implemented; single-layer document/patch history implemented; drawing UI integrated; files pending.
See [decision 0001](decisions/0001-native-stack.md) for alternatives and tradeoffs.

## Stack

| Area | Decision |
| --- | --- |
| Desktop application | C++20, Qt 6, Qt Quick Controls and QML |
| Initial compatibility floor | Qt 6.8; pin exact supported patch versions in M1 CI |
| Build | CMake 3.28+, Ninja, checked-in CMake presets |
| Document/raster engine | C++ with Qt Core/Gui; QImage-backed raster storage |
| View | QML controls around a C++ canvas item |
| Tests | Qt Test, Qt Quick Test, CTest; Python stdlib for repo tooling |
| Formats | Qt image plugins; OpenRaster for layered projects from M3 |
| Distribution | Flatpak and portable AppImage, x86_64 first |
| Optional inference | Separate local worker; runtime/model chosen in M6 |

Qt 6.8 is a minimum API target, not a commitment to an old unpatched binary.
Release builds use maintained, pinned dependencies. M1 must prove the floor on
the selected Linux environments; record any necessary adjustment in an ADR.
Use Qt Core, Gui, Qml, Quick, QuickControls2, Concurrent, Network (updater only),
Test and QuickTest as needed. Add dependencies only when their feature lands.

## Proposed tree

Create these directories with real code, not empty placeholders:

```text
src/
  app/          application startup, actions, document controller
  document/     layers, pixels, selections, revisions, undo commands
  tools/        pointer-driven edit operations and previews
  render/       viewport transforms, compositing, canvas scene-graph bridge
  io/           image/project codecs, atomic saves, recovery
  platform/     dialogs, clipboard, input, XDG locations, install provenance
  updates/      update state machine, package-specific backends
  ai/           optional worker protocol and capability discovery (later)
qml/
  components/   reusable controls and design tokens
  panels/       tool options, layers, settings, updates
tests/
  unit/         engine, commands, codecs, updater policy
  integration/  file lifecycle, clipboard, input, packaging
  ui/           keyboard navigation and representative workflows
  fixtures/     generated images and documented provenance
packaging/      desktop entry, AppStream metadata, Flatpak/AppImage recipes
```

Dependency direction: QML → application actions/controllers → document/tools/io.
Rendering consumes read-only document snapshots. Platform and updater services
are injected behind narrow interfaces. The document engine has no dependency on
QML, network access, GitHub, installation format, or AI providers.

## Document and edit model

- One document per window initially; opening another document may create another
  window. No tab system in the initial scope.
- Document coordinates are image pixels; viewport coordinates are logical UI units.
  One tested transform handles pan, zoom, device pixel ratio, and pointer mapping.
- Start with 8-bit sRGB RGBA. Store pixels in premultiplied form internally and
  unpremultiply at codec boundaries. Convert tagged imports to sRGB; treat untagged
  imports as sRGB with an explicit documented assumption. Wide-gamut/HDR editing is
  deferred. Preserve alpha; never render checkerboard into saved pixels.
- A document owns dimensions, ordered stable layer IDs, active layer, revision,
  selection mask, and history. A layer owns name, visibility, opacity, and pixels.
  M1 has one layer but uses the same ownership boundary.
- M1 may use a full QImage per layer and bounded dirty-rectangle undo patches.
  M3 introduces 256×256 pixel tile storage behind the same raster interface before
  large multilayer documents ship. Missing tiles mean transparent pixels.
- All durable edits are commands with before/after state or deterministic patch
  data. A stroke is one command, regardless of input-event count. Dialog previews,
  selection moves, and transforms commit once or cancel without changing pixels.
- Distinguish content revision from UI state. Zoom, panel visibility, and active
  tool do not dirty a document. Content and layer-property changes do. Selection
  changes have session history where useful, but do not mark saved pixels dirty.
- Track a saved revision, not only a boolean flag. Undo to the saved content state
  clears the dirty marker; branching after undo invalidates redo. Trimming history
  must not incorrectly make an unsaved document appear saved.
- Text and shapes remain editable during their active transaction, then rasterize
  on commit. Persistent vector objects and later re-editable text are outside 1.0.
  This avoids pretending OpenRaster preserves an object model it does not store.

### Tool contract

Each tool implements begin/update/commit/cancel over normalized input samples
(position, time, buttons, modifiers, pressure, optional tilt and device identity).
The controller owns pointer capture and tool switching. A tool produces a preview
and a document command; it cannot save, start network requests, or mutate QML state.
Escape cancels an active operation. Tool-switch behavior must be specified and
tested; text requires explicit commit/cancel before switching.

Selection is a bounded alpha mask. Cut/move/delete affect only the selected region
on the active layer unless an action explicitly says otherwise. Sampling merged
colors is a separate, visible option. Erasing on a transparent layer clears alpha;
an explicit opaque-background mode uses the secondary color. Do not guess based
on the source file extension.

## Rendering and responsiveness

CPU rasterization is the authoritative output. Use QPainter/QImage for initial
brush operations, deterministic compositing, and export. The screen displays that
result; GPU-only effects may not alter saved pixels independently.

M1 uses a C++ QQuickPaintedItem adapter for a bounded proof of concept. Its upload
cost is a known risk, not a promised final solution. Benchmark before expanding it.
Move to a QQuickItem scene-graph adapter with dirty-tile textures by M3 if the
[quality budgets](QUALITY.md) fail. Keep the document interface stable either way.
QML Canvas is not the authoritative image store.

The GUI thread owns document command ordering. Workers receive immutable snapshots
and return candidate results with source revision IDs. Apply them only if the
revision still matches, otherwise discard or ask the user to retry. Never share a
mutably accessed QImage across workers. Scene-graph resources belong to the render
thread and must be released through Qt's documented lifecycle.

Long fills, codecs, thumbnails, saves, and inference run off the interaction path.
Provide progress/cancel for lengthy tasks. Bound task queues and memory; calculate
allocation sizes with overflow checks. Refuse excessive operations before allocation
with a useful explanation. Initial limits and budgets live in [QUALITY.md](QUALITY.md).

## Files and recovery

PNG is the lossless interchange baseline; add JPEG, BMP, and WebP in M2. Add static
GIF import in M4 and warn that only the first frame is used. Do not claim animation
editing, GIF export, SVG rendering, or PSD compatibility in 1.0.

From M3, use `.ora` (OpenRaster) as the editable layered project format: ordered
raster layers, opacity, visibility, canvas size, merged preview, and thumbnail.
Read/write the interoperable subset; document unsupported blending modes and
produce an explicit import warning or reject instead of silently corrupting work.
Microsoft `.paint` compatibility is a separate deferred research item.

Choose and license-review a maintained ZIP implementation at M3. Cap entry count,
total decoded bytes, dimensions, compression ratio, and path lengths; reject
absolute paths, parent traversal, duplicate conflicting entries, and symlinks.
Prefer streaming reads over extracting untrusted archives to disk.

Save a frozen revision to a sibling temporary file using QSaveFile where possible,
then commit. Disable direct-write fallback for ordinary paths: a failed save must
leave the original intact. Portal-selected destinations may have different
capabilities; test atomic replacement and, when unavailable, offer Save a Copy
with clear failure behavior. Never report atomic guarantees a backend cannot give.
If editing continues during save, only the saved snapshot's revision becomes clean.

Save retains the document's current backing format where it is lossless for its
contents. Adding layers to a flat image makes the next Save offer an `.ora` project.
Export writes a flattened copy and never marks a layered project saved. JPEG export
requires a background choice for transparency and a quality setting.

Autosave recovery uses app-private XDG storage, atomic generation files, and a
versioned manifest. Start with a 30-second idle checkpoint, bounded disk usage,
and at least one previous valid generation. Recovery must not replace the user's
original file. Offer Restore, Save Copy, or Discard after a crash; clean up after
explicit dismissal or successful recovery. Recovery contains artwork, so keep it
local, with user-only permissions and a visible Clear Recovery Data control.

## Integration boundaries

- Use XDG config/data/cache locations and native/portal dialogs; no hard-coded
  home paths or unrestricted Flatpak home-directory access.
- Use Qt clipboard MIME data. Cap decoded clipboard content like any file import.
- Package identity determines update backend; see [RELEASES.md](RELEASES.md).
- Optional AI submits a snapshot only after a user action; outputs become a preview
  and one undoable command. Worker failure leaves the document untouched.
- Translate user-facing strings with Qt's translation tools from the first UI.
- No public binary plugin ABI in 1.0. Keep internal interfaces simple and testable.
