# ADR 0006: local blur evidence and silent opaque fallback

Status: accepted
Date: 2026-10-19

## Context

A translucent window can expose sharp desktop content on machines without blur.
The approved on-demand setup design offers optional guidance and uses opaque rendering
silently when no supported setup is available.

## Decision

A C++ BlurSupport service probes local desktop evidence on a worker. Its initial
recipe is Ubuntu 24.04, GNOME 46, X11 (xcb), matching the previously recorded
[desktop validation](../plans/0039-window-transparency.md). It reads GNOME extension
state and Blur My Shell application settings; missing/failed/unrecognized evidence
never enables transparency. Known app identifiers must be whitelisted, or all
applications enabled, using the extension’s inclusion/exclusion mode and application patterns. This is configuration evidence,
not a guarantee about every compositor frame. Other environments stay opaque.

The supported session receives one copyable Python setup block. OpenPaint never
executes it. Explicitly running the block in a terminal downloads a missing,
checksum-pinned Blur My Shell archive from GNOME Extensions, installs it for the
current user, backs up touched settings and configures/enables application blur.
It requires no sudo, does not replace existing extensions, and never forces logout.
This replaces the earlier multi-step Extension Manager recipe at the maintainer's
request. See [plan 0060](../plans/0060-zoom-and-blur-setup.md) for the pinned source,
settings affected and validation limits.

Each app probe is read-only, has a deadline/output limit, uses explicit argv and
runs off the GUI thread. The app makes no setup network calls or host sandbox
escape; Flatpak and Snap skip the probe. Results are not persisted or logged.

Appearance retains its saved glass/opacity preferences separately from effective
glass rendering. The latter requires current blur evidence, so fallback does not
overwrite a user's choice. A separate application welcome revision is saved only on dismissal.
Glass setup has no onboarding state and opens on demand from Appearance or Updates. Clipboard writes require the copy action.

## Alternatives

Bundling an extension would change desktop configuration and introduce shell
version compatibility requirements. Guessing capability from the desktop name
alone leaves sharp transparency. Capturing the desktop to simulate blur changes
the privacy model. None of these is used.

## Consequences

KDE, Wayland, unknown versions, unavailable packages and failed probes currently
use opaque surfaces with no warning. The on-demand setup panel stays accessible
but hides the command when no recipe exists and explains the verified scope. Additional recipes need separate desktop
validation. Setup is an explicit user action outside OpenPaint. The copied block changes
user-level GNOME settings and contacts GNOME only when the extension is missing.

The state service is tested with injected read-only command results and isolated
settings. UI tests inject capability results rather than invoking host tools.
See [implementation plan](../plans/0054-blur-onboarding-design.md).

The setup recipe now selects static wallpaper blur, avoiding the live-window
partial-redraw path implicated in the reported flicker. It leaves global compositor
redraw settings unchanged. This is a mitigation, not proof of flicker-free rendering
on every GPU. Existing readiness still accepts either compositor blur mode.
