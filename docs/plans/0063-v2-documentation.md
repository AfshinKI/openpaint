# Documentation: reconcile the shipped v2 editor

Status: documentation updated; protected review pending

## Outcome and acceptance

- [x] Current entry points describe Linux/macOS v2 and layered project saving.
- [x] Feature status distinguishes shipped scope from unverified M3/platform gates.
- [x] Release progression captures intermediate and superseded UI decisions.
- [x] PR/tag/publication metadata replaces stale active pre-release handoffs,
  while preserving historical test evidence and limitations.
- [x] Repository links, privacy checks, website build and tooling tests pass.

## Evidence and handoff

On 2026-10-25, compared source layout, canvas input routing, project save guards,
release plans, changelog, local v2 tag and public GitHub release/PR/workflow metadata.
Updated the feature ledger, roadmap, build and contributor guidance, architecture,
privacy wording and plan status. No application behavior or screenshots changed.

Validation: `python3 tools/check_repo.py`, `python3 tools/build_site.py`,
`python3 -m unittest discover -s tools/tests -v` (42 tests), and
`git diff --check` pass. Native application tests were not rerun for documentation.
The link check initially caught two historical links to the renamed roadmap
handoff; both now point to the current handoff.

Next: protected PR review. Release acceptance still needs public binary checksum
verification, deployed-page inspection and physical Mac evidence, as recorded in
[plan 0062](0062-macos-v2-release.md). This audit does not claim those checks ran.

## Public-entry-point follow-up

The second pass reviewed Contributing, Security, GitHub issue/PR templates,
installation, packaging README/release notes, website navigation/FAQ and publishing
guidance, preview provenance, references, templates and current design/quality
contracts. Corrected pre-M1 claims in Contributing and bug reporting, the no-release
claim in Security, obsolete update-check instructions, Flatpak channel wording,
and the missing macOS packaging entry point. Separated future release walkthrough
gates from shipped behavior and fixed the capture selector for split shell suites.
Historical plans/ADRs and provenance remain historical; no new external reference
parity audit or release certification is claimed. Repeated repository checks and
website staging after these changes; no tooling or application behavior changed.
