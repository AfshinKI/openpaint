// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: picker
    required property Editor editor
    property string draftFamily: ""
    readonly property var families: Qt.fontFamilies()
    readonly property var matches: families.filter(family => family.toLowerCase().includes(search.text.toLowerCase()))
    title: qsTr("Choose font")
    modal: true
    width: Math.min(parent.width - 48, 560)
    height: Math.min(parent.height - 48, 520)
    anchors.centerIn: parent
    standardButtons: Dialog.Apply | Dialog.Cancel
    onOpened: {
        draftFamily = editor.textFamily;
        search.clear();
        fonts.currentIndex = matches.indexOf(draftFamily);
        if (fonts.currentIndex >= 0)
            fonts.positionViewAtIndex(fonts.currentIndex, ListView.Contain);
        search.forceActiveFocus();
    }
    onAccepted: editor.textFamily = draftFamily
    onApplied: {
        editor.textFamily = draftFamily;
        close();
    }
    contentItem: ColumnLayout {
        spacing: 10
        TextField {
            id: search
            objectName: "fontSearch"
            Layout.fillWidth: true
            placeholderText: qsTr("Search installed fonts")
            Accessible.name: placeholderText
            color: Theme.text
            placeholderTextColor: Theme.secondaryText
            selectByMouse: true
            onTextChanged: fonts.currentIndex = picker.matches.length ? 0 : -1
            Keys.onDownPressed: {
                fonts.forceActiveFocus();
                if (fonts.currentIndex < 0 && fonts.count > 0)
                    fonts.currentIndex = 0;
            }
        }
        ListView {
            id: fonts
            objectName: "fontList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: picker.matches
            boundsBehavior: Flickable.StopAtBounds
            keyNavigationEnabled: true
            highlightMoveDuration: 0
            ScrollBar.vertical: ScrollBar {}
            Keys.onSpacePressed: {
                if (currentIndex >= 0)
                    picker.draftFamily = picker.matches[currentIndex];
            }
            Keys.onReturnPressed: {
                if (currentIndex >= 0) {
                    picker.draftFamily = picker.matches[currentIndex];
                    picker.accept();
                }
            }
            delegate: ItemDelegate {
                id: row
                required property string modelData
                required property int index
                width: fonts.width
                height: 74
                clip: true
                highlighted: ListView.isCurrentItem
                Accessible.name: modelData
                Keys.onReturnPressed: {
                    picker.draftFamily = modelData;
                    picker.accept();
                }
                Keys.onEnterPressed: {
                    picker.draftFamily = modelData;
                    picker.accept();
                }
                Keys.onSpacePressed: picker.draftFamily = modelData
                onClicked: {
                    fonts.currentIndex = index;
                    picker.draftFamily = modelData;
                    fonts.forceActiveFocus();
                }
                background: Rectangle {
                    radius: Theme.controlRadius
                    color: row.highlighted ? Theme.selected : row.hovered ? Theme.hover : "transparent"
                    border.width: row.modelData === picker.draftFamily ? 1 : 0
                    border.color: Theme.accent
                }
                contentItem: Item {
                    Text {
                        width: parent.width
                        height: 20
                        text: row.modelData
                        color: Theme.text
                        elide: Text.ElideRight
                        font.pixelSize: 13
                    }
                    Text {
                        width: parent.width
                        y: 23
                        height: 36
                        verticalAlignment: Text.AlignVCenter
                        clip: true
                        text: qsTr("The quick brown fox · Aa Bb 0123")
                        font.family: row.modelData
                        font.pixelSize: 21
                        color: Theme.text
                        elide: Text.ElideRight
                        renderType: Text.NativeRendering
                    }
                }
            }
            Label {
                anchors.centerIn: parent
                visible: fonts.count === 0
                text: qsTr("No matching fonts")
                color: Theme.secondaryText
            }
        }
        Label {
            Layout.fillWidth: true
            text: picker.draftFamily
            font.family: picker.draftFamily
            font.pixelSize: 26
            color: Theme.text
            elide: Text.ElideRight
            Accessible.name: qsTr("Selected font: %1").arg(picker.draftFamily)
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("Applies to selected text or what you type next. Size and style stay unchanged.")
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
        }
    }
}
