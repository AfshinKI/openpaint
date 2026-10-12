// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

Popup {
    id: palette
    required property Editor editor
    width: 224
    padding: 8
    focus: true
    popupType: Popup.Item
    onOpened: {
        const button = buttons.itemAt(Math.max(0, Math.min(editor.shapeKind, 18)));
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
            model: [qsTr("Line"), qsTr("Rectangle"), qsTr("Ellipse"), qsTr("Rounded rectangle"), qsTr("Triangle"), qsTr("Diamond"), qsTr("Pentagon"), qsTr("Hexagon"), qsTr("Right arrow"), qsTr("Left arrow"), qsTr("Up arrow"), qsTr("Down arrow"), qsTr("Five-point star"), qsTr("Speech bubble"), qsTr("Curve"), qsTr("Polygon"), qsTr("Four-point star"), qsTr("Six-point star"), qsTr("Oval speech bubble")]
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
                checked: palette.editor.tool === Editor.Shape && palette.editor.shapeKind === index
                onClicked: {
                    palette.editor.shapeKind = index;
                    palette.editor.tool = Editor.Shape;
                    palette.close();
                }
                contentItem: ShapePreview {
                    kind: choice.index
                    color: Theme.text
                }
                Accessible.name: text
                ToolTip.visible: hovered
                ToolTip.delay: 400
                ToolTip.text: text
                KeyNavigation.right: buttons.itemAt((index + 1) % 19)
                KeyNavigation.left: buttons.itemAt((index + 18) % 19)
                KeyNavigation.down: buttons.itemAt((index + 5) % 19)
                KeyNavigation.up: buttons.itemAt((index + 14) % 19)
            }
        }
    }
}
