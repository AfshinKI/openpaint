// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

PaintButton {
    id: control
    property bool ribbonIcon: false
    property string symbol
    readonly property bool colored: Theme.coloredIcon(symbol ? "icons/" + symbol + ".svg" : action ? action.icon.source : "")
    property string shortcutLabel: ""
    implicitWidth: 32
    implicitHeight: 32
    padding: ribbonIcon ? 2 : colored ? 4 : 6
    Binding {
        target: control
        property: "leftPadding"
        value: 2
        when: control.ribbonIcon
    }
    Binding {
        target: control
        property: "rightPadding"
        value: 2
        when: control.ribbonIcon
    }
    Binding {
        target: control
        property: "topPadding"
        value: 2
        when: control.ribbonIcon
    }
    Binding {
        target: control
        property: "bottomPadding"
        value: 2
        when: control.ribbonIcon
    }
    display: AbstractButton.IconOnly
    icon.source: "icons/" + symbol + (symbol === "fill" && checked ? "-selected" : "") + ".svg"
    icon.width: ribbonIcon ? 28 : colored ? 24 : 20
    icon.height: ribbonIcon ? 28 : colored ? 24 : 20
    icon.color: colored ? "transparent" : enabled ? Theme.text : Theme.secondaryText
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: text.replace(/&/g, "") + (shortcutLabel ? " (" + shortcutLabel + ")" : "")
}
