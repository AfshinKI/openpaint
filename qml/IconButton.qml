// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

PaintButton {
    id: control
    property string symbol
    property string shortcutLabel: ""
    implicitWidth: 32
    implicitHeight: 32
    display: AbstractButton.IconOnly
    icon.source: "icons/" + symbol + ".svg"
    icon.width: 20
    icon.height: 20
    icon.color: enabled ? Theme.text : Theme.secondaryText
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: text.replace(/&/g, "") + (shortcutLabel ? " (" + shortcutLabel + ")" : "")
}
