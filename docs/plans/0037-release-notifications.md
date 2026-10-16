# Feature: GitHub stable-release notifications

Status: implemented; PR review pending
Feature IDs: F25
Milestone: post-v1 notifications (not automatic installation)

## User outcome and behavior

Updates offers Check now, cancellation, an optional daily check (off by default),
and an explicit browser action. A newer stable version shows a dismissible in-app
notice without interrupting drawing. No download, installation, restart or shell
command is performed. GitHub availability is separate from package availability.
Portable/DEB packaging includes Qt TLS and OpenSSL libraries with the existing
inventory/source-license workflow; host CA certificates remain the trust store.
The current Flatpak retains its no-network sandbox and browser/manual workflow.

Checks contact only api.github.com over HTTPS with ordinary network metadata and
a fixed application User-Agent. No token, unique ID, document information or artwork
is sent. Local XDG settings store opt-in and request/backoff timestamps. Disabling
checks cancels any pending request. No background service exists.

## Implementation and boundaries

A GUI-thread UpdateService owns bounded Qt Network requests and an hourly timer.
Automatic attempts are at least 24 hours apart, including across app restarts;
manual attempts have a 60-second cooldown. Rate limits defer according to Retry-After
and rate-limit reset headers, with at least an hour fallback. Responses cap at
256 KiB and 15 seconds. Redirects are rejected; TLS validation remains enabled.
Only a valid stable vMAJOR.MINOR.PATCH release is compared numerically. Preview
builds can discover a stable release of the same base version. Release links are
constructed from a fixed project URL and validated version, never server URLs.
Release notes/HTML are not rendered. Errors never report an unverified up-to-date
state. Full signed installation stays a separate M5 feature.

## Acceptance cases

- [x] Default-off produces no automatic requests in tests; Flatpak compile-time
  policy preserves the existing no-network sandbox (packaged runtime check pending).
- [x] Fake transport verifies request URL/headers, newer/equal/older versions,
  malformed/prerelease metadata, redirects, rate limits, oversize and failures.
- [x] Cancellation and stale responses cannot change state; daily scheduling and
  persisted cooldown survive restart using a fake clock.
- [x] Updates controls fit minimum size and artwork/history remain unchanged.
- [x] Build, tests, formatting, lint and repo/privacy checks pass.

## Validation and handoff

Dev build, repository checks, format-check/qml-lint, updates/shell/shell-more
CTest suites and all 26 Python tooling tests passed. Inspected the 1024px dark
Updates screenshot. A standalone probe using the actual UpdateService successfully
queried GitHub over HTTPS and recognized v1.0.0 as newer than the local prerelease.
The network-restricted sandbox correctly produced an error; the approved external
request succeeded. No token or artwork was used.

Packaged TLS/CA validation and real-desktop checks remain outstanding; packaging
CI must pass before merge. Next: protected PR review. Independent of glass/editing PRs.

A temporary integration tree combining PRs #41–#45 passed all 14 CTest suites.
Mechanical merge resolutions retained both new canvas tests, both QML service/dialog
declarations and both CMake source lists; appended documentation was combined.
Final release-check review added fail-closed malformed consent and preserved rate
backoff on oversized responses, covered by focused tests.
