// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

MenuItem {
    id: control
    implicitHeight: 36
    palette.text: Theme.text
    palette.highlightedText: Theme.text
    background: Rectangle {
        radius: Theme.controlRadius
        color: control.highlighted ? Theme.hover : "transparent"
        border.width: control.visualFocus ? 2 : 0
        border.color: Theme.accent
    }
}
