// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Menu {
    implicitWidth: 248
    padding: 6
    popupType: Popup.Item
    background: GlassSurface {
        radius: Theme.surfaceRadius
    }
    delegate: PaintMenuItem {}
}
