// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

FocusScope {
    id: tile
    required property Editor editor
    required property int layerIndex
    property bool backgroundLayer: false
    readonly property var layerData: editor.layers[editor.layers.length - 1 - layerIndex] || null
    readonly property bool selected: editor.activeLayer === layerIndex
    signal contextRequested(point position)
    signal propertiesRequested
    signal dragMoved(string layerId, point position)
    signal dragReleased(string layerId, point position)
    signal dragCancelled
    objectName: backgroundLayer ? "backgroundLayerTile" : "layerTile" + layerIndex
    implicitHeight: 108
    activeFocusOnTab: true
    Accessible.role: Accessible.ListItem
    Accessible.name: (backgroundLayer ? qsTr("Background") : layerData ? layerData.name : "") + (layerData && !layerData.visible ? qsTr(", hidden") : "")
    Accessible.selected: selected
    Accessible.onPressAction: editor.selectLayer(layerIndex)
    HoverHandler {
        id: hover
    }
    Rectangle {
        anchors.fill: parent
        radius: 6
        color: tile.selected ? Theme.selected : hover.hovered ? Theme.hover : Theme.panel
        border.width: tile.selected || tile.activeFocus ? 2 : 1
        border.color: tile.selected || tile.activeFocus ? Theme.accent : Theme.divider
    }
    LayerThumbnail {
        anchors.fill: parent
        anchors.margins: 10
        editor: tile.editor
        layerIndex: tile.layerIndex
        opacity: tile.layerData && tile.layerData.visible ? 1 : 0.35
    }
    MouseArea {
        id: pointer
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: tile.editor.canEditLayers
        preventStealing: pressed
        property point pressPoint
        property string pressedId: ""
        property bool moving: false
        property bool cancelledDrag: false
        onPressed: mouse => {
            tile.forceActiveFocus();
            pressPoint = Qt.point(mouse.x, mouse.y);
            pressedId = tile.layerData ? tile.layerData.id : "";
            moving = false;
            cancelledDrag = false;
            if (mouse.button === Qt.LeftButton)
                tile.editor.selectLayer(tile.layerIndex);
        }
        onPositionChanged: mouse => {
            if (!pressed || cancelledDrag || !(pressedButtons & Qt.LeftButton) || tile.backgroundLayer)
                return;
            if (Math.abs(mouse.y - pressPoint.y) + Math.abs(mouse.x - pressPoint.x) > 10)
                moving = true;
            if (moving)
                tile.dragMoved(pressedId, Qt.point(mouse.x, mouse.y));
        }
        onReleased: mouse => {
            if (moving)
                tile.dragReleased(pressedId, Qt.point(mouse.x, mouse.y));
            moving = false;
        }
        onCanceled: {
            moving = false;
            tile.dragCancelled();
        }
        onClicked: mouse => {
            if (mouse.button === Qt.RightButton)
                tile.contextRequested(Qt.point(mouse.x, mouse.y));
        }
        onDoubleClicked: mouse => {
            if (mouse.button === Qt.LeftButton)
                tile.propertiesRequested();
        }
    }
    IconButton {
        id: eye
        objectName: "layerEye" + tile.layerIndex
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 4
        symbol: tile.layerData && tile.layerData.visible ? "layer-eye" : "layer-eye-hidden"
        text: tile.layerData && tile.layerData.visible ? qsTr("Hide layer") : qsTr("Show layer")
        visible: hover.hovered || tile.activeFocus || activeFocus || (tile.layerData && !tile.layerData.visible)
        enabled: tile.editor.canEditLayers
        onClicked: tile.editor.toggleLayerVisibility(tile.layerIndex)
        background: Rectangle {
            radius: 4
            color: eye.hovered || eye.activeFocus ? Theme.hover : Theme.panel
            border.color: Theme.divider
        }
    }
    ToolTip.visible: hover.hovered && !eye.hovered && !pointer.pressed
    ToolTip.delay: 650
    ToolTip.text: (tile.layerData ? tile.layerData.name + " · " + tile.layerData.opacity + "%" : "") + "\n" + qsTr("Right-click for layer options")
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape) {
            pointer.moving = false;
            pointer.cancelledDrag = true;
            tile.dragCancelled();
        } else if (event.key === Qt.Key_Menu || (event.key === Qt.Key_F10 && (event.modifiers & Qt.ShiftModifier))) {
            tile.contextRequested(Qt.point(width / 2, height / 2));
        } else if (event.key === Qt.Key_F2) {
            tile.propertiesRequested();
        } else if (event.key === Qt.Key_Space || event.key === Qt.Key_Return) {
            tile.editor.selectLayer(tile.layerIndex);
        } else if ((event.modifiers & Qt.AltModifier) && (event.key === Qt.Key_Up || event.key === Qt.Key_Down) && !tile.backgroundLayer) {
            tile.editor.reorderLayer(tile.layerData.id, tile.layerIndex + (event.key === Qt.Key_Up ? 1 : -1));
        } else if (event.key === Qt.Key_Delete && !tile.backgroundLayer) {
            tile.editor.selectLayer(tile.layerIndex);
            tile.editor.deleteLayer();
        } else {
            event.accepted = false;
            return;
        }
        event.accepted = true;
    }
}
