# ADR 0004: bounded release notifications through GitHub

Status: accepted
Date: 2026-10-16

## Context

Users need notification of newer public releases. The existing browser link cannot
compare versions; automatic installation requires a separate signed update design.

## Decision

Use Qt Network for explicit or opt-in daily HTTPS GET requests to the fixed GitHub
latest-stable endpoint. No credentials, redirects, arbitrary URLs, artwork or active
release-note content. Validate bounded JSON and stable version syntax. Persist opt-in
and cooldown timestamps locally. Fake transport/clock tests cover failure paths.
Flatpak keeps its current no-network permission and manual browser workflow.

## Alternatives and consequences

A custom server adds operations without improving this notification use case.
Public GitHub requests need no embedded secret, but can be rate-limited and expose
ordinary connection metadata. Notifications are not artifact authentication and
cannot authorize installation. Offline editing and package ownership are unchanged.
