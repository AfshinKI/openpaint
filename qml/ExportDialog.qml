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
            color: Theme.text
        }
        PaintButton {
            objectName: "exportFormat"
            text: ["PNG", "JPEG", "BMP", qsTr("WebP"), qsTr("Monochrome BMP (1-bit)"), qsTr("OpenRaster project")][dialog.formatIndex] + " ▾"
            Accessible.name: qsTr("Export format")
            Layout.fillWidth: true
            onClicked: formats.open()
            PaintMenu {
                id: formats
                objectName: "exportFormatsMenu"
                y: parent.height
                PaintMenuItem {
                    symbol: "rectangle"
                    text: "PNG"
                    onTriggered: dialog.formatIndex = 0
                }
                PaintMenuItem {
                    symbol: "rectangle"
                    text: "JPEG"
                    onTriggered: dialog.formatIndex = 1
                }
                PaintMenuItem {
                    symbol: "rectangle"
                    text: "BMP"
                    onTriggered: dialog.formatIndex = 2
                }
                PaintMenuItem {
                    symbol: "rectangle"
                    text: "WebP"
                    onTriggered: dialog.formatIndex = 3
                }
                PaintMenuItem {
                    symbol: "rectangle"
                    text: qsTr("Monochrome BMP (1-bit)")
                    onTriggered: dialog.formatIndex = 4
                }
                PaintMenuItem {
                    symbol: "layers"
                    text: qsTr("OpenRaster project")
                    visible: dialog.saveAsMode
                    height: visible ? implicitHeight : 0
                    onTriggered: dialog.formatIndex = 5
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
            visible: !dialog.requiresOpaque && !dialog.savesDocument
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
            color: Theme.text
            text: dialog.formatIndex === 5 ? qsTr("Preserves editable layers, visibility and opacity.") : dialog.formatIndex === 4 ? qsTr("Monochrome BMP replaces transparency with the chosen background and converts colors to black or white at a 50% grayscale threshold, without dithering.") : dialog.formatIndex === 1 ? qsTr("JPEG loses detail and replaces transparency with the chosen background.") : dialog.formatIndex === 2 ? qsTr("BMP replaces transparency with the chosen background.") : dialog.formatIndex === 3 && !dialog.lossless ? qsTr("Lossy WebP reduces detail. Transparency is preserved unless a background is selected.") : qsTr("Lossless pixels. Transparency is preserved unless a background is selected.")
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
            text: dialog.savesDocument ? qsTr("Saves your drawing to the chosen destination.") : qsTr("This writes a flattened image copy without marking your editable drawing saved. Use PNG for a flat drawing or OpenRaster for a layered project to save the document. Source metadata is removed.")
        }
    }
}
