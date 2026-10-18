// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Menu {
    implicitWidth: 248
    padding: 6
    popupType: Popup.Item
    background: GlassSurface {
        opaqueBackdrop: true
        menuSurface: true
        backdropLayers: backdropSource && hostWindow ? hostWindow.glassOverlayBackdropLayers : []
        radius: Theme.surfaceRadius
    }
    delegate: PaintMenuItem {}
}
