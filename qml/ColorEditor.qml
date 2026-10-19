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
    property string colorTarget: "primary"
    property int selectedSlot: -1
    property int replacementSlot: -1
    signal colorChosen(color chosen)
    function begin(color, target: string) {
        colorTarget = target;
        colorDraft.color = color;
        hexField.text = colorDraft.hex;
        selectedSlot = -1;
        replacementSlot = -1;
        open();
    }
    function addColor() {
        if (paletteStore.add(colorDraft.color))
            selectedSlot = paletteStore.colors.length - 1;
    }
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(664, parent.width - 32)
    title: qsTr("Edit colors")
    modal: true
    closePolicy: Popup.CloseOnEscape
    onAccepted: colorChosen(colorDraft.color)
    onOpened: hexField.forceActiveFocus()
    ColorSelection {
        id: colorDraft
        objectName: "colorSelection"
    }
    Connections {
        target: colorDraft
        function onChanged() {
            hexField.text = colorDraft.hex;
        }
    }
    contentItem: ColumnLayout {
        Accessible.name: dialog.colorTarget === "primary" ? qsTr("Edit primary color") : qsTr("Edit secondary color")
        spacing: 12
        RowLayout {
            spacing: 12
            ColorSpectrum {
                objectName: "colorSpectrum"
                selection: colorDraft
                Layout.preferredWidth: 256
                Layout.preferredHeight: 256
                Accessible.role: Accessible.Pane
                Accessible.name: qsTr("Hue and saturation spectrum. Use the numeric controls for keyboard colorDraft.")
            }
            Rectangle {
                Layout.preferredWidth: 36
                Layout.preferredHeight: 256
                radius: 4
                color: colorDraft.color
                border.color: Theme.divider
                Accessible.role: Accessible.Indicator
                Accessible.name: qsTr("Selected color %1").arg(colorDraft.hex)
            }
            Slider {
                id: brightness
                objectName: "brightnessSlider"
                orientation: Qt.Vertical
                from: 0
                to: 100
                stepSize: 1
                Keys.onPressed: function (event) {
                    if (event.key === Qt.Key_Home || event.key === Qt.Key_End) {
                        colorDraft.setHsv(colorDraft.hue, colorDraft.saturation, event.key === Qt.Key_End ? 100 : 0);
                        event.accepted = true;
                    }
                }
                value: colorDraft.value
                Layout.preferredWidth: 24
                Layout.preferredHeight: 256
                Accessible.name: qsTr("Brightness, 0 to 100")
                onMoved: colorDraft.setHsv(colorDraft.hue, colorDraft.saturation, value)
                background: Rectangle {
                    x: (brightness.width - width) / 2
                    y: brightness.topPadding
                    width: 10
                    height: brightness.availableHeight
                    radius: 5
                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: "white"
                        }
                        GradientStop {
                            position: 1
                            color: "black"
                        }
                    }
                }
                handle: Rectangle {
                    x: (brightness.width - width) / 2
                    y: brightness.topPadding + brightness.visualPosition * (brightness.availableHeight - height)
                    width: 16
                    height: 16
                    radius: 8
                    color: "white"
                    border.width: 3
                    border.color: brightness.visualFocus ? Theme.accent : Theme.divider
                }
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: 10
                TextField {
                    id: hexField
                    objectName: "hexField"
                    Layout.preferredWidth: 140
                    implicitHeight: 32
                    maximumLength: 7
                    selectByMouse: true
                    color: Theme.text
                    validator: RegularExpressionValidator {
                        regularExpression: /#?[0-9a-fA-F]{6}/
                    }
                    Accessible.name: qsTr("Hexadecimal color")
                    onTextEdited: if (acceptableInput)
                        colorDraft.setHex(text)
                    background: GlassSurface {
                        backdropLayers: hostWindow ? [hostWindow.background, hostWindow.canvasController] : []
                        radius: Theme.controlRadius
                        color: Theme.hover
                        border.width: hexField.activeFocus ? 2 : 1
                        border.color: hexField.activeFocus ? Theme.accent : Theme.divider
                    }
                }
                ComboBox {
                    id: colorMode
                    objectName: "colorMode"
                    model: ["RGB", "HSV"]
                    implicitHeight: 32
                    Layout.preferredWidth: 140
                    Accessible.name: qsTr("Color model")
                    palette.buttonText: Theme.text
                    palette.text: Theme.text
                    background: GlassSurface {
                        backdropLayers: hostWindow ? [hostWindow.background, hostWindow.canvasController] : []
                        radius: Theme.controlRadius
                        color: Theme.hover
                        border.color: colorMode.visualFocus ? Theme.accent : Theme.divider
                    }
                    indicator: Label {
                        x: colorMode.width - width - 12
                        y: (colorMode.height - height) / 2
                        text: "⌄"
                        color: Theme.text
                    }
                    delegate: PaintMenuItem {
                        symbol: "palette"
                        required property int index
                        highlighted: colorMode.highlightedIndex === index
                        required property string modelData
                        text: modelData
                        width: colorMode.popup.availableWidth
                    }
                    popup: Popup {
                        padding: 6
                        implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding
                        background: Rectangle {
                            radius: Theme.surfaceRadius
                            color: Theme.panel
                            border.color: Theme.divider
                        }
                        y: colorMode.height
                        width: colorMode.width
                        contentItem: ListView {
                            implicitHeight: contentHeight
                            model: colorMode.popup.visible ? colorMode.delegateModel : null
                            currentIndex: colorMode.highlightedIndex
                        }
                    }
                }
                Repeater {
                    model: 3
                    delegate: RowLayout {
                        id: channel
                        required property int index
                        readonly property bool rgb: colorMode.currentIndex === 0
                        readonly property string channelName: rgb ? [qsTr("Red"), qsTr("Green"), qsTr("Blue")][index] : [qsTr("Hue °"), qsTr("Saturation %"), qsTr("Value %")][index]
                        PaintSpinBox {
                            id: channelInput
                            objectName: "channelInput" + channel.index
                            implicitWidth: 120
                            up.indicator: Item {}
                            down.indicator: Item {}
                            background: Rectangle {
                                radius: Theme.controlRadius
                                color: Theme.hover
                                border.color: channelInput.activeFocus ? Theme.accent : Theme.divider
                            }
                            implicitHeight: 32
                            from: 0
                            to: channel.rgb ? 255 : channel.index === 0 ? 359 : 100
                            editable: true
                            value: channel.rgb ? [colorDraft.red, colorDraft.green, colorDraft.blue][channel.index] : Math.round([colorDraft.hue, colorDraft.saturation, colorDraft.value][channel.index]) % (channel.index === 0 ? 360 : 101)
                            Accessible.name: channel.channelName
                            onValueModified: {
                                if (channel.rgb) {
                                    let values = [colorDraft.red, colorDraft.green, colorDraft.blue];
                                    values[channel.index] = value;
                                    colorDraft.setRgb(values[0], values[1], values[2]);
                                } else {
                                    let values = [colorDraft.hue, colorDraft.saturation, colorDraft.value];
                                    values[channel.index] = value;
                                    colorDraft.setHsv(values[0], values[1], values[2]);
                                }
                            }
                        }
                        Label {
                            text: channel.channelName
                            color: Theme.text
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
        RowLayout {
            spacing: 24
            ColumnLayout {
                spacing: 8
                Label {
                    text: qsTr("Basic colors")
                    color: Theme.text
                    Layout.preferredHeight: 32
                    verticalAlignment: Text.AlignVCenter
                }
                GridLayout {
                    columns: 12
                    columnSpacing: 0
                    rowSpacing: 2
                    Repeater {
                        model: ["#f08080", "#ef3030", "#774444", "#994444", "#641414", "#a6f5f5", "#72efef", "#3388ee", "#2445cc", "#10109c", "#224477", "#151580", "#ffff99", "#ffff22", "#ffaa55", "#e07a33", "#995b22", "#8b852f", "#9278ed", "#5233bf", "#249cbb", "#103b55", "#144238", "#53105b", "#b3ff88", "#7cfa48", "#55b51e", "#22e435", "#57ad29", "#809b67", "#efa0ef", "#eb69d3", "#e323e3", "#a513ad", "#8880bb", "#da3589", "#357e25", "#347867", "#428a95", "#396c71", "#18391b", "#3c4526", "#73247a", "#502094", "#000000", "#888888", "#cccccc", "#ffffff"]
                        delegate: ColorSwatch {
                            required property string modelData
                            objectName: "basicDialogSwatch"
                            implicitWidth: 30
                            implicitHeight: 30
                            diameter: 20
                            swatchColor: modelData
                            checked: colorDraft.hex.toLowerCase() === modelData
                            onClicked: colorDraft.color = modelData
                        }
                    }
                }
            }
            ColumnLayout {
                Layout.maximumWidth: 180
                spacing: 8
                RowLayout {
                    Label {
                        text: qsTr("Custom colors")
                        color: Theme.text
                        Layout.fillWidth: true
                    }
                    PaintButton {
                        objectName: "addCustomColor"
                        text: "+"
                        implicitWidth: 32
                        implicitHeight: 32
                        Accessible.name: qsTr("Add custom color")
                        enabled: dialog.paletteStore.colors.length < 24 && hexField.acceptableInput
                        onClicked: dialog.addColor()
                    }
                    PaintButton {
                        id: managePalette
                        text: "⋯"
                        implicitWidth: 28
                        implicitHeight: 32
                        Accessible.name: qsTr("Manage custom colors")
                        onClicked: paletteMenu.open()
                        PaintMenu {
                            id: paletteMenu
                            y: managePalette.height
                            PaintMenuItem {
                                symbol: "palette"
                                text: qsTr("Replace selected")
                                enabled: dialog.selectedSlot >= 0 && hexField.acceptableInput
                                onTriggered: dialog.paletteStore.replace(dialog.selectedSlot, colorDraft.color)
                            }
                            PaintMenuItem {
                                symbol: "delete"
                                text: qsTr("Remove selected")
                                enabled: dialog.selectedSlot >= 0
                                onTriggered: if (dialog.paletteStore.remove(dialog.selectedSlot))
                                    dialog.selectedSlot = -1
                            }
                            PaintMenuItem {
                                symbol: "delete"
                                text: qsTr("Clear all")
                                enabled: dialog.paletteStore.colors.length > 0
                                onTriggered: if (dialog.paletteStore.clear())
                                    dialog.selectedSlot = -1
                            }
                        }
                    }
                }
                GridLayout {
                    columns: 6
                    columnSpacing: 2
                    rowSpacing: 2
                    Repeater {
                        model: 24
                        delegate: ColorSwatch {
                            required property int index
                            objectName: "customDialogSwatch"
                            implicitWidth: 28
                            implicitHeight: 30
                            diameter: 20
                            empty: index >= dialog.paletteStore.colors.length
                            swatchColor: empty ? "transparent" : dialog.paletteStore.colors[index]
                            checked: !empty && dialog.selectedSlot === index
                            text: empty ? qsTr("Empty custom slot %1. Save current color in next free slot.").arg(index + 1) : qsTr("Custom color %1: %2").arg(index + 1).arg(swatchColor.toString())
                            onClicked: {
                                if (empty) {
                                    if (hexField.acceptableInput)
                                        dialog.addColor();
                                } else {
                                    dialog.selectedSlot = index;
                                    colorDraft.color = swatchColor;
                                }
                            }
                        }
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: dialog.paletteStore.error.length ? dialog.paletteStore.error : qsTr("Custom colors last until you close the app. OK adds to empty slots in both palettes. Cancel discards the color draft.")
            color: Theme.secondaryText
        }
    }
    footer: DialogButtonBox {
        padding: 12
        spacing: 8
        background: Rectangle {
            color: Theme.window
            radius: Theme.dialogRadius
        }
        PaintButton {
            id: acceptButton
            objectName: "acceptColor"
            text: qsTr("OK")
            enabled: hexField.acceptableInput
            highlighted: true
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
        PaintButton {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
}
