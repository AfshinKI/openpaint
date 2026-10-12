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
    title: (editor.dirty ? "* " : "") + editor.fileName + qsTr(" — OpenPaint")
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
    property string pendingAction: ""
    function performPending() {
        const action = pendingAction;
        pendingAction = "";
        if (action === "close") {
            discardClose = true;
            close();
        } else if (action === "new")
            editor.newDocument();
        else if (action === "open")
            openDialog.open();
    }
    function requestChange(action: string) {
        if (editor.busy)
            return;
        editor.finish();
        pendingAction = action;
        if (editor.dirty)
            closeDialog.open();
        else
            performPending();
    }
    function saveDrawing() {
        editor.finish();
        if (editor.fileUrl.toString() === "")
            saveDialog.open();
        else
            editor.saveFile(editor.fileUrl);
    }
    onClosing: function (close) {
        editor.finish();
        if (editor.busy)
            close.accepted = false;
        else if (editor.dirty && !discardClose) {
            close.accepted = false;
            requestChange("close");
        }
    }
    ColorPalette {
        id: customPalette
        objectName: "customPalette"
    }
    Editor {
        id: editor
        objectName: "editor"
    }
    Connections {
        target: editor
        function onNoticeChanged() {
            noticeDialog.open();
        }
        function onErrorChanged() {
            errorDialog.open();
        }
        function onSaved(success: bool) {
            if (success && !editor.dirty)
                window.performPending();
            else
                window.pendingAction = "";
        }
    }
    Action {
        id: newAction
        objectName: "newAction"
        text: qsTr("&New")
        shortcut: StandardKey.New
        enabled: !editor.busy
        onTriggered: window.requestChange("new")
    }
    Action {
        id: openAction
        objectName: "openAction"
        text: qsTr("&Open…")
        shortcut: StandardKey.Open
        enabled: !editor.busy
        onTriggered: window.requestChange("open")
    }
    Action {
        id: saveAction
        objectName: "saveAction"
        text: qsTr("&Save")
        shortcut: StandardKey.Save
        enabled: !editor.busy
        onTriggered: window.saveDrawing()
    }
    Action {
        id: saveAsAction
        text: qsTr("Save &As…")
        shortcut: StandardKey.SaveAs
        enabled: !editor.busy
        onTriggered: saveDialog.open()
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
        enabled: editor.canUndo && !editor.loading
        onTriggered: editor.undo()
    }
    Action {
        id: redoAction
        text: qsTr("Redo")
        shortcut: StandardKey.Redo
        enabled: editor.canRedo && !editor.loading
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
        background: Rectangle {
            color: Theme.window
        }
        delegate: MenuBarItem {
            id: menuEntry
            background: Rectangle {
                radius: Theme.controlRadius
                color: menuEntry.highlighted ? Theme.hover : "transparent"
            }
        }
        PaintMenu {
            objectName: "fileMenu"
            title: qsTr("&File")
            PaintMenuItem {
                action: newAction
            }
            PaintMenuItem {
                action: openAction
            }
            PaintMenuItem {
                action: saveAction
            }
            PaintMenuItem {
                action: saveAsAction
            }
            MenuSeparator {}
            PaintMenuItem {
                action: quitAction
            }
        }
        PaintMenu {
            title: qsTr("&Edit")
            PaintMenuItem {
                action: undoAction
            }
            PaintMenuItem {
                action: redoAction
            }
        }
        PaintMenu {
            title: qsTr("&View")
            PaintMenuItem {
                action: fitAction
            }
            PaintMenuItem {
                action: zoomInAction
            }
            PaintMenuItem {
                action: zoomOutAction
            }
            PaintMenu {
                title: qsTr("&Theme")
                PaintMenuItem {
                    action: systemThemeAction
                }
                PaintMenuItem {
                    action: lightThemeAction
                }
                PaintMenuItem {
                    action: darkThemeAction
                }
            }
        }
        PaintMenu {
            title: qsTr("&Help")
            PaintMenuItem {
                action: aboutAction
            }
        }
    }
    header: ToolBar {
        objectName: "mainToolbar"
        height: 112
        background: Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: Theme.surfaceRadius
            color: Theme.panel
            border.color: Theme.divider
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8
            ColumnLayout {
                IconButton {
                    symbol: "save"
                    shortcutLabel: "Ctrl+S"
                    action: saveAction
                    Accessible.name: qsTr("Save drawing")
                }
                Label {
                    text: qsTr("File")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                RowLayout {
                    IconButton {
                        symbol: "undo"
                        shortcutLabel: "Ctrl+Z"
                        action: undoAction
                        Accessible.name: text
                    }
                    IconButton {
                        symbol: "redo"
                        shortcutLabel: "Ctrl+Y"
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
                    IconButton {
                        symbol: "pencil"
                        objectName: "pencilButton"
                        text: qsTr("Pencil")
                        checkable: true
                        checked: !editor.eraser
                        onClicked: editor.eraser = false
                        Accessible.name: text
                    }
                    IconButton {
                        symbol: "eraser"
                        text: qsTr("Eraser")
                        checkable: true
                        checked: editor.eraser
                        onClicked: editor.eraser = true
                        Accessible.name: text
                    }
                }
                PaintCheckBox {
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
                PaintSpinBox {
                    id: sizeBox
                    from: 1
                    to: 128
                    value: editor.brushSize
                    onValueModified: editor.brushSize = value
                    Accessible.name: qsTr("Stroke size in pixels")
                    implicitWidth: 104
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
                spacing: 2
                ColorSwatch {
                    objectName: "primaryButton"
                    implicitWidth: 32
                    implicitHeight: 32
                    diameter: 26
                    swatchColor: editor.primary
                    checked: true
                    text: qsTr("Edit primary color")
                    onClicked: colorDialog.begin(editor.primary, "primary")
                }
                ColorSwatch {
                    objectName: "secondaryButton"
                    implicitWidth: 32
                    implicitHeight: 32
                    diameter: 24
                    swatchColor: editor.secondary
                    text: qsTr("Edit secondary color · right-click to draw")
                    onClicked: colorDialog.begin(editor.secondary, "secondary")
                }
            }
            ColumnLayout {
                GridLayout {
                    columns: 10
                    rowSpacing: 0
                    columnSpacing: 0
                    Repeater {
                        model: ["#000000", "#6b7280", "#8b4513", "#e53935", "#ff8c00", "#ffd600", "#2e9b46", "#00a6a6", "#1976d2", "#7b1fa2", "#ffffff", "#d1d5db", "#d7a47b", "#ff9aa2", "#ffc777", "#fff59d", "#a5d6a7", "#80deea", "#90caf9", "#ce93d8"]
                        delegate: ColorSwatch {
                            objectName: "colorSwatch"
                            required property string modelData
                            swatchColor: modelData
                            checked: editor.primary.toString() === modelData
                            onClicked: editor.primary = modelData
                        }
                    }
                }
                RowLayout {
                    spacing: 0
                    Repeater {
                        model: 10
                        delegate: ColorSwatch {
                            objectName: "customToolbarSwatch"
                            required property int index
                            empty: index >= customPalette.colors.length
                            swatchColor: empty ? "transparent" : customPalette.colors[index]
                            checked: !empty && editor.primary.toString() === swatchColor.toString()
                            text: empty ? qsTr("Empty custom slot %1 · edit colors").arg(index + 1) : qsTr("Custom color %1").arg(swatchColor.toString())
                            onClicked: {
                                if (empty)
                                    colorDialog.begin(editor.primary, "primary");
                                else
                                    editor.primary = swatchColor;
                            }
                        }
                    }
                }
                Label {
                    text: qsTr("Colors")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ColumnLayout {
                IconButton {
                    symbol: "palette"
                    icon.color: "transparent"
                    text: qsTr("Edit colors")
                    onClicked: colorDialog.begin(editor.primary, "primary")
                }
                IconButton {
                    symbol: "swap"
                    text: qsTr("Swap primary and secondary colors")
                    onClicked: {
                        const old = editor.primary;
                        editor.primary = editor.secondary;
                        editor.secondary = old;
                    }
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
        enabled: !editor.loading
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
                text: editor.busy ? (editor.loading ? qsTr("Opening PNG…") : qsTr("Saving PNG…")) : qsTr("Space+drag to pan")
                color: Theme.secondaryText
                Layout.fillWidth: true
            }
            BusyIndicator {
                running: editor.busy
                visible: editor.busy
                implicitWidth: 24
                implicitHeight: 24
            }
            ToolButton {
                text: qsTr("Cancel")
                visible: editor.busy
                onClicked: editor.cancelIo()
            }
            IconButton {
                symbol: "fit"
                shortcutLabel: "Ctrl+0"
                action: fitAction
            }
            IconButton {
                symbol: "zoom-out"
                shortcutLabel: "Ctrl+-"
                action: zoomOutAction
                Accessible.name: qsTr("Zoom out")
            }
            Label {
                text: Math.round(canvas.zoom * 100) + "%"
                Layout.preferredWidth: 44
                horizontalAlignment: Text.AlignHCenter
            }
            IconButton {
                symbol: "zoom-in"
                shortcutLabel: "Ctrl++"
                action: zoomInAction
                Accessible.name: qsTr("Zoom in")
            }
        }
    }
    FileDialog {
        id: openDialog
        objectName: "openDialog"
        title: qsTr("Open PNG")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("PNG images (*.png)")]
        onAccepted: editor.openFile(selectedFile)
    }
    FileDialog {
        id: saveDialog
        objectName: "saveDialog"
        title: qsTr("Save PNG")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "png"
        nameFilters: [qsTr("PNG images (*.png)")]
        onAccepted: editor.saveFile(selectedFile)
        onRejected: window.pendingAction = ""
    }
    PaintDialog {
        id: errorDialog
        objectName: "errorDialog"
        title: qsTr("File operation failed")
        anchors.centerIn: parent
        width: Math.min(window.width - 64, 480)
        modal: true
        standardButtons: Dialog.Ok
        Label {
            width: parent.width
            text: editor.error
            wrapMode: Text.WordWrap
            color: Theme.text
        }
    }
    PaintDialog {
        id: noticeDialog
        title: qsTr("PNG metadata")
        anchors.centerIn: parent
        width: Math.min(window.width - 64, 480)
        modal: true
        standardButtons: Dialog.Ok
        Label {
            width: parent.width
            text: editor.notice
            wrapMode: Text.WordWrap
            color: Theme.text
        }
    }
    ColorEditor {
        id: colorDialog
        paletteStore: customPalette
        onColorChosen: function (chosen) {
            if (colorTarget === "primary")
                editor.primary = chosen;
            else
                editor.secondary = chosen;
        }
    }

    PaintDialog {
        id: closeDialog
        title: qsTr("Save changes?")
        objectName: "unsavedDialog"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel
        onAccepted: window.saveDrawing()
        onDiscarded: window.performPending()
        onRejected: window.pendingAction = ""
        Label {
            text: qsTr("Save your changes before continuing?")
            color: Theme.text
        }
    }
    PaintDialog {
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
            text: qsTr("Open-source painting for Linux.\n\nDevelopment build: drawing, erasing, colors, undo and PNG files.\n\nGPL-3.0-or-later. Independent of Microsoft.")
            wrapMode: Text.WordWrap
            color: Theme.text
        }
    }
}
