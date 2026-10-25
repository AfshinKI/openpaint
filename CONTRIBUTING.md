# Contributing to OpenPaint

Thanks for helping improve OpenPaint for Linux and macOS. The v2 editor ships
drawing tools, selections, transforms, raster layers and OpenRaster projects.
Start with the [current feature status](docs/FEATURES.md) and
[remaining roadmap gates](docs/ROADMAP.md), rather than the completed M1 plan.

## Pick a change

Use the [roadmap](docs/ROADMAP.md) and [feature checklist](docs/FEATURES.md).
For a feature, describe the user-visible result and acceptance cases before coding.
Small corrections can go straight to a pull request. Larger work should have an
issue and a short [feature plan](docs/templates/FEATURE.md) to avoid duplicate work.

Follow [the implementation workflow](docs/DEVELOPMENT.md). Every change should be
small enough to review and leave the repository in a usable state.

## Branches and pull requests

Follow the shared [branch and scope rules](AGENTS.md#branches-and-scope). `main` is
protected: all changes go through a branch and pull request, including docs and
maintenance. Do not push directly to `main` or bypass its protections.

Use `<type>/<specific-change>` with a short, lowercase, hyphenated description.
Useful types are `feat`, `fix`, `docs`, `build`, and `test`. Examples:

- `build/qt-cmake-shell`
- `feat/stroke-undo`
- `fix/png-save-failure`
- `docs/branch-workflow`

Branch names describe the work, never the person or assistant doing it. No `codex`,
`claude`, or other assistant/provider names. Avoid broad names such as `m1`,
`all-tools`, or `misc-improvements` that invite scope growth.

Start from current `origin/main`, state one outcome and its acceptance cases, and
open a PR when that slice is ready. Include only directly necessary implementation,
tests, and docs. If a prerequisite or new requirement makes the change too broad
to review as one outcome, split it before continuing. Put unrelated discoveries
in separate issues/PRs. Merge through the protected workflow before the next
dependent slice; after merge, start a fresh branch rather than extending the old one.

## Local checks

Python 3.11+ and Git are required for the current checks and tooling tests:

```sh
python3 tools/check_repo.py
python3 -m unittest discover -s tools/tests -v
```

The checks validate local documentation links, basic text hygiene, and selected
privacy/secret patterns. They do not prove that a repository contains no secrets;
review your staged diff and commit metadata as well.

For application work, use the checked-in CMake presets and compiler/toolkit setup
in [the build guide](docs/BUILD.md). Run the relevant engine, file, input and shell
tests plus C++/QML checks; [QUALITY.md](docs/QUALITY.md) maps changes to evidence.
For website changes, also run `python3 tools/build_site.py`. Documentation-only
changes need documentation checks, not an application rebuild.

## Working with assistants

[AGENTS.md](AGENTS.md) is the shared contract. [CLAUDE.md](CLAUDE.md) imports it for
Claude Code; Codex reads AGENTS.md. Other tools should be directed to the same file.
No assistant is required. You are responsible for reviewing generated changes,
checking their provenance, and reporting which tests actually ran.

Example task:

> Read AGENTS.md, the current feature checklist and the plan for the requested
> change. Implement its next unfinished step only. Run its acceptance checks,
> update the plan with evidence, and report
> any remaining limitation. Do not mark later milestones complete.

## Before opening a pull request

- Explain the problem, resulting behavior, and validation in the PR template.
- Update the relevant docs and changelog for user-visible behavior.
- Include synthetic before/after screenshots for visible UI changes.
- Include failure, cancel, and undo behavior for editing or file operations.
- Exclude personal files, private diagnostics, tokens, and build artifacts.
- Inspect `git diff --cached` and `git diff --cached --check`.
- Use a GitHub-provided noreply email if you want to keep your email private.
  Set it in your local Git configuration before committing and inspect
  `git log -1 --format=fuller` before pushing. Public history still exposes the
  author name/handle you choose; noreply does not make commits anonymous.

Contributions are provided under the repository's GPL-3.0-or-later license unless
an explicitly documented third-party license applies. Preserve attribution;
never remove a contributor's credit to satisfy the privacy policy.

Be respectful and specific in reviews. Report reproducible problems without
personal attacks. Security-sensitive reports belong in the process in
[SECURITY.md](SECURITY.md), not a public issue containing private data.
