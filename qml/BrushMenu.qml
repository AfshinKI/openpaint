// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import OpenPaint

PaintMenu {
    id: brushMenuRoot
    required property Editor editor
    implicitWidth: 292
    Instantiator {
        model: [qsTr("Brush"), qsTr("Calligraphy brush"), qsTr("Calligraphy pen"), qsTr("Airbrush"), qsTr("Oil brush"), qsTr("Crayon"), qsTr("Marker"), qsTr("Natural pencil"), qsTr("Watercolor brush")]
        delegate: PaintMenuItem {
            id: item
            required property int index
            required property string modelData
            text: modelData
            leftPadding: 32
            checkable: true
            autoExclusive: true
            indicator: Text {
                x: 8
                y: (item.height - height) / 2
                width: 16
                text: item.checked ? "✓" : ""
                color: Theme.text
            }
            checked: brushMenuRoot.editor.tool === Editor.Brush && brushMenuRoot.editor.brushStyle === index
            onTriggered: {
                brushMenuRoot.editor.brushStyle = index;
                brushMenuRoot.editor.tool = Editor.Brush;
            }
            contentItem: RowLayout {
                spacing: 8
                Label {
                    text: item.text
                    color: Theme.text
                    Layout.fillWidth: true
                }
                BrushPreview {
                    style: item.index
                    color: Theme.text
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 28
                }
            }
            Accessible.name: text
        }
        onObjectAdded: (index, object) => brushMenuRoot.insertItem(index, object)
        onObjectRemoved: (index, object) => brushMenuRoot.removeItem(object)
    }
}
