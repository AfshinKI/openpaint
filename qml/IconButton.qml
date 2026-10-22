// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import OpenPaint

PaintButton {
    id: control
    property string symbol
    readonly property bool colored: Theme.coloredIcon(symbol ? "icons/" + symbol + ".svg" : action ? action.icon.source : "")
    property color paintColor: "black"
    property string shortcutLabel: ""
    implicitWidth: 32
    implicitHeight: 32
    padding: colored ? 4 : 6
    display: AbstractButton.IconOnly
    icon.source: "icons/" + symbol + ".svg"
    icon.width: colored ? 24 : 20
    icon.height: colored ? 24 : 20
    icon.color: colored ? "transparent" : enabled ? Theme.text : Theme.secondaryText
    IconImage {
        objectName: "toolPaintOverlay"
        z: 1
        anchors.centerIn: parent
        width: control.icon.width
        height: control.icon.height
        source: control.symbol === "fill" || control.symbol === "brush" ? "icons/" + control.symbol + "-paint.svg" : ""
        visible: control.display === AbstractButton.IconOnly && (control.symbol === "fill" || control.symbol === "brush") && control.paintColor.a > 0
        color: control.paintColor
        opacity: control.enabled ? 1 : 0.5
    }
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: text.replace(/&/g, "") + (shortcutLabel ? " (" + shortcutLabel + ")" : "")
}
