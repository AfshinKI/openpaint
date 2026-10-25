# Releases and in-app updates

The current stable release is v2.0.0, published 2026-10-22 UTC. Release and platform evidence are tracked in
[plan 0062](plans/0062-macos-v2-release.md); prior v1.3.0 evidence remains in
[plan 0057](plans/0057-release-1.3.0.md). Ordinary CI runs validate development
packages; explicit main-branch release runs publish only after every check passes.
No signing keys, OpenPaint update feed or automatic in-app installer exists.

## Distribution strategy

| Format | Initial audience | Update owner |
| --- | --- | --- |
| Portable tar.gz | Uninstalled GitHub download; extract and run | Manual replacement |
| Flatpak | Installed Linux app; GNOME and KDE | Manual bundle replacement; no OpenPaint remote yet |
| AppImage | Portable download from GitHub Releases | Manual replacement; historical alpha format |
| Source | Developers and distribution packagers | User or distribution package manager |
| DEB | Ubuntu 24.04 amd64 installer; validation in [plan 0030](plans/0030-debian-installer.md) | APT; manual download until an update repository exists |
| DMG | Apple silicon/macOS 14.4+; v2 validation in [plan 0062](plans/0062-macos-v2-release.md) | Manual app replacement; ad-hoc signed, not notarized |
| RPM | Deferred; Flatpak available for other distributions | Distribution/package manager |

The native release toolchain is pinned to Qt 6.12.0; [plan 0023](plans/0023-maintained-qt.md)
records the migration. Flatpak uses its maintained KDE runtime with exact commits
recorded per artifact.

Linux packages target x86_64. macOS targets Apple silicon, with its own native
build, editor and package checks. Other architectures need equivalent validation. M2 tests Ubuntu 24.04 and the then-supported stable
Fedora release; record exact versions in each release's matrix. Build AppImages
against the oldest supported ABI baseline. Release tests cover installed artifacts
on GNOME/KDE, Wayland/X11; an offscreen test is not proof of desktop integration.

Use `io.github.AfshinKI.OpenPaint` as the initial desktop/Flatpak identity, matching
the public repository namespace. This public project identifier is intentionally
not a private personal path. If the project moves to an organization, resolve the
permanent app ID before the first public package; migration afterward affects data
directories and update identity.

Flatpak starts as a tested manifest and downloadable test bundle. Flathub is the
preferred eventual stable channel, but publication requires its submission/review
process. A GitHub `.flatpak` bundle alone does not establish an update repository.
Until a remote exists, explain manual replacement accurately in the Updates screen.

## Version and artifact contract

Use semantic `MAJOR.MINOR.PATCH` versions, with prerelease suffixes such as
`0.1.0-alpha.1`. Tags are `v<version>`. Stable users receive stable releases only;
prerelease enrollment is explicit. Downgrades are never automatic.

A release contains versioned architecture-specific binaries, SHA-256 checksums,
matching source/build recipes, third-party notices, a software bill of materials,
release notes, supported systems, installation instructions, and known limitations.
Linux downloads are unsigned and the macOS bundle uses an ad-hoc signature;
checksums detect corruption but do not authenticate a release. Repository signing and signed update metadata remain future M5 work.

Binary paths/debug metadata must not include a developer's home directory; build in
controlled paths and use compiler prefix mapping as appropriate. Do not ship local
cache files, secrets, sample personal images, or signing credentials. Build once,
test the resulting artifacts, then promote those same bytes.

## Updates screen and package ownership

Installed version, source, and channel come from trusted build/package metadata.
Do not infer write authority merely because a file is writable. An unrecognized
installation gets a release-page link and manual instructions.

All builds provide an Updates screen and an explicit “View releases” action.
Network-capable builds check on startup, when Updates opens, and daily while running;
Flatpak retains browser-only checking because it has no network permission.
Automatic installation is not implemented. Do not show simulated
network results or claim “up to date” without a successful authoritative check.

Automatic checks default on, with one deferred startup attempt and at most daily
checks while running. Cooldowns and server backoff survive restart. The old automatic-check preference is ignored.
No background service. Describe the host contacted, the IP/
ordinary request metadata it will receive, in the privacy documentation. The editor
remains usable offline. Use backoff/timeouts and distinguish offline, rate limit,
invalid metadata, and no update. Do not render active HTML from release notes.

### Flatpak / distribution backend

Never replace files inside a Flatpak deployment or a system package. Hand off to
the desktop software manager through a tested integration where available; fall
back to copyable instructions for the actual install source. Do not invoke
`flatpak-spawn --host` or request broad host access just to self-update. A hosted
release version is not proof that the user's Flatpak remote has it yet.

Do not request passwords or run a shell with elevated privileges from the app.
Package-managed updates, signatures, transactions, and rollback remain the package
manager's responsibility. Show a “Managed by your software manager” state if the
sandbox cannot safely query available versions.

### Portable AppImage backend (M5)

