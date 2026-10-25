// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    property bool saveAsMode: false
    property int formatIndex: 0
    readonly property bool savesDocument: saveAsMode && (formatIndex === 5 || (formatIndex === 0 && !editor.needsProject))
    property int backgroundIndex: 0
    property color secondaryDraft: "white"
    property bool acceptedDraft: false
    readonly property int quality: qualityInput.value
    readonly property bool lossless: losslessInput.checked
    readonly property bool requiresOpaque: formatIndex === 1 || formatIndex === 2 || formatIndex === 4
    readonly property bool flatten: requiresOpaque || flattenInput.checked
    readonly property color backgroundColor: backgroundIndex === 0 ? "white" : backgroundIndex === 1 ? "black" : secondaryDraft
    readonly property string suffix: ["png", "jpg", "bmp", "webp", "bmp", "ora"][formatIndex]
    readonly property string fileFilter: [qsTr("PNG images (*.png)"), qsTr("JPEG images (*.jpg *.jpeg)"), qsTr("BMP images (*.bmp)"), qsTr("WebP images (*.webp)"), qsTr("Monochrome BMP images (*.bmp)"), qsTr("OpenRaster projects (*.ora)")][formatIndex]
    signal chooseDestination
    title: saveAsMode ? qsTr("Save As") : qsTr("Export a copy")
    modal: true
    anchors.centerIn: parent
    width: 460
    standardButtons: Dialog.Ok | Dialog.Cancel
    onOpened: {
        standardButton(Dialog.Ok).text = qsTr("Choose location…");
        acceptedDraft = false;
        formatIndex = saveAsMode && editor.needsProject ? 5 : 0;
        backgroundIndex = 0;
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
            font.bold: true
            color: Theme.text
        }
        PaintButton {
            id: formatButton
            objectName: "exportFormat"
            text: ["PNG", "JPEG", "BMP", qsTr("WebP"), qsTr("Monochrome BMP (1-bit)"), qsTr("OpenRaster project")][dialog.formatIndex] + " ▾"
            Accessible.name: dialog.saveAsMode ? qsTr("Save as format") : qsTr("Export format")
            contentItem: RowLayout {
                spacing: 12
                Label {
                    Layout.fillWidth: true
                    text: [qsTr("PNG image"), qsTr("JPEG image"), qsTr("BMP image"), qsTr("WebP image"), qsTr("Monochrome BMP"), qsTr("OpenRaster project")][dialog.formatIndex]
                    color: Theme.text
                    elide: Text.ElideRight
                }
                Label {
                    text: dialog.formatIndex === 4 ? "1-bit · .bmp" : "." + dialog.suffix
                    color: Theme.secondaryText
                }
                Label {
                    text: "⌄"
                    color: Theme.secondaryText
                }
            }
            Layout.fillWidth: true
            onClicked: formats.open()
            PaintMenu {
                id: formats
                objectName: "exportFormatsMenu"
                width: formatButton.width
                y: parent.height + 4
                PaintMenuItem {
                    symbol: "save"
                    text: "PNG"
                    checkable: true
                    checked: dialog.formatIndex === 0
                    onTriggered: dialog.formatIndex = 0
                }
                PaintMenuItem {
                    symbol: "save"
                    text: "JPEG"
                    checkable: true
                    checked: dialog.formatIndex === 1
                    onTriggered: dialog.formatIndex = 1
                }
                PaintMenuItem {
                    symbol: "save"
                    text: "BMP"
                    checkable: true
                    checked: dialog.formatIndex === 2
                    onTriggered: dialog.formatIndex = 2
                }
                PaintMenuItem {
                    symbol: "save"
                    text: "WebP"
                    checkable: true
                    checked: dialog.formatIndex === 3
                    onTriggered: dialog.formatIndex = 3
                }
                PaintMenuItem {
                    symbol: "save"
                    text: qsTr("Monochrome BMP (1-bit)")
                    checkable: true
                    checked: dialog.formatIndex === 4
                    onTriggered: dialog.formatIndex = 4
                }
                PaintMenuItem {
                    symbol: "layers"
                    text: qsTr("OpenRaster project")
                    visible: dialog.saveAsMode
                    height: visible ? implicitHeight : 0
                    checkable: true
                    checked: dialog.formatIndex === 5
                    onTriggered: dialog.formatIndex = 5
                }
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
            text: dialog.formatIndex === 5 ? qsTr("Keeps layers, visibility and opacity editable.") : dialog.formatIndex === 4 ? qsTr("1-bit black and white at 50% gray, without dithering.") : dialog.formatIndex === 1 ? qsTr("Compact files for photos. Transparency uses the background below.") : dialog.formatIndex === 2 ? qsTr("Uncompressed color image with a solid background.") : dialog.formatIndex === 3 ? qsTr("Choose lossless pixels or a smaller file with adjustable quality.") : qsTr("Lossless pixels with transparency.")
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: Theme.divider
        }
        PaintCheckBox {
            id: losslessInput
            objectName: "exportLossless"
            text: qsTr("Lossless WebP")
            checked: true
            visible: dialog.formatIndex === 3
        }
        RowLayout {
            Layout.fillWidth: true
            visible: dialog.formatIndex === 1 || (dialog.formatIndex === 3 && !dialog.lossless)
            Label {
                Layout.fillWidth: true
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
            visible: !dialog.requiresOpaque && !dialog.savesDocument
        }
        RowLayout {
            Layout.fillWidth: true
            visible: dialog.flatten && !dialog.savesDocument
            Label {
                Layout.fillWidth: true
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
                        symbol: "fill"
                        text: qsTr("White")
                        onTriggered: dialog.backgroundIndex = 0
                    }
                    PaintMenuItem {
                        symbol: "fill"
                        text: qsTr("Black")
                        onTriggered: dialog.backgroundIndex = 1
                    }
                    PaintMenuItem {
                        symbol: "fill"
                        text: qsTr("Secondary color")
                        onTriggered: dialog.backgroundIndex = 2
                    }
                }
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
            text: dialog.savesDocument ? qsTr("Your drawing will be saved to the location you choose next.") : dialog.editor.needsProject ? qsTr("Exports a flattened copy without changing your drawing’s saved state. Choose OpenRaster in Save As to save editable layers.") : qsTr("Exports a copy without changing your drawing’s saved state. Use PNG in Save As to save the drawing.")
        }
    }
}
