# Contributing to OpenPaint

Thanks for helping bring a familiar painting experience to Linux.
The app is not implemented yet; the next task is
[M1: the first working canvas](docs/plans/0001-foundation.md).

## Pick a change

Use the [roadmap](docs/ROADMAP.md) and [feature checklist](docs/FEATURES.md).
For a feature, describe the user-visible result and acceptance cases before coding.
Small corrections can go straight to a pull request. Larger work should have an
issue and a short [feature plan](docs/templates/FEATURE.md) to avoid duplicate work.

Follow [the implementation workflow](docs/DEVELOPMENT.md). Every change should be
small enough to review and leave the repository in a usable state.

## Local checks

Python 3.11+ and Git are required for the current checks and tooling tests:

```sh
python3 tools/check_repo.py
python3 -m unittest discover -s tools/tests -v
```

The checks validate local documentation links, basic text hygiene, and selected
privacy/secret patterns. They do not prove that a repository contains no secrets;
review your staged diff and commit metadata as well.

Once application development starts, M1 will introduce reproducible CMake presets
and compiler/toolkit installation instructions. See the
[development guide](docs/DEVELOPMENT.md) for the intended toolchain.

## Working with assistants

[AGENTS.md](AGENTS.md) is the shared contract. [CLAUDE.md](CLAUDE.md) imports it for
Claude Code; Codex reads AGENTS.md. Other tools should be directed to the same file.
No assistant is required. You are responsible for reviewing generated changes,
checking their provenance, and reporting which tests actually ran.

Example task:

> Read AGENTS.md and docs/plans/0001-foundation.md. Implement the next unfinished
> step only. Run its acceptance checks, update the plan with evidence, and report
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
