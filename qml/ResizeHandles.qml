// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import OpenPaint

Item {
    id: handles
    required property Editor editor
    required property CanvasItem canvas
    readonly property bool canvasMode: editor.resizeBounds.width <= 0 && !editor.previewActive && !editor.hasSelection
    readonly property rect activeBounds: canvasMode ? Qt.rect(0, 0, editor.imageSize.width, editor.imageSize.height) : editor.resizeBounds
    property bool dragging: false
    property rect initialBounds
    property rect draftBounds
    property point pressPoint
    property int activeCorner: 0
    readonly property rect bounds: dragging ? draftBounds : activeBounds
    visible: bounds.width > 0 && bounds.height > 0 && !editor.busy
    function corner(rect, index) {
        return Qt.point(rect.x + (index % 2 ? rect.width : 0), rect.y + (index >= 2 ? rect.height : 0));
    }
    function adjust(point) {
        const opposite = corner(initialBounds, 3 - activeCorner);
        const left = activeCorner % 2 === 0;
        const top = activeCorner < 2;
        const x = Math.round(Math.max(left ? (canvasMode ? opposite.x - 16384 : 0) : opposite.x + 1, Math.min(left ? opposite.x - 1 : (canvasMode ? opposite.x + 16384 : editor.imageSize.width), point.x)));
        const y = Math.round(Math.max(top ? (canvasMode ? opposite.y - 16384 : 0) : opposite.y + 1, Math.min(top ? opposite.y - 1 : (canvasMode ? opposite.y + 16384 : editor.imageSize.height), point.y)));
        draftBounds = Qt.rect(left ? x : opposite.x, top ? y : opposite.y, Math.abs(x - opposite.x), Math.abs(y - opposite.y));
    }
    function apply() {
        const result = draftBounds;
        dragging = false;
        if (canvasMode)
            editor.resizeCanvas(Math.round(result.x), Math.round(result.y), Math.round(result.width), Math.round(result.height));
        else
            editor.resizeActiveBounds(Math.round(result.x), Math.round(result.y), Math.round(result.width), Math.round(result.height));
    }
    Connections {
        target: handles.canvas
        function onViewportChanged() {
            handles.dragging = false;
        }
    }
    Connections {
        target: handles.editor
        function onChanged() {
            handles.dragging = false;
        }
    }
    Rectangle {
        x: handles.canvas.origin.x + handles.bounds.x * handles.canvas.zoom
        y: handles.canvas.origin.y + handles.bounds.y * handles.canvas.zoom
        width: handles.bounds.width * handles.canvas.zoom
        height: handles.bounds.height * handles.canvas.zoom
        color: "transparent"
        border.width: 1
        border.color: Theme.accent
        visible: handles.dragging
    }
    Repeater {
        model: 4
        delegate: Item {
            id: handle
            required property int index
            objectName: (handles.canvasMode ? "canvasResizeCorner" : "resizeCorner") + index
            readonly property point position: handles.corner(handles.bounds, index)
            x: handles.canvas.origin.x + position.x * handles.canvas.zoom - width / 2
            y: handles.canvas.origin.y + position.y * handles.canvas.zoom - height / 2
            width: 16
            height: 16
            activeFocusOnTab: true
            Accessible.role: Accessible.Button
            Accessible.name: (handles.canvasMode ? qsTr("Canvas: ") : "") + [qsTr("Resize top left"), qsTr("Resize top right"), qsTr("Resize bottom left"), qsTr("Resize bottom right")][index]
            Keys.onEscapePressed: event => {
                if (handles.dragging)
                    handles.dragging = false;
                else
                    handles.editor.cancel();
                event.accepted = true;
            }
            Keys.onPressed: event => {
                let dx = 0, dy = 0;
                if (event.key === Qt.Key_Left)
                    dx = -1;
                else if (event.key === Qt.Key_Right)
                    dx = 1;
                else if (event.key === Qt.Key_Up)
                    dy = -1;
                else if (event.key === Qt.Key_Down)
                    dy = 1;
                else
                    return;
                handles.initialBounds = handles.activeBounds;
                handles.activeCorner = index;
                const step = event.modifiers & Qt.ShiftModifier ? 10 : 1;
                handles.adjust(Qt.point(position.x + dx * step, position.y + dy * step));
                handles.apply();
                event.accepted = true;
            }
            Rectangle {
                anchors.centerIn: parent
                width: handle.activeFocus ? 10 : 8
                height: width
                color: Theme.panel
                border.color: handle.activeFocus ? Theme.accent : Theme.text
                border.width: handle.activeFocus ? 2 : 1
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: handle.index === 0 || handle.index === 3 ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor
                preventStealing: true
                onPressed: mouse => {
                    handle.forceActiveFocus();
                    handles.initialBounds = handles.activeBounds;
                    handles.draftBounds = handles.initialBounds;
                    handles.activeCorner = handle.index;
                    handles.pressPoint = mapToItem(handles.canvas, mouse.x, mouse.y);
                    handles.dragging = true;
                }
                function move(mouse) {
                    const point = mapToItem(handles.canvas, mouse.x, mouse.y);
                    const start = handles.corner(handles.initialBounds, handle.index);
                    handles.adjust(Qt.point(start.x + (point.x - handles.pressPoint.x) / handles.canvas.zoom, start.y + (point.y - handles.pressPoint.y) / handles.canvas.zoom));
                }
                onPositionChanged: mouse => {
                    if (handles.dragging)
                        move(mouse);
                }
                onReleased: mouse => {
                    if (handles.dragging) {
                        move(mouse);
                        handles.apply();
                    }
                }
                onCanceled: handles.dragging = false
            }
            ToolTip.visible: activeFocus
            ToolTip.text: Accessible.name + qsTr(" · arrow keys resize; Shift = 10 px")
        }
    }
}
