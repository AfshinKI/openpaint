// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import OpenPaint

Rectangle {
    id: gallery
    required property Editor editor
    implicitWidth: 100
    implicitHeight: 76
    radius: Theme.controlRadius
    color: Theme.panel
    border.color: Theme.divider
    Flickable {
        id: scroll
        anchors.fill: parent
        anchors.margins: 2
        clip: true
        contentWidth: width
        contentHeight: grid.height
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {
            width: 3
        }
        Grid {
            id: grid
            columns: 4
            Repeater {
                id: choices
                model: [qsTr("Line"), qsTr("Rectangle"), qsTr("Ellipse"), qsTr("Rounded rectangle"), qsTr("Triangle"), qsTr("Diamond"), qsTr("Pentagon"), qsTr("Hexagon"), qsTr("Right arrow"), qsTr("Left arrow"), qsTr("Up arrow"), qsTr("Down arrow"), qsTr("Five-point star"), qsTr("Speech bubble"), qsTr("Curve"), qsTr("Polygon"), qsTr("Four-point star"), qsTr("Six-point star"), qsTr("Oval speech bubble")]
                delegate: PaintButton {
                    id: choice
                    flat: true
                    required property int index
                    required property string modelData
                    objectName: "ribbonShape" + index
                    width: 24
                    height: 24
                    padding: 4
                    text: modelData
                    enabled: !gallery.editor.textActive
                    checkable: true
                    checked: gallery.editor.tool === Editor.Shape && gallery.editor.shapeKind === index
                    onClicked: {
                        gallery.editor.shapeKind = index;
                        gallery.editor.tool = Editor.Shape;
                    }
                    function reveal() {
                        scroll.contentY = Math.max(0, Math.min(y, grid.height - scroll.height));
                    }
                    onActiveFocusChanged: {
                        if (activeFocus)
                            reveal();
                    }
                    onCheckedChanged: {
                        if (checked)
                            reveal();
                    }
                    contentItem: ShapePreview {
                        kind: choice.index
                        color: Theme.text
                    }
                    ToolTip.visible: hovered
                    ToolTip.delay: 400
                    ToolTip.text: text
                    KeyNavigation.right: choices.itemAt((index + 1) % 19)
                    KeyNavigation.left: choices.itemAt((index + 18) % 19)
                    KeyNavigation.down: choices.itemAt((index + 4) % 19)
                    KeyNavigation.up: choices.itemAt((index + 15) % 19)
                }
            }
        }
    }
}
