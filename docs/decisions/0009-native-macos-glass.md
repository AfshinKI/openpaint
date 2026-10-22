# ADR 0009: Native macOS glass and distribution

Status: accepted
Date: 2026-10-21

## Context

The maintainer requested macOS distribution with the existing glass UI. Linux's
GNOME extension recipe is not applicable on macOS. Document behavior must remain
shared and offline.

## Decision

Use a small Objective-C++ AppKit adapter behind the Qt Quick content view.
NSVisualEffectView supplies behind-window glass, respects Reduce Transparency,
and never receives pointer input. Build Apple silicon/macOS 14.4+ on the hosted
macos-15 runner; bundle Qt with macdeployqt and audit runtime dependencies.
Use system libarchive with installed development headers. Generate the app icon
from the original project SVG. Distribute a DMG and matching source notices.

## Alternatives

Opaque Qt surfaces remain the fallback. Wallpaper snapshots cannot reflect live
windows behind the app. A separate Swift UI would duplicate editing/view code.
A universal Intel build adds a separate validation target and is deferred.

## Consequences

The adapter stays on the GUI thread and needs Cocoa validation in addition to
shared editor tests. Hosted smoke tests do not replace physical visual review.
Ad-hoc signing provides bundle integrity but no Apple Developer ID trust;
notarization requires maintainer credentials and is not claimed. Revisit signing
and Intel support when those release environments are available.

References: [Qt macOS deployment](https://doc.qt.io/qt-6/macos-deployment.html),
[NSVisualEffectView](https://developer.apple.com/documentation/appkit/nsvisualeffectview).
