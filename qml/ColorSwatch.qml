// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Button {
    id: control
    property color swatchColor: "white"
    implicitWidth: 32
    implicitHeight: 32
    hoverEnabled: true
    Accessible.name: text.length ? text : qsTr("Color %1").arg(swatchColor.toString())
    contentItem: Item {}
    background: Rectangle {
        x: (control.width - 24) / 2
        y: (control.height - 24) / 2
        width: 24
        height: 24
        radius: 12
        color: control.swatchColor
        border.width: control.visualFocus || control.checked ? 3 : control.hovered ? 2 : 1
        border.color: control.visualFocus || control.checked ? Theme.accent : Theme.divider
    }
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: Accessible.name
}
