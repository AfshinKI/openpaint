// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenPaint

PaintDialog {
    id: dialog
    required property UpdateService service
    property string browserError: ""
    signal blurHelpRequested
    function focusBlurHelp() {
        (blurHelp.visible ? blurHelp : doneButton).forceActiveFocus();
    }
    title: qsTr("Updates")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 48, 800)
    height: Math.min(parent.height - 48, 440)
    standardButtons: Dialog.NoButton
    onOpened: {
        browserError = "";
        BlurSupport.refresh();
        service.refresh();
    }
    footer: RowLayout {
        spacing: 12
        PaintButton {
            id: blurHelp
            objectName: "backgroundBlurHelp"
            visible: !BlurSupport.ready
            text: qsTr("Enable Glass")
            Layout.leftMargin: 20
            Layout.bottomMargin: 12
            onClicked: dialog.blurHelpRequested()
        }
        Item {
            Layout.fillWidth: true
        }
        PaintButton {
            objectName: "skipUpdateVersion"
            text: qsTr("Skip this version")
            visible: dialog.service.available
            enabled: !dialog.service.busy && !dialog.service.skipped
            Layout.bottomMargin: 12
            onClicked: {
                if (!dialog.service.skipVersion())
                    dialog.browserError = dialog.service.message;
            }
        }
        PaintButton {
            id: doneButton
            objectName: "updatesDone"
            text: qsTr("Done")
            Layout.rightMargin: 12
            Layout.bottomMargin: 12
            onClicked: dialog.close()
        }
    }
    contentItem: ScrollView {
        id: scroll
        clip: true
        contentWidth: availableWidth
        GridLayout {
            width: scroll.availableWidth
            columns: dialog.width < 680 ? 1 : 2
            columnSpacing: 20
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 350
                spacing: 14
                RowLayout {
                    spacing: 10
                    Image {
                        objectName: "applicationIcon"
                        source: Appearance.applicationIcon
                        sourceSize: Qt.size(36, 36)
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        Accessible.name: qsTr("OpenPaint icon")
                    }
                    Label {
                        text: qsTr("OpenPaint %1").arg(AppInfo.version)
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.text
                    }
                }
                Label {
                    objectName: "releaseCheckStatus"
                    text: dialog.service.summary
                    font.pixelSize: 26
                    font.bold: true
                    color: Theme.text
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    text: dialog.service.skipped ? qsTr("Version %1 is skipped. You’ll be notified about newer releases. You can still get this version below.").arg(dialog.service.latestVersion) : dialog.service.available ? qsTr("Version %1 is ready. Get it on GitHub.").arg(dialog.service.latestVersion) : (dialog.service.supported ? qsTr("Updates are checked automatically.") : qsTr("Find the latest release on GitHub."))
                    color: Theme.secondaryText
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                PaintButton {
                    objectName: "viewReleasesButton"
                    text: dialog.service.available ? qsTr("Get update ↗") : qsTr("View releases ↗")
                    onClicked: {
                        if (!(dialog.service.available ? dialog.service.openRelease() : AppInfo.openReleases()))
                            dialog.browserError = qsTr("Open this address in your browser: %1").arg(AppInfo.releaseUrl.toString());
                    }
                }
                Label {
                    text: qsTr("Checks api.github.com. No artwork is sent.")
                    visible: dialog.service.supported
                    color: Theme.secondaryText
                    font.pixelSize: 11
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    objectName: "designerCredit"
                    text: qsTr("Designer: Afshin Kashani, PhD, P.Eng")
                    color: Theme.secondaryText
                    font.pixelSize: 11
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    text: dialog.browserError
                    visible: text.length > 0
                    color: Theme.text
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                }
            }
            AppPreview {
                objectName: "updatesAppPreview"
                corner: true
                angled: true
                glass: Appearance.effectiveGlassEnabled
                Layout.fillWidth: true
                Layout.preferredWidth: 340
                Layout.preferredHeight: 270
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }
}
