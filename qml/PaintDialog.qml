// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import OpenPaint

Dialog {
    id: control
    focus: true
    header: Label {
        text: control.title
        visible: control.title.length > 0
        leftPadding: 20
        rightPadding: 20
        topPadding: 16
        bottomPadding: 8
        font.pixelSize: 16
        font.bold: true
        color: Theme.text
        wrapMode: Text.WordWrap
    }
    padding: 20
    background: GlassSurface {
        radius: Theme.dialogRadius
    }
    footer: DialogButtonBox {
        padding: 12
        spacing: 8
        background: Item {}
        delegate: PaintButton {}
    }
}
