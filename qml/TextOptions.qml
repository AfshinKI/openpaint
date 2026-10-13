// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenPaint

Rectangle {
    id: dialog
    required property Editor editor
    signal returnToText
    width: Math.min(parent.width - 32, 820)
    height: controls.implicitHeight + 16
    radius: Theme.surfaceRadius
    color: Theme.panel
    border.color: Theme.divider
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: wheel => wheel.accepted = true
    }
    RowLayout {
        id: controls
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6
        PaintButton {
            objectName: "fontButton"
            text: dialog.editor.textFamily
            Layout.fillWidth: true
            Layout.minimumWidth: 120
            onClicked: {
                fontDialog.selectedFont = Qt.font({
                    family: dialog.editor.textFamily,
                    pixelSize: dialog.editor.textSize,
                    bold: dialog.editor.textBold,
                    italic: dialog.editor.textItalic
                });
                fontDialog.open();
            }
            Accessible.name: qsTr("Choose text font")
        }
        PaintSpinBox {
            objectName: "textSize"
            from: 1
            to: 512
            editable: true
            value: dialog.editor.textSize
            onValueModified: dialog.editor.textSize = value
            Accessible.name: qsTr("Text size in pixels")
            implicitWidth: 108
        }
        PaintButton {
            objectName: "textBold"
            text: "B"
            font.bold: true
            implicitWidth: 32
            checkable: true
            checked: dialog.editor.textBold
            Accessible.name: qsTr("Bold")
            onToggled: {
                dialog.editor.textBold = checked;
                dialog.returnToText();
            }
        }
        PaintButton {
            text: "I"
            font.italic: true
            implicitWidth: 32
            checkable: true
            checked: dialog.editor.textItalic
            Accessible.name: qsTr("Italic")
            onToggled: {
                dialog.editor.textItalic = checked;
                dialog.returnToText();
            }
        }
        PaintButton {
            text: "U"
            font.underline: true
            implicitWidth: 32
            checkable: true
            checked: dialog.editor.textUnderline
            Accessible.name: qsTr("Underline")
            onToggled: {
                dialog.editor.textUnderline = checked;
                dialog.returnToText();
            }
        }
        PaintButton {
            text: "S"
            font.strikeout: true
            implicitWidth: 32
            checkable: true
            checked: dialog.editor.textStrikeout
            Accessible.name: qsTr("Strikeout")
            onToggled: {
                dialog.editor.textStrikeout = checked;
                dialog.returnToText();
            }
        }
        Repeater {
            model: 3
            delegate: IconButton {
                required property int index
                symbol: ["align-left", "align-center", "align-right"][index]
                text: [qsTr("Align left"), qsTr("Align center"), qsTr("Align right")][index]
                highlighted: dialog.editor.textAlignment === index
                onClicked: {
                    dialog.editor.textAlignment = index;
                    dialog.returnToText();
                }
            }
        }
        PaintCheckBox {
            text: qsTr("Background")
            Accessible.name: qsTr("Opaque text background")
            checked: dialog.editor.textBackground
            onToggled: {
                dialog.editor.textBackground = checked;
                dialog.returnToText();
            }
        }
    }
    FontDialog {
        id: fontDialog
        title: qsTr("Choose text font")
        onRejected: dialog.returnToText()
        onAccepted: {
            dialog.editor.textFamily = selectedFont.family;
            dialog.editor.textSize = Math.max(1, Math.min(512, selectedFont.pixelSize > 0 ? selectedFont.pixelSize : Math.round(selectedFont.pointSize * 96 / 72)));
            dialog.editor.textBold = selectedFont.bold;
            dialog.editor.textItalic = selectedFont.italic;
            dialog.editor.textUnderline = selectedFont.underline;
            dialog.editor.textStrikeout = selectedFont.strikeout;
            dialog.returnToText();
        }
    }
}
