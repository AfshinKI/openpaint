// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import OpenPaint

Item {
    id: ruler
    required property CanvasItem viewport
    required property real imageLength
    property bool vertical: false
    readonly property real offset: vertical ? viewport.origin.y : viewport.origin.x
    readonly property real zoomFactor: viewport.zoom
    readonly property real length: vertical ? height : width
    readonly property color inkColor: Theme.text
    readonly property color tickColor: Theme.secondaryText
    readonly property real marker: vertical ? viewport.cursorPosition.y : viewport.cursorPosition.x
    readonly property real majorStep: {
        const desired = 64 / zoomFactor;
        const base = Math.pow(10, Math.floor(Math.log10(desired)));
        const ratio = desired / base;
        return Math.max(1, (ratio <= 1 ? 1 : ratio <= 2 ? 2 : ratio <= 5 ? 5 : 10) * base);
    }
    clip: true
    Accessible.role: Accessible.StaticText
    Accessible.name: vertical ? qsTr("Vertical ruler in pixels") : qsTr("Horizontal ruler in pixels")
    onOffsetChanged: ticks.requestPaint()
    onZoomFactorChanged: ticks.requestPaint()
    onImageLengthChanged: ticks.requestPaint()
    onInkColorChanged: ticks.requestPaint()
    onTickColorChanged: ticks.requestPaint()
    onMajorStepChanged: ticks.requestPaint()
    GlassSurface {
        objectName: "rulerSurface"
        anchors.fill: parent
        anchors.margins: 2
        radius: Theme.controlRadius
        raisedSurface: false
        backdropSource: null
    }
    Canvas {
        id: ticks
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();
            ctx.clearRect(0, 0, width, height);
            const thickness = (ruler.vertical ? width : height) - 2;
            const step = Math.max(1, ruler.majorStep / 5);
            const first = Math.max(0, Math.ceil(-ruler.offset / ruler.zoomFactor / step));
            const last = Math.min(Math.floor(ruler.imageLength / step), Math.floor((ruler.length - ruler.offset) / ruler.zoomFactor / step));
            ctx.font = "10px sans-serif";
            ctx.lineWidth = 1;
            for (let index = first; index <= last; ++index) {
                const value = index * step;
                const position = ruler.offset + value * ruler.zoomFactor;
                const major = Math.abs(value / ruler.majorStep - Math.round(value / ruler.majorStep)) < 0.0001;
                ctx.strokeStyle = ruler.tickColor;
                ctx.beginPath();
                if (ruler.vertical) {
                    ctx.moveTo(thickness, position);
                    ctx.lineTo(thickness - (major ? 8 : 4), position);
                } else {
                    ctx.moveTo(position, thickness);
                    ctx.lineTo(position, thickness - (major ? 8 : 4));
                }
                ctx.stroke();
                if (major) {
                    ctx.fillStyle = ruler.inkColor;
                    ctx.save();
                    if (ruler.vertical) {
                        ctx.translate(14, position + 3);
                        ctx.rotate(-Math.PI / 2);
                        ctx.fillText(String(Math.round(value)), 0, 0);
                    } else {
                        ctx.fillText(String(Math.round(value)), position + 3, 14);
                    }
                    ctx.restore();
                }
            }
        }
    }
    Rectangle {
        visible: ruler.viewport.cursorVisible && ruler.viewport.cursorPosition.x >= 0 && ruler.viewport.cursorPosition.y >= 0 && ruler.viewport.cursorPosition.x <= ruler.viewport.editor.imageSize.width && ruler.viewport.cursorPosition.y <= ruler.viewport.editor.imageSize.height
        color: Theme.accent
        opacity: 0.8
        x: ruler.vertical ? 2 : ruler.offset + ruler.marker * ruler.zoomFactor
        y: ruler.vertical ? ruler.offset + ruler.marker * ruler.zoomFactor : 2
        width: ruler.vertical ? parent.width - 4 : 1
        height: ruler.vertical ? 1 : parent.height - 4
    }
}
