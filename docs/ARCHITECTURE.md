# Architecture

Status: native shell, bounded raster layers, patch/stack history, drawing UI and background image/project lifecycle shipped in v2.0.0. See [layer decision](decisions/0008-bounded-layer-projects.md).
See [decision 0001](decisions/0001-native-stack.md) for alternatives and tradeoffs.

## Stack

| Area | Decision |
| --- | --- |
| Desktop application | C++20, Qt 6, Qt Quick Controls and QML |
| Initial compatibility floor | Qt 6.8; CI pins Qt 6.12.0; packaged runtimes are recorded separately |
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

## Current source layout

```text
src/
  app/          startup, Editor controller, selection/transform actions
  document/     layers, pixels, revisions and undo history
  color/        color conversions, session palettes and spectrum
  tools/        brush, fill, shapes, text, selection and transforms
  render/       canvas scene graph and brush/shape/layer previews
  io/           bounded image/OpenRaster codecs and atomic saves
  platform/     install identity, appearance, blur detection and native Mac glass
  updates/      bounded release checks (installation remains manual)
qml/            shared controls, menus, dialogs, panels, icons and theme tokens
tests/          engine, file lifecycle, input and shell workflows
packaging/      Linux and macOS package recipes and metadata
tools/          repository, packaging, release and website checks
```

Recovery, an AI worker module and automatic installation backends are future work;
the tree above describes existing modules rather than proposed placeholders.

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
  The initial layered implementation retains bounded full rasters (32 layers,
  128 MiB total layer pixels). Tile storage remains required before raising those
  bounds for large multilayer documents; see ADR 0008.
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

Fill workers return a candidate raster and changed rectangle. `Document::applyPatch`
checks source revision, bounds and history budget before committing one patch.
Cancellation is rechecked on the GUI thread before application.

Dimension-changing commands use bounded full-raster before/after history alongside
ordinary patches; see [ADR 0002](decisions/0002-dimension-history.md). Transform
workers return image/mask results; the controller checks cancellation and revision
before committing or opening a selected-content preview.

Shape previews use `beginPreview`/`updatePreview` on the GUI-owned document. Each
update restores the old dirty rectangle from the original snapshot before placing
the new clipped patch. Ending a pointer gesture can retain the preview; explicit
commit uses the same one-command history boundary as strokes.

Text uses the same preview transaction. C++ renders bounded plain text with
QTextDocument; QML TextEdit owns keyboard input and local text undo. During editing
the canvas displays the immutable base under the inline editor. Explicit commit
flushes input-method composition before committing one raster history patch.

Floating selection content uses an immutable source image and the same document
preview transaction. Its original mask is retained for cancellation; the destination
mask moves with the content. Move clears the source before source-over compositing,
so overlap never samples earlier previews. Clipboard PNG decode shares the bounded
file parser. Both encoding and decoding run on workers; only the GUI thread
accesses the clipboard. Cut clears pixels after successful uncancelled encoding.

### Tool contract

Each tool implements begin/update/commit/cancel over normalized input samples
(position, time, buttons, modifiers, pressure, optional tilt and device identity).
The controller owns pointer capture and tool switching. A tool produces a preview
and a document command; it cannot save, start network requests, or mutate QML state.
Escape cancels an active operation. Tool-switch behavior must be specified and
tested; text requires explicit commit/cancel before switching.

Selection is currently a bounded binary QRegion mask owned by Document. It is
transient state outside pixel history and cannot change during a document
transaction. Fill workers receive an immutable copy; stroke, brush and preview
paths apply the same boundary. Free-form input records a bounded polygon and builds the odd-even region once at
gesture end; complexity limits reject excessive masks. The renderer caches a
simplified outline independently of floating position. Future feathering can
extend the mask to alpha coverage. Cut/move/delete affect only the selected region
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

PNG chunk CRC/profile validation links system zlib (zlib license). A bounded TIFF
IFD0 reader extracts orientation only; EXIF/text is stripped and orientation applied
exactly once before the document receives pixels. ICC profiles are limited to
4 MiB decoded and must describe a Qt-supported color space. The PNG output device
records short/failed writes and lets the codec unwind before cancelling QSaveFile;
it never reports a failed underlying write as a successful save.

