// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel
    required property Editor editor
    readonly property bool propertiesOpen: properties.visible
    readonly property var currentLayer: editor.layers[editor.layers.length - 1 - editor.activeLayer] || null
    property string draggedId: ""
    property int draggedIndex: -1
    property int dropIndex: -1
    property point dragPoint
    color: Theme.panel
    function cancelDrag() {
        draggedId = "";
        dropIndex = -1;
    }
    function updateDrag(layerId: string, position: point) {
        if (!editor.canEditLayers) {
            cancelDrag();
            return;
        }
        if (!draggedId)
            draggedIndex = editor.activeLayer;
        draggedId = layerId;
        dragPoint = position;
        const local = list.mapFromItem(panel, position.x, position.y);
        if (local.x < 0 || local.x > list.width || local.y < 0 || local.y > list.height) {
            dropIndex = -1;
            return;
        }
        const row = Math.max(0, Math.min(list.count - 1, Math.floor((local.y + list.contentY) / 120)));
        dropIndex = editor.layers.length - 1 - row;
    }
    function finishDrag(layerId: string, position: point) {
        // Escape clears draggedId even while the pointer remains held.
        if (!draggedId)
            return;
        updateDrag(layerId, position);
        const target = dropIndex;
        cancelDrag();
        if (target > 0)
            editor.reorderLayer(layerId, target);
    }
    function openMenu(index: int, position: point) {
        if (!editor.canEditLayers)
            return;
        editor.selectLayer(index);
        layerMenu.popup(position.x, position.y);
    }
    function openProperties(index: int) {
        if (!editor.canEditLayers)
            return;
        editor.selectLayer(index);
        layerName.text = currentLayer.name;
        layerOpacity.value = currentLayer.opacity;
        properties.open();
    }
    onVisibleChanged: if (!visible)
        cancelDrag()
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: wheel => wheel.accepted = true
        onClicked: mouse => {
            if (mouse.button === Qt.RightButton)
                panel.openMenu(panel.editor.activeLayer, Qt.point(mouse.x, mouse.y));
        }
    }
    Rectangle {
        width: 1
        height: parent.height
        color: Theme.divider
    }
    Label {
        id: heading
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        text: qsTr("Layers")
        font.bold: true
        color: Theme.text
    }
    ListView {
        id: list
        objectName: "layersList"
        anchors.top: heading.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        anchors.bottom: backgroundSection.top
        anchors.bottomMargin: 16
        clip: true
        spacing: 12
        // A count model keeps pointer capture intact when selection notifies the UI.
        model: Math.max(0, panel.editor.layers.length - 1)
        ScrollBar.vertical: ScrollBar {}
        delegate: LayerTile {
            id: tile
            required property int index
            width: list.width
            height: 108
            editor: panel.editor
            layerIndex: panel.editor.layers.length - 1 - index
            opacity: panel.draggedId === (layerData ? layerData.id : "") ? 0.35 : 1
            onContextRequested: position => panel.openMenu(layerIndex, tile.mapToItem(panel, position.x, position.y))
            onPropertiesRequested: panel.openProperties(layerIndex)
            onDragMoved: (layerId, position) => panel.updateDrag(layerId, tile.mapToItem(panel, position.x, position.y))
            onDragReleased: (layerId, position) => panel.finishDrag(layerId, tile.mapToItem(panel, position.x, position.y))
            onDragCancelled: panel.cancelDrag()
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                y: panel.dropIndex > panel.draggedIndex ? -4 : parent.height + 2
                height: 2
                color: Theme.accent
                visible: panel.draggedId !== "" && panel.dropIndex === tile.layerIndex
            }
        }
    }
    Column {
        id: backgroundSection
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        spacing: 12
        Rectangle {
            objectName: "backgroundDivider"
            width: parent.width
            height: 1
            color: Theme.divider
        }
        Label {
            text: qsTr("Background")
            color: Theme.secondaryText
            font.pixelSize: 12
        }
        LayerTile {
            id: backgroundTile
            width: parent.width
            editor: panel.editor
            layerIndex: 0
            backgroundLayer: true
            onContextRequested: position => panel.openMenu(0, backgroundTile.mapToItem(panel, position.x, position.y))
            onPropertiesRequested: panel.openProperties(0)
        }
    }
    LayerThumbnail {
        x: 16
        y: Math.max(heading.y + heading.height, Math.min(panel.height - 108, panel.dragPoint.y - 54))
        width: panel.width - 32
        height: 108
        z: 2
        editor: panel.editor
        layerIndex: panel.draggedIndex
        opacity: 0.7
        visible: panel.draggedId !== ""
    }
    Timer {
        interval: 40
        repeat: true
        running: panel.draggedId !== ""
        onTriggered: {
            const local = list.mapFromItem(panel, panel.dragPoint.x, panel.dragPoint.y);
            const delta = local.y < 28 ? -8 : local.y > list.height - 28 ? 8 : 0;
            list.contentY = Math.max(0, Math.min(Math.max(0, list.contentHeight - list.height), list.contentY + delta));
            panel.updateDrag(panel.draggedId, panel.dragPoint);
        }
    }
    Connections {
        target: panel.editor
        function onChanged() {
            if (!panel.editor.canEditLayers)
                panel.cancelDrag();
        }
        function onDocumentSessionChanged() {
            panel.cancelDrag();
        }
    }
    PaintMenu {
        id: layerMenu
        objectName: "layerContextMenu"
        PaintMenuItem {
            objectName: "newLayerMenuItem"
            text: qsTr("New layer")
            icon.source: "icons/layer-add.svg"
            enabled: panel.editor.canEditLayers && panel.editor.layers.length < 32
            onTriggered: panel.editor.addLayer()
        }
        MenuSeparator {}
        PaintMenuItem {
            text: panel.currentLayer && panel.currentLayer.visible ? qsTr("Hide layer") : qsTr("Show layer")
            icon.source: panel.currentLayer && panel.currentLayer.visible ? "icons/layer-eye.svg" : "icons/layer-eye-hidden.svg"
            onTriggered: panel.editor.toggleLayerVisibility(panel.editor.activeLayer)
        }
        PaintMenuItem {
            text: qsTr("Duplicate layer")
            enabled: panel.editor.layers.length < 32
            onTriggered: panel.editor.addLayer(true)
        }
        PaintMenuItem {
            text: qsTr("Merge down")
            enabled: panel.editor.activeLayer > 0 && panel.currentLayer && panel.currentLayer.visible && panel.editor.layers[panel.editor.layers.length - panel.editor.activeLayer].visible
            onTriggered: panel.editor.mergeLayerDown()
        }
        PaintMenuItem {
            text: qsTr("Delete layer")
            enabled: panel.editor.activeLayer > 0
            onTriggered: panel.editor.deleteLayer()
        }
        MenuSeparator {}
        PaintMenuItem {
            text: qsTr("Move up")
            enabled: panel.editor.activeLayer > 0 && panel.editor.activeLayer < panel.editor.layers.length - 1
            onTriggered: panel.editor.moveLayer(panel.editor.activeLayer + 1)
        }
        PaintMenuItem {
            text: qsTr("Move down")
            enabled: panel.editor.activeLayer > 1
            onTriggered: panel.editor.moveLayer(panel.editor.activeLayer - 1)
        }
        PaintMenuItem {
            objectName: "layerPropertiesMenuItem"
            text: qsTr("Properties…")
            onTriggered: panel.openProperties(panel.editor.activeLayer)
        }
        MenuSeparator {}
        PaintMenuItem {
            text: qsTr("Picker: sample visible layers")
            checkable: true
            checked: panel.editor.sampleMerged
            onTriggered: panel.editor.sampleMerged = checked
        }
    }
    PaintDialog {
        id: properties
        objectName: "layerPropertiesDialog"
        parent: Overlay.overlay
        anchors.centerIn: parent
        title: panel.editor.activeLayer === 0 ? qsTr("Background properties") : qsTr("Layer properties")
        width: 320
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: {
            if (layerName.text.trim().length > 0)
                panel.editor.renameLayer(layerName.text);
            panel.editor.setLayerOpacity(layerOpacity.value);
        }
        ColumnLayout {
            anchors.fill: parent
            Label {
                text: qsTr("Name")
                color: Theme.text
            }
            TextField {
                id: layerName
                objectName: "layerNameField"
                Layout.fillWidth: true
                maximumLength: 128
                Accessible.name: qsTr("Layer name")
            }
            Label {
                text: qsTr("Opacity (%)")
                color: Theme.text
            }
            SpinBox {
                id: layerOpacity
                objectName: "layerOpacity"
                from: 0
                to: 100
                editable: true
                Accessible.name: qsTr("Layer opacity percent")
            }
        }
    }
}
