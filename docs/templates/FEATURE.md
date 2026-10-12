# Feature: <short outcome>

Status: proposed
Feature IDs: <Fxx>
Milestone: <Mxx>

## User outcome and scope

Who needs this, what can they do afterward, and what is outside this change?
Link the relevant design/architecture contract and any reference evidence.

## Behavior

Describe the normal flow, defaults, shortcuts, disabled/busy/empty/error states,
cancel, undo/redo, save/export effects, and accessibility. State whether any data
leaves the device. Describe changes to existing behavior explicitly.

## Implementation

List affected modules, ownership/threading, command/model interfaces, dependencies,
format migrations, and the smallest reviewable sequence. Record alternatives only
when they explain a material decision. Add an ADR for architectural changes.

## Acceptance cases

- [ ] Given <starting state>, when <action>, then <observable result>.
- [ ] Failure/cancel leaves <expected state> intact.
- [ ] Undo/redo and save/reopen preserve <relevant content>.
- [ ] Keyboard/accessibility and privacy requirements are checked where applicable.

## Validation and evidence

Record commands, test names, environment, results, and synthetic UI captures.
Separate automated results from manual checks and unverified claims.

## Progress and handoff

- [ ] First independently reviewable step.
- [ ] Next step.

Record decisions, remaining questions, known limitations, and the next unfinished
action. Remove template prompts when filling out a real plan.
