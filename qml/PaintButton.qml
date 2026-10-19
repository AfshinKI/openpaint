// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Button {
    id: control
    implicitHeight: 36
    implicitWidth: Math.max(36, implicitContentWidth + 24)
    hoverEnabled: true
    Accessible.name: text.replace(/&/g, "")
    palette.buttonText: enabled ? Theme.text : Theme.secondaryText
    palette.brightText: Theme.text
    palette.dark: Theme.selected
    background: Item {
        opacity: control.enabled ? 1 : 0.5
        Rectangle {
            anchors.fill: parent
            visible: !Appearance.glassEnabled || (!control.checked && !control.highlighted)
            radius: Theme.controlRadius
            color: control.down ? Theme.pressed : control.checked || control.highlighted ? Theme.selected : control.hovered ? Theme.hover : control.flat ? "transparent" : Theme.dark && Appearance.glassEnabled ? "#414B58" : Theme.panel
            border.width: control.visualFocus ? 2 : control.flat && !control.checked && !control.highlighted ? 0 : 1
            border.color: control.visualFocus ? Theme.accent : control.checked ? Theme.accent : Theme.divider
        }
        GlassSurface {
            objectName: "selectedToolSurface"
            anchors.fill: parent
            radius: Theme.controlRadius
            selectedSurface: true
            visible: Appearance.glassEnabled && (control.checked || control.highlighted)
            // Never sample the toolbar or popup containing this selected control.
            backdropLayers: hostWindow ? [hostWindow.background, hostWindow.canvasController] : []
            border.width: control.visualFocus ? 2 : 1
            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: "white"
                opacity: control.down ? 0.12 : control.hovered ? 0.06 : 0
            }
        }
    }
}
