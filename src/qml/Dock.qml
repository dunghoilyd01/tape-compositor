// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: dockRoot

    required property QtObject compositor

    readonly property int iconSize: compositor && compositor.config ? compositor.config.dockIconSize : 48

    width: Math.max(84, dockRow.implicitWidth + 24)
    height: iconSize + 14
    radius: 8
    // KDE Breeze Dark panel surface
    color: "#e6232629"
    border.width: 1
    border.color: "#3b4045"

    Row {
        id: dockRow
        anchors.centerIn: parent
        spacing: 10

        // App Dashboard launcher button (R7 — full grid in M3)
        Rectangle {
            width: dockRoot.iconSize
            height: dockRoot.iconSize
            radius: 6
            color: "#31363b"
            border.width: 1
            border.color: "#3daee9"

            Text {
                anchors.centerIn: parent
                text: "⊞"
                color: "#fcfcfc"
                font.pixelSize: 20
            }
        }

        // Divider when windows exist
        Rectangle {
            visible: dockRoot.compositor && dockRoot.compositor.windowModel
                ? dockRoot.compositor.windowModel.count > 0
                : false
            width: 1
            height: dockRoot.iconSize - 10
            anchors.verticalCenter: parent.verticalCenter
            color: "#3b4045"
        }

        // Running & minimized windows
        Repeater {
            model: dockRoot.compositor ? dockRoot.compositor.windowModel : null

            delegate: Item {
                id: dockItem
                required property QtObject windowObject
                required property string title
                required property string appId
                required property bool activated
                required property bool minimized

                width: dockRoot.iconSize
                height: dockRoot.iconSize

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: dockItem.activated ? "#31363b" : "#2a2e32"
                    border.width: 1
                    border.color: dockItem.minimized
                        ? "#f67400"
                        : (dockItem.activated ? "#3daee9" : "#3b4045")
                    opacity: dockItem.minimized ? 0.78 : 1.0

                    Text {
                        anchors.centerIn: parent
                        text: dockItem.appId.length > 0 ? dockItem.appId.slice(0, 2).toUpperCase() : "AP"
                        color: "#fcfcfc"
                        font.pixelSize: 14
                        font.weight: Font.Bold
                    }
                }

                // Breeze Plasma Blue active underline bar
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -4
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: dockItem.activated ? 16 : 6
                    height: 3
                    radius: 1.5
                    color: dockItem.activated ? "#3daee9" : "#7f8c8d"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!dockRoot.compositor || !dockRoot.compositor.windowModel || !dockItem.windowObject) {
                            return
                        }
                        if (dockItem.minimized || !dockItem.activated) {
                            dockRoot.compositor.windowModel.activateWindow(dockItem.windowObject)
                        } else {
                            dockRoot.compositor.windowModel.toggleMinimize(dockItem.windowObject)
                        }
                    }
                }
            }
        }
    }
}
