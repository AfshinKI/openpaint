# ADR 0002: dimension-changing raster history

Status: accepted
Date: 2026-10-12

## Context

Dirty-rectangle patches cannot represent crops or rotations that change image
size. These edits must preserve the same revision, undo-budget and failure rules.

## Decision

Keep bounded patches for ordinary edits. Add a full-raster replacement command
with before/after images and revisions for geometry changes. Validate dimensions,
format, source revision and combined image bytes before modifying history or
pixels. Undo/redo replaces the raster and clears transient selection. Selection
transforms continue to use the existing floating preview transaction.

## Alternatives

Resetting the document would lose history and saved-state tracking. Migrating to
tiles now would expand this feature unnecessarily; M3 retains that architecture
boundary. Full snapshots are acceptable under the current 32-MP and 256-MiB limits.

## Consequences

Large transforms may trim older history within the same budget. A command too
large for that budget is rejected. New geometry tools reuse this command instead
of changing dimensions directly. Tests must cover dirty state, redo branching,
failed validation and pixel/dimension equality through undo/redo.
