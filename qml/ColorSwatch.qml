// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Button {
    id: control
    property color swatchColor: "white"
    property bool empty: false
    property int diameter: 16
    implicitWidth: 24
    implicitHeight: 24
    hoverEnabled: true
    Accessible.name: text.length ? text : qsTr("Color %1").arg(swatchColor.toString())
    contentItem: Item {}
    background: Rectangle {
        x: (control.width - control.diameter) / 2
        y: (control.height - control.diameter) / 2
        width: control.diameter
        height: control.diameter
        radius: width / 2
        color: control.empty ? "transparent" : control.swatchColor
        border.width: control.visualFocus || control.checked ? 3 : control.hovered ? 2 : 1
        border.color: control.visualFocus || control.checked ? Theme.accent : control.empty ? Theme.secondaryText : Theme.divider
    }
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: Accessible.name
}
