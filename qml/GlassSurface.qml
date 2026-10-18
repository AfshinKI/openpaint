// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Effects
import OpenPaint

Rectangle {
    id: surface
    // Explicit lower layers exclude floating panels and popup content: no feedback loop.
    readonly property var hostWindow: Window.window
    property Item backdropSource: hostWindow ? hostWindow.canvasController : null
    property bool opaqueBackdrop: false
    property bool menuSurface: false
    property bool raisedSurface: true
    property real baseTintPower: 2
    property list<Item> backdropLayers: !backdropSource ? [] : hostWindow && backdropSource === hostWindow.canvasController ? hostWindow.glassBackdropLayers : [backdropSource]
    readonly property bool backdropBlurEnabled: Appearance.glassEnabled && surfaceAlpha < 1 && hostWindow !== null && backdropSource !== null && GraphicsInfo.api !== GraphicsInfo.Software
    readonly property real surfaceAlpha: Appearance.glassEnabled && !(opaqueBackdrop && GraphicsInfo.api === GraphicsInfo.Software) ? Appearance.surfaceOpacity / 100 : 1
    // Fixed bars already sit on a tint. Over artwork, retain enough light material
    // to keep dark labels readable even when the blurred source is black.
    readonly property real lightPopupTint: menuSurface ? Math.pow(surfaceAlpha, 5) : 0.18 + 0.82 * surfaceAlpha * surfaceAlpha * surfaceAlpha
    readonly property real tintAlpha: Theme.dark ? (opaqueBackdrop ? Math.max(0.72, surfaceAlpha) : surfaceAlpha) : opaqueBackdrop ? lightPopupTint : raisedSurface ? 0.45 + 0.55 * surfaceAlpha * surfaceAlpha : Math.pow(surfaceAlpha, baseTintPower)
    radius: Theme.surfaceRadius
    color: Theme.panel
    border.color: Appearance.glassEnabled ? (Theme.dark ? "#687887" : "#383D454D") : Theme.divider
    gradient: Appearance.glassEnabled ? sheen : null
    Loader {
        anchors.fill: parent
        z: -1
        active: surface.visible && surface.backdropBlurEnabled
        sourceComponent: Item {
            Item {
                id: backdrop
                anchors.fill: parent
                visible: false
                Rectangle {
                    anchors.fill: parent
                    color: Theme.workspace
                    visible: surface.opaqueBackdrop
                }
                Repeater {
                    model: surface.backdropLayers
                    ShaderEffectSource {
                        required property Item modelData
                        anchors.fill: parent
                        sourceItem: modelData
                        sourceRect: {
                            if (!surface.hostWindow || !surface.parent || !modelData)
                                return Qt.rect(0, 0, 1, 1);
                            // Track panel and source geometry when popups open or resize.
                            const layout = [surface.x, surface.y, surface.parent.x, surface.parent.y, surface.hostWindow.width, surface.hostWindow.height, modelData.x, modelData.y, modelData.width, modelData.height];
                            const point = surface.mapToItem(modelData, 0, 0);
                            return Qt.rect(point.x, point.y, surface.width, surface.height);
                        }
                        textureSize: Qt.size(Math.max(1, surface.width / 2), Math.max(1, surface.height / 2))
                        live: true
                        recursive: false
                    }
                }
            }
            Rectangle {
                id: roundedMask
                anchors.fill: parent
                radius: surface.radius
                color: "white"
                layer.enabled: true
                visible: false
            }
            MultiEffect {
                anchors.fill: parent
                source: backdrop
                blurEnabled: true
                blurMax: 64
                blur: 1
                blurMultiplier: !Theme.dark && surface.opaqueBackdrop ? 0.75 : 0
                autoPaddingEnabled: false
                maskEnabled: surface.radius > 0
                maskSource: roundedMask
            }
        }
    }
    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: Math.max(0, surface.radius - 1)
        color: "transparent"
        border.color: Qt.rgba(1, 1, 1, 0.65 * (1 - surface.surfaceAlpha))
        visible: Appearance.glassEnabled && !Theme.dark && surface.surfaceAlpha < 1 && surface.border.width > 0
    }
    Gradient {
        id: sheen
        GradientStop {
            position: 0
            color: Qt.rgba(Theme.dark ? 0.23 : 1, Theme.dark ? 0.28 : 1, Theme.dark ? 0.33 : 1, surface.tintAlpha)
        }
        GradientStop {
            position: 0.48
            color: Qt.rgba(Theme.panel.r, Theme.panel.g, Theme.panel.b, surface.tintAlpha)
        }
        GradientStop {
            position: 1
            color: Qt.rgba(Theme.dark ? 0.10 : 0.94, Theme.dark ? 0.14 : 0.94, Theme.dark ? 0.18 : 0.94, surface.tintAlpha)
        }
    }
}
