// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import OpenPaint

ScreenshotPreview {
    objectName: "appearancePreviewImage"
    implicitHeight: 170
    Accessible.role: Accessible.Graphic
    Accessible.name: corner ? (glass ? qsTr("OpenPaint corner with glass appearance") : qsTr("OpenPaint corner with opaque appearance")) : qsTr("OpenPaint with a generated mountain painting")
}
