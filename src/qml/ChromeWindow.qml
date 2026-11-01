// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound

import QtQuick
import QtWayland.Compositor

Item {
    id: chromeRoot

    required property QtObject compositor
    required property QtObject window
    property real availableX: 0
    property real availableY: 28
    property real availableWidth: 1280
    property real availableHeight: 700

    readonly property bool isServerDecorated: window ? window.serverDecorated : true
    readonly property int titleBarHeight: isServerDecorated ? 30 : 0
    readonly property bool isActive: window ? window.activated : false

    x: window ? window.x : 100
    y: window ? window.y : 80
    width: surfaceItem.implicitWidth > 0 ? surfaceItem.implicitWidth : (window ? window.width : 640)
    height: (surfaceItem.implicitHeight > 0 ? surfaceItem.implicitHeight : (window ? window.height : 420)) + titleBarHeight
    z: window ? window.zOrder : 0
    visible: window ? !window.minimized : false

    // KDE Breeze window frame & active accent border
    Rectangle {
        anchors.fill: parent
        anchors.margins: -1
        radius: chromeRoot.isServerDecorated ? 5 : 0
        color: "#232629"
        border.width: 1
        border.color: chromeRoot.isActive ? "#3daee9" : "#3b4045"
    }

    // KDE Breeze Server-Side Titlebar
    Rectangle {
        id: titleBar
        visible: chromeRoot.isServerDecorated
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: chromeRoot.titleBarHeight
        radius: 4
        color: chromeRoot.isActive ? "#2a2e32" : "#202326"

        // Square bottom corners + Breeze separator line
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 4
            color: parent.color
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: chromeRoot.isActive ? "#3daee9" : "#31363b"
            opacity: chromeRoot.isActive ? 0.55 : 1.0
        }

        // Drag to move window + raise/activate on press
        MouseArea {
            id: titleDragArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            property point pressOffset: Qt.point(0, 0)

            onPressed: function(mouse) {
                if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                    chromeRoot.compositor.windowModel.activateWindow(chromeRoot.window)
                }
                pressOffset = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: function(mouse) {
                if (pressed && chromeRoot.window && !chromeRoot.window.maximized) {
                    chromeRoot.window.x = chromeRoot.x + mouse.x - pressOffset.x
                    chromeRoot.window.y = Math.max(chromeRoot.availableY, chromeRoot.y + mouse.y - pressOffset.y)
                }
            }

            onDoubleClicked: {
                if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                    chromeRoot.compositor.windowModel.toggleMaximize(
                        chromeRoot.window,
                        chromeRoot.availableX,
                        chromeRoot.availableY,
                        chromeRoot.availableWidth,
                        chromeRoot.availableHeight
                    )
                }
            }
        }

        // Left side: Breeze application badge + window title
        Row {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Rectangle {
                width: 16
                height: 16
                radius: 3
                anchors.verticalCenter: parent.verticalCenter
                color: chromeRoot.isActive ? "#3daee9" : "#4d545e"

                Text {
                    anchors.centerIn: parent
                    text: chromeRoot.window && chromeRoot.window.appId.length > 0
                        ? chromeRoot.window.appId.slice(0, 1).toUpperCase()
                        : "W"
                    color: "#fcfcfc"
                    font.pixelSize: 10
                    font.weight: Font.Bold
                }
            }

            Text {
                width: Math.max(0, titleBar.width - 140)
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                text: chromeRoot.window ? chromeRoot.window.title : ""
                color: chromeRoot.isActive ? "#fcfcfc" : "#7f8c8d"
                font.pixelSize: 12
                font.weight: Font.Medium
            }
        }

        // Right side: KDE Breeze window control buttons (Minimize, Maximize, Close)
        Row {
            id: breezeButtons
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // Minimize (Breeze chevron/dash)
            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: minArea.containsMouse ? "#3b4045" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "─"
                    color: chromeRoot.isActive ? "#fcfcfc" : "#7f8c8d"
                    font.pixelSize: 11
                }

                MouseArea {
                    id: minArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                            chromeRoot.compositor.windowModel.toggleMinimize(chromeRoot.window)
                        }
                    }
                }
            }

            // Maximize / Restore (Breeze square/diamond)
            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: maxArea.containsMouse ? "#3b4045" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: (chromeRoot.window && chromeRoot.window.maximized) ? "❐" : "□"
                    color: chromeRoot.isActive ? "#fcfcfc" : "#7f8c8d"
                    font.pixelSize: 11
                }

                MouseArea {
                    id: maxArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                            chromeRoot.compositor.windowModel.toggleMaximize(
                                chromeRoot.window,
                                chromeRoot.availableX,
                                chromeRoot.availableY,
                                chromeRoot.availableWidth,
                                chromeRoot.availableHeight
                            )
                        }
                    }
                }
            }

            // Close (Breeze crimson circle on hover)
            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: closeArea.containsMouse ? "#da4453" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: closeArea.containsMouse
                        ? "#fcfcfc"
                        : (chromeRoot.isActive ? "#fcfcfc" : "#7f8c8d")
                    font.pixelSize: 11
                }

                MouseArea {
                    id: closeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                            chromeRoot.compositor.windowModel.closeWindow(chromeRoot.window)
                        }
                    }
                }
            }
        }
    }

    // Wayland client surface item
    ShellSurfaceItem {
        id: surfaceItem
        anchors.top: chromeRoot.isServerDecorated ? titleBar.bottom : parent.top
        anchors.left: parent.left
        shellSurface: chromeRoot.window ? chromeRoot.window.shellSurface : null
        moveItem: chromeRoot
        autoCreatePopupItems: true

        onSurfaceDestroyed: {
            if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                chromeRoot.compositor.windowModel.removeWindow(chromeRoot.window)
            }
        }

        TapHandler {
            gesturePolicy: TapHandler.ReleaseWithinBounds
            onTapped: {
                if (chromeRoot.compositor && chromeRoot.compositor.windowModel && chromeRoot.window) {
                    chromeRoot.compositor.windowModel.activateWindow(chromeRoot.window)
                }
            }
        }
    }
}
