# Feature: 40% default glass opacity

Status: implemented; local checks passed; PR deferred
Feature IDs: F01

## Acceptance

- Fresh settings and Restore defaults use enabled glass at 40%.
- Existing saved opacity and theme choices survive restart.
- Bounds remain 40–100%; artwork/history are unchanged.

## Validation

Dev build, appearancePersistenceAndBounds and glassAppearanceSettings passed
with Qt 6.12/offscreen software. Saved 62% survives restart; reset yields 40%.
The shell test verifies opacity adjustment, opaque mode and unchanged artwork.
Next: later PR review; no release dispatched.
