# Releases and in-app updates

Status: implementation plan. No packages, update feed, signing keys, or release
automation exist yet. Do not add a Download button or an enabled self-updater
until a corresponding artifact and verification path have been tested.

## Distribution strategy

| Format | Initial audience | Update owner |
| --- | --- | --- |
| Flatpak | Recommended installed Linux app; GNOME and KDE | Configured Flatpak repository and desktop software manager |
| AppImage | Portable download from GitHub Releases | OpenPaint's verified portable updater from M5 |
| Source | Developers and distribution packagers | User or distribution package manager |
| deb/rpm | Deferred; add only with maintenance capacity | Distribution/package manager; never overwrite from inside the app |

Start x86_64. Add aarch64 only after native build, install, drawing, save, and update
tests on that architecture. M2 tests Ubuntu 24.04 and the then-supported stable
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
Flatpak commits use repository signing. AppImage update metadata has a separate
verification signature; checksums alone do not authenticate a release.

Binary paths/debug metadata must not include a developer's home directory; build in
controlled paths and use compiler prefix mapping as appropriate. Do not ship local
cache files, secrets, sample personal images, or signing credentials. Build once,
test the resulting artifacts, then promote those same bytes.

## Updates screen and package ownership

Installed version, source, and channel come from trusted build/package metadata.
Do not infer write authority merely because a file is writable. An unrecognized
installation gets a release-page link and manual instructions.

All builds provide Help/Settings → Updates with a manual Check action and release
notes. Initial alpha builds use a clearly labeled “View releases” action and report
that automatic checking/installation is not implemented. Do not show simulated
network results or claim “up to date” without a successful authoritative check.

Automatic checks are off by default. An opt-in permits at most a daily check while
the app is running. No background service. Describe the host contacted, the IP/
ordinary request metadata it will receive, and how to turn checks off. The editor
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

Current CI runs repository checks only. M1 adds application build/unit tests and
format/static checks; M2 adds packaging jobs; M5 adds signed publishing/update jobs.
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

Before 1.0, declare the support policy: initially the latest stable minor line
receives fixes, prereleases have no stability promise. Never call a build supported
without its installation and update evidence.
