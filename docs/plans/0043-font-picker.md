# Feature: consistent searchable font picker

Status: implemented; local checks passed; PR deferred
Feature IDs: F07

## Scope and acceptance

Replace the platform-dependent native FontDialog with a shared PaintDialog.
Show installed font families, searchable by name, with samples rendered in each font.
Arrow keys browse, Space selects, Enter applies; Cancel/Escape preserve the draft.
Apply changes only family, retaining size/style and returning focus to inline text.
No artwork commits, network access or bundled fonts. Installed font inventories may
still differ between hosts/sandboxes, but the picker layout is shared.

- [x] Search, empty results, keyboard selection and Apply/Cancel work.
- [x] Text stays active; size/style and committed image remain unchanged.
- [x] Light/dark minimum-size captures, build, lint and targeted text tests pass.

Qt 6.12 dev build, strict QML lint, fontPickerWorkflow and inlineTextWorkflow
passed. Inspected light/dark 1024×640 captures with generated text. Fixed row
clipping for fonts with tall metrics. Installed package execution is not retested;
the platform font dialog is removed, but installed font availability is host-specific.
Next: later PR and package validation.
