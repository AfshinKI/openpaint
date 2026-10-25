# Privacy and public-repository hygiene

These are product requirements. The editor keeps core editing offline. Manual and default-on startup/daily release checks contact
api.github.com; see the release-check disclosure below. It opens local PNG/JPEG/BMP/WebP images and bounded OpenRaster projects, saves PNG
or OpenRaster files, and exports PNG/JPEG/BMP/WebP copies, normalizes EXIF orientation and strips source metadata. File → Updates opens the public GitHub Releases page only after the explicit View releases action. The browser makes an ordinary request to GitHub; no artwork or document information is included. Recovery remains planned; bounded release notifications are implemented.

## Current local settings

Custom colors are session-only: up to 10 ribbon colors and 24 editor colors stay
in memory and reset on launch. No palette settings are read or written. Older
palette files, if present from a prior build, are ignored and left untouched.
Remove selected and Clear all remain available for the editor's session list.

## Product commitments

Core editing works without a network connection, account, analytics, advertising,
or telemetry. No automatic crash uploads. No unique installation identifier.
Settings remain in local platform storage. Recent-file management, recovery
artwork and visible clearing controls are future requirements, not shipped features. Do not include document paths or contents in logs.

Manual and automatic update checks contact api.github.com. Automatic checks are
enabled by default on network-capable builds, at startup and daily while running;
Opening Updates also checks, subject to cooldown/backoff. Updates discloses the destination. The former checkbox and saved opt-out are no longer used. A web host can see an IP address even when the app sends
no personal fields. Any future diagnostics export must be opt-in and previewable/editable; no such export is shipped.

Export strips author/device/GPS/text metadata by default. Color-profile conversion
and required orientation handling are separate from personal metadata removal.
Do not copy arbitrary EXIF/XMP chunks during save. Fixtures must test this behavior.

Local AI model downloads require explicit initiation and show size, source,
license, and storage location. Models are never downloaded silently. If a future
cloud provider is added, preview what is sent, identify the provider, and require
an explicit action for each upload. Tokens belong in the desktop secret store,
never project files or plaintext configuration. No provider is selected yet.

## What must stay out of the repository

- Access tokens, passwords, keys, session cookies, `.env` files, signing material.
- Home-directory paths, private hostnames, device serials, personal email addresses.
- Personal photos, screenshots of private desktops, real documents, raw crash dumps.
- Local AI conversation history, machine-specific preferences, shell history,
  recent-file lists, build output, and extracted metadata from private files.

Use procedural fixtures with a fixed seed, generic filenames, and documented
licenses. Review screenshot metadata as well as visible content. Keep working
scratch data in ignored directories. An ignore rule does not untrack an already
committed file; inspect Git history before initial publication.

## Intentional public identity

The GitHub repository URL, owner handle, and derived application ID identify the
public project and are allowed. Contributor attribution and third-party license
notices remain intact. The policy excludes private data, not necessary provenance.
Commit author names and GitHub activity can be public; use your chosen public name
and GitHub noreply email before creating commits. Do not invent someone else's
identity or assume GitHub hides existing commit metadata retroactively.

## Checks and limits

Run `python3 tools/check_repo.py` before publication and inspect the staged diff.
The current script catches selected high-confidence token/private-key patterns,
home paths, private email literals, and prohibited filenames in tracked content
(or the candidate tree before Git initialization). It reports locations without
printing matched secret values. It is a lightweight check, not a comprehensive
secret scanner; it cannot inspect private data inside images or prove anonymity.

Before the first binary release, add a pinned established secret-scanning tool to
the release audit, review history and build artifacts, and manually inspect all
redistributed media. CI, host security settings, and human review complement each
other. Never upload suspected secrets to a third-party checker to validate them.

If private data is exposed, revoke affected credentials first, remove the material,
and coordinate history cleanup. A deletion commit alone does not erase history,
forks, caches, or previously downloaded copies.

## Release-check disclosure

Startup and opening Updates send a bounded HTTPS request to api.github.com. GitHub receives the IP
address and ordinary connection metadata with a fixed application User-Agent; no
artwork, paths, credentials or installation identifier is sent. Automatic checks
default on for network-capable builds: once per startup after a short delay, then
at most daily while running. Persisted cooldown/server backoff still applies.
There is no check button or checkbox. Local platform settings retain
request/backoff timestamps. Browser navigation
requires an explicit action. Flatpak retains no network permission and manual links.

Theme choice, glass enablement and opacity persist locally in platform configuration. These preferences contain no artwork or identifiers and cause no network requests.

## Public website screenshots

Release screenshots follow the [website checklist](RELEASES.md#website-and-readme-release-checklist).
Use generated artwork with no private content and record the capture method.
Inspect visible text and PNG metadata before committing. Use a separate installed
app instance with isolated settings for release marketing captures. Crop to the
app and deliberately staged original backdrop; never include private desktop
content or interrupt a user's unsaved document. A source harness is allowed for
clearly labeled test evidence, but must not be presented as an installed capture.

Linux blur onboarding reads local desktop/version, extension settings and
extension activation/configuration without networking. The setup command is copied only
on request; OpenPaint never runs it or changes desktop settings. A local welcome
revision records dismissal. No capability results or command output are logged.

The separate Welcome to OpenPaint stores only its local dismissal revision. Get
involved opens the public contribution guide on GitHub only after a click; the
browser sends ordinary connection metadata. Glass setup is now on demand and
stores no separate dismissal state.

The optional Copy setup block performs network and desktop changes only when the
user pastes and runs it in their terminal. It downloads a pinned/checksummed Blur
My Shell archive from extensions.gnome.org only if missing, enables GNOME user
extensions and configures application blur. GNOME receives ordinary download
connection metadata; no artwork or app history is sent. Previous settings are
backed up locally. OpenPaint itself never executes the block or installs software.

The copied glass setup selects live application blur. Where the extension exposes
`hacks-level`, it disables clipped redraws through the no-artifact setting; newer
schemas without that key retain their own redraw handling. This affects the whole compositor
and may increase GPU use or reduce desktop performance; the setup panel discloses
this before copying. Original settings are backed up before changes. OpenPaint
does not capture desktop pixels or execute this setup itself.

About offers explicit browser actions for the public project website and GitHub
Issues contact page. Opening About itself makes no request. External links carry
no artwork or document metadata; the browser supplies ordinary connection metadata.
