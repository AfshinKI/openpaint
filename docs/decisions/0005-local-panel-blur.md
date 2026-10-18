# ADR 0005: layered app-surface blur

Status: accepted
Date: 2026-10-18

## Context

Plain desktop transparency mixes background text with app controls. User feedback
also requires separate frosted surfaces and no fully transparent toolbar gaps.

## Decision

Paint one continuous base across the client window. Fixed bars add a tint above
that base. For floating panels, use Qt Quick Effects MultiEffect with bounded
ShaderEffectSource captures of explicitly ordered lower layers: the window base,
menu bar, ribbon, status bar, and canvas. Composite half-resolution captures and
apply a rounded mask. Do not sample the whole window or include the effect in its own source.
Foreground controls and authoritative document pixels remain outside the effect.
Use tinted surfaces without shader blur on Qt's software renderer.

## Alternatives

Capturing the desktop inside the app would introduce privacy and platform
constraints. Desktop blur remains compositor-owned; the local GNOME extension
experiment supplies that effect separately. Uniform tint alone does not blur
artwork behind floating controls.

## Consequences

Qt Quick Effects is an additional QML import from the existing Qt installation.
Visible translucent panels allocate GPU textures; opaque and disabled glass do
not. Desktop blur intensity cannot vary by fixed bar through this app effect.
The source list excludes other floating panels, avoiding recursive feedback but
not providing arbitrary recursive panel stacking. Hardware rendering is validated separately from the software-rendered UI suite;
see [plan 0039](../plans/0039-window-transparency.md).

Source: https://doc.qt.io/qt-6/qml-qtquick-effects-multieffect.html

## Popup correction after visual review

Menus and dialogs sample the fixed layers plus floating tool controls and the
update notice. Those floating controls sample only the base layers, so there is
no feedback loop. Popup captures start with an opaque theme-colored base: blending
a partially transparent blurred capture over its sharp original otherwise leaves
sharp details visible. Popups therefore show blurred app content but do not reveal
the external desktop through their backdrop. Software rendering uses solid popup
surfaces. General recursive stacking of popups over other popups is still deferred.
