// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    signal glassSetupRequested
    function focusGlassSetup() {
        if (enableGlass.visible)
            enableGlass.forceActiveFocus();
        else
            glassToggle.forceActiveFocus();
    }
    onOpened: BlurSupport.refresh()
    title: qsTr("Glass appearance")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 48, 440)
    standardButtons: Dialog.Close
    contentItem: ColumnLayout {
        spacing: 16
        PaintCheckBox {
            id: glassToggle
            objectName: "glassEnabled"
            text: qsTr("Enable glass appearance")
            checked: Appearance.effectiveGlassEnabled
            enabled: BlurSupport.ready
            opacity: enabled ? 1 : 0.45
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
            enabled: Appearance.effectiveGlassEnabled
            opacity: enabled ? 1 : 0.45
            Accessible.name: qsTr("Glass surface opacity, 40 to 100 percent")
            onMoved: Appearance.surfaceOpacity = Math.round(value)
        }
        Label {
            text: qsTr("Soften the workspace. Keep your artwork and controls clear.")
            color: Theme.text
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        PaintButton {
            id: enableGlass
            objectName: "enableGlassSetup"
            text: BlurSupport.ready ? qsTr("Glass setup") : qsTr("Enable Glass")
            visible: true
            onClicked: dialog.glassSetupRequested()
        }
        PaintButton {
            objectName: "restoreAppearance"
            text: qsTr("Restore defaults")
            onClicked: Appearance.restoreDefaults()
        }
    }
}
