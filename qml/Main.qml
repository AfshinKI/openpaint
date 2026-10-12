// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenPaint

ApplicationWindow {
    id: window
    readonly property bool textInputFocused: activeFocusItem instanceof TextInput || activeFocusItem instanceof TextEdit
    readonly property Editor editorController: editor
    readonly property CanvasItem canvasController: canvas
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
        if (editor.textActive) {
            pendingAction = action;
            textTransition.open();
            return;
        }
        editor.finish();
        pendingAction = action;
        if (editor.dirty)
            closeDialog.open();
        else
            performPending();
    }
    function saveDrawing() {
        editor.commitPreview();
        if (editor.fileUrl.toString() === "")
            saveDialog.open();
        else
            editor.saveFile(editor.fileUrl);
    }
    onClosing: function (close) {
        if (editor.busy) {
            close.accepted = false;
            return;
        }
        if (editor.textActive) {
            close.accepted = false;
            window.requestChange("close");
            return;
        }
        editor.finish();
        if (editor.busy)
            close.accepted = false;
        else if (editor.dirty && !discardClose) {
            close.accepted = false;
            requestChange("close");
        }
    }
    SelectionOptions {
        id: selectionOptions
        objectName: "selectionOptions"
        editor: window.editorController
        parent: Overlay.overlay
    }
    TextOptions {
        id: textOptions
        objectName: "textOptions"
        editor: window.editorController
        parent: Overlay.overlay
        onClosed: {
            if (editor.textActive)
                textOverlay.focusInput();
        }
    }
    PaintDialog {
        id: textTransition
        objectName: "textTransition"
        title: qsTr("Finish editing text?")
        anchors.centerIn: parent
        modal: true
        property bool resumeAction: false
        standardButtons: Dialog.Apply | Dialog.Discard | Dialog.Cancel
        onApplied: {
            editor.commitPreview();
            resumeAction = true;
            close();
        }
        onDiscarded: {
            editor.cancel();
            resumeAction = true;
        }
        onRejected: {
            resumeAction = false;
            window.pendingAction = "";
        }
        onClosed: {
            if (resumeAction) {
                resumeAction = false;
                const action = window.pendingAction;
                window.pendingAction = "";
                window.requestChange(action);
            } else if (editor.textActive)
                textOverlay.focusInput();
        }
        Label {
            text: qsTr("Apply text, discard this text box, or keep editing.")
            color: Theme.text
        }
    }
    ShapeOptions {
        id: shapeOptions
        objectName: "shapeOptions"
        editor: window.editorController
        parent: Overlay.overlay
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
    Shortcut {
        sequence: "Escape"
        enabled: editor.loading
        onActivated: editor.cancelIo()
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
        onTriggered: {
            editor.commitPreview();
            saveDialog.open();
        }
    }
    Action {
        id: quitAction
        objectName: "quitAction"
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"
        onTriggered: window.close()
    }
    Action {
        id: undoAction
        text: qsTr("Undo")
        shortcut: StandardKey.Undo
        enabled: editor.canUndo && !editor.loading && !editor.textActive
        onTriggered: editor.undo()
    }
    Action {
        id: redoAction
        text: qsTr("Redo")
        shortcut: "Ctrl+Y"
        enabled: editor.canRedo && !editor.loading && !editor.textActive
        onTriggered: editor.redo()
    }
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        enabled: !editor.textActive
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
        id: rotateRightAction
        text: editor.hasSelection ? qsTr("Rotate selection right 90°") : qsTr("Rotate image right 90°")
        enabled: editor.canSelect && !editor.busy
        onTriggered: editor.transformImage(0)
    }
    Action {
        id: rotateLeftAction
        text: editor.hasSelection ? qsTr("Rotate selection left 90°") : qsTr("Rotate image left 90°")
        enabled: editor.canSelect && !editor.busy
        onTriggered: editor.transformImage(1)
    }
    Action {
        id: rotateHalfAction
        text: editor.hasSelection ? qsTr("Rotate selection 180°") : qsTr("Rotate image 180°")
        enabled: editor.canSelect && !editor.busy
        onTriggered: editor.transformImage(2)
    }
    Action {
        id: flipHorizontalAction
        text: editor.hasSelection ? qsTr("Flip selection horizontally") : qsTr("Flip image horizontally")
        enabled: editor.canSelect && !editor.busy
        onTriggered: editor.transformImage(3)
    }
    Action {
        id: flipVerticalAction
        text: editor.hasSelection ? qsTr("Flip selection vertically") : qsTr("Flip image vertically")
        enabled: editor.canSelect && !editor.busy
        onTriggered: editor.transformImage(4)
    }
    Action {
        id: cropAction
        text: qsTr("Crop to selection")
        enabled: editor.canSelect && editor.hasSelection && !editor.busy
        onTriggered: editor.transformImage(5)
    }
    Action {
        id: copyAction
        text: qsTr("Copy")
        shortcut: "Ctrl+C"
        enabled: editor.canSelect && !editor.busy && editor.hasSelection && !window.textInputFocused
        onTriggered: editor.copySelection()
    }
    Action {
        id: cutAction
        text: qsTr("Cut")
        shortcut: "Ctrl+X"
        enabled: editor.canSelect && !editor.busy && editor.hasSelection && !window.textInputFocused
        onTriggered: editor.cutSelection()
    }
    Action {
        id: pasteAction
        text: qsTr("Paste")
        shortcut: "Ctrl+V"
        enabled: editor.canSelect && !editor.busy && !window.textInputFocused
        onTriggered: editor.pasteClipboard()
    }
    Action {
        id: selectAllAction
        objectName: "selectAllAction"
        text: qsTr("Select all")
        shortcut: "Ctrl+A"
        enabled: editor.canSelect && !window.textInputFocused
        onTriggered: editor.selectAll()
    }
    Action {
        id: deselectAction
        text: qsTr("Deselect")
        shortcut: "Ctrl+D"
        enabled: editor.canSelect && editor.hasSelection && !window.textInputFocused
        onTriggered: editor.deselect()
    }
    Action {
        id: deleteSelectionAction
        text: qsTr("Delete selection")
        shortcut: "Delete"
        enabled: editor.canSelect && editor.hasSelection && !window.textInputFocused
        onTriggered: editor.deleteSelection()
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

    menuBar: ToolBar {
        leftPadding: 4
        rightPadding: 4
        topPadding: 2
        bottomPadding: 2
        background: Rectangle {
            color: Theme.window
        }
        contentItem: RowLayout {
            spacing: 4
            MenuBar {
                Layout.fillWidth: true
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
                    MenuSeparator {}
                    PaintMenuItem {
                        action: cutAction
                    }
                    PaintMenuItem {
                        action: copyAction
                    }
                    PaintMenuItem {
                        action: pasteAction
                    }
                    MenuSeparator {}
                    PaintMenuItem {
                        action: selectAllAction
                    }
                    PaintMenuItem {
                        action: deselectAction
                    }
                    PaintMenuItem {
                        action: deleteSelectionAction
                    }
                }
                PaintMenu {
                    title: qsTr("&Image")
                    PaintMenuItem {
                        action: cropAction
                    }
                    PaintMenuItem {
                        action: rotateRightAction
                    }
                    PaintMenuItem {
                        action: rotateLeftAction
                    }
                    PaintMenuItem {
                        action: rotateHalfAction
                    }
                    PaintMenuItem {
                        action: flipHorizontalAction
                    }
                    PaintMenuItem {
                        action: flipVerticalAction
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
            IconButton {
                symbol: "save"
                shortcutLabel: "Ctrl+S"
                action: saveAction
                Accessible.name: text
            }
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
                    objectName: "selectionButton"
                    symbol: "selection"
                    text: editor.freeFormSelection ? qsTr("Select free-form") : qsTr("Select rectangle")
                    Accessible.name: text
                    highlighted: editor.tool === Editor.Select
                    enabled: editor.canSelect
                    onClicked: editor.tool = Editor.Select
                    Layout.alignment: Qt.AlignHCenter
                }
                PaintButton {
                    objectName: "selectionMenuButton"
                    text: qsTr("Select ▾")
                    enabled: editor.canSelect || editor.floatingActive
                    onClicked: selectionMenu.open()
                    PaintMenu {
                        id: selectionMenu
                        objectName: "selectionMenu"
                        y: parent.height
                        PaintMenuItem {
                            text: qsTr("Rectangle")
                            checkable: true
                            checked: editor.tool === Editor.Select && !editor.freeFormSelection
                            enabled: editor.canSelect
                            onTriggered: {
                                editor.freeFormSelection = false;
                                editor.tool = Editor.Select;
                            }
                        }
                        PaintMenuItem {
                            objectName: "freeFormSelectionItem"
                            text: qsTr("Free-form")
                            checkable: true
                            checked: editor.tool === Editor.Select && editor.freeFormSelection
                            enabled: editor.canSelect
                            onTriggered: {
                                editor.freeFormSelection = true;
                                editor.tool = Editor.Select;
                            }
                        }
                        MenuSeparator {}
                        PaintMenuItem {
                            action: selectAllAction
                        }
                        PaintMenuItem {
                            text: qsTr("Invert selection")
                            enabled: editor.canSelect && editor.hasSelection
                            onTriggered: editor.invertSelection()
                        }
                        PaintMenuItem {
                            text: qsTr("Transparent selection")
                            checkable: true
                            checked: editor.transparentSelection
                            onTriggered: editor.transparentSelection = !editor.transparentSelection
                        }
                        PaintMenuItem {
                            action: deselectAction
                        }
                        PaintMenuItem {
                            text: editor.floatingActive ? qsTr("Selection position…") : qsTr("Selection bounds…")
                            enabled: editor.hasSelection
                            onTriggered: selectionOptions.open()
                        }
                        PaintMenuItem {
                            action: deleteSelectionAction
                        }
                    }
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                IconButton {
                    symbol: "crop"
                    objectName: "cropButton"
                    action: cropAction
                    Accessible.name: text
                    Layout.alignment: Qt.AlignHCenter
                }
                PaintButton {
                    objectName: "imageMenuButton"
                    text: qsTr("Image ▾")
                    enabled: editor.canSelect && !editor.busy
                    onClicked: imageMenu.open()
                    PaintMenu {
                        id: imageMenu
                        objectName: "imageMenu"
                        y: parent.height
                        PaintMenuItem {
                            action: cropAction
                        }
                        PaintMenuItem {
                            action: rotateRightAction
                        }
                        PaintMenuItem {
                            action: rotateLeftAction
                        }
                        PaintMenuItem {
                            action: rotateHalfAction
                        }
                        PaintMenuItem {
                            action: flipHorizontalAction
                        }
                        PaintMenuItem {
                            action: flipVerticalAction
                        }
                    }
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                GridLayout {
                    columns: 3
                    IconButton {
                        symbol: "pencil"
                        objectName: "pencilButton"
                        text: qsTr("Pencil")
                        enabled: !editor.textActive
                        checkable: true
                        autoExclusive: true
                        checked: editor.tool === Editor.Pencil
                        onClicked: editor.tool = Editor.Pencil
                        Accessible.name: text
                    }
                    IconButton {
                        symbol: "fill"
                        objectName: "fillButton"
                        text: qsTr("Fill")
                        enabled: !editor.textActive
                        checkable: true
                        autoExclusive: true
                        checked: editor.tool === Editor.Fill
                        onClicked: editor.tool = Editor.Fill
                        Accessible.name: text
                    }
                    IconButton {
                        symbol: "text"
                        objectName: "textButton"
                        text: qsTr("Text")
                        enabled: !editor.textActive
                        checkable: true
                        autoExclusive: true
                        checked: editor.tool === Editor.Text
                        onClicked: editor.tool = Editor.Text
                        Accessible.name: text
                    }
                    IconButton {
                        symbol: "eraser"
                        objectName: "eraserButton"
                        text: qsTr("Eraser")
                        enabled: !editor.textActive
                        checkable: true
                        autoExclusive: true
                        checked: editor.tool === Editor.Eraser
                        onClicked: editor.tool = Editor.Eraser
                        Accessible.name: text
                    }
                    IconButton {
                        symbol: "eyedropper"
                        objectName: "pickerButton"
                        text: qsTr("Color picker")
                        enabled: !editor.textActive
                        checkable: true
                        autoExclusive: true
                        checked: editor.tool === Editor.ColorPicker
                        onClicked: editor.tool = Editor.ColorPicker
                        Accessible.name: text
                    }
                }
                Label {
                    text: qsTr("Tools")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                IconButton {
                    id: brushesButton
                    objectName: "brushesButton"
                    enabled: !editor.textActive
                    symbol: "brush"
                    text: qsTr("Brushes")
                    highlighted: editor.tool === Editor.Brush
                    onClicked: brushesMenu.open()
                    Accessible.name: text
                    BrushMenu {
                        id: brushesMenu
                        objectName: "brushesMenu"
                        editor: window.editorController
                        y: brushesButton.height
                    }
                }
                Label {
                    text: qsTr("Brushes")
                    color: Theme.secondaryText
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                IconButton {
                    id: shapesButton
                    objectName: "shapesButton"
                    enabled: !editor.textActive
                    symbol: "rectangle"
                    text: qsTr("Shapes")
                    highlighted: editor.tool === Editor.Shape
                    onClicked: shapesMenu.open()
                    ShapePalette {
                        id: shapesMenu
                        objectName: "shapesMenu"
                        editor: window.editorController
                        y: shapesButton.height
                    }
                }
                Label {
                    text: qsTr("Shapes")
                    color: Theme.secondaryText
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                PaintSpinBox {
                    id: sizeBox
                    objectName: "strokeSize"
                    from: editor.tool === Editor.Fill ? 0 : 1
                    to: editor.tool === Editor.Fill ? 255 : 128
                    value: editor.tool === Editor.Fill ? editor.tolerance : editor.brushSize
                    onValueModified: {
                        if (editor.tool === Editor.Fill)
                            editor.tolerance = value;
                        else
                            editor.brushSize = value;
                    }
                    Accessible.name: editor.tool === Editor.Fill ? qsTr("Fill tolerance") : qsTr("Stroke size in pixels")
                    implicitWidth: 104
                }
                PaintCheckBox {
                    text: qsTr("Transparent")
                    visible: editor.tool === Editor.Eraser
                    checked: editor.transparentEraser
                    onToggled: editor.transparentEraser = checked
                }
                PaintSpinBox {
                    objectName: "brushOpacity"
                    visible: editor.tool === Editor.Brush
                    from: 0
                    to: 100
                    value: editor.opacity
                    onValueModified: editor.opacity = value
                    Accessible.name: qsTr("Brush opacity percent")
                    implicitWidth: 104
                }
                Label {
                    text: editor.tool === Editor.Brush ? qsTr("Size / opacity %") : editor.tool === Editor.Fill ? qsTr("Tolerance") : qsTr("Size · px")
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
        enabled: !editor.loading || editor.filling
        focus: true
        Accessible.name: qsTr("Drawing canvas")
        Accessible.role: Accessible.Canvas
        InlineText {
            id: textOverlay
            editor: window.editorController
            canvas: window.canvasController
        }
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
                text: editor.floatingActive ? qsTr("Drag to move · Enter applies · Escape cancels") : editor.textActive ? qsTr("Ctrl+Enter applies text · Escape cancels") : editor.previewActive ? (editor.shapeKind === 15 ? qsTr("Add points · Enter applies") : qsTr("Enter to apply · Escape to cancel")) : editor.transforming ? qsTr("Transforming image…") : editor.clipboardBusy ? qsTr("Processing clipboard…") : editor.filling ? qsTr("Filling…") : editor.busy ? (editor.loading ? qsTr("Opening PNG…") : qsTr("Saving PNG…")) : qsTr("Space+drag to pan")
                color: Theme.secondaryText
                Layout.fillWidth: true
            }
            PaintButton {
                objectName: "shapeOptionsButton"
                text: qsTr("Options")
                visible: editor.previewActive
                onClicked: editor.floatingActive ? selectionOptions.open() : editor.textActive ? textOptions.open() : shapeOptions.open()
            }
            PaintButton {
                objectName: "applyPreviewButton"
                text: qsTr("Apply")
                visible: editor.previewActive
                onClicked: {
                    editor.commitPreview();
                }
            }
            PaintButton {
                text: qsTr("Cancel")
                visible: editor.previewActive
                onClicked: editor.cancel()
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
