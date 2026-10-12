// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

SpinBox {
    id: control
    implicitHeight: 36
    implicitWidth: 104
    background: Rectangle {
        radius: Theme.controlRadius
        color: Theme.panel
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus ? Theme.accent : Theme.divider
    }
    up.indicator: Rectangle {
        x: control.width - width - 2
        y: 2
        width: 28
        height: control.height - 4
        radius: 4
        color: control.up.pressed ? Theme.pressed : control.up.hovered ? Theme.hover : "transparent"
        Text {
            anchors.centerIn: parent
            text: "+"
            color: Theme.text
            font.pixelSize: 20
        }
    }
    down.indicator: Rectangle {
        x: 2
        y: 2
        width: 28
        height: control.height - 4
        radius: 4
        color: control.down.pressed ? Theme.pressed : control.down.hovered ? Theme.hover : "transparent"
        Text {
            anchors.centerIn: parent
            text: "−"
            color: Theme.text
            font.pixelSize: 20
        }
    }
}
