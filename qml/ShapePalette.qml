// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

Popup {
    id: shapePopup
    required property Editor editor
    width: 224
    padding: 8
    focus: true
    popupType: Popup.Item
    onOpened: {
        const button = buttons.itemAt(Math.max(0, Math.min(editor.shapeKind, 19)));
        if (button)
            button.forceActiveFocus();
    }
    background: Rectangle {
        radius: Theme.surfaceRadius
        color: Theme.panel
        border.color: Theme.divider
    }
    contentItem: GridLayout {
        columns: 5
        rowSpacing: 4
        columnSpacing: 4
        Repeater {
            id: buttons
            model: [qsTr("Line"), qsTr("Rectangle"), qsTr("Ellipse"), qsTr("Rounded rectangle"), qsTr("Triangle"), qsTr("Diamond"), qsTr("Pentagon"), qsTr("Hexagon"), qsTr("Right arrow"), qsTr("Left arrow"), qsTr("Up arrow"), qsTr("Down arrow"), qsTr("Five-point star"), qsTr("Speech bubble"), qsTr("Curve"), qsTr("Polygon"), qsTr("Four-point star"), qsTr("Six-point star"), qsTr("Oval speech bubble"), qsTr("Spline")]
            delegate: PaintButton {
                id: choice
                required property int index
                required property string modelData
                objectName: "shapeChoice" + index
                text: modelData
                implicitWidth: 38
                implicitHeight: 38
                padding: 5
                checkable: true
                autoExclusive: true
                checked: shapePopup.editor.tool === Editor.Shape && shapePopup.editor.shapeKind === index
                onClicked: {
                    shapePopup.editor.shapeKind = index;
                    shapePopup.editor.tool = Editor.Shape;
                    shapePopup.close();
                }
                contentItem: ShapePreview {
                    kind: choice.index
                    color: Theme.text
                }
                Accessible.name: text
                ToolTip.visible: hovered
                ToolTip.delay: 400
                ToolTip.text: text
                KeyNavigation.right: buttons.itemAt((index + 1) % 20)
                KeyNavigation.left: buttons.itemAt((index + 19) % 20)
                KeyNavigation.down: buttons.itemAt((index + 5) % 20)
                KeyNavigation.up: buttons.itemAt((index + 15) % 20)
            }
        }
    }
}
