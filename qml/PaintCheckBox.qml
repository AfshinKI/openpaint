// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

CheckBox {
    id: control
    implicitHeight: 32
    indicator: Rectangle {
        x: control.leftPadding
        y: (control.height - height) / 2
        implicitWidth: 22
        implicitHeight: 22
        radius: 4
        color: control.checked ? Theme.accent : Theme.panel
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus || control.checked ? Theme.accent : Theme.divider
        Text {
            anchors.centerIn: parent
            text: "✓"
            visible: control.checked
            color: Theme.accentText
            font.pixelSize: 16
        }
    }
}
