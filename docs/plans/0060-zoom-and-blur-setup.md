# Feature: zoom slider and reliable blur setup progress

Status: implemented and locally validated
Feature IDs: F11; appearance setup
Milestone: development polish

## User outcome and scope

Restore the original monochrome Image icons, including distinct 90°/180° symbols.
Add a bottom-right zoom slider alongside percentage, minus/plus and fit controls.
Replace the repeated package-install guidance with one copyable setup block.
Continue the maintainer-requested single branch; no release or desktop changes.

## Behavior and implementation

The slider shares the existing canvas zoom operation and 1–3200% range, with
logarithmic travel for usable low-zoom precision. Keyboard/wheel/fit changes keep
its position and percentage synchronized. Zoom never changes saved pixels.

Blur setup keeps the verified Ubuntu 24.04/GNOME 46/X11 scope. The app only probes
local configuration and copies a setup block; it never runs it. At the maintainer's
request, the user-run Python block installs a missing Blur My Shell from GNOME,
enables user extensions and configures application blur without sudo or Extension
Manager. It backs up touched settings, preserves unrelated list entries, and does
not replace an existing extension. A newly discovered extension may require a
sign-out; the script never forces one. Configuration evidence still gates Glass.

Download: GNOME Extensions version 76, version_tag 76770, SHA-256
`0dfde0d3575b4c7e32647e94010fa37f15ac933f4fe61c46bfe200e772114d85`.
The official archive and GNOME 46 metadata/schema were inspected without installing
it. The script verifies the pinned hash and a 2 MiB download bound before using
GNOME's native extension installer. No extension code is bundled in OpenPaint.
Source: [GNOME listing](https://extensions.gnome.org/extension/3193/blur-my-shell/).

Backup is `$XDG_STATE_HOME/openpaint/blur-setup-backup.json` (default
`~/.local/state/openpaint/blur-setup-backup.json`), retained on repeat runs. It records
application settings and GNOME's enabled/disabled extension state before changes.
The setup changes extension-wide application opacity/dynamic opacity, enables user
extensions, and removes exclusions matching OpenPaint; other applications using
the extension may reflect those settings.

## Acceptance cases

- [x] Previous Image icons restored; colored Tools icons retained.
- [x] Slider pointer/keyboard changes zoom; other zoom controls keep it synchronized.
- [x] Minimum window layout, endpoints and document immutability hold.
- [x] One copyable block handles missing or existing extensions without an apt-install loop.
- [x] Check again observes fresh setup stages and ready state; failures do not claim support.

## Validation and evidence

Build, formatting (95 files), zero-warning QML lint and repository checks pass.
All 20 CTest suites and 35 Python tooling tests pass. The final resource revision
also passes 8 blur unit cases, 6 focused offscreen UI cases and 4 X11/OpenGL
UI cases (counts include harness initialization/cleanup). Script tests mock every
download and desktop mutation; no setup was run against this desktop.
Development captures: zoom-slider-1024.png and glass-one-block-setup.png.
The other machine's compositor is not accessible; real installation/blur remains
to be checked there.
Reference: [Microsoft zoom improvements](https://blogs.windows.com/windows-insider/2023/06/01/updated-paint-app-for-windows-11-begins-rolling-out-to-windows-insiders-with-dark-mode-and-zoom-improvements/).

## Progress and handoff

Local implementation and validation complete. Publish to existing PR #56; next
step is user verification of the setup on the other Ubuntu machine. No merge or release.
