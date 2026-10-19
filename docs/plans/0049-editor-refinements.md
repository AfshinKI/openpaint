# Feature batch: editor refinements before v2

Status: seven slices and requested follow-up refinements implemented; local checks passed
Feature IDs: F01, F07, F25

## Scope and sequence

The maintainer requested these seven refinements, one feature at a time, with
local acceptance before later PR/release runs. Each slice has an independent
branch based on current origin/main and its own acceptance/evidence record.
The `feat/editor-refinements` integration branch combines them with the prior
release-documentation closure;
it records the validated development batch, not authorization to publish v2.

| Order | Outcome | Plan | Feature branch |
| --- | --- | --- | --- |
| 1 | Glass defaults to 40%, retaining saved choices | [0042](0042-glass-default.md) | `feat/glass-default-opacity` |
| 2 | Searchable in-app font family previews | [0043](0043-font-picker.md) | `feat/font-picker` |
| 3 | Dashed text box and blinking black caret | [0044](0044-text-outline.md) | `feat/text-editing-outline` |
| 4 | Default-on startup/daily release checks | [0045](0045-startup-updates.md) | `feat/startup-update-checks` |
| 5 | Rounded normal-window client corners | [0046](0046-rounded-window.md) | `feat/rounded-window` |
| 6 | Neutral frosted selected controls with light outlines | [0047](0047-frosted-selection.md) | `feat/frosted-tool-selection` |
| 7 | Designer attribution in Updates | [0048](0048-designer-credit.md) | `feat/designer-credit` |

## Combined acceptance and evidence

- [x] Seven focused feature checks pass before integration.
- [x] Combined dev build, format-check and strict QML lint pass.
- [x] All 15 CTest suites pass on Qt 6.12/offscreen software (64.40 seconds).
- [x] Repository checks, all 27 tooling tests and whitespace checks pass.
- [x] Final font Apply-button check and OpenGL visual/input checks; see [0050](0050-appearance-refinements.md).

Independent selected-material tests also passed on X11/OpenGL. Captures use only
synthetic artwork in ignored build/evidence-v2 directories, not personal files.
The font picker uses native font rendering within the shared dialog; the font
inventory still depends on fonts installed in the host or sandbox.

## Limits and handoff

No PR, CI dispatch, package installation, release version bump or publication was
performed. Installed-package, Wayland and full accessibility certification remain
later checks. Flatpak retains its no-network permission and manual browser update
path. Explicit existing update opt-outs and glass opacity preferences are retained.
The compositor still owns native title bars and desktop blur; the new corner
geometry covers the application client area. The existing eraser ribbon can
overflow its palette at 1024px width; track this separately from selected styling.

The selected application icon and subsequent spline/eraser refinements are complete
with local evidence in [0052](0052-icon-and-spline.md). Next: maintainer local review
before scoped PR/CI and v2 release preparation. M3 layers/project/recovery remain
outside this UI batch.
