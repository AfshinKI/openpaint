// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

ApplicationWindow {
    id: window
    objectName: "mainWindow"
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 640
    visible: true
    title: qsTr("OpenPaint — Development preview")
    color: Theme.workspace

    palette.window: Theme.window
    palette.windowText: Theme.text
    palette.base: Theme.panel
    palette.text: Theme.text
    palette.button: Theme.panel
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.accentText

    Action {
        id: quitAction
        objectName: "quitAction"
        text: qsTr("&Quit")
        shortcut: StandardKey.Quit
        onTriggered: window.close()
    }

    Action {
        id: aboutAction
        objectName: "aboutAction"
        text: qsTr("&About OpenPaint")
        shortcut: "F1"
        onTriggered: aboutDialog.open()
    }

    ActionGroup {
        Action {
            id: systemThemeAction
            objectName: "systemThemeAction"
            text: qsTr("&System")
            checkable: true
            checked: Theme.mode === 0
            onTriggered: Theme.mode = 0
        }
        Action {
            id: lightThemeAction
            objectName: "lightThemeAction"
            text: qsTr("&Light")
            checkable: true
            checked: Theme.mode === 1
            onTriggered: Theme.mode = 1
        }
        Action {
            id: darkThemeAction
            objectName: "darkThemeAction"
            text: qsTr("&Dark")
            checkable: true
            checked: Theme.mode === 2
            onTriggered: Theme.mode = 2
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                action: quitAction
            }
        }
        Menu {
            title: qsTr("&View")
            Menu {
                title: qsTr("&Theme")
                MenuItem {
                    action: systemThemeAction
                }
                MenuItem {
                    action: lightThemeAction
                }
                MenuItem {
                    action: darkThemeAction
                }
            }
        }
        Menu {
            title: qsTr("&Help")
            MenuItem {
                action: aboutAction
            }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 64, 560)
        height: welcome.implicitHeight + 64
        color: Theme.panel
        radius: 8
        border.color: Theme.divider

        ColumnLayout {
            id: welcome
            anchors.fill: parent
            anchors.margins: 32
            spacing: 16

            Label {
                text: qsTr("OpenPaint")
                font.pixelSize: 32
                font.bold: true
                color: Theme.text
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("The familiar joy of Paint. Built for Linux.")
                font.pixelSize: 18
                color: Theme.text
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            Label {
                objectName: "previewNotice"
                text: qsTr("Development preview. Drawing and file editing are not available yet.")
                color: Theme.secondaryText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            Button {
                objectName: "aboutButton"
                action: aboutAction
                text: qsTr("About OpenPaint")
                highlighted: true
                implicitHeight: 36
                palette.dark: Theme.accent
                palette.brightText: Theme.accentText
                palette.highlight: Theme.text
                Accessible.name: text
                focus: true
                Layout.topMargin: 8
            }
        }
    }

    footer: ToolBar {
        Label {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Development preview · Offline · No document open")
            color: Theme.secondaryText
        }
    }

    Dialog {
        id: aboutDialog
        objectName: "aboutDialog"
        anchors.centerIn: parent
        width: Math.min(window.width - 64, 440)
        title: qsTr("About OpenPaint")
        modal: true
        standardButtons: Dialog.Ok
        closePolicy: Popup.CloseOnEscape

        Label {
            width: parent.width
            text: qsTr("Open-source painting for Linux.\n\nThis development preview includes the application window and theme controls. Drawing and file editing are not available yet.\n\nLicensed under GPL-3.0-or-later. Independent of Microsoft.")
            wrapMode: Text.WordWrap
            color: Theme.text
        }
    }
}
