// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
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
        anchors.fill: parent
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
        font.family: box.editor.textFamily
        font.pixelSize: box.editor.textSize * box.canvas.zoom
        font.bold: box.editor.textBold
        font.italic: box.editor.textItalic
        font.underline: box.editor.textUnderline
        font.strikeout: box.editor.textStrikeout
        horizontalAlignment: box.editor.textAlignment === 1 ? TextEdit.AlignHCenter : box.editor.textAlignment === 2 ? TextEdit.AlignRight : TextEdit.AlignLeft
        renderType: Text.NativeRendering
        onTextChanged: {
            if (box.editor.textReady && text !== box.editor.textDraft && !box.editor.setTextDraft(text))
                text = box.editor.textDraft;
        }
        Keys.onEscapePressed: event => {
            box.editor.cancel();
            event.accepted = true;
        }
        Keys.onPressed: event => {
            if (event.key === Qt.Key_Y && (event.modifiers & Qt.ControlModifier)) {
                input.redo();
                event.accepted = true;
            } else if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && (event.modifiers & Qt.ControlModifier)) {
                box.editor.commitPreview();
                event.accepted = true;
            }
        }
        Accessible.name: qsTr("Text on canvas")
        cursorDelegate: Rectangle {
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
            width: 1
            height: Math.max(1, input.font.pixelSize)
            color: Theme.accent
            visible: input.activeFocus && input.selectionStart === input.selectionEnd && blinkOn
        }
    }
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: Theme.accent
    }
    Connections {
        target: box.editor
        function onTextEditingReady() {
            input.text = box.editor.textDraft;
            input.forceActiveFocus();
        }
    }
}