PNG is the lossless Save baseline. JPEG/BMP/static WebP imports use signature-selected
Qt codecs with encoded/dimension limits and fresh sRGB pixel copies. Animated WebP
and BMP V4/V5 color headers are rejected. Imports do not retain non-PNG backing
URLs; Save requests a PNG destination. Export serializes PNG/JPEG/BMP/WebP copies with explicit alpha/loss choices.
It encodes a sanitized immutable snapshot to a bounded file result, then uses
QSaveFile without direct-write fallback. Export completion never marks a revision
saved or resumes a pending Save/Close operation. Add static
GIF import in M4 and warn that only the first frame is used. Do not claim animation
editing, GIF export, SVG rendering, or PSD compatibility in 1.0.

From M3, use `.ora` (OpenRaster) as the editable layered project format: ordered
raster layers, opacity, visibility, canvas size, merged preview, and thumbnail.
Read/write the interoperable subset; document unsupported blending modes and
produce an explicit import warning or reject instead of silently corrupting work.
Microsoft `.paint` compatibility is a separate deferred research item.

The initial project codec uses system libarchive (BSD license); see ADR 0008. Cap entry count,
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

Release notifications use a GUI-thread Qt Network service with bounded asynchronous requests, immutable parsed version strings and local schedule preferences. See [ADR 0004](decisions/0004-release-checks.md); document mutations and installation remain outside this service.

Desktop blur evidence is a bounded worker-side local probe, separate from saved
appearance intent and document state. See [ADR 0006](decisions/0006-desktop-blur-guidance.md).

Color selection scans immutable document snapshots on a cancellable worker. It
returns a QRegion built from sorted horizontal runs, coalescing repeated rows and
bounding output to 16,384 rectangles. The GUI controller checks cancellation and
the captured document revision before replacing the mask. Selecting does not mutate
pixels or content history; existing document commands handle subsequent edits.

Inline text retains font runs in the transient Qt Quick text document. C++ merges
requested character properties and clones the bounded plain-input document for
raster preview/commit; saved pixels still belong to Document. Logical-pixel layout
and view scaling keep zoom out of text formatting. See [ADR 0007](decisions/0007-inline-text-formats.md).

## Initial layer integration

Document::image is the active layer raster; composite returns the visible stack.
Patch commands carry stable layer IDs. Stack/property commands retain bounded
before/after snapshots; both pixel and structural history share saved revisions.
Selecting a layer clears the transient selection without dirtying content. Layer
changes are disabled during gestures, previews and asynchronous operations. Hidden
layers cannot receive drawing or paste previews. Whole-image transforms process
every layer on a worker and commit the stack once; selected-content transforms
remain local to the active layer. Layered selection delete/move clears alpha.

The initial OpenRaster subset accepts a flat source-over stack of PNG layers,
including contained offsets. Unsupported groups, blend modes and off-canvas pixels
are rejected. Limits and the dependency choice are in ADR 0008. Recovery remains
planned. No full OpenRaster baseline or cross-application certification is claimed.

The bottom raster is the fixed Background in the thumbnail rail: the engine rejects
moving/deleting it or moving another layer below it. No codec extension is needed;
the lowest stored raster retains this role on reopen. Merge down may update its
pixels while preserving its identity. Dragging resolves the captured stable layer
ID on drop and commits one reorder command. Per-tile visibility changes retain the
active layer. The controller initializes a flat document's layers once per document
session; panel toggles after undo do not recreate a removed layer.

Color inversion uses the existing cancellable transform worker but remains an
active-layer pixel edit. The worker preserves alpha and clips to the selection;
the GUI thread checks revision/cancellation before committing one bounded patch.
It does not transform or flatten the stack, and rejects hidden layers or active edits.

## macOS appearance boundary

`MacGlass.mm` is the AppKit-only adapter. A non-interactive NSVisualEffectView sits
behind the Qt content view and follows window size, appearance preferences and
Reduce Transparency. All access stays on the GUI thread. The shared document,
commands, persistence and canvas remain C++/Qt; no pixels or edits live in the
adapter. BlurSupport attaches it only on Cocoa and never offers GNOME commands
there. Tests with injected desktop probes retain their isolated behavior.