Use a narrow C++ UpdateService with states matching [DESIGN.md](DESIGN.md). M5
must include a signed-metadata schema/spec, deterministic fixtures, verifier, and
adversarial tests before any download can be installed.

The intended signed manifest includes schema version, monotonically increasing
release sequence, semantic version/channel, architecture, minimum updater version,
release timestamp, expiry, release-notes URL, artifact HTTPS URL, exact byte length,
and SHA-256 digest. Sign the exact UTF-8 manifest bytes with an established Ed25519
implementation (planned: libsodium), using a detached signature. Do not invent
cryptography or depend on reserialized JSON matching the signed input.

Embed trusted public keys in the application. Downloaded keys are not trust roots.
Store the highest accepted sequence per channel, reject replays/downgrades, reject
expired or excessive-lifetime manifests, and give a clear clock-error message when
the system clock makes validity uncertain. A fresh installation cannot remember
history it never saw; short validity windows bound replay exposure. M5 specifies
the exact bounds, schema and time policy before implementation.

Key rotation requires a cross-signed transition or a release already trusted by
the old key, with an overlap period. Keep a separate offline recovery/root key and
document compromised-key recovery before enabling portable installation. An
unsigned web download is not an emergency bypass for failed verification.

1. Fetch bounded manifest and signature responses over HTTPS; verify signature
   before trusting URLs, fields, size, or version. Strictly validate schema/types,
   reject duplicate keys, and enforce known architectures/channels and bounds.
2. Allow only configured release hosts and documented GitHub artifact redirect
   hosts; reject non-HTTPS redirects and unexpected destinations. Pin this policy
   in tests; never execute server-supplied commands.
3. Download to a unique sibling temporary file, enforce byte/disk-space limits,
   verify exact size and digest, and mark executable only after verification.
   Support cancellation; interrupted downloads never become runnable updates.
4. Resolve the real portable launcher path and verify that it is the expected,
   user-owned, writable regular file. Reject unsafe symlink/path changes and shared
   install locations; otherwise provide a manual Save Download option.
5. “Restart to update” resolves Save/Discard/Cancel for **all** open documents.
   Save failure or Cancel cancels restart. An update helper coordinates shutdown,
   file locking, fsync, and atomic same-filesystem replacement after the app exits.
6. Keep the prior version as a backup. Require a startup-health handshake before
   cleaning the backup. On a launch failure restore the prior executable or offer
   explicit recovery instructions if automatic restoration fails. A health check
   must not overwrite settings/documents in an incompatible format.
7. Reopen only documents that were saved or deliberately checkpointed for recovery;
   never silently discard work or re-run a partially applied edit.

AppImage's documented update metadata/zsync facilities may optimize transfer
later, but they are not our authentication boundary. The official documentation
describes integrity and signature support separately; independently verify our
signed manifest and digest before installation.

Required failure tests: tampered signature/hash, wrong architecture, expired/replayed
manifest, unsafe redirect, oversized/truncated response, offline/rate-limited host,
full disk, permission failure, cancellation at each phase, path substitution,
multiple windows/processes, unsaved work, failed save, and failed startup/rollback.

## Release pipeline

Current CI runs repository checks, native builds/tests, formatting/QML checks,
portable/DEB source/inventory packaging, Ubuntu package lifecycle checks and Flatpak
sandbox packaging. M5 adds signed
publishing/update jobs.
Do not commit a workflow that pretends to build targets that do not exist.

Release sequence:

1. Verify feature gates, support matrix, changelog, notices, and source compatibility.
2. Create a versioned release candidate from reviewed `main`; run full tests and
   produce artifacts with pinned toolchains/dependencies.
3. Test install, launch, editing, save, uninstall, upgrade from the previous release,
   and updater failure cases on clean environments.
4. Sign artifacts/metadata in a protected release environment. Pull-request jobs
   have no signing secrets or write token. Pin Actions to full commit SHAs, use
   minimum permissions, and separate untrusted PR execution from publishing.
5. Review the GitHub draft release; publish the tested source and binaries; publish
   update metadata last so it never points to unavailable artifacts.
6. Verify a real fresh install and upgrade from the public endpoints. Record
   artifact digests and outcomes in release evidence.

Repository settings to enable explicitly: required CI on `main`, restricted force
push/deletion, dependency alerts, private vulnerability reporting, secret scanning
where available, and protected release environment. These are setup tasks, not
effects of committing YAML. A sole maintainer can configure review requirements
appropriately without pretending an independent reviewer exists.

Support policy: the latest stable minor line receives fixes; prereleases have no
stability promise. Never call a build supported
without its installation and update evidence.

## Stable publishing automation

In Actions → Repository checks → Run workflow, select `main` and enter the exact
CMake version (for example `1.0.0`) in `release_version`. Blank means validation
only. Tags and release titles are exactly `v<version>`; stable builds have no suffix.
PR/push builds share a `dev.<run number>` suffix across all formats and never publish.

