# Feature: skip one update version

Status: implemented; local validation passed; protected review pending
Feature IDs: F01
Target: v2.1.0

## Outcome and acceptance

Updates offers “Skip this version” for an available stable release. Remember the
exact version locally across restarts and hide its startup notice immediately.
Continue checking for newer releases; a later version notifies normally. Manually
opening Updates still shows the skipped release and its download link. Skipping
does not disable checks, download anything, restart, or touch artwork.

The button is disabled during checks or after skipping. Failed settings writes
show an error and do not claim success. Existing cooldown and metadata validation
remain unchanged. QSettings stores only the version string; nothing is transmitted.

## Implementation and validation

UpdateService owns the persisted preference and notification filter. QML binds the
notice and button to skipped state. Fake-network tests cover persistence, repeated
checks, later releases, manual availability and no-op before metadata is verified.
Combined Qt 6.12.0 Debug build, formatting and zero-warning QML lint pass.
CTest updates and shell suites pass (32.92 seconds). New fake-network cases verify
actual deferred startup after restart, repeated manual checks, a later release,
manual availability and a failed settings write. Repository checks and diff hygiene
pass. No live-network calls or user settings were used in these tests.
Next: protected PR review and required hosted checks.
