# Feature: default-on startup release checks

Status: implemented; local checks passed; PR deferred
Feature IDs: F25

## Behavior and acceptance

Network-capable builds default to automatic release notifications. A deferred
check runs 1.5 seconds after startup, even when a previous launch checked that day,
unless the persisted cooldown/server backoff blocks it. While running, automatic
checks remain at most daily. An explicit saved opt-out or malformed preference
stays offline. No automatic installation, restart, artwork upload or telemetry.
Flatpak keeps its existing no-network permission and browser-only update path.

- [x] Fresh settings check after startup without blocking the editor.
- [x] Rapid relaunch and server backoff suppress requests; later startup can check.
- [x] Opt-out persists and cancels requests; fake transport covers network failures.
- [x] UI/privacy/release contracts disclose the new default and endpoint.

## Validation

Qt 6.12 dev build, all 18 updater test results and updatesWorkflow passed.
The added startup/server-backoff assertion passed in a focused rerun. Shell tests
persist an explicit opt-out before loading QML, so they never make real requests.
No package installation or live network certification is claimed.
Next: later PR/package review.
