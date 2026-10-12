// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    title: qsTr("Selection bounds")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Close
    width: 360
    GridLayout {
        columns: 2
        Label {
            text: qsTr("X")
            color: Theme.text
        }
        PaintSpinBox {
            from: 0
            to: dialog.editor.imageSize.width - 1
            editable: true
            value: dialog.editor.selectionRect.x
            onValueModified: dialog.editor.selectRect(value, dialog.editor.selectionRect.y, dialog.editor.selectionRect.width, dialog.editor.selectionRect.height)
        }
        Label {
            text: qsTr("Y")
            color: Theme.text
        }
        PaintSpinBox {
            from: 0
            to: dialog.editor.imageSize.height - 1
            editable: true
            value: dialog.editor.selectionRect.y
            onValueModified: dialog.editor.selectRect(dialog.editor.selectionRect.x, value, dialog.editor.selectionRect.width, dialog.editor.selectionRect.height)
        }
        Label {
            text: qsTr("Width")
            color: Theme.text
        }
        PaintSpinBox {
            from: 1
            to: dialog.editor.imageSize.width
            editable: true
            value: dialog.editor.selectionRect.width
            onValueModified: dialog.editor.selectRect(dialog.editor.selectionRect.x, dialog.editor.selectionRect.y, value, dialog.editor.selectionRect.height)
        }
        Label {
            text: qsTr("Height")
            color: Theme.text
        }
        PaintSpinBox {
            from: 1
            to: dialog.editor.imageSize.height
            editable: true
            value: dialog.editor.selectionRect.height
            onValueModified: dialog.editor.selectRect(dialog.editor.selectionRect.x, dialog.editor.selectionRect.y, dialog.editor.selectionRect.width, value)
        }
    }
}
