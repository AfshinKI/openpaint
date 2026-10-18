# Privacy and public-repository hygiene

These are product requirements. The editor keeps core editing offline. Explicit or opt-in release checks contact
api.github.com; see the release-check disclosure below. It opens local PNG/JPEG/BMP/WebP images and saves PNG
files or exports PNG/JPEG/BMP/WebP copies, normalizes EXIF orientation and strips source metadata. File → Updates opens the public GitHub Releases page only after the explicit View releases action. The browser makes an ordinary request to GitHub; no artwork or document information is included. Recovery remains planned; bounded release notifications are implemented.

## Current local settings

Custom colors are session-only: up to 10 ribbon colors and 24 editor colors stay
in memory and reset on launch. No palette settings are read or written. Older
palette files, if present from a prior build, are ignored and left untouched.
Remove selected and Clear all remain available for the editor's session list.

## Product commitments

Core editing works without a network connection, account, analytics, advertising,
or telemetry. No automatic crash uploads. No unique installation identifier.
Recent files, settings, and recovery artwork remain in local XDG storage, with
visible controls to clear them. Do not include document paths or contents in logs.

Manual update checks contact the configured release host. Optional daily checks
are disabled by default; enabling them discloses the destination and ordinary
network metadata involved. A web host can see an IP address even when the app sends
no personal fields. Diagnostics are opt-in exports that users can preview and edit.

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

Check now sends a bounded HTTPS request to api.github.com. GitHub receives the IP
address and ordinary connection metadata with a fixed application User-Agent; no
artwork, paths, credentials or installation identifier is sent. Automatic checks
are off by default, at most daily while the app runs, and can be disabled in Updates.
Local XDG settings retain consent and request/backoff timestamps. Browser navigation
requires an explicit action. Flatpak retains no network permission and manual links.

Theme choice, glass enablement and opacity persist locally in XDG configuration. These preferences contain no artwork or identifiers and cause no network requests.
