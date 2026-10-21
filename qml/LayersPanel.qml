// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel
    required property Editor editor
    signal closeRequested
    color: Theme.panel
    border.color: Theme.divider
    radius: 8
    readonly property var currentLayer: editor.layers.length ? editor.layers[editor.layers.length - 1 - editor.activeLayer] : null
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: wheel => wheel.accepted = true
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        RowLayout {
            Label {
                text: qsTr("Layers")
                color: Theme.text
                font.bold: true
                Layout.fillWidth: true
            }
            PaintButton {
                text: qsTr("Close")
                onClicked: panel.closeRequested()
            }
        }
        RowLayout {
            PaintButton {
                objectName: "addLayerButton"
                text: qsTr("Add")
                enabled: panel.editor.canEditLayers && panel.editor.layers.length < 32
                onClicked: panel.editor.addLayer()
            }
            PaintButton {
                text: qsTr("Duplicate")
                enabled: panel.editor.canEditLayers && panel.editor.layers.length < 32
                onClicked: panel.editor.addLayer(true)
            }
        }
        ListView {
            id: list
            objectName: "layersList"
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            spacing: 4
            model: panel.editor.layers
            ScrollBar.vertical: ScrollBar {}
            delegate: ItemDelegate {
                id: row
                required property var modelData
                width: list.width
                height: 48
                text: modelData.name
                highlighted: modelData.index === panel.editor.activeLayer
                enabled: panel.editor.canEditLayers
                Accessible.name: text + (modelData.visible ? qsTr(", visible") : qsTr(", hidden"))
                onClicked: panel.editor.selectLayer(modelData.index)
                background: Rectangle {
                    radius: 6
                    color: row.highlighted ? Theme.selected : row.hovered ? Theme.hover : "transparent"
                    border.color: row.activeFocus ? Theme.accent : Theme.divider
                    border.width: row.highlighted || row.activeFocus ? 1 : 0
                }
                contentItem: RowLayout {
                    LayerThumbnail {
                        editor: panel.editor
                        layerIndex: row.modelData.index
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 32
                    }
                    Label {
                        text: row.modelData.visible ? "●" : "○"
                        color: Theme.text
                    }
                    Label {
                        text: row.text
                        color: Theme.text
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Label {
                        text: row.modelData.opacity + "%"
                        color: Theme.text
                    }
                }
            }
        }
        TextField {
            objectName: "layerNameField"
            Layout.fillWidth: true
            text: panel.currentLayer ? panel.currentLayer.name : ""
            maximumLength: 128
            implicitHeight: 32
            enabled: panel.editor.canEditLayers
            Accessible.name: qsTr("Layer name")
            onEditingFinished: panel.editor.renameLayer(text)
        }
        RowLayout {
            PaintCheckBox {
                text: qsTr("Visible")
                checked: panel.currentLayer ? panel.currentLayer.visible : true
                enabled: panel.editor.canEditLayers
                onToggled: panel.editor.setLayerVisible(checked)
            }
            Label {
                text: qsTr("Opacity")
                color: Theme.text
            }
            SpinBox {
                objectName: "layerOpacity"
                from: 0
                to: 100
                implicitWidth: 100
                value: panel.currentLayer ? panel.currentLayer.opacity : 100
                enabled: panel.editor.canEditLayers
                Accessible.name: qsTr("Layer opacity percent")
                onValueModified: panel.editor.setLayerOpacity(value)
            }
        }
        RowLayout {
            PaintButton {
                text: qsTr("Up")
                enabled: panel.editor.canEditLayers && panel.editor.activeLayer < panel.editor.layers.length - 1
                onClicked: panel.editor.moveLayer(panel.editor.activeLayer + 1)
            }
            PaintButton {
                text: qsTr("Down")
                enabled: panel.editor.canEditLayers && panel.editor.activeLayer > 0
                onClicked: panel.editor.moveLayer(panel.editor.activeLayer - 1)
            }
            PaintButton {
                text: qsTr("Delete")
                enabled: panel.editor.canEditLayers && panel.editor.layers.length > 1
                onClicked: panel.editor.deleteLayer()
            }
        }
        PaintButton {
            text: qsTr("Merge down")
            enabled: panel.editor.canEditLayers && panel.editor.activeLayer > 0 && panel.currentLayer.visible && panel.editor.layers[panel.editor.layers.length - panel.editor.activeLayer].visible
            onClicked: panel.editor.mergeLayerDown()
        }
        PaintCheckBox {
            text: qsTr("Picker: sample visible layers")
            checked: panel.editor.sampleMerged
            onToggled: panel.editor.sampleMerged = checked
        }
    }
}
