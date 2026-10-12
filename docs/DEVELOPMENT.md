# Implementing a feature

The documentation is the project harness: it gives humans and agents the same
scope, boundaries, and completion criteria. Keep it specific enough to execute
without relying on an earlier chat.

## Working environment

Today: Python 3.11+ runs the repository checks; Git is also needed for tooling tests.
No application target exists yet.
Planned M1 baseline: C++20 compiler (GCC 13+ or Clang 18+), CMake 3.28+, Ninja,
Qt 6.8+ with Quick Controls, and Qt Test/QuickTest. Pin exact tested versions and
install commands in M1 rather than documenting untested distro package names now.

M1 will supply `dev`, `release`, and `asan` CMake presets. The intended commands
below are a future interface and **do not run in the current docs-only repository**:

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure
```

No download of tools or dependencies should be hidden inside a normal build.
Packaging recipes pin their sources/checksums and document prerequisite setup.

## Feature workflow

1. **Define the outcome.** Select a feature ID and milestone. Write a small user
   story plus observable acceptance cases. For nontrivial changes, copy
   [the feature template](templates/FEATURE.md) into `docs/plans/` with a numbered,
   descriptive name. A typo or small bug fix does not need a new design document.
2. **Read the relevant contract.** Use [DESIGN.md](DESIGN.md) for UI behavior,
   [ARCHITECTURE.md](ARCHITECTURE.md) for data and ownership, and
   [RELEASES.md](RELEASES.md) for install/update behavior. Record ambiguity in the
   plan. If a decision changes a module boundary, format, dependency policy, or
   security model, add an ADR using [this template](templates/DECISION.md).
3. **Trace the operation.** Identify the application action, tool/command, model
   changes, view state, serialization effects, and threading. Document empty,
   busy, disabled, failure, cancel, and undo states. Plan the smallest vertical slice.
4. **Implement the behavior.** Start with model/command logic and its focused tests;
   then connect controls. Reuse existing actions. Avoid speculative abstractions,
   unrelated cleanup, and UI-only mock behavior presented as implemented features.
5. **Validate risks.** Run the checks that exercise the changed behavior; use
   [QUALITY.md](QUALITY.md) to select them. For a regression, demonstrate the
   reproducer fails before and passes after where feasible. Record commands and
   results honestly, including manual checks not possible in the environment.
6. **Synchronize docs.** Update feature status, active plan, and relevant contracts.
   Add a changelog entry for user-visible behavior. Mark acceptance items complete
   only with actual evidence; leave external/manual gates open when unverified.
7. **Review and hand off.** Inspect the diff for private information, unnecessary
   dependencies, and unrelated edits. Open a focused PR with screenshots where
   useful, validation, and material limits. Keep unfinished steps in the plan.

## Worked example: fill tolerance (M2 / F04)

Outcome: fill neighboring colors within a chosen tolerance without crossing the
selection boundary. The slider only appears with the Fill tool and supports
keyboard/numeric adjustment.

- Plan: define the color-distance metric, alpha handling, contiguous connectivity,
  selection-mask behavior, tolerance scale, and whether merged sampling is supported.
- Model: operate on an immutable layer snapshot and selection mask; return a pixel
  patch. Detect invalid sizes before allocating the work queue.
- Controller: bind the operation to a source revision; apply one undo command when
  complete. Cancel or a stale revision leaves the document unchanged.
- UI: show relevant options, progress on large fills, and recoverable error text.
- Tests: exact-color fill, partially transparent edges, zero/full tolerance,
  disconnected islands, selection clipping, undo/redo, cancellation, and large input.
- Documentation: update F04 evidence and the chosen metric; attach a synthetic
  screenshot. A slider that does not affect fill behavior is not completion.

## Code conventions

Use RAII and explicit ownership; avoid owning raw pointers and mutable global state.
Prefer value types and small interfaces; use Qt ownership for QObject trees
consistently. Use `enum class`, checked arithmetic at file/size boundaries, and
explicit error results that the UI can translate. Keep error logs free of image
contents, personal paths, and prompts.

Use `PascalCase` for C++ types/QML components and `camelCase` for functions and
variables. M1 introduces a clang-format configuration and QML formatting/lint
commands, then enforces the checked-in rules. Do not add a second style tool for
the same job. User-facing strings go through translation APIs.

Signals describe state changes; actions express user intent. QML binds to typed
properties/models, not large JSON blobs or pixel arrays. Network services are
replaceable in tests and disabled for ordinary editing workflows.

## Agent portability

Keep [AGENTS.md](../AGENTS.md) short and use task-specific docs for detail.
[CLAUDE.md](../CLAUDE.md) imports it to avoid two divergent instruction sets.
No model-specific hidden memory is required. Put decisions and unfinished work
in the repository, but never copy raw conversations, credentials, or private
personal context into a plan. Provider settings and local overrides stay ignored.

Codex instruction discovery and Claude imports are documented in the official
[Codex guide](https://developers.openai.com/codex/guides/agents-md) and
[Claude Code guide](https://code.claude.com/docs/en/memory).

## Done means reviewable

A feature is complete when its observable acceptance cases pass, undo/save/cancel
semantics hold, relevant privacy/accessibility constraints are met, documentation
matches behavior, and validation is recorded. Compiling is necessary but not
sufficient. Maintain one honest status ledger rather than optimistic badges.
