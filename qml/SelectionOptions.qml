// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    title: editor.floatingActive ? qsTr("Selection position") : qsTr("Selection bounds")
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
            from: dialog.editor.floatingActive ? 1 - dialog.editor.selectionRect.width : 0
            to: dialog.editor.imageSize.width - 1
            editable: true
            value: dialog.editor.selectionRect.x
            onValueModified: dialog.editor.floatingActive ? dialog.editor.moveFloating(value, dialog.editor.floatingPosition.y) : dialog.editor.selectRect(value, dialog.editor.selectionRect.y, dialog.editor.selectionRect.width, dialog.editor.selectionRect.height)
        }
        Label {
            text: qsTr("Y")
            color: Theme.text
        }
        PaintSpinBox {
            from: dialog.editor.floatingActive ? 1 - dialog.editor.selectionRect.height : 0
            to: dialog.editor.imageSize.height - 1
            editable: true
            value: dialog.editor.selectionRect.y
            onValueModified: dialog.editor.floatingActive ? dialog.editor.moveFloating(dialog.editor.floatingPosition.x, value) : dialog.editor.selectRect(dialog.editor.selectionRect.x, value, dialog.editor.selectionRect.width, dialog.editor.selectionRect.height)
        }
        Label {
            text: qsTr("Width")
            color: Theme.text
        }
        PaintSpinBox {
            from: 1
            to: dialog.editor.imageSize.width
            editable: true
            enabled: !dialog.editor.floatingActive
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
            enabled: !dialog.editor.floatingActive
            value: dialog.editor.selectionRect.height
            onValueModified: dialog.editor.selectRect(dialog.editor.selectionRect.x, dialog.editor.selectionRect.y, dialog.editor.selectionRect.width, value)
        }
    }
}
