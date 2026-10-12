// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    objectName: "colorEditor"
    required property ColorPalette paletteStore
    property color originalColor: "black"
    property string colorTarget: "primary"
    property int selectedSlot: -1
    signal colorChosen(color chosen)
    function begin(color, target: string) {
        originalColor = color;
        colorTarget = target;
        selection.color = color;
        hexField.text = selection.hex;
        selectedSlot = -1;
        open();
    }
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(720, parent.width - 32)
    title: colorTarget === "primary" ? qsTr("Edit primary color") : qsTr("Edit secondary color")
    modal: true
    closePolicy: Popup.CloseOnEscape
    onAccepted: colorChosen(selection.color)
    onOpened: hexField.forceActiveFocus()
    ColorSelection {
        id: selection
        objectName: "colorSelection"
    }
    Connections {
        target: selection
        function onChanged() {
            hexField.text = selection.hex;
        }
    }
    contentItem: ColumnLayout {
        spacing: 12
        RowLayout {
            spacing: 24
            ColorWheel {
                id: wheel
                objectName: "colorWheel"
                selection: selection
                Layout.preferredWidth: 256
                Layout.preferredHeight: 256
                Accessible.role: Accessible.Pane
                Accessible.name: qsTr("Hue ring and saturation/value square. Numeric controls are available alongside.")
            }
            ColumnLayout {
                spacing: 12
                Layout.fillWidth: true
                RowLayout {
                    Label {
                        text: qsTr("HEX")
                        color: Theme.secondaryText
                    }
                    TextField {
                        id: hexField
                        objectName: "hexField"
                        Layout.fillWidth: true
                        maximumLength: 7
                        selectByMouse: true
                        validator: RegularExpressionValidator {
                            regularExpression: /#?[0-9a-fA-F]{6}/
                        }
                        Accessible.name: qsTr("Hexadecimal color")
                        onTextEdited: if (acceptableInput)
                            selection.setHex(text)
                        background: Rectangle {
                            radius: Theme.controlRadius
                            color: Theme.panel
                            border.width: hexField.activeFocus ? 2 : 1
                            border.color: hexField.activeFocus ? Theme.accent : Theme.divider
                        }
                    }
                }
                RowLayout {
                    spacing: 8
                    ColumnLayout {
                        Label {
                            text: qsTr("Red")
                            color: Theme.secondaryText
                        }
                        PaintSpinBox {
                            objectName: "redInput"
                            from: 0
                            to: 255
                            editable: true
                            value: selection.red
                            onValueModified: selection.setRgb(value, selection.green, selection.blue)
                            Accessible.name: qsTr("Red, 0 to 255")
                        }
                    }
                    ColumnLayout {
                        Label {
                            text: qsTr("Green")
                            color: Theme.secondaryText
                        }
                        PaintSpinBox {
                            from: 0
                            to: 255
                            editable: true
                            value: selection.green
                            onValueModified: selection.setRgb(selection.red, value, selection.blue)
                            Accessible.name: qsTr("Green, 0 to 255")
                        }
                    }
                    ColumnLayout {
                        Label {
                            text: qsTr("Blue")
                            color: Theme.secondaryText
                        }
                        PaintSpinBox {
                            from: 0
                            to: 255
                            editable: true
                            value: selection.blue
                            onValueModified: selection.setRgb(selection.red, selection.green, value)
                            Accessible.name: qsTr("Blue, 0 to 255")
                        }
                    }
                }
                RowLayout {
                    spacing: 8
                    ColumnLayout {
                        Label {
                            text: qsTr("Hue °")
                            color: Theme.secondaryText
                        }
                        PaintSpinBox {
                            from: 0
                            to: 359
                            editable: true
                            value: Math.round(selection.hue) % 360
                            onValueModified: selection.setHsv(value, selection.saturation, selection.value)
                            Accessible.name: qsTr("Hue in degrees")
                        }
                    }
                    ColumnLayout {
                        Label {
                            text: qsTr("Saturation %")
                            color: Theme.secondaryText
                        }
                        PaintSpinBox {
                            from: 0
                            to: 100
                            editable: true
                            value: Math.round(selection.saturation)
                            onValueModified: selection.setHsv(selection.hue, value, selection.value)
                            Accessible.name: qsTr("Saturation percent")
                        }
                    }
                    ColumnLayout {
                        Label {
                            text: qsTr("Value %")
                            color: Theme.secondaryText
                        }
                        PaintSpinBox {
                            from: 0
                            to: 100
                            editable: true
                            value: Math.round(selection.value)
                            onValueModified: selection.setHsv(selection.hue, selection.saturation, value)
                            Accessible.name: qsTr("Value percent")
                        }
                    }
                }
                RowLayout {
                    spacing: 12
                    Rectangle {
                        implicitWidth: 64
                        implicitHeight: 36
                        radius: Theme.controlRadius
                        color: dialog.originalColor
                        border.color: Theme.divider
                    }
                    Label {
                        text: qsTr("Original")
                        color: Theme.secondaryText
                    }
                    Rectangle {
                        implicitWidth: 64
                        implicitHeight: 36
                        radius: Theme.controlRadius
                        color: selection.color
                        border.color: Theme.divider
                    }
                    Label {
                        text: qsTr("New")
                        color: Theme.secondaryText
                    }
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: Theme.divider
        }
        Label {
            text: qsTr("Custom colors · %1 / 10").arg(dialog.paletteStore.colors.length)
            font.bold: true
            color: Theme.text
        }
        RowLayout {
            spacing: 4
            Repeater {
                model: dialog.paletteStore.colors
                delegate: ColorSwatch {
                    required property string modelData
                    required property int index
                    swatchColor: modelData
                    checked: dialog.selectedSlot === index
                    text: qsTr("Custom color %1: %2").arg(index + 1).arg(modelData)
                    onClicked: {
                        dialog.selectedSlot = index;
                        selection.color = modelData;
                    }
                }
            }
            Label {
                visible: dialog.paletteStore.colors.length === 0
                text: qsTr("Add colors here to keep them for your next drawing.")
                color: Theme.secondaryText
            }
            Item {
                Layout.fillWidth: true
            }
        }
        RowLayout {
            PaintButton {
                objectName: "addCustomColor"
                text: qsTr("Add color")
                enabled: dialog.paletteStore.colors.length < 10 && hexField.acceptableInput
                onClicked: if (dialog.paletteStore.add(selection.color))
                    dialog.selectedSlot = dialog.paletteStore.colors.length - 1
            }
            PaintButton {
                text: qsTr("Replace selected")
                enabled: dialog.selectedSlot >= 0 && hexField.acceptableInput
                onClicked: dialog.paletteStore.replace(dialog.selectedSlot, selection.color)
            }
            PaintButton {
                text: qsTr("Remove selected")
                enabled: dialog.selectedSlot >= 0
                onClicked: {
                    dialog.paletteStore.remove(dialog.selectedSlot);
                    dialog.selectedSlot = -1;
                }
            }
            PaintButton {
                text: qsTr("Clear all")
                enabled: dialog.paletteStore.colors.length > 0
                onClicked: {
                    dialog.paletteStore.clear();
                    dialog.selectedSlot = -1;
                }
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: dialog.paletteStore.error.length ? dialog.paletteStore.error : qsTr("Palette buttons save locally immediately. Cancel only discards the drawing-color change.")
            color: Theme.secondaryText
        }
    }
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        padding: 12
        spacing: 8
        background: Item {}
        PaintButton {
            objectName: "acceptColor"
            text: qsTr("OK")
            enabled: hexField.acceptableInput
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
        PaintButton {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
}
