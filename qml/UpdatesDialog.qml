// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property UpdateService service
    property string browserError: ""
    title: qsTr("Updates")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 48, 520)
    standardButtons: Dialog.Close
    onOpened: browserError = ""
    contentItem: ColumnLayout {
        spacing: 12
        Label {
            text: qsTr("OpenPaint %1").arg(AppInfo.version)
            font.pixelSize: 20
            font.bold: true
            color: Theme.text
        }
        Label {
            objectName: "updateIdentity"
            text: qsTr("%1 · %2").arg(AppInfo.channel).arg(AppInfo.installation)
            color: Theme.text
        }
        Label {
            text: AppInfo.instructions
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        Label {
            objectName: "releaseCheckStatus"
            text: dialog.service.message
            textFormat: Text.PlainText
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        RowLayout {
            PaintButton {
                objectName: "checkReleaseButton"
                text: qsTr("Check now")
                enabled: dialog.service.supported && !dialog.service.busy
                onClicked: dialog.service.check()
            }
            PaintButton {
                text: qsTr("Cancel check")
                visible: dialog.service.busy
                onClicked: dialog.service.cancel()
            }
        }
        Label {
            text: qsTr("Checks api.github.com for stable releases. GitHub receives your IP address and ordinary request metadata; no artwork or document information is sent.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        PaintCheckBox {
            objectName: "automaticReleaseChecks"
            text: qsTr("Check at startup and daily while running")
            enabled: dialog.service.supported
            checked: dialog.service.automatic
            onToggled: dialog.service.automatic = checked
        }
        Label {
            text: qsTr("Updates are downloaded and installed manually. A GitHub release may not yet be available in your package manager.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        PaintButton {
            objectName: "viewReleasesButton"
            text: qsTr("View releases")
            onClicked: {
                if (!(dialog.service.available ? dialog.service.openRelease() : AppInfo.openReleases()))
                    dialog.browserError = qsTr("The browser could not be opened. Open the address below manually.");
            }
        }
        Label {
            text: qsTr("Opens GitHub in your browser. GitHub receives ordinary browser request information.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        Label {
            text: AppInfo.releaseUrl.toString()
            Layout.fillWidth: true
            wrapMode: Text.WrapAnywhere
            color: Theme.text
        }
        Label {
            text: dialog.browserError
            visible: text.length > 0
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
    }
}
