// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import OpenPaint

Rectangle {
    id: surface
    readonly property real surfaceAlpha: Appearance.glassEnabled ? Appearance.surfaceOpacity / 100 : 1
    radius: Theme.surfaceRadius
    color: Theme.panel
    border.color: Appearance.glassEnabled ? (Theme.dark ? "#687887" : "#B8C8D5") : Theme.divider
    gradient: Appearance.glassEnabled ? sheen : null
    Gradient {
        id: sheen
        GradientStop {
            position: 0
            color: Qt.rgba(Theme.dark ? 0.23 : 1, Theme.dark ? 0.28 : 1, Theme.dark ? 0.33 : 1, surface.surfaceAlpha)
        }
        GradientStop {
            position: 0.48
            color: Qt.rgba(Theme.panel.r, Theme.panel.g, Theme.panel.b, surface.surfaceAlpha)
        }
        GradientStop {
            position: 1
            color: Qt.rgba(Theme.dark ? 0.10 : 0.90, Theme.dark ? 0.14 : 0.94, Theme.dark ? 0.18 : 0.98, surface.surfaceAlpha)
        }
    }
}
