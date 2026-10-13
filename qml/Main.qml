// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenPaint

ApplicationWindow {
    id: window
    property Action lastRotation: rotateRightAction
    property Action lastFlip: flipHorizontalAction
    readonly property bool documentOptionsOpen: resizeDialog.visible || exportOptions.visible || exportDialog.visible
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
    UpdatesDialog {
        id: updatesDialog
        objectName: "updatesDialog"
        parent: Overlay.overlay
    }
    ExportDialog {
        id: exportOptions
        objectName: "exportOptions"
        editor: window.editorController
        parent: Overlay.overlay
        onChooseDestination: exportDialog.open()
    }
    ResizeDialog {
        id: resizeDialog
        objectName: "resizeDialog"
        editor: window.editorController
        parent: Overlay.overlay
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
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        z: 3
        visible: window.editorController.tool === Editor.Text
        onReturnToText: {
            if (window.editorController.textReady)
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
        icon.source: "icons/new.svg"
        objectName: "newAction"
        text: qsTr("&New")
        shortcut: StandardKey.New
        enabled: !editor.busy && !window.documentOptionsOpen
        onTriggered: window.requestChange("new")
    }
    Action {
        id: openAction
        icon.source: "icons/open.svg"
        objectName: "openAction"
        text: qsTr("&Open…")
        shortcut: StandardKey.Open
        enabled: !editor.busy && !window.documentOptionsOpen
        onTriggered: window.requestChange("open")
    }
    Action {
        id: saveAction
        icon.source: "icons/save.svg"
        objectName: "saveAction"
        text: qsTr("&Save")
        shortcut: StandardKey.Save
        enabled: !editor.busy && !window.documentOptionsOpen
        onTriggered: window.saveDrawing()
    }
    Action {
        id: saveAsAction
        icon.source: "icons/save.svg"
        text: qsTr("Save &As…")
        shortcut: StandardKey.SaveAs
        enabled: !editor.busy && !window.documentOptionsOpen
        onTriggered: {
            editor.commitPreview();
            saveDialog.open();
        }
    }
    Action {
        id: exportAction
        icon.source: "icons/export.svg"
        objectName: "exportAction"
        text: qsTr("Export &copy…")
        enabled: !editor.busy && !editor.previewActive && !window.documentOptionsOpen
        onTriggered: exportOptions.open()
    }
    Action {
        id: quitAction
        icon.source: "icons/exit.svg"
        objectName: "quitAction"
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"
        enabled: !window.documentOptionsOpen
        onTriggered: window.close()
    }
    Action {
        id: undoAction
        icon.source: "icons/undo.svg"
        text: qsTr("Undo")
        shortcut: StandardKey.Undo
        enabled: editor.canUndo && !editor.loading && !editor.textActive && !window.documentOptionsOpen
        onTriggered: editor.undo()
    }
    Action {
        id: redoAction
        icon.source: "icons/redo.svg"
        text: qsTr("Redo")
        shortcut: "Ctrl+Y"
        enabled: editor.canRedo && !editor.loading && !editor.textActive && !window.documentOptionsOpen
        onTriggered: editor.redo()
    }
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        enabled: !editor.textActive && !window.documentOptionsOpen
        onActivated: editor.redo()
    }
    Action {
        id: fitAction
        icon.source: "icons/fit.svg"
        text: qsTr("Fit")
        shortcut: "Ctrl+0"
        onTriggered: canvas.fit()
    }
    Action {
        id: zoomInAction
        icon.source: "icons/zoom-in.svg"
        text: qsTr("Zoom in")
        shortcut: StandardKey.ZoomIn
        onTriggered: canvas.zoomBy(1.25)
    }
    Action {
        id: zoomOutAction
        icon.source: "icons/zoom-out.svg"
        text: qsTr("Zoom out")
        shortcut: StandardKey.ZoomOut
        onTriggered: canvas.zoomBy(0.8)
    }
    Action {
        id: rotateRightAction
        objectName: "rotateRightAction"
        icon.source: "icons/rotate-right.svg"
        text: editor.hasSelection ? qsTr("Rotate selection right 90°") : qsTr("Rotate image right 90°")
        enabled: editor.canSelect && !editor.busy && !window.documentOptionsOpen
        onTriggered: {
            window.lastRotation = rotateRightAction;
            editor.transformImage(0);
        }
    }
    Action {
        id: rotateLeftAction
        objectName: "rotateLeftAction"
        icon.source: "icons/rotate-left.svg"
        text: editor.hasSelection ? qsTr("Rotate selection left 90°") : qsTr("Rotate image left 90°")
        enabled: editor.canSelect && !editor.busy && !window.documentOptionsOpen
        onTriggered: {
            window.lastRotation = rotateLeftAction;
            editor.transformImage(1);
        }
    }
    Action {
        id: rotateHalfAction
        objectName: "rotateHalfAction"
        icon.source: "icons/rotate-half.svg"
        text: editor.hasSelection ? qsTr("Rotate selection 180°") : qsTr("Rotate image 180°")
        enabled: editor.canSelect && !editor.busy && !window.documentOptionsOpen
        onTriggered: {
            window.lastRotation = rotateHalfAction;
            editor.transformImage(2);
        }
    }
    Action {
        id: flipHorizontalAction
        objectName: "flipHorizontalAction"
        icon.source: "icons/flip.svg"
        text: editor.hasSelection ? qsTr("Flip selection horizontally") : qsTr("Flip image horizontally")
        enabled: editor.canSelect && !editor.busy && !window.documentOptionsOpen
        onTriggered: {
            window.lastFlip = flipHorizontalAction;
            editor.transformImage(3);
        }
    }
    Action {
        id: flipVerticalAction
        objectName: "flipVerticalAction"
        icon.source: "icons/flip-vertical.svg"
        text: editor.hasSelection ? qsTr("Flip selection vertically") : qsTr("Flip image vertically")
        enabled: editor.canSelect && !editor.busy && !window.documentOptionsOpen
        onTriggered: {
            window.lastFlip = flipVerticalAction;
            editor.transformImage(4);
        }
    }
    Action {
        id: resizeAction
        icon.source: "icons/resize.svg"
        objectName: "resizeAction"
        text: editor.hasSelection ? qsTr("Resize/skew selection…") : qsTr("Resize/skew image…")
        enabled: editor.canSelect && !editor.busy && !window.documentOptionsOpen
        onTriggered: resizeDialog.open()
    }
    Action {
        id: cropAction
        icon.source: "icons/crop.svg"
        text: qsTr("Crop to selection")
        enabled: editor.canSelect && editor.hasSelection && !editor.busy && !window.documentOptionsOpen
        onTriggered: editor.transformImage(5)
    }
    Action {
        id: copyAction
        icon.source: "icons/copy.svg"
        text: qsTr("Copy")
        shortcut: "Ctrl+C"
        enabled: editor.canSelect && !editor.busy && editor.hasSelection && !window.textInputFocused && !window.documentOptionsOpen
        onTriggered: editor.copySelection()
    }
    Action {
        id: cutAction
        icon.source: "icons/cut.svg"
        text: qsTr("Cut")
        shortcut: "Ctrl+X"
        enabled: editor.canSelect && !editor.busy && editor.hasSelection && !window.textInputFocused && !window.documentOptionsOpen
        onTriggered: editor.cutSelection()
    }
    Action {
        id: pasteAction
        icon.source: "icons/paste.svg"
        text: qsTr("Paste")
        shortcut: "Ctrl+V"
        enabled: editor.canSelect && !editor.busy && !window.textInputFocused && !window.documentOptionsOpen
        onTriggered: editor.pasteClipboard()
    }
    Action {
        id: selectAllAction
        icon.source: "icons/selection.svg"
        objectName: "selectAllAction"
        text: qsTr("Select all")
        shortcut: "Ctrl+A"
        enabled: editor.canSelect && !window.textInputFocused && !window.documentOptionsOpen
        onTriggered: editor.selectAll()
    }
    Action {
        id: deselectAction
        icon.source: "icons/selection.svg"
        text: qsTr("Deselect")
        shortcut: "Ctrl+D"
        enabled: editor.canSelect && editor.hasSelection && !window.textInputFocused && !window.documentOptionsOpen
        onTriggered: editor.deselect()
    }
    Action {
        id: deleteSelectionAction
        icon.source: "icons/delete.svg"
        text: qsTr("Delete selection")
        shortcut: "Delete"
        enabled: editor.canSelect && editor.hasSelection && !window.textInputFocused && !window.documentOptionsOpen
        onTriggered: editor.deleteSelection()
    }
    Action {
        id: aboutAction
        icon.source: "icons/info.svg"
        objectName: "aboutAction"
        text: qsTr("&About OpenPaint")
        shortcut: "F1"
        onTriggered: aboutDialog.open()
    }
    ActionGroup {
        Action {
            id: systemThemeAction
            icon.source: "icons/menu.svg"
            objectName: "systemThemeAction"
            text: qsTr("&System")
            checkable: true
            checked: Theme.mode === 0
            onTriggered: Theme.mode = 0
        }
        Action {
            id: lightThemeAction
            icon.source: "icons/palette.svg"
            objectName: "lightThemeAction"
            text: qsTr("&Light")
            checkable: true
            checked: Theme.mode === 1
            onTriggered: Theme.mode = 1
        }
        Action {
            id: darkThemeAction
            icon.source: "icons/palette.svg"
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
                objectName: "primaryMenuBar"
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
                    PaintMenuItem {
                        action: exportAction
                    }
                    MenuSeparator {}
                    PaintMenuItem {
                        objectName: "updatesMenuItem"
                        symbol: "update"
                        text: qsTr("Updates…")
                        onTriggered: updatesDialog.open()
                    }
                    PaintMenuItem {
                        action: aboutAction
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
            }
            ToolSeparator {}
            IconButton {
                objectName: "quickSaveButton"
                symbol: "save"
                shortcutLabel: "Ctrl+S"
                action: saveAction
                Accessible.name: text
            }
            IconButton {
                objectName: "quickUndoButton"
                symbol: "undo"
                shortcutLabel: "Ctrl+Z"
                action: undoAction
                Accessible.name: text
            }
            IconButton {
                objectName: "quickRedoButton"
                symbol: "redo"
                shortcutLabel: "Ctrl+Y"
                action: redoAction
                Accessible.name: text
            }
            Item {
                Layout.fillWidth: true
            }
            IconButton {
                objectName: "quickUpdatesButton"
                flat: true
                symbol: "update"
                text: qsTr("Check for updates")
                onClicked: updatesDialog.open()
            }
            IconButton {
                objectName: "settingsButton"
                flat: true
                symbol: "settings"
                text: qsTr("Settings")
                onClicked: settingsMenu.open()
                PaintMenu {
                    id: settingsMenu
                    objectName: "settingsMenu"
                    x: parent.width - width
                    y: parent.height
                    title: qsTr("Settings")
                    PaintMenuItem {
                        text: qsTr("Appearance")
                        enabled: false
                    }
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
            spacing: 4
            ColumnLayout {
                spacing: 0
                Item {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 56
                    Layout.alignment: Qt.AlignHCenter
                    IconButton {
                        id: selectionMain
                        icon.width: 26
                        icon.height: 26
                        padding: 4
                        objectName: "selectionButton"
                        symbol: editor.freeFormSelection ? "lasso" : "selection"
                        text: editor.freeFormSelection ? qsTr("Select free-form") : qsTr("Select rectangle")
                        Accessible.name: text
                        background: Rectangle {
                            radius: Theme.controlRadius
                            color: selectionMain.down ? Theme.pressed : selectionMain.hovered ? Theme.hover : "transparent"
                        }
                        enabled: editor.canSelect
                        onClicked: editor.tool = Editor.Select
                        width: parent.width
                        height: 36
                    }
                    PaintButton {
                        id: selectionDropdown
                        objectName: "selectionMenuButton"
                        text: "⌄"
                        Accessible.name: qsTr("Selection options")
                        y: 36
                        width: parent.width
                        height: 20
                        background: Rectangle {
                            radius: Theme.controlRadius
                            color: selectionDropdown.down ? Theme.pressed : selectionDropdown.hovered ? Theme.hover : "transparent"
                        }
                        enabled: editor.canSelect || editor.floatingActive
                        onClicked: selectionMenu.open()
                        PaintMenu {
                            id: selectionMenu
                            objectName: "selectionMenu"
                            y: parent.height
                            PaintMenuItem {
                                symbol: "selection"
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
                                symbol: "lasso"
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
                                symbol: "invert"
                                text: qsTr("Invert selection")
                                enabled: editor.canSelect && editor.hasSelection
                                onTriggered: editor.invertSelection()
                            }
                            PaintMenuItem {
                                symbol: "opacity"
                                text: qsTr("Transparent selection")
                                checkable: true
                                checked: editor.transparentSelection
                                onTriggered: editor.transparentSelection = !editor.transparentSelection
                            }
                            PaintMenuItem {
                                action: deselectAction
                            }
                            PaintMenuItem {
                                symbol: "resize"
                                text: editor.floatingActive ? qsTr("Selection position…") : qsTr("Selection bounds…")
                                enabled: editor.hasSelection
                                onTriggered: selectionOptions.open()
                            }
                            PaintMenuItem {
                                action: deleteSelectionAction
                            }
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.controlRadius
                        color: "transparent"
                        border.color: Theme.accent
                        border.width: selectionMain.visualFocus || selectionDropdown.visualFocus ? 2 : editor.tool === Editor.Select ? 1 : 0
                    }
                }
                Label {
                    text: qsTr("Selection")
                    color: Theme.secondaryText
                    Layout.topMargin: 4
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                spacing: 4
                GridLayout {
                    columns: 2
                    rowSpacing: 2
                    columnSpacing: 4
                    IconButton {
                        flat: true
                        symbol: "crop"
                        objectName: "cropButton"
                        action: cropAction
                    }
                    RowLayout {
                        spacing: 0
                        IconButton {
                            flat: true
                            objectName: "rotateButton"
                            icon.source: window.lastRotation.icon.source
                            action: window.lastRotation
                        }
                        PaintButton {
                            flat: true
                            objectName: "rotateMenuButton"
                            text: "⌄"
                            Accessible.name: qsTr("Rotation options")
                            implicitWidth: 24
                            implicitHeight: 32
                            enabled: rotateRightAction.enabled
                            onClicked: rotateMenu.open()
                            PaintMenu {
                                id: rotateMenu
                                objectName: "rotateMenu"
                                y: parent.height
                                PaintMenuItem {
                                    action: rotateRightAction
                                }
                                PaintMenuItem {
                                    action: rotateLeftAction
                                }
                                PaintMenuItem {
                                    action: rotateHalfAction
                                }
                            }
                        }
                    }
                    IconButton {
                        flat: true
                        symbol: "resize"
                        objectName: "resizeButton"
                        action: resizeAction
                    }
                    RowLayout {
                        spacing: 0
                        IconButton {
                            flat: true
                            objectName: "flipButton"
                            icon.source: window.lastFlip.icon.source
                            action: window.lastFlip
                        }
                        PaintButton {
                            flat: true
                            objectName: "flipMenuButton"
                            text: "⌄"
                            Accessible.name: qsTr("Flip options")
                            implicitWidth: 24
                            implicitHeight: 32
                            enabled: flipHorizontalAction.enabled
                            onClicked: flipMenu.open()
                            PaintMenu {
                                id: flipMenu
                                objectName: "flipMenu"
                                y: parent.height
                                PaintMenuItem {
                                    action: flipHorizontalAction
                                }
                                PaintMenuItem {
                                    action: flipVerticalAction
                                }
                            }
                        }
                    }
                }
                Label {
                    text: qsTr("Image")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                GridLayout {
                    columns: 3
                    IconButton {
                        flat: true
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
                        flat: true
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
                        flat: true
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
                        flat: true
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
                        flat: true
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
                spacing: 0
                Item {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 56
                    Layout.alignment: Qt.AlignHCenter
                    IconButton {
                        id: brushesButton
                        flat: true
                        objectName: "brushesButton"
                        enabled: !editor.textActive
                        symbol: "brush"
                        icon.width: 28
                        icon.height: 28
                        width: 40
                        height: 36
                        padding: 4
                        text: qsTr("Brush")
                        background: Rectangle {
                            radius: Theme.controlRadius
                            color: brushesButton.down ? Theme.pressed : brushesButton.hovered ? Theme.hover : "transparent"
                        }
                        onClicked: editor.tool = Editor.Brush
                        Accessible.name: text
                    }
                    PaintButton {
                        id: brushDropdown
                        objectName: "brushMenuButton"
                        flat: true
                        text: "⌄"
                        width: 40
                        height: 20
                        y: 36
                        background: Rectangle {
                            radius: Theme.controlRadius
                            color: brushDropdown.down ? Theme.pressed : brushDropdown.hovered ? Theme.hover : "transparent"
                        }
                        enabled: !editor.textActive && !editor.busy
                        Accessible.name: qsTr("Brush styles")
                        onClicked: brushesMenu.open()
                        BrushMenu {
                            id: brushesMenu
                            objectName: "brushesMenu"
                            editor: window.editorController
                            y: brushDropdown.height
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.controlRadius
                        color: "transparent"
                        border.color: Theme.accent
                        border.width: brushesButton.visualFocus || brushDropdown.visualFocus ? 2 : editor.tool === Editor.Brush ? 1 : 0
                    }
                }
                Label {
                    text: qsTr("Brushes")
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 4
                    color: Theme.secondaryText
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                spacing: 2
                RowLayout {
                    spacing: 2
                    ShapeGallery {
                        editor: window.editorController
                    }
                    ColumnLayout {
                        spacing: 2
                        IconButton {
                            id: shapesButton
                            flat: true
                            objectName: "shapesButton"
                            enabled: !editor.textActive
                            symbol: "rectangle"
                            implicitWidth: 24
                            implicitHeight: 32
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
                        PaintButton {
                            flat: true
                            text: "▾"
                            implicitWidth: 24
                            implicitHeight: 32
                            Accessible.name: qsTr("Shape outline and fill")
                            onClicked: shapeStyleMenu.open()
                            PaintMenu {
                                id: shapeStyleMenu
                                y: parent.height
                                PaintMenuItem {
                                    symbol: "rectangle"
                                    text: qsTr("Outline")
                                    checkable: true
                                    checked: editor.shapeOutline
                                    onTriggered: editor.shapeOutline = !editor.shapeOutline
                                }
                                PaintMenuItem {
                                    symbol: "fill"
                                    text: qsTr("Fill")
                                    checkable: true
                                    checked: editor.shapeFill
                                    onTriggered: editor.shapeFill = !editor.shapeFill
                                }
                            }
                        }
                    }
                }
                Label {
                    text: qsTr("Shapes")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            ColumnLayout {
                visible: editor.tool === Editor.Fill || editor.tool === Editor.Eraser
                PaintSpinBox {
                    objectName: "fillTolerance"
                    visible: editor.tool === Editor.Fill
                    from: 0
                    to: 255
                    value: editor.tolerance
                    onValueModified: editor.tolerance = value
                    Accessible.name: qsTr("Fill tolerance")
                    implicitWidth: 104
                }
                PaintCheckBox {
                    text: qsTr("Transparent")
                    visible: editor.tool === Editor.Eraser
                    checked: editor.transparentEraser
                    onToggled: editor.transparentEraser = checked
                }
                Label {
                    text: editor.tool === Editor.Fill ? qsTr("Tolerance") : qsTr("Eraser")
                    color: Theme.secondaryText
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            ToolSeparator {
                visible: editor.tool === Editor.Fill || editor.tool === Editor.Eraser
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
                    checked: !editor.editingSecondary
                    checkable: true
                    autoExclusive: true
                    enabled: !editor.busy
                    text: qsTr("Choose drawing color")
                    onClicked: editor.editingSecondary = false
                }
                ColorSwatch {
                    objectName: "secondaryButton"
                    implicitWidth: 32
                    implicitHeight: 32
                    diameter: 24
                    swatchColor: editor.secondary
                    checked: editor.editingSecondary
                    checkable: true
                    autoExclusive: true
                    enabled: !editor.busy
                    text: qsTr("Choose background and fill color")
                    onClicked: editor.editingSecondary = true
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
                            checked: editor.selectedColor.toString() === modelData
                            onClicked: editor.selectedColor = modelData
                        }
                    }
                }
                RowLayout {
                    spacing: 0
                    Repeater {
                        model: 10
                        delegate: ColorSwatch {
                            id: toolbarSlot
                            objectName: "customToolbarSwatch"
                            required property int index
                            empty: customPalette.toolbarColors[index] === ""
                            swatchColor: empty ? "transparent" : customPalette.toolbarColors[index]
                            checked: !empty && editor.selectedColor.toString() === swatchColor.toString()
                            text: empty ? qsTr("Empty custom slot %1 · edit colors").arg(index + 1) : qsTr("Custom color %1").arg(swatchColor.toString())
                            onClicked: {
                                if (empty)
                                    colorDialog.begin(editor.selectedColor, editor.editingSecondary ? "secondary" : "primary");
                                else
                                    editor.selectedColor = swatchColor;
                            }
                            function replaceColor() {
                                colorDialog.begin(empty ? editor.selectedColor : swatchColor, editor.editingSecondary ? "secondary" : "primary");
                                if (!empty)
                                    colorDialog.replacementSlot = index;
                            }
                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: slotMenu.open()
                            }
                            Keys.onPressed: function (event) {
                                if (event.key === Qt.Key_Menu || (event.key === Qt.Key_F10 && (event.modifiers & Qt.ShiftModifier))) {
                                    slotMenu.open();
                                    event.accepted = true;
                                }
                            }
                            PaintMenu {
                                id: slotMenu
                                y: parent.height
                                PaintMenuItem {
                                    symbol: "palette"
                                    text: toolbarSlot.empty ? qsTr("Set color…") : qsTr("Replace color…")
                                    onTriggered: toolbarSlot.replaceColor()
                                }
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
            Item {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 72
                Layout.alignment: Qt.AlignTop
                IconButton {
                    id: editColorsOption
                    objectName: "editColorsButton"
                    background: Rectangle {
                        radius: width / 2
                        color: editColorsOption.down ? Theme.pressed : editColorsOption.hovered ? Theme.hover : "transparent"
                        border.width: editColorsOption.visualFocus ? 2 : 0
                        border.color: Theme.accent
                    }
                    flat: true
                    width: 28
                    height: 28
                    x: 6
                    y: 10
                    icon.width: 26
                    icon.height: 26
                    padding: 1
                    symbol: "palette"
                    icon.color: "transparent"
                    text: qsTr("Edit colors")
                    onClicked: colorDialog.begin(editor.selectedColor, editor.editingSecondary ? "secondary" : "primary")
                }
                IconButton {
                    id: transparencyOption
                    objectName: "transparentBackgroundButton"
                    background: Rectangle {
                        anchors.centerIn: parent
                        width: 28
                        height: 28
                        radius: 14
                        color: transparencyOption.down ? Theme.pressed : transparencyOption.hovered ? Theme.hover : "transparent"
                        border.width: transparencyOption.visualFocus ? 2 : transparencyOption.highlighted || transparencyOption.hovered ? 1 : 0
                        border.color: Theme.accent
                    }
                    width: 28
                    height: 28
                    x: 6
                    y: 40
                    padding: 3
                    icon.width: 22
                    icon.height: 22
                    flat: true
                    symbol: "transparent"
                    icon.color: "transparent"
                    text: qsTr("Transparent background color")
                    highlighted: editor.secondary.a === 0
                    enabled: !editor.busy
                    onClicked: {
                        editor.editingSecondary = true;
                        editor.selectedColor = "transparent";
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
        editor: window.editorController
        enabled: !window.editorController.loading || window.editorController.filling
        focus: true
        Accessible.name: qsTr("Drawing canvas")
        Accessible.role: Accessible.Canvas
        InlineText {
            id: textOverlay
            editor: window.editorController
            canvas: window.canvasController
        }
        ResizeHandles {
            anchors.fill: parent
            editor: window.editorController
            canvas: window.canvasController
            z: 2
        }
    }
    Column {
        objectName: "floatingToolControls"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 12
        anchors.topMargin: 24
        spacing: 8
        z: 2
        enabled: !editor.busy
        property real panelHeight: Math.max(120, Math.min(280, (parent.height - 64) / 2))
        ToolSlider {
            visible: editor.tool === Editor.Pencil || editor.tool === Editor.Brush || editor.tool === Editor.Eraser || editor.tool === Editor.Shape
            height: parent.panelHeight
            label: qsTr("Size")
            symbol: "stroke-size"
            controlName: "strokeSize"
            minimum: 1
            maximum: 128
            value: editor.brushSize
            suffix: qsTr(" px")
            onAdjusted: value => editor.brushSize = value
        }
        ToolSlider {
            visible: editor.tool === Editor.Brush
            height: parent.panelHeight
            label: qsTr("Opacity")
            symbol: "opacity"
            controlName: "brushOpacity"
            minimum: 0
            maximum: 100
            value: editor.opacity
            suffix: "%"
            onAdjusted: value => editor.opacity = value
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
                text: editor.floatingActive ? qsTr("Drag to move · Enter applies · Escape cancels") : editor.textActive ? qsTr("Click outside text to apply · Escape cancels") : editor.previewActive ? (editor.shapeKind === 15 ? qsTr("Add points · Enter applies") : qsTr("Enter to apply · Escape to cancel")) : editor.transforming ? qsTr("Transforming image…") : editor.clipboardBusy ? qsTr("Processing clipboard…") : editor.filling ? qsTr("Filling…") : editor.exporting ? qsTr("Exporting copy…") : editor.busy ? (editor.loading ? qsTr("Opening image…") : qsTr("Saving PNG…")) : qsTr("Space+drag to pan")
                color: Theme.secondaryText
                Layout.fillWidth: true
            }
            PaintButton {
                objectName: "shapeOptionsButton"
                text: qsTr("Options")
                visible: editor.previewActive && !editor.textActive
                onClicked: editor.floatingActive ? selectionOptions.open() : shapeOptions.open()
            }
            PaintButton {
                objectName: "applyPreviewButton"
                text: qsTr("Apply")
                visible: editor.previewActive && !editor.textActive
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
        title: qsTr("Open image")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.bmp *.webp)"), qsTr("PNG images (*.png)"), qsTr("JPEG images (*.jpg *.jpeg)"), qsTr("BMP images (*.bmp)"), qsTr("WebP images (*.webp)")]
        onAccepted: editor.openFile(selectedFile)
    }
    FileDialog {
        id: exportDialog
        objectName: "exportFileDialog"
        title: qsTr("Export copy")
        fileMode: FileDialog.SaveFile
        defaultSuffix: exportOptions.suffix
        nameFilters: [exportOptions.fileFilter]
        onAccepted: editor.exportFile(selectedFile, exportOptions.formatIndex, exportOptions.quality, exportOptions.lossless, exportOptions.flatten, exportOptions.backgroundColor)
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
            customPalette.remember(chosen, replacementSlot);
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
