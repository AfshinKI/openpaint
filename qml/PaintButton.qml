// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Button {
    id: control
    implicitHeight: 36
    implicitWidth: Math.max(36, implicitContentWidth + 24)
    hoverEnabled: true
    Accessible.name: text.replace(/&/g, "")
    palette.buttonText: enabled ? Theme.text : Theme.secondaryText
    palette.brightText: Theme.text
    palette.dark: Theme.selected
    background: Rectangle {
        radius: Theme.controlRadius
        color: control.down ? Theme.pressed : control.checked || control.highlighted ? Theme.selected : control.hovered ? Theme.hover : control.flat ? "transparent" : Theme.dark && Appearance.glassEnabled ? "#414B58" : Theme.panel
        border.width: control.visualFocus ? 2 : control.flat && !control.checked && !control.highlighted ? 0 : 1
        border.color: control.visualFocus ? Theme.accent : control.checked ? Theme.accent : Theme.divider
        opacity: control.enabled ? 1 : 0.5
    }
}
