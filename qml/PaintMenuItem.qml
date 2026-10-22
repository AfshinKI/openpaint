// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import OpenPaint

MenuItem {
    id: control
    property string symbol: subMenu ? "menu" : ""
    icon.source: symbol.length ? "icons/" + symbol + ".svg" : action ? action.icon.source : ""
    icon.width: 18
    icon.height: 18
    icon.color: Theme.coloredIcon(symbol.length ? "icons/" + symbol + ".svg" : action ? action.icon.source : "") ? "transparent" : enabled ? Theme.text : Theme.secondaryText
    implicitHeight: 36
    leftPadding: 40
    palette.windowText: enabled ? Theme.text : Theme.secondaryText
    opacity: enabled ? 1 : 0.55
    palette.text: Theme.text
    palette.highlightedText: Theme.text
    indicator: Label {
        x: 6
        y: (control.height - height) / 2
        width: 24
        text: control.checked ? "✓" : ""
        color: Theme.dark ? "#F4FAFF" : "#536879"
        style: Text.Raised
        styleColor: Theme.dark ? "#687887" : "#FFFFFF"
        horizontalAlignment: Text.AlignHCenter
    }
    rightPadding: subMenu ? 24 : 6
    // Own the hint color: Qt's native hint does not follow runtime theme changes.
    contentItem: RowLayout {
        spacing: 8
        IconLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: 24
            icon: control.icon
            text: control.text
            font: control.font
            color: control.enabled ? Theme.text : Theme.secondaryText
            alignment: Qt.AlignLeft
            spacing: 6
        }
        Label {
            objectName: "menuShortcutHint"
            text: control.action ? AppInfo.shortcutText(control.action.shortcut) : ""
            visible: text.length > 0
            color: Theme.secondaryText
            font: control.font
        }
    }
    background: GlassSurface {
        backdropLayers: hostWindow ? [hostWindow.background, hostWindow.canvasController] : []
        visible: control.highlighted || control.visualFocus || control.checked
        selectedSurface: control.highlighted || control.checked
        radius: Theme.controlRadius
        color: control.highlighted ? Theme.hover : "transparent"
        border.width: control.visualFocus ? 2 : 0
        border.color: Theme.accent
    }
}
