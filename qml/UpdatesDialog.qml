// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    property string browserError: ""
    title: qsTr("Updates")
    modal: true
    anchors.centerIn: parent
    width: 480
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
            color: Theme.secondaryText
        }
        Label {
            text: AppInfo.instructions
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
        }
        Label {
            text: qsTr("Automatic update checking and installation are not available in this version.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
        }
        PaintButton {
            objectName: "viewReleasesButton"
            text: qsTr("View releases")
            onClicked: {
                if (!AppInfo.openReleases())
                    dialog.browserError = qsTr("The browser could not be opened. Open the address below manually.");
            }
        }
        Label {
            text: qsTr("Opens GitHub in your browser. GitHub receives ordinary browser request information.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.secondaryText
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
