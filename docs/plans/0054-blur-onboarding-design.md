# Feature: welcome and glass setup

Status: implemented locally; revised workflow and visual checks passed.

## User outcome and scope

A one-time Welcome to OpenPaint introduces the app and invites contributions.
Enable Glass is a separate, on-demand setup panel opened from Appearance or
Updates. Both use real app pictures. This supersedes the original combined
[design prototype](../designs/blur-onboarding.html). Work remains on the current
branch at the maintainer's request.

## Behavior and acceptance

- [x] Welcome appears once regardless of blur support, waits for idle drawing and
  other popups, and offers Start creating, Escape and Get involved. Closing saves
  `onboarding/welcomeRevision=1`; restoring appearance defaults does not reset it.
- [x] Get involved opens the fixed public contribution-guide URL only after a click.
  No document or system data is sent by the app.
- [x] Without requirements, the glass checkbox is unchecked and disabled. The
  opacity slider is disabled too. Saved glass intent/opacity remain unchanged.
- [x] Enable Glass opens setup from Appearance and Updates, including on unknown
  systems. Setup never appears automatically and has no dismissal preference.
- [x] Real corner pictures have rounded edges and soft shadows. Short instructions
  and an 11px selectable monospace command are shown only for a known setup route.
- [x] Copy writes the exact command without executing it. Check again refreshes
  support; Use Glass appears only when ready and explicitly enables the preference.
- [x] Unknown/missing dependencies use the opaque appearance without warnings or
  guessed commands. The on-demand panel remains accessible for its comparison.
- [x] Closing setup returns focus to its opener; drawing and saved pixels survive
  every flow. Light/dark 1024×640 layouts keep actions available.

## Implementation and portability

BlurSupport uses bounded, read-only local probes on a worker; a GUI-owned busy
flag stays set until results are applied. Current validated recipe scope remains
Ubuntu 24.04 / GNOME 46 / X11. Extension Manager is offered only with a cached
APT package candidate; installed Blur My Shell receives enable/preferences guidance.
Configuration evidence requires an active extension and application blur enabled
for OpenPaint (or all applications), using the extension’s inclusion/exclusion mode and application patterns.

No one install command is portable across every Linux desktop. Unknown systems,
missing tools, sandboxes and failed probes therefore receive no command and use
opaque rendering safely. OpenPaint never executes copied commands, installs
packages, escapes a sandbox or alters desktop settings. See [ADR 0006](../decisions/0006-desktop-blur-guidance.md).

Appearance owns saved intent and the separate welcome revision. BlurSupport owns
capability evidence and explicit clipboard access, with no persisted setup state.
ScreenshotPreview draws bundled screenshots with QPainter rounded clipping and a
small bounded shadow. This works with Qt's software renderer without shader or
compositor dependencies. Artwork pixels are never changed by these views.

Updates preserves version, attribution and a concise privacy disclosure. It checks
at startup and on opening, respecting cooldown/backoff, without a check button or checkbox. Enable Glass appears in its footer only while glass is unavailable. Local capability checks on opening
Updates are independent of release networking.

Image [provenance](../assets/README.md) and [embedded preview notes](../../qml/previews/README.md)
identify historical release and development captures. No image is relabeled as a
new release or proof of native compositor blur. The comparison says Glass appearance.

## Validation

Prior implementation: full 16-suite CTest run and 27 Python tooling tests passed.
The revised implementation adds separate welcome and glass-setup workflows,
contribution URL coverage and welcome persistence checks. Final results follow
below. Captures use isolated settings and generated artwork only; platform
results are injected, so screenshots verify UI rather than package installation.

## Progress and handoff

- [x] Approved actual app images, opaque fallback and separate welcome/setup design.
- [x] Implement native flows and renderer-independent preview presentation.
- [x] Revised workflow, lint and visual checks recorded below.
- [ ] Next release gate: fresh installed-package extension setup/recheck validation,
  then release images/version and protected PR/release workflow.

Release number is unchanged. KDE, Wayland and unverified versions use the opaque
fallback. Broader desktop support requires recorded evidence; universal blur
installation is not promised.


Revised-flow validation (2026-10-19): development build succeeded; formatting
(86 C++/QML files), zero-warning QML lint, repository checks and whitespace checks
passed. Native UI captures were visually reviewed in light/dark at 1024×640:
`build/blur-evidence/welcome-light-1024.png`, `welcome-dark-1024.png`,
`enable-glass-light-1024.png`, `enable-glass-dark-1024.png`,
`glass-settings-disabled-1024.png`, and `enable-glass-fallback-1024.png`.
The screenshots confirm that rounded clipping and shadows work on the software
renderer; no new real-compositor or installed-package certification is claimed.
Final revised `ctest --preset dev`: all 16 suites passed (77.62 seconds), including
welcome persistence, contribution navigation, disabled glass controls, setup
reopening, clipboard/recheck, unsupported fallback and document preservation.

## Updates simplification acceptance (2026-10-19)

- Startup and opening Updates check automatically, retaining rate-limit backoff.
- Repeated openings during a request/cooldown preserve the existing result.
- No checkbox, Check now or Cancel check in the panel; no automatic installation.
- Up-to-date status requires a verified response; errors never imply success.
- Compact left-hand status/action and a rounded, angled actual-app corner card.
- Preserve attribution, Enable Glass access, keyboard navigation and artwork.

Revision validation: development build, formatting (86 files), zero-warning QML
lint, repository and whitespace checks passed. All 16 CTest suites passed
(77.38 seconds). Targeted coverage checks startup/cooldown, legacy preference
migration, repeated opening, valid-current versus offline status, removed controls,
keyboard release navigation and preserved artwork. Light/dark Updates captures at
1024×640 were visually reviewed in `build/blur-evidence/updates-light-1024.png`
and `updates-dark-1024.png`. Their waiting status is intentional: shell captures
disable networking in memory; updater tests use fake transport. Native compositor
and clean-installed-package validation remain the release gate noted above.

## Glass detection correction

Acceptance: an active extension in inclusion mode with OpenPaint allowed must not
be rejected because unrelated applications are excluded. Exclusion mode must
reject matching OpenPaint patterns and failed settings reads.

The local extension implementation (Blur My Shell 75) confirmed mutually exclusive
inclusion/exclusion modes and case-insensitive `*`/`?` patterns. Detection now follows
those rules for the X11 OpenPaint window class; unsupported list syntax fails closed.
Read-only desktop inspection confirmed active extension/application blur, OpenPaint
inclusion and unrelated exclusions. No desktop settings were changed.

Validation: development build, blur-support regression suite and glassSetupWorkflow
passed; formatting, QML lint, repository and whitespace checks passed. The UI test
used the software renderer; the corrected running window still needs visual
confirmation on the desktop. Launch the rebuilt development executable to use the
fix; already installed binaries are unchanged.

Release follow-up: real read-only desktop probing returned ready on Ubuntu 24.04 / GNOME 46 / X11. Qt 6.12 native glass workflow and color-dialog checks passed. Enable Glass hides on readiness and focus returns to Done. Fresh extension installation on a clean desktop remains untested; package lifecycle and release evidence are tracked in [v1.3.0](0057-release-1.3.0.md).
