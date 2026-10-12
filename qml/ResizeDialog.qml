// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property Editor editor
    property int sourceWidth: 1
    property int sourceHeight: 1
    property bool percentages: false
    property bool applyRequested: false
    readonly property int outputWidth: percentages ? Math.max(1, Math.round(widthInput.value * sourceWidth / 100)) : widthInput.value
    readonly property int outputHeight: percentages ? Math.max(1, Math.round(heightInput.value * sourceHeight / 100)) : heightInput.value
    function setPercent(value: bool) {
        const w = outputWidth;
        const h = outputHeight;
        percentages = value;
        widthInput.value = value ? Math.max(1, Math.round(w * 100 / sourceWidth)) : w;
        heightInput.value = value ? Math.max(1, Math.round(h * 100 / sourceHeight)) : h;
    }
    title: editor.hasSelection ? qsTr("Resize/skew selection") : qsTr("Resize/skew image")
    modal: true
    anchors.centerIn: parent
    width: 460
    standardButtons: Dialog.Ok | Dialog.Cancel
    onOpened: {
        applyRequested = false;
        sourceWidth = editor.hasSelection ? editor.selectionRect.width : editor.imageSize.width;
        sourceHeight = editor.hasSelection ? editor.selectionRect.height : editor.imageSize.height;
        percentages = false;
        aspect.checked = true;
        widthInput.value = sourceWidth;
        heightInput.value = sourceHeight;
        skewX.value = skewY.value = 0;
        smooth.checked = true;
    }
    onAccepted: applyRequested = true
    onRejected: applyRequested = false
    onClosed: {
        if (applyRequested) {
            applyRequested = false;
            editor.resizeImage(outputWidth, outputHeight, skewX.value, skewY.value, smooth.checked);
        }
    }
    ColumnLayout {
        spacing: 10
        PaintButton {
            objectName: "resizeUnits"
            text: dialog.percentages ? qsTr("Percentage ▾") : qsTr("Pixels ▾")
            onClicked: units.open()
            PaintMenu {
                id: units
                y: parent.height
                PaintMenuItem {
                    text: qsTr("Pixels")
                    onTriggered: dialog.setPercent(false)
                }
                PaintMenuItem {
                    text: qsTr("Percentage")
                    onTriggered: dialog.setPercent(true)
                }
            }
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Width")
                color: Theme.text
            }
            PaintSpinBox {
                id: widthInput
                objectName: "resizeWidth"
                Accessible.name: qsTr("Resize width")
                editable: true
                from: 1
                to: dialog.percentages ? 1000 : aspect.checked ? Math.min(16384, Math.floor(16384 * dialog.sourceWidth / dialog.sourceHeight)) : 16384
                onValueModified: {
                    if (aspect.checked)
                        heightInput.value = dialog.percentages ? value : Math.max(1, Math.round(value * dialog.sourceHeight / dialog.sourceWidth));
                }
            }
            Label {
                text: qsTr("Height")
                color: Theme.text
            }
            PaintSpinBox {
                id: heightInput
                objectName: "resizeHeight"
                Accessible.name: qsTr("Resize height")
                editable: true
                from: 1
                to: dialog.percentages ? 1000 : aspect.checked ? Math.min(16384, Math.floor(16384 * dialog.sourceHeight / dialog.sourceWidth)) : 16384
                onValueModified: {
                    if (aspect.checked)
                        widthInput.value = dialog.percentages ? value : Math.max(1, Math.round(value * dialog.sourceWidth / dialog.sourceHeight));
                }
            }
        }
        PaintCheckBox {
            id: aspect
            objectName: "resizeAspect"
            text: qsTr("Maintain aspect ratio")
            checked: true
            onClicked: {
                if (checked)
                    heightInput.value = dialog.percentages ? widthInput.value : Math.max(1, Math.round(widthInput.value * dialog.sourceHeight / dialog.sourceWidth));
            }
        }
        PaintCheckBox {
            id: smooth
            text: qsTr("Smooth scaling (turn off for pixel art)")
            checked: true
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Horizontal skew (°)")
                color: Theme.text
            }
            PaintSpinBox {
                id: skewX
                objectName: "horizontalSkew"
                Accessible.name: qsTr("Horizontal skew in degrees")
                editable: true
                from: -89
                to: 89
            }
            Label {
                text: qsTr("Vertical skew (°)")
                color: Theme.text
            }
            PaintSpinBox {
                id: skewY
                objectName: "verticalSkew"
                Accessible.name: qsTr("Vertical skew in degrees")
                editable: true
                from: -89
                to: 89
            }
        }
        Label {
            text: qsTr("Before skew: %1 × %2 px").arg(dialog.outputWidth).arg(dialog.outputHeight)
            color: Theme.secondaryText
        }
        Label {
            text: qsTr("Skew expands the result; uncovered areas are transparent.")
            color: Theme.secondaryText
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
    }
}
