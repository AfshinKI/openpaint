// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    title: qsTr("Text options")
    modal: true
    width: 450
    anchors.centerIn: parent
    standardButtons: Dialog.Close
    contentItem: ColumnLayout {
        spacing: 8
        RowLayout {
            PaintButton {
                objectName: "fontButton"
                text: dialog.editor.textFamily
                Layout.preferredWidth: 240
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
            }
        }
        GridLayout {
            columns: 2
            PaintCheckBox {
                text: qsTr("Bold")
                checked: dialog.editor.textBold
                onToggled: dialog.editor.textBold = checked
            }
            PaintCheckBox {
                text: qsTr("Italic")
                checked: dialog.editor.textItalic
                onToggled: dialog.editor.textItalic = checked
            }
            PaintCheckBox {
                text: qsTr("Underline")
                checked: dialog.editor.textUnderline
                onToggled: dialog.editor.textUnderline = checked
            }
            PaintCheckBox {
                text: qsTr("Strikeout")
                checked: dialog.editor.textStrikeout
                onToggled: dialog.editor.textStrikeout = checked
            }
        }
        RowLayout {
            Label {
                text: qsTr("Alignment")
                color: Theme.text
            }
            PaintButton {
                text: dialog.editor.textAlignment === 1 ? qsTr("Center") : dialog.editor.textAlignment === 2 ? qsTr("Right") : qsTr("Left")
                onClicked: alignmentMenu.open()
                PaintMenu {
                    id: alignmentMenu
                    PaintMenuItem {
                        symbol: "align-left"
                        text: qsTr("Left")
                        onTriggered: dialog.editor.textAlignment = 0
                    }
                    PaintMenuItem {
                        symbol: "align-center"
                        text: qsTr("Center")
                        onTriggered: dialog.editor.textAlignment = 1
                    }
                    PaintMenuItem {
                        symbol: "align-right"
                        text: qsTr("Right")
                        onTriggered: dialog.editor.textAlignment = 2
                    }
                }
                Accessible.name: qsTr("Text alignment")
            }
            PaintCheckBox {
                text: qsTr("Opaque background")
                checked: dialog.editor.textBackground
                onToggled: dialog.editor.textBackground = checked
            }
        }
        GridLayout {
            columns: 4
            Label {
                text: "X"
                color: Theme.text
            }
            PaintSpinBox {
                from: 0
                to: dialog.editor.imageSize.width - 1
                value: dialog.editor.textRect.x
                onValueModified: dialog.editor.setTextBounds(value, dialog.editor.textRect.y, dialog.editor.textRect.width, dialog.editor.textRect.height)
                Accessible.name: qsTr("Text box X")
            }
            Label {
                text: "Y"
                color: Theme.text
            }
            PaintSpinBox {
                from: 0
                to: dialog.editor.imageSize.height - 1
                value: dialog.editor.textRect.y
                onValueModified: dialog.editor.setTextBounds(dialog.editor.textRect.x, value, dialog.editor.textRect.width, dialog.editor.textRect.height)
                Accessible.name: qsTr("Text box Y")
            }
            Label {
                text: qsTr("Width")
                color: Theme.text
            }
            PaintSpinBox {
                from: 1
                to: dialog.editor.imageSize.width
                value: dialog.editor.textRect.width
                onValueModified: dialog.editor.setTextBounds(dialog.editor.textRect.x, dialog.editor.textRect.y, value, dialog.editor.textRect.height)
                Accessible.name: qsTr("Text box width")
            }
            Label {
                text: qsTr("Height")
                color: Theme.text
            }
            PaintSpinBox {
                from: 1
                to: dialog.editor.imageSize.height
                value: dialog.editor.textRect.height
                onValueModified: dialog.editor.setTextBounds(dialog.editor.textRect.x, dialog.editor.textRect.y, dialog.editor.textRect.width, value)
                Accessible.name: qsTr("Text box height")
            }
        }
        Label {
            text: qsTr("Text outside the box is clipped. Apply converts text to pixels.")
            color: Theme.secondaryText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
    FontDialog {
        id: fontDialog
        title: qsTr("Choose text font")
        onAccepted: {
            dialog.editor.textFamily = selectedFont.family;
            dialog.editor.textSize = Math.max(1, Math.min(512, selectedFont.pixelSize > 0 ? selectedFont.pixelSize : Math.round(selectedFont.pointSize * 96 / 72)));
            dialog.editor.textBold = selectedFont.bold;
            dialog.editor.textItalic = selectedFont.italic;
            dialog.editor.textUnderline = selectedFont.underline;
            dialog.editor.textStrikeout = selectedFont.strikeout;
        }
    }
}
