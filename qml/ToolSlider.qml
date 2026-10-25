// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import OpenPaint

GlassSurface {
    id: panel
    required property string label
    required property string symbol
    required property string controlName
    property int minimum: 0
    property int maximum: 100
    property int value: 0
    property string suffix: ""
    signal adjusted(int value)
    implicitWidth: 36
    implicitHeight: 204
    radius: Theme.surfaceRadius
    // The floating panel consumes input so its margins never paint the canvas.
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: wheel => wheel.accepted = true
    }
    IconImage {
        source: "icons/" + panel.symbol + ".svg"
        color: Theme.text
        sourceSize.width: 16
        sourceSize.height: 16
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        width: 16
        height: 16
        Accessible.name: panel.label
    }
    Slider {
        id: slider
        objectName: panel.controlName
        orientation: Qt.Vertical
        from: panel.minimum
        to: panel.maximum
        stepSize: 1
        value: panel.value
        x: (panel.width - width) / 2
        y: 32
        width: 28
        height: Math.max(18, panel.height - 40)
        padding: 0
        Accessible.name: panel.label + " " + panel.minimum + "–" + panel.maximum + panel.suffix
        onMoved: panel.adjusted(Math.round(value))
        ToolTip.visible: hovered || pressed
        ToolTip.text: panel.label + ": " + Math.round(value) + panel.suffix
        background: Rectangle {
            x: (slider.width - width) / 2
            y: slider.topPadding + 9
            width: 4
            height: slider.availableHeight - 18
            radius: 2
            color: Theme.divider
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: slider.position * parent.height
                radius: 2
                color: Theme.accent
            }
        }
        handle: Rectangle {
            x: (slider.width - width) / 2
            y: slider.topPadding + slider.visualPosition * (slider.availableHeight - height)
            width: 18
            height: 18
            radius: 9
            color: Theme.accent
            border.width: slider.visualFocus ? 3 : 2
            border.color: slider.visualFocus ? Theme.text : Theme.panel
        }
    }
}
