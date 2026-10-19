// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

CheckBox {
    id: control
    implicitHeight: 32
    palette.windowText: Theme.text
    palette.text: Theme.text
    palette.buttonText: Theme.text
    indicator: GlassSurface {
        backdropLayers: hostWindow ? [hostWindow.background, hostWindow.canvasController] : []
        selectedSurface: control.checked
        x: control.leftPadding
        y: (control.height - height) / 2
        implicitWidth: 22
        implicitHeight: 22
        radius: 4
        color: control.checked ? Theme.accent : Theme.panel
        border.width: control.visualFocus ? 2 : 1
        border.color: Appearance.glassEnabled && control.checked ? Theme.selectionOutline : control.visualFocus || control.checked ? Theme.accent : Theme.divider
        Text {
            anchors.centerIn: parent
            text: "✓"
            visible: control.checked
            color: Theme.dark ? "#F4FAFF" : "#536879"
            style: Text.Raised
            styleColor: Theme.dark ? "#687887" : "#FFFFFF"
            font.pixelSize: 16
        }
    }
}
