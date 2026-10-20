// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    property string browserError: ""
    title: ""
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 48, 780)
    height: Math.min(parent.height - 48, 400)
    closePolicy: Popup.CloseOnEscape
    standardButtons: Dialog.NoButton
    background: Rectangle {
        color: Theme.panel
        radius: 18
        border.color: Theme.divider
    }
    onOpened: {
        browserError = "";
        startButton.forceActiveFocus();
    }
    onClosed: Appearance.dismissWelcome()
    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        PaintButton {
            id: startButton
            objectName: "welcomeStart"
            text: qsTr("Start creating")
            Layout.rightMargin: 20
            Layout.bottomMargin: 16
            onClicked: dialog.close()
        }
    }
    contentItem: ScrollView {
        id: scroll
        clip: true
        contentWidth: availableWidth
        GridLayout {
            width: scroll.availableWidth
            columns: dialog.width < 620 ? 1 : 2
            columnSpacing: 24
            rowSpacing: 12
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 350
                spacing: 14
                Label {
                    text: qsTr("Welcome to OpenPaint")
                    color: Theme.text
                    font.pixelSize: 26
                    font.bold: true
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("A simple space for your next idea.")
                    color: Theme.secondaryText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Made for Linux. Open to everyone.")
                    color: Theme.text
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    text: qsTr("Help shape OpenPaint: share ideas, report bugs, or contribute code.")
                    color: Theme.secondaryText
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                PaintButton {
                    objectName: "welcomeContribute"
                    text: qsTr("Get involved")
                    onClicked: if (!AppInfo.openContributing())
                        dialog.browserError = qsTr("Visit github.com/AfshinKI/openpaint to get involved.")
                }
                Label {
                    text: qsTr("Opens GitHub in your browser.")
                    font.pixelSize: 11
                    color: Theme.secondaryText
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    text: dialog.browserError
                    visible: text.length > 0
                    color: Theme.text
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
            }
            AppPreview {
                corner: false
                Layout.fillWidth: true
                Layout.preferredWidth: 300
                Layout.preferredHeight: 285
            }
        }
    }
}
