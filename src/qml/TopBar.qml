// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: topBarRoot

    required property QtObject compositor

    readonly property bool debugOn: compositor && compositor.logManager
        ? compositor.logManager.debugEnabled
        : false

    // KDE Breeze Dark panel background
    color: "#e6232629"

    // Breeze Plasma Blue bottom border accent
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#31363b"
    }

    // Left section: App launcher trigger + active application title + Global Menu bar
    Row {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 14

        // Tape Launcher / System Menu button (Breeze Plasma Blue accent)
        Rectangle {
            width: 20
            height: 20
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: "#3daee9"

            Text {
                anchors.centerIn: parent
                text: "T"
                color: "#fcfcfc"
                font.pixelSize: 11
                font.weight: Font.Bold
            }
        }

        // Focused application name
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: topBarRoot.compositor && topBarRoot.compositor.windowModel
                ? topBarRoot.compositor.windowModel.activeAppName
                : "Tape"
            color: "#fcfcfc"
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        // Global menu bar slot (placeholder until M9)
        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12
            Repeater {
                model: ["File", "Edit", "View", "Window", "Help"]
                delegate: Text {
                    required property string modelData
                    text: modelData
                    color: "#bdc3c7"
                    font.pixelSize: 12
                }
            }
        }
    }

    // Right section: Debug toggle badge, IM indicator placeholder, window count, clock
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        // Interactive Debug Log Toggle Pill (R5, Breeze Positive Green / Neutral)
        Rectangle {
            width: dbgLabel.implicitWidth + 14
            height: 18
            radius: 4
            color: topBarRoot.debugOn ? "#27ae60" : "#31363b"
            border.width: 1
            border.color: topBarRoot.debugOn ? "#2ecc71" : "#4d545e"

            Text {
                id: dbgLabel
                anchors.centerIn: parent
                text: topBarRoot.debugOn ? "DBG ON" : "DBG OFF"
                color: "#fcfcfc"
                font.pixelSize: 10
                font.weight: Font.Bold
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (topBarRoot.compositor) {
                        topBarRoot.compositor.toggleDebugLogging()
                    }
                }
            }
        }

        // Input method indicator placeholder (fcitx5 in M6)
        Text {
            text: "EN"
            color: "#fcfcfc"
            font.pixelSize: 11
            font.weight: Font.Medium
        }

        // Active client window counter
        Text {
            text: topBarRoot.compositor && topBarRoot.compositor.windowModel
                ? topBarRoot.compositor.windowModel.count + " win"
                : "0 win"
            color: "#7f8c8d"
            font.pixelSize: 11
        }

        // Clock
        Text {
            id: clockText
            color: "#fcfcfc"
            font.pixelSize: 12
            font.weight: Font.Medium
            text: Qt.formatDateTime(new Date(), "ddd HH:mm")

            Timer {
                interval: 15000
                running: true
                repeat: true
                onTriggered: clockText.text = Qt.formatDateTime(new Date(), "ddd HH:mm")
            }
        }
    }
}
