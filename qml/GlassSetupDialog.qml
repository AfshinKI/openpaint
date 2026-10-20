// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    property string copyStatus: ""
    title: qsTr("Enable Glass")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 48, 720)
    height: Math.min(parent.height - 48, 560)
    standardButtons: Dialog.NoButton
    footer: RowLayout {
        spacing: 10
        PaintButton {
            id: checkButton
            objectName: "refreshBlurSupport"
            text: BlurSupport.busy ? qsTr("Checking…") : qsTr("Check again")
            enabled: !BlurSupport.busy
            Layout.leftMargin: 20
            Layout.bottomMargin: 12
            onClicked: BlurSupport.refresh()
        }
        PaintButton {
            objectName: "useGlass"
            text: qsTr("Use Glass")
            visible: BlurSupport.ready
            Layout.bottomMargin: 12
            onClicked: {
                Appearance.glassEnabled = true;
                if (Appearance.surfaceOpacity === 100)
                    Appearance.surfaceOpacity = 40;
                dialog.close();
            }
        }
        Item {
            Layout.fillWidth: true
        }
        PaintButton {
            text: qsTr("Close")
            Layout.rightMargin: 20
            Layout.bottomMargin: 12
            onClicked: dialog.close()
        }
    }
    closePolicy: Popup.CloseOnEscape
    background: Rectangle {
        color: Theme.panel
        radius: Theme.dialogRadius
        border.color: Theme.divider
    }
    onOpened: {
        copyStatus = "";
        BlurSupport.refresh();
        checkButton.forceActiveFocus();
    }
    contentItem: ScrollView {
        id: scroll
        clip: true
        contentWidth: availableWidth
        ColumnLayout {
            width: scroll.availableWidth
            spacing: 10
            Label {
                text: qsTr("A softer workspace.")
                font.pixelSize: 22
                font.bold: true
                color: Theme.text
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Label {
                text: qsTr("Choose your look.")
                color: Theme.secondaryText
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    AppPreview {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150
                    }
                    Label {
                        text: qsTr("Glass appearance")
                        color: Theme.text
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    AppPreview {
                        glass: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150
                    }
                    Label {
                        text: qsTr("Opaque")
                        color: Theme.text
                    }
                }
            }
            Label {
                text: BlurSupport.ready ? qsTr("Glass is ready. Make it yours.") : BlurSupport.setupAvailable ? BlurSupport.instructions : qsTr("Enjoy OpenPaint’s clear, solid look on this desktop.")
                color: Theme.text
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Rectangle {
                objectName: "glassCommandWell"
                visible: BlurSupport.setupAvailable && !BlurSupport.ready
                Layout.fillWidth: true
                implicitHeight: commandText.implicitHeight + 56
                radius: 8
                color: Theme.dark ? "#15191F" : "#F0F2F5"
                border.color: Theme.divider
                Label {
                    id: terminalLabel
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: 12
                    text: qsTr("TERMINAL")
                    font.pixelSize: 10
                    font.letterSpacing: 1.2
                    color: Theme.secondaryText
                }
                TextEdit {
                    id: commandText
                    objectName: "blurSetupCommand"
                    anchors.left: parent.left
                    anchors.right: copyButton.left
                    anchors.top: terminalLabel.bottom
                    anchors.margins: 12
                    text: BlurSupport.command
                    color: Theme.text
                    font.family: "monospace"
                    font.pixelSize: 11
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.WrapAnywhere
                    Accessible.name: qsTr("Desktop blur setup command")
                }
                IconButton {
                    id: copyButton
                    objectName: "copyBlurCommand"
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 5
                    symbol: "copy"
                    text: qsTr("Copy command")
                    Accessible.name: text
                    enabled: BlurSupport.setupAvailable
                    onClicked: dialog.copyStatus = BlurSupport.copyCommand() ? qsTr("Command copied") : qsTr("Select the command and copy it manually.")
                }
            }
            Label {
                objectName: "blurCopyStatus"
                text: dialog.copyStatus
                visible: text.length > 0
                color: Theme.text
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                onTextChanged: function (text) {
                    if (text.length)
                        Accessible.announce(text, Accessible.Polite);
                }
            }
        }
    }
}
