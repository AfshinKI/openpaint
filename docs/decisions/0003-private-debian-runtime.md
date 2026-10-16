# ADR 0003: Private runtime in the Ubuntu installer

Status: accepted
Date: 2026-10-16

## Context

OpenPaint requires Qt 6.8+ and pins Qt 6.12. Ubuntu 24.04 supplies Qt 6.4.
Installing only the native executable cannot satisfy the current runtime contract.
The existing portable recipe already audits libraries, plugins, licenses and sources.

## Decision

Reuse that recipe for an Ubuntu 24.04 amd64 DEB with system build identity.
Keep bundled libraries under /usr/lib/openpaint, outside the system library search
path. Install the desktop entry, icon, metadata and command into standard locations.
Calculate remaining dependencies with dpkg-shlibdeps over all ELF objects. Map
only bundled SONAMEs to the package itself and exclude self-dependencies; do not
ignore missing dependency information. Do not ship a global ld.so configuration.

## Alternatives

System Qt packages are preferable for distribution-maintained builds but cannot
satisfy the supported Ubuntu baseline. Lowering Qt requirements is a separate
porting project. Flatpak remains the cross-distribution installed option.

## Consequences

OpenPaint releases must maintain bundled library security updates and corresponding
sources. Package upgrades/removal are owned by APT; no self-update privileges.
The package is Ubuntu-specific until other distributions pass installed tests.
