// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

PaintButton {
    id: control
    property string symbol
    property string shortcutLabel: ""
    implicitWidth: 36
    implicitHeight: 36
    display: AbstractButton.IconOnly
    icon.source: "icons/" + symbol + ".svg"
    icon.width: 24
    icon.height: 24
    icon.color: enabled ? Theme.text : Theme.secondaryText
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: text.replace(/&/g, "") + (shortcutLabel ? " (" + shortcutLabel + ")" : "")
}
