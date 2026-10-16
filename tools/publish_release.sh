#!/usr/bin/env bash
# Only invoked by the gated main-branch release job.
set -euo pipefail
: "${RELEASE_VERSION:?}" "${SOURCE_SHA:?}" "${GH_REPO:?}"
tag="v${RELEASE_VERSION}"
python3 - <<'PYTHON'
import os
from pathlib import Path
Path('release-notes.md').write_text(Path('packaging/release-notes.md').read_text().replace('{VERSION}', os.environ['RELEASE_VERSION']))
PYTHON
# A tag must name these tested sources, including when resuming a draft.
if git ls-remote --exit-code origin "refs/tags/$tag" >/dev/null 2>&1; then
  git fetch --no-tags origin "refs/tags/$tag"
  test "$(git rev-parse 'FETCH_HEAD^{commit}')" = "$SOURCE_SHA"
fi
if gh release view "$tag" --json isDraft,targetCommitish > build-release.json 2>/dev/null; then
  test "$(python3 -c 'import json; print(json.load(open("build-release.json"))["isDraft"])')" = True || {
    echo 'Published release is immutable; nothing changed.'
    exit 0
  }
  test "$(python3 -c 'import json; print(json.load(open("build-release.json"))["targetCommitish"])')" = "$SOURCE_SHA"
else
  gh release create "$tag" --target "$SOURCE_SHA" --draft --title "$tag" --notes-file release-notes.md
fi
(cd release && sha256sum -c SHA256SUMS)
gh release upload "$tag" release/* --clobber
gh release edit "$tag" --draft=false --prerelease=false --latest --title "$tag"
