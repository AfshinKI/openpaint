# Automatic alpha releases and portable Linux download

## Acceptance

- A successful main push publishes a uniquely versioned alpha from that exact
  commit only after all repository, native, sanitizer and package jobs pass.
- PRs build/test packages but never publish. Main runs are not cancelled mid-upload.
- Draft upload is promoted only after all assets arrive; reruns never replace a
  published release. Version uses the workflow run number, consistently embedded
  in the executable and filenames.
- Main download is a relocatable Linux x86_64 tar.gz with bundled Qt/libraries and
  an executable `openpaint` launcher. Extract and run; no FUSE, root or installer.
- Only three uploaded assets: portable download, consolidated corresponding
  sources/notices archive, checksums. GitHub's automatic source archives remain.
- Build recipes, source availability, inventories and licenses remain available;
  developer artifacts stay in Actions rather than cluttering release downloads.

## Implementation

The packaging job retains AppImage/Flatpak validation but publishes the portable
AppDir archive. AppRun and the portable launcher share the same relative library
layout. Portable installation identity explains manual archive replacement.
Publishing uses the same run's artifacts after all prerequisite jobs succeed.
Feature branches run checks through their PR, avoiding duplicate push builds.
Each main commit keeps its own run so rapid merges do not cancel a release.
The project remains alpha; automatic in-app updating is a separate milestone.

## Reported AppImage failure

The downloaded executable had execute-only permissions (111). Restoring owner
read permission allowed `APPIMAGE_EXTRACT_AND_RUN=1 ... --version` to succeed.
The archive preserves read/execute permissions and avoids runtime self-extraction.
No desktop/security policy changes are needed. DMG is not a Linux package.

## Validation

Built the portable package with bundled Qt/dependencies and corresponding sources
inside the pinned Ubuntu 24.04 container. Extracted the tar.gz into a separate
folder and ran its launcher successfully (`OpenPaint 0.1.0-alpha.test`). All 16
repository-tool tests pass, including launcher permissions and bundled library
archive coverage. Workflow YAML parses; repository checks pass. CI must verify
publication gating and build artifacts before merge. The first main run verifies
real GitHub publication; no release has been claimed before that run completes.
