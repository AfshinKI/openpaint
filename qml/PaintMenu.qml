// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Menu {
    implicitWidth: 248
    padding: 6
    popupType: Popup.Item
    background: Rectangle {
        radius: Theme.surfaceRadius
        color: Theme.panel
        border.color: Theme.divider
    }
    delegate: PaintMenuItem {}
}
