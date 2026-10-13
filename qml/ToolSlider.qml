// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

Rectangle {
    id: panel
    required property string label
    required property string controlName
    property int minimum: 0
    property int maximum: 100
    property int value: 0
    property string suffix: ""
    signal adjusted(int value)
    implicitWidth: 64
    implicitHeight: 204
    radius: Theme.surfaceRadius
    color: Theme.panel
    border.color: Theme.divider
    // The floating panel consumes input so its margins never paint the canvas.
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: wheel => wheel.accepted = true
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 4
        Label {
            text: panel.label
            color: Theme.text
            font.pixelSize: 11
            Layout.alignment: Qt.AlignHCenter
        }
        Slider {
            id: slider
            objectName: panel.controlName
            orientation: Qt.Vertical
            from: panel.minimum
            to: panel.maximum
            stepSize: 1
            value: panel.value
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: 32
            Accessible.name: panel.label + " " + panel.minimum + "–" + panel.maximum + panel.suffix
            onMoved: panel.adjusted(Math.round(value))
            ToolTip.visible: hovered || pressed
            ToolTip.text: Math.round(value) + panel.suffix
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
        Label {
            text: panel.value + panel.suffix
            color: Theme.text
            font.pixelSize: 11
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
