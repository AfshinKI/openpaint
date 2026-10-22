// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

PaintButton {
    id: control
    property string symbol
    readonly property bool colored: Theme.coloredIcon(symbol ? "icons/" + symbol + ".svg" : action ? action.icon.source : "")
    property string shortcutLabel: ""
    implicitWidth: 32
    implicitHeight: 32
    padding: colored ? 4 : 6
    display: AbstractButton.IconOnly
    icon.source: "icons/" + symbol + ".svg"
    icon.width: colored ? 24 : 20
    icon.height: colored ? 24 : 20
    icon.color: colored ? "transparent" : enabled ? Theme.text : Theme.secondaryText
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: text.replace(/&/g, "") + (shortcutLabel ? " (" + shortcutLabel + ")" : "")
}
