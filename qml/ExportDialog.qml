// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    property int formatIndex: 0
    property int backgroundIndex: 0
    property color secondaryDraft: "white"
    property bool acceptedDraft: false
    readonly property int quality: qualityInput.value
    readonly property bool lossless: losslessInput.checked
    readonly property bool requiresOpaque: formatIndex === 1 || formatIndex === 2
    readonly property bool flatten: requiresOpaque || flattenInput.checked
    readonly property color backgroundColor: backgroundIndex === 0 ? "white" : backgroundIndex === 1 ? "black" : secondaryDraft
    readonly property string suffix: ["png", "jpg", "bmp", "webp"][formatIndex]
    readonly property string fileFilter: [qsTr("PNG images (*.png)"), qsTr("JPEG images (*.jpg *.jpeg)"), qsTr("BMP images (*.bmp)"), qsTr("WebP images (*.webp)")][formatIndex]
    signal chooseDestination
    title: qsTr("Export a copy")
    modal: true
    anchors.centerIn: parent
    width: 460
    standardButtons: Dialog.Ok | Dialog.Cancel
    onOpened: {
        acceptedDraft = false;
        formatIndex = backgroundIndex = 0;
        qualityInput.value = 90;
        losslessInput.checked = true;
        flattenInput.checked = false;
        secondaryDraft = Qt.rgba(editor.secondary.r, editor.secondary.g, editor.secondary.b, 1);
    }
    onAccepted: acceptedDraft = true
    onRejected: acceptedDraft = false
    onClosed: {
        if (acceptedDraft) {
            acceptedDraft = false;
            chooseDestination();
        }
    }
    contentItem: ColumnLayout {
        spacing: 12
        Label {
            text: qsTr("File format")
            color: Theme.text
        }
        PaintButton {
            objectName: "exportFormat"
            text: ["PNG", "JPEG", "BMP", "WebP"][dialog.formatIndex] + " ▾"
            Accessible.name: qsTr("Export format")
            Layout.fillWidth: true
            onClicked: formats.open()
            PaintMenu {
                id: formats
                y: parent.height
                PaintMenuItem {
                    text: "PNG"
                    onTriggered: dialog.formatIndex = 0
                }
                PaintMenuItem {
                    text: "JPEG"
                    onTriggered: dialog.formatIndex = 1
                }
                PaintMenuItem {
                    text: "BMP"
                    onTriggered: dialog.formatIndex = 2
                }
                PaintMenuItem {
                    text: "WebP"
                    onTriggered: dialog.formatIndex = 3
                }
            }
        }
        PaintCheckBox {
            id: losslessInput
            objectName: "exportLossless"
            text: qsTr("Lossless WebP")
            checked: true
            visible: dialog.formatIndex === 3
        }
        RowLayout {
            visible: dialog.formatIndex === 1 || (dialog.formatIndex === 3 && !dialog.lossless)
            Label {
                text: qsTr("Quality")
                color: Theme.text
            }
            PaintSpinBox {
                id: qualityInput
                objectName: "exportQuality"
                Accessible.name: qsTr("Export quality")
                editable: true
                from: 1
                to: 99
                value: 90
            }
        }
        PaintCheckBox {
            id: flattenInput
            objectName: "exportFlatten"
            text: qsTr("Replace transparency with a background")
            enabled: !dialog.requiresOpaque
            visible: !dialog.requiresOpaque
        }
        RowLayout {
            visible: dialog.flatten
            Label {
                text: qsTr("Background")
                color: Theme.text
            }
            Rectangle {
                implicitWidth: 24
                implicitHeight: 24
                radius: 12
                color: dialog.backgroundColor
                border.color: Theme.divider
            }
            PaintButton {
                objectName: "exportBackground"
                text: [qsTr("White"), qsTr("Black"), qsTr("Secondary color")][dialog.backgroundIndex] + " ▾"
                onClicked: backgrounds.open()
                PaintMenu {
                    id: backgrounds
                    y: parent.height
                    PaintMenuItem {
                        text: qsTr("White")
                        onTriggered: dialog.backgroundIndex = 0
                    }
                    PaintMenuItem {
                        text: qsTr("Black")
                        onTriggered: dialog.backgroundIndex = 1
                    }
                    PaintMenuItem {
                        text: qsTr("Secondary color")
                        onTriggered: dialog.backgroundIndex = 2
                    }
                }
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
            text: dialog.formatIndex === 1 ? qsTr("JPEG loses detail and replaces transparency with the chosen background.") : dialog.formatIndex === 2 ? qsTr("BMP replaces transparency with the chosen background.") : dialog.formatIndex === 3 && !dialog.lossless ? qsTr("Lossy WebP reduces detail. Transparency is preserved unless a background is selected.") : qsTr("Lossless pixels. Transparency is preserved unless a background is selected.")
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
            text: qsTr("This writes a separate copy without marking your drawing saved. Source metadata is removed.")
        }
    }
}
