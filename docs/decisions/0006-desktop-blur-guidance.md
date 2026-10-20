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

A locally available Extension Manager package candidate permits installation
instructions. An installed extension permits enable/preferences instructions.
Each read-only subprocess has a deadline and output limit, uses explicit argv,
and runs off the GUI thread. No setup command is executed by OpenPaint. No remote
package query, host sandbox escape, privileged process or desktop screenshot is
used. Flatpak and Snap skip the probe. Probe results are not persisted or logged.

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
but hides the command when no recipe exists. Additional recipes need separate desktop
validation. Setup instructions include extension configuration, not just package
installation. Installing a package remains a user action outside OpenPaint.

The state service is tested with injected read-only command results and isolated
settings. UI tests inject capability results rather than invoking host tools.
See [implementation plan](../plans/0054-blur-onboarding-design.md).
