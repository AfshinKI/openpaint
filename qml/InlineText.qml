// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import OpenPaint

Rectangle {
    id: box
    required property Editor editor
    required property CanvasItem canvas
    function focusInput() {
        input.forceActiveFocus();
    }
    x: canvas.origin.x + editor.textRect.x * canvas.zoom
    y: canvas.origin.y + editor.textRect.y * canvas.zoom
    width: editor.textRect.width * canvas.zoom
    height: editor.textRect.height * canvas.zoom
    visible: editor.textActive
    color: !editor.hasSelection && editor.textBackground && editor.textDraft.length > 0 ? editor.textBackgroundColor : "transparent"
    TextEdit {
        id: input
        objectName: "inlineTextInput"
        width: box.editor.textRect.width
        height: box.editor.textRect.height
        scale: box.canvas.zoom
        transformOrigin: Item.TopLeft
        clip: true
        enabled: box.editor.textReady
        text: box.editor.textDraft
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap
        selectByMouse: true
        activeFocusOnTab: true
        Keys.onTabPressed: event => {
            input.nextItemInFocusChain(true).forceActiveFocus(Qt.TabFocusReason);
            event.accepted = true;
        }
        Keys.onBacktabPressed: event => {
            input.nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason);
            event.accepted = true;
        }
        persistentSelection: true
        color: box.editor.hasSelection ? "transparent" : box.editor.textColor
        selectionColor: Theme.accent
        selectedTextColor: Theme.accentText
        font.pixelSize: 20
        Component.onCompleted: box.editor.attachTextDocument(textDocument)
        onCursorPositionChanged: {
            box.editor.syncTextCursor(cursorPosition, selectionStart, selectionEnd);
            box.editor.syncTextFont(cursorSelection.font);
        }
        onSelectionStartChanged: box.editor.syncTextCursor(cursorPosition, selectionStart, selectionEnd)
        onSelectionEndChanged: box.editor.syncTextCursor(cursorPosition, selectionStart, selectionEnd)
        horizontalAlignment: box.editor.textAlignment === 1 ? TextEdit.AlignHCenter : box.editor.textAlignment === 2 ? TextEdit.AlignRight : TextEdit.AlignLeft
        renderType: Text.NativeRendering
        onTextChanged: {
            if (box.editor.textReady && text !== box.editor.textDraft && !box.editor.setTextDraft(text)) {
                box.editor.clearInsertionFont();
                undo();
            }
        }
        Keys.onEscapePressed: event => {
            box.editor.cancel();
            event.accepted = true;
        }
        Keys.onPressed: event => {
            if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
                box.editor.clearInsertionFont();
                input.cursorPosition = box.editor.undoText(Boolean(event.modifiers & Qt.ShiftModifier));
                event.accepted = true;
            } else if (event.key === Qt.Key_Y && (event.modifiers & Qt.ControlModifier)) {
                box.editor.clearInsertionFont();
                input.cursorPosition = box.editor.undoText(true);
                event.accepted = true;
            } else if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && (event.modifiers & Qt.ControlModifier)) {
                box.editor.commitPreview();
                event.accepted = true;
            }
        }
        Accessible.name: qsTr("Text on canvas")
        cursorDelegate: Item {}
        Rectangle {
            id: caret
            objectName: "textCaret"
            property bool blinkOn: true
            Timer {
                interval: 500
                repeat: true
                running: input.activeFocus && box.editor.textReady
                onTriggered: caret.blinkOn = !caret.blinkOn
                onRunningChanged: caret.blinkOn = true
            }
            Connections {
                target: input
                function onCursorPositionChanged() {
                    caret.blinkOn = true;
                }
            }
            x: Math.max(1, input.cursorRectangle.x)
            y: input.cursorRectangle.y
            width: 2 / box.canvas.zoom
            height: Math.max(1, input.cursorRectangle.height)
            color: "black"
            visible: input.activeFocus && input.selectionStart === input.selectionEnd && blinkOn
        }
    }
    Shape {
        id: outline
        objectName: "textOutline"
        anchors.fill: parent
        // Screen-space geometry keeps the marquee one pixel wide at every zoom.
        ShapePath {
            strokeColor: "white"
            strokeWidth: 1
            fillColor: "transparent"
            startX: 0.5
            startY: 0.5
            PathLine {
                x: outline.width - 0.5
                y: 0.5
            }
            PathLine {
                x: outline.width - 0.5
                y: outline.height - 0.5
            }
            PathLine {
                x: 0.5
                y: outline.height - 0.5
            }
            PathLine {
                x: 0.5
                y: 0.5
            }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 1
            strokeStyle: ShapePath.DashLine
            dashPattern: [4, 4]
            fillColor: "transparent"
            startX: 0.5
            startY: 0.5
            PathLine {
                x: outline.width - 0.5
                y: 0.5
            }
            PathLine {
                x: outline.width - 0.5
                y: outline.height - 0.5
            }
            PathLine {
                x: 0.5
                y: outline.height - 0.5
            }
            PathLine {
                x: 0.5
                y: 0.5
            }
        }
    }
    Connections {
        target: box.editor
        function onTextDraftChanged() {
            if (input.text !== box.editor.textDraft)
                input.text = box.editor.textDraft;
        }
        function onTextEditingReady() {
            box.editor.clearInsertionFont();
            const defaults = Qt.font({
                family: box.editor.textFamily,
                pixelSize: box.editor.textSize,
                bold: box.editor.textBold,
                italic: box.editor.textItalic,
                underline: box.editor.textUnderline,
                strikeout: box.editor.textStrikeout
            });
            input.text = box.editor.textDraft;
            input.font = defaults;
            input.cursorSelection.font = defaults;
            input.forceActiveFocus();
        }
        function onTextFontRequested(font) {
            if (!box.editor.textReady)
                return;
            if (input.selectionStart !== input.selectionEnd)
                box.editor.formatTextRange(font, input.selectionStart, input.selectionEnd);
            else
                box.editor.setInsertionFont(font, input.cursorSelection.font, input.cursorPosition);
        }
    }
}