The release job waits for repository checks, all three Linux native builds,
portable, DEB, Flatpak, macOS bundle/native tests and clean Ubuntu
install/upgrade/remove tests. It downloads artifacts
from that same run, verifies their checksums and source identities, then combines
format-specific sources/notices/inventories into one compliance archive. The six
public assets are DEB, Flatpak, portable tar.gz, macOS DMG,
sources-and-notices.tar and SHA256SUMS.

The job uploads to a draft first and then publishes it as stable/latest. A failed
upload leaves a draft; rerun the same source commit to resume. Existing published
assets are never overwritten. An existing tag must resolve to the tested commit.
Concurrent promotions of the same version are serialized. Release write permissions
are confined to this job; no PR has a publishing token or signing secret.

Before dispatching a stable version, record desktop install/draw/save/reopen tests
and known limitations in the release plan. The automated pipeline does not replace
manual desktop acceptance. Full GNOME/KDE, Wayland/X11 and input/accessibility
certification remains future work; report exact tested environments.

Release notification implementation and limits: [plan 0037](plans/0037-release-notifications.md) and [ADR 0004](decisions/0004-release-checks.md).

## Project website

The static product page and GitHub Pages publishing instructions are in
[website/README.md](../website/README.md). Its separate workflow stages selected
public assets and deploys only main-branch website changes; PRs only build.
Website publication does not publish application releases or change the updater.

### Website and README release checklist

Before publication of every version:

1. Compare page claims, download instructions and README with the release build.
   Keep planned features explicitly separate from shipped features.
2. Capture light and dark mode plus every illustrated control that changed. Use
   generated artwork and isolated settings. For glass images, enable glass and set
   Surface opacity to 40% in both themes. Use an original or redistributable
   backdrop in a staged region; exclude personal windows, desktop icons, recent
   paths and metadata. Preserve open artwork and restore any desktop changes.
   Crop to the app window without surrounding desktop; use transparent rounded
   outer corners and record the crop/mask treatment. Keep the captured glass
   appearance intact; do not paint over controls or synthesize application UI.
3. Name images with the release version. Record the application source commit,
   capture method, theme and rendering conditions in the asset provenance notes.
   Do not claim harness captures came from the installed executable.
4. Update website/release.json, website image references, captions and README
   together. Run the site build and check desktop/mobile layouts and links.
   Repository CI rejects a website release version that differs from CMake.
   This guard does not replace visual review of the images.
5. Merge through protected review, publish the application, and verify the GitHub
   Pages workflow and public page show the intended version and working downloads.
   Record deployment evidence in the release plan; a commit alone is not proof
   of publication. Refresh or clearly label any older screenshot retained in docs.

Development icon update: the original Open palette light SVG is the installed
launcher fallback. The running application selects light/dark artwork from its
Light/Dark/System preference. Launcher/dock adoption of window icons depends on
the desktop and must be checked during package QA.

Development Enable Glass provides optional setup instructions on the validated
Ubuntu 24.04 / GNOME 46 / X11 path, gated by local package/extension evidence.
Other setups use opaque surfaces silently. No installer or extension is bundled
or executed by OpenPaint; commands require explicit user copying and execution.
See [implementation and remaining package validation](plans/0054-blur-onboarding-design.md).

### Runtime dependency review (2026-10-20)

The upstream continuous runtime asset was replaced, and the checksum gate rejected
it in PR #52. The binary and matching source pins now use type2-runtime commit
`fedfad51acc2860b2916a1eef32ba4a500ff31ef`. Its only change from the previous pin
is build-path normalization for libfuse/squashfuse debug information. Extraction
permissions remain 0700 and dependency versions are unchanged. The downloaded
binary digest was checked against GitHub's published asset digest; the source
archive was downloaded and hashed independently. Continuous URLs can move again;
checksum mismatches must continue to stop packaging, never trigger automatic trust.

Release evidence: [v2](plans/0062-macos-v2-release.md),
[prior v1.3.0](plans/0057-release-1.3.0.md).

## v2 macOS release lane

The `macos-15` Apple silicon runner builds Qt 6.12.0 for macOS 14.4+, runs the
editor suite and native Cocoa workflows, then audits a self-contained application
bundle and creates `OpenPaint-<version>-macos-arm64.dmg`. Release promotion now
requires this job alongside all existing Linux gates. The public release contains
six assets: DEB, Flatpak, portable Linux archive, macOS DMG, combined
sources-and-notices archive and SHA256SUMS. The Mac compliance archive is included
inside the combined archive, with the DMG hash and source revision in its inventory.

The current signing mode is ad-hoc, not Developer ID/notarized. State that on the
download page and in release notes. Never claim Gatekeeper certification or
physical Mac visual validation from offscreen/hosted tests. Native macOS glass
uses AppKit and needs no Linux extension command. Intel Mac packaging is deferred.

### Skip one release (v2.1 development)

Updates → Skip this version remembers the current available release locally and
suppresses its startup notice across launches. Checks continue; newer versions
notify normally. Opening Updates manually still offers the skipped release’s
GitHub download link. See [plan 0071](plans/0071-skip-update-version.md).
