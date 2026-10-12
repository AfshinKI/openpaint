// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenPaint

ApplicationWindow {
    id: window
    objectName: "mainWindow"
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 640
    visible: true
    title: (editor.dirty ? "* " : "") + qsTr("Untitled — OpenPaint")
    color: Theme.workspace
    palette.window: Theme.window
    palette.windowText: Theme.text
    palette.base: Theme.panel
    palette.text: Theme.text
    palette.button: Theme.panel
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.accentText
    property bool discardClose: false
    onClosing: function (close) {
        editor.finish();
        if (editor.dirty && !discardClose) {
            close.accepted = false;
            closeDialog.open();
        }
    }
    Editor {
        id: editor
        objectName: "editor"
    }
    Action {
        id: quitAction
        objectName: "quitAction"
        text: qsTr("&Quit")
        shortcut: StandardKey.Quit
        onTriggered: window.close()
    }
    Action {
        id: undoAction
        text: qsTr("Undo")
        shortcut: StandardKey.Undo
        enabled: editor.canUndo
        onTriggered: editor.undo()
    }
    Action {
        id: redoAction
        text: qsTr("Redo")
        shortcut: StandardKey.Redo
        enabled: editor.canRedo
        onTriggered: editor.redo()
    }
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        onActivated: editor.redo()
    }
    Action {
        id: fitAction
        text: qsTr("Fit")
        shortcut: "Ctrl+0"
        onTriggered: canvas.fit()
    }
    Action {
        id: zoomInAction
        text: qsTr("Zoom in")
        shortcut: StandardKey.ZoomIn
        onTriggered: canvas.zoomBy(1.25)
    }
    Action {
        id: zoomOutAction
        text: qsTr("Zoom out")
        shortcut: StandardKey.ZoomOut
        onTriggered: canvas.zoomBy(0.8)
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
            title: qsTr("&Edit")
            MenuItem {
                action: undoAction
            }
            MenuItem {
                action: redoAction
            }
        }
        Menu {
            title: qsTr("&View")
            MenuItem {
                action: fitAction
            }
            MenuItem {
                action: zoomInAction
            }
            MenuItem {
                action: zoomOutAction
            }
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
    header: ToolBar {
        height: 116
        background: Rectangle {
            color: Theme.panel
            border.color: Theme.divider
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8
            ColumnLayout {
                RowLayout {
                    ToolButton {
                        action: undoAction
                        Accessible.name: text
                    }
                    ToolButton {
                        action: redoAction
                        Accessible.name: text
                    }
                }
                Label {
                    text: qsTr("History")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                RowLayout {
                    Button {
                        objectName: "pencilButton"
                        implicitWidth: 72
                        text: qsTr("Pencil")
                        checkable: true
                        checked: !editor.eraser
                        onClicked: editor.eraser = false
                        Accessible.name: text
                    }
                    Button {
                        implicitWidth: 72
                        text: qsTr("Eraser")
                        checkable: true
                        checked: editor.eraser
                        onClicked: editor.eraser = true
                        Accessible.name: text
                    }
                }
                CheckBox {
                    text: qsTr("Transparent")
                    checked: editor.transparentEraser
                    onToggled: editor.transparentEraser = checked
                    Accessible.name: text
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                SpinBox {
                    id: sizeBox
                    from: 1
                    to: 128
                    value: editor.brushSize
                    onValueModified: editor.brushSize = value
                    Accessible.name: qsTr("Stroke size in pixels")
                    implicitWidth: 96
                }
                Label {
                    text: qsTr("Size · px")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                RowLayout {
                    Button {
                        id: primaryButton
                        objectName: "primaryButton"
                        text: qsTr("Primary")
                        implicitWidth: 72
                        onClicked: {
                            colorDialog.selectedColor = editor.primary;
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: editor.primary
                            radius: 4
                            border.color: primaryButton.activeFocus ? Theme.accent : Theme.divider
                            border.width: 2
                        }
                        contentItem: Label {
                            text: primaryButton.text
                            color: editor.primary.hslLightness > 0.5 ? "black" : "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Accessible.name: qsTr("Choose primary color")
                    }
                    Button {
                        implicitWidth: 56
                        text: qsTr("Swap")
                        onClicked: {
                            const old = editor.primary;
                            editor.primary = editor.secondary;
                            editor.secondary = old;
                        }
                        Accessible.name: qsTr("Swap primary and secondary colors")
                    }
                    Rectangle {
                        implicitWidth: 24
                        implicitHeight: 24
                        color: editor.secondary
                        border.color: Theme.divider
                        Accessible.name: qsTr("Secondary color")
                        Accessible.role: Accessible.Indicator
                    }
                }
                Label {
                    text: qsTr("Right-click: secondary")
                    color: Theme.secondaryText
                }
            }
            ColumnLayout {
                GridLayout {
                    columns: 10
                    rowSpacing: 4
                    columnSpacing: 4
                    Repeater {
                        model: ["#000000", "#6b7280", "#8b4513", "#e53935", "#ff8c00", "#ffd600", "#2e9b46", "#00a6a6", "#1976d2", "#7b1fa2", "#ffffff", "#d1d5db", "#d7a47b", "#ff9aa2", "#ffc777", "#fff59d", "#a5d6a7", "#80deea", "#90caf9", "#ce93d8"]
                        delegate: Button {
                            id: swatch
                            required property string modelData
                            implicitWidth: 24
                            implicitHeight: 24
                            onClicked: editor.primary = modelData
                            background: Rectangle {
                                radius: 12
                                color: swatch.modelData
                                border.width: swatch.activeFocus || editor.primary.toString() === swatch.modelData ? 3 : 1
                                border.color: swatch.activeFocus ? Theme.accent : Theme.divider
                            }
                            Accessible.name: qsTr("Color %1").arg(modelData)
                            ToolTip.visible: hovered
                            ToolTip.text: modelData
                        }
                    }
                }
                Label {
                    text: qsTr("Colors")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
    CanvasItem {
        id: canvas
        objectName: "canvas"
        anchors.fill: parent
        editor: editor
        focus: true
        Accessible.name: qsTr("Drawing canvas")
        Accessible.role: Accessible.Canvas
    }
    footer: ToolBar {
        height: 36
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 12
            spacing: 12
            Label {
                text: Math.floor(canvas.cursorPosition.x) + ", " + Math.floor(canvas.cursorPosition.y) + " px"
                color: Theme.secondaryText
                Layout.preferredWidth: 115
            }
            Label {
                text: editor.imageSize.width + " × " + editor.imageSize.height + " px"
                color: Theme.secondaryText
            }
            Label {
                text: qsTr("Space+drag to pan")
                color: Theme.secondaryText
                Layout.fillWidth: true
            }
            ToolButton {
                action: fitAction
            }
            ToolButton {
                text: "−"
                action: zoomOutAction
                Accessible.name: qsTr("Zoom out")
            }
            Label {
                text: Math.round(canvas.zoom * 100) + "%"
                Layout.preferredWidth: 44
                horizontalAlignment: Text.AlignHCenter
            }
            ToolButton {
                text: "+"
                action: zoomInAction
                Accessible.name: qsTr("Zoom in")
            }
        }
    }
    ColorDialog {
        id: colorDialog
        title: qsTr("Primary color")
        onAccepted: editor.primary = selectedColor
    }
    Dialog {
        id: closeDialog
        title: qsTr("Discard this drawing?")
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Discard | Dialog.Cancel
        onDiscarded: {
            window.discardClose = true;
            window.close();
        }
        Label {
            text: qsTr("PNG saving arrives in the next development slice.\nThis drawing has unsaved changes.")
            color: Theme.text
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
            text: qsTr("Open-source painting for Linux.\n\nDevelopment build: pencil, eraser, colors and undo. PNG files are the next slice.\n\nGPL-3.0-or-later. Independent of Microsoft.")
            wrapMode: Text.WordWrap
            color: Theme.text
        }
    }
}
