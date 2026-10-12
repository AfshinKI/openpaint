# Feature: keyboard and desktop file workflow

Status: implemented; protected merge requires CI success
Feature IDs: F01, F15, F16, F26
Milestone: M1

## Outcome and scope

A keyboard user can reach editing controls, save generated artwork, reopen it,
and cancel or confirm unsaved changes. Controls expose useful accessible names,
roles and focus state. Validate the actual desktop file portal separately from
the deterministic Qt fallback. No new editing tools or packaging claims.

## Acceptance

- [x] Tab navigation reaches pencil, colors and size; Space/arrow keys operate them.
- [x] Ctrl+S, typed filename, New, Ctrl+O and keyboard selection preserve exact pixels.
- [x] Escape cancels file/unsaved dialogs and restores usable focus; save/discard
      choices work from the keyboard without silently losing edits.
- [x] Qt accessibility reports names/roles for actionable controls and the canvas.
- [x] Native portal save/open/cancel is exercised with synthetic files only;
      replacement preserves content or reports failure without loss.
- [x] Debug, sanitizer, X11 scaling, formatting and documentation checks pass.

## Evidence and handoff

Qt 6.8.3 / Ubuntu 24.04: all seven Debug and ASan/UBSan suites pass with
leak detection. All 17 shell results pass on isolated X11/software at 100%, 125%,
150% and 200% scale. The optional native-portal test is exercised separately by
`python3 tools/validate_portal.py`; its save, overwrite, reopen and cancel checks
passed against the GNOME FileChooser portal version 3. Only temporary synthetic
artwork and unique test dialog titles are used.

Dialog focus is explicit, Ctrl+Q works even when the platform supplies no default
Quit binding, Ctrl+Y and Ctrl+Shift+Z both redo on every platform, and CLI help
now describes PNG support. Keyboard tests wait for
the unsaved prompt’s closed signal before opening another dialog; transient
window activation alone is not sufficient during popup animations. Rapid popup
handoffs remain a desktop polish follow-up.

Automated keyboard/accessible-tree evidence is distinct
from human screen-reader usability certification, which remains M4. Portal tests
on an unpackaged app do not certify sandboxed Flatpak document grants (M2/M5).
Next: complete startup/stroke/viewport/memory measurements and close the M1 ledger.
