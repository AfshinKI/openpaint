// SPDX-License-Identifier: GPL-3.0-or-later
pragma Singleton

import QtQuick
import OpenPaint

QtObject {
    function coloredIcon(source: url): bool {
        return /\/(pencil|eyedropper|brush|eraser|eraser-paper)\.svg$/.test(source.toString());
    }
    // 0 follows the desktop, 1 is light, 2 is dark. Persisted with appearance preferences.
    readonly property int controlRadius: 6
    readonly property int surfaceRadius: 8
    readonly property int dialogRadius: 12
    readonly property color hover: dark ? "#343B45" : "#E9EDF2"
    readonly property color pressed: dark ? "#46515E" : "#D9E0E8"
    readonly property color selected: Appearance.effectiveGlassEnabled ? (dark ? "#53606C" : "#E3E8EE") : (dark ? "#294C45" : "#D9EEE8")
    readonly property color selectionOutline: dark ? "#D8E2EA" : "#FFFFFF"
    readonly property int mode: Appearance.themeMode
    readonly property bool dark: mode === 2 || (mode === 0 && Application.styleHints.colorScheme === Qt.ColorScheme.Dark)
    readonly property color window: dark ? "#181B20" : "#F5F6F8"
    readonly property color panel: dark ? "#22262D" : "#FFFFFF"
    readonly property color workspace: dark ? "#111418" : "#E2E5EA"
    readonly property color text: dark ? "#F1F4F8" : "#18212B"
    readonly property color secondaryText: dark ? "#ADB8C5" : "#4C5867"
    readonly property color accent: dark ? "#B8C5D2" : "#677887"
    readonly property color accentText: dark ? "#10211D" : "#FFFFFF"
    readonly property color divider: dark ? "#454E5A" : "#C9CED6"
}
