# Working on OpenPaint

OpenPaint is a Linux desktop paint app. A native canvas supports drawing, erasing,
colors, history and navigation; file editing is the next slice. Do not claim planned features exist.

## Start with the relevant contract

- Product scope/status: [feature checklist](docs/FEATURES.md) and [roadmap](docs/ROADMAP.md).
- UI behavior: [design](docs/DESIGN.md).
- Engine, threading, or persistence: [architecture](docs/ARCHITECTURE.md).
- Implementation steps and completion criteria: [development](docs/DEVELOPMENT.md).
- Packaging or network behavior: [releases](docs/RELEASES.md) and [privacy](docs/PRIVACY.md).
- First application task: [M1 execution plan](docs/plans/0001-foundation.md).

Read what the task needs. Keep small fixes small. For changes spanning subsystems,
use [the feature template](docs/templates/FEATURE.md); record architectural changes
with [the decision template](docs/templates/DECISION.md).

## Branches and scope

- `main` is protected. Never commit or push directly to it, bypass protection, or
  bypass required checks. Publish changes on a branch and open a PR targeting `main`.
- Until stable v1, the maintainer authorizes auto-merge after all CI jobs pass.
  Respect required reviews and branch protection; never bypass them.
- Start each branch from current `origin/main`. Name it only for the specific
  feature or change, such as `feat/fill-tolerance` or `docs/branch-workflow`.
  Never include `codex`, `claude`, another assistant/provider name, or an author name.
- One branch and PR deliver one bounded, reviewable outcome, including its necessary
  tests and docs. A milestone is several PRs, not one ever-growing branch.
- Define acceptance cases before implementation. Once they pass, submit for review
  and merge through the protected workflow before starting the next dependent slice.
  Do not append another feature, unrelated fix, or opportunistic refactor.
- If work expands beyond the original outcome, split it into smaller scoped PRs
  before continuing. Track follow-up work separately; never reuse a merged branch.

## Engineering rules

- Use C++20 and Qt 6/QML. Keep pixel operations and document mutations out of QML.
- All user edits go through undoable document commands. Views never own saved pixels.
- Preserve originals on save failures; preserve unsaved documents before any restart.
- Keep core editing offline. No telemetry, silent uploads, credentials, private paths,
  or private images in source, fixtures, examples, logs, or screenshots.
- Do not copy Microsoft code, icons, fonts, or screenshots into the product.
- Use generated, redistributable fixtures. Record third-party sources and licenses.
- Add targeted behavioral tests for engine, persistence, input, and updater changes.
  Docs-only changes need documentation checks, not artificial application tests.
- Update the relevant spec and feature status in the same change as behavior.
  Never mark a feature verified without recorded evidence.
- Do not weaken checks to make a task pass. Report missing dependencies or skipped
  validation explicitly. Keep private agent preferences outside tracked files.

## Commands available now

```sh
python3 tools/check_repo.py
python3 -m unittest discover -s tools/tests -v
cmake --preset dev
cmake --build --preset dev
cmake --build --preset dev --target format-check qml-lint
ctest --preset dev
```

Application commands require the dependencies in [the build guide](docs/BUILD.md).
Do not invent successful builds or install instructions.

## Handoff and review

Summarize changed behavior, validation actually run, remaining limits, and the next
unfinished step in an active plan. Agents use the same review process as humans;
do not introduce provider-specific runtime dependencies or automatic publishing.

Review closely for untracked document mutations, worker/render-thread races,
data loss after failed save/update, accidental networking, and private information.
