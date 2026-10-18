// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    title: qsTr("Glass appearance")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 48, 440)
    standardButtons: Dialog.Close
    contentItem: ColumnLayout {
        spacing: 16
        PaintCheckBox {
            objectName: "glassEnabled"
            text: qsTr("Enable glass appearance")
            checked: Appearance.glassEnabled
            onToggled: Appearance.glassEnabled = checked
        }
        Label {
            text: qsTr("Surface opacity: %1%").arg(Appearance.surfaceOpacity)
            color: Theme.text
        }
        Slider {
            objectName: "glassOpacity"
            Layout.fillWidth: true
            from: 40
            to: 100
            stepSize: 1
            value: Appearance.surfaceOpacity
            enabled: Appearance.glassEnabled
            Accessible.name: qsTr("Glass surface opacity, 40 to 100 percent")
            onMoved: Appearance.surfaceOpacity = Math.round(value)
        }
        Label {
            text: qsTr("Reveal the desktop through the workspace and glass surfaces. Text and paper stay opaque; transparent artwork shows a checkerboard. Floating panels blur artwork beneath them; desktop blur depends on your desktop setup. Preferences are saved on this device.")
            color: Theme.text
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        PaintButton {
            objectName: "restoreAppearance"
            text: qsTr("Restore defaults")
            onClicked: Appearance.restoreDefaults()
        }
    }
}
