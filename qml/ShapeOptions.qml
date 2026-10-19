// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    title: qsTr("Shape options")
    modal: true
    width: 440
    anchors.centerIn: parent
    standardButtons: Dialog.Apply | Dialog.Cancel
    onApplied: {
        editor.finish();
        close();
    }
    onRejected: editor.cancel()
    contentItem: ColumnLayout {
        spacing: 12
        Label {
            text: dialog.editor.shapeKind === 19 ? qsTr("Spline passes through your points. Adjust endpoints, then Apply or Cancel.") : qsTr("Adjust the preview, then Apply or Cancel.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Start X")
                color: Theme.text
            }
            PaintSpinBox {
                objectName: "shapeStartX"
                from: -1000000
                to: 1000000
                editable: true
                value: dialog.editor.shapeStart.x
                onValueModified: dialog.editor.setShapeGeometry(value, dialog.editor.shapeStart.y, dialog.editor.shapeEnd.x, dialog.editor.shapeEnd.y)
                Accessible.name: qsTr("Shape start X")
                implicitWidth: 160
            }
            Label {
                text: qsTr("Start Y")
                color: Theme.text
            }
            PaintSpinBox {
                from: -1000000
                to: 1000000
                editable: true
                value: dialog.editor.shapeStart.y
                onValueModified: dialog.editor.setShapeGeometry(dialog.editor.shapeStart.x, value, dialog.editor.shapeEnd.x, dialog.editor.shapeEnd.y)
                Accessible.name: qsTr("Shape start Y")
                implicitWidth: 160
            }
            Label {
                text: qsTr("End X")
                color: Theme.text
            }
            PaintSpinBox {
                from: -1000000
                to: 1000000
                editable: true
                value: dialog.editor.shapeEnd.x
                onValueModified: dialog.editor.setShapeGeometry(dialog.editor.shapeStart.x, dialog.editor.shapeStart.y, value, dialog.editor.shapeEnd.y)
                Accessible.name: qsTr("Shape end X")
                implicitWidth: 160
            }
            Label {
                text: qsTr("End Y")
                color: Theme.text
            }
            PaintSpinBox {
                from: -1000000
                to: 1000000
                editable: true
                value: dialog.editor.shapeEnd.y
                onValueModified: dialog.editor.setShapeGeometry(dialog.editor.shapeStart.x, dialog.editor.shapeStart.y, dialog.editor.shapeEnd.x, value)
                Accessible.name: qsTr("Shape end Y")
                implicitWidth: 160
            }
        }
        RowLayout {
            visible: dialog.editor.shapeKind === 14
            Label {
                text: qsTr("Curve bends %")
                color: Theme.text
            }
            PaintSpinBox {
                from: -200
                to: 200
                value: dialog.editor.curveBend1
                onValueModified: dialog.editor.curveBend1 = value
                Accessible.name: qsTr("First curve bend percent")
            }
            PaintSpinBox {
                from: -200
                to: 200
                value: dialog.editor.curveBend2
                onValueModified: dialog.editor.curveBend2 = value
                Accessible.name: qsTr("Second curve bend percent")
            }
        }
        PaintCheckBox {
            text: qsTr("Outline")
            checked: dialog.editor.shapeOutline
            onToggled: dialog.editor.shapeOutline = checked
        }
        PaintCheckBox {
            text: qsTr("Fill")
            enabled: dialog.editor.shapeKind !== 19 && dialog.editor.shapeKind !== 14 && dialog.editor.shapeKind !== 0
            checked: dialog.editor.shapeFill
            onToggled: dialog.editor.shapeFill = checked
        }
    }
}
