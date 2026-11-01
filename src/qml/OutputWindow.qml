// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window

Window {
    id: outputRoot

    required property QtObject compositor
    property int outputIndex: 0

    readonly property int topBarHeight: compositor && compositor.config ? compositor.config.topBarHeight : 28
    readonly property int dockReservedHeight: 68

    width: 1280
    height: 800
    color: "#0f1422"

    onClosing: Qt.quit()

    // Wallpaper background
    Rectangle {
        id: wallpaperLayer
        anchors.fill: parent
        z: 0
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: outputRoot.compositor && outputRoot.compositor.config
                    ? outputRoot.compositor.config.wallpaperPrimaryColor
                    : "#1a233a"
            }
            GradientStop {
                position: 1.0
                color: outputRoot.compositor && outputRoot.compositor.config
                    ? outputRoot.compositor.config.wallpaperSecondaryColor
                    : "#0f1422"
            }
        }

        // Empty-desktop helper watermark when no client windows are open
        Column {
            anchors.centerIn: parent
            spacing: 6
            visible: outputRoot.compositor && outputRoot.compositor.windowModel
                ? outputRoot.compositor.windowModel.count === 0
                : true

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Tape Compositor"
                color: "#60ffffff"
                font.pixelSize: 22
                font.weight: Font.DemiBold
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: outputRoot.compositor
                    ? "WAYLAND_DISPLAY=" + outputRoot.compositor.socketNameString + "  •  Super+Shift+D toggles debug log"
                    : ""
                color: "#40ffffff"
                font.pixelSize: 12
                font.family: "monospace"
            }
        }
    }

    // Stacking window layer
    Item {
        id: windowStackLayer
        anchors.fill: parent
        z: 10

        Repeater {
            model: outputRoot.compositor ? outputRoot.compositor.windowModel : null

            delegate: ChromeWindow {
                required property QtObject windowObject
                compositor: outputRoot.compositor
                window: windowObject
                availableX: 0
                availableY: outputRoot.topBarHeight
                availableWidth: outputRoot.width
                availableHeight: outputRoot.height - outputRoot.topBarHeight - outputRoot.dockReservedHeight
            }
        }
    }

    // Top Panel (layer: top, KDE Breeze styled)
    TopBar {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: outputRoot.topBarHeight
        z: 1000
        compositor: outputRoot.compositor
    }

    // Bottom Dock (layer: top, KDE Breeze styled)
    Dock {
        id: dock
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        z: 1000
        compositor: outputRoot.compositor
    }

    // Compositor global shortcuts (PLAN §7.4)
    Shortcut {
        sequence: "Meta+Shift+D"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (outputRoot.compositor) {
                outputRoot.compositor.toggleDebugLogging()
            }
        }
    }

    Shortcut {
        sequence: "Alt+F4"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (outputRoot.compositor && outputRoot.compositor.windowModel) {
                outputRoot.compositor.windowModel.closeActiveWindow()
            }
        }
    }

    Shortcut {
        sequence: "Meta+H"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (outputRoot.compositor && outputRoot.compositor.windowModel) {
                outputRoot.compositor.windowModel.minimizeActiveWindow()
            }
        }
    }

    Shortcut {
        sequence: "Meta+M"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (outputRoot.compositor && outputRoot.compositor.windowModel) {
                outputRoot.compositor.windowModel.maximizeActiveWindow(
                    0,
                    outputRoot.topBarHeight,
                    outputRoot.width,
                    outputRoot.height - outputRoot.topBarHeight - outputRoot.dockReservedHeight
                )
            }
        }
    }
}
