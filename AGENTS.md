# Working on OpenPaint

OpenPaint is a Linux desktop paint app. This repository is currently a documentation
and tooling foundation; do not claim that planned application features exist.

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
```

Application build commands in the M1 plan are a future contract until M1 implements
them. Do not invent successful builds or install instructions.

## Handoff and review

Summarize changed behavior, validation actually run, remaining limits, and the next
unfinished step in an active plan. Agents use the same review process as humans;
do not introduce provider-specific runtime dependencies or automatic publishing.

Review closely for untracked document mutations, worker/render-thread races,
data loss after failed save/update, accidental networking, and private information.
