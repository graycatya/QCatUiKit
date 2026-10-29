// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

Window {
    id: pet
    required property var probe
    width: 420
    height: 320
    color: "transparent"
    title: "QCat Linux prototype overlay"
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus
    property bool bubbleVisible: true
    Item {
        x: 120; y: 110; width: 180; height: 170
        Shape {
            anchors.fill: parent
            ShapePath {
                fillColor: "#e6a64b"
                strokeColor: "transparent"
                PathAngleArc {
                    centerX: 90; centerY: 85
                    radiusX: 90; radiusY: 85
                    startAngle: 0; sweepAngle: 360
                }
            }
        }
        Label {
            anchors.centerIn: parent
            text: qsTr("Drag / click\nLinux prototype body")
            horizontalAlignment: Text.AlignHCenter
            color: "#18212b"
        }
        MouseArea {
            anchors.fill: parent
            onPressed: pet.probe.bodyPressed()
            onReleased: pet.probe.bodyReleased()
            onCanceled: pet.probe.bodyReleased()
        }
    }
    Rectangle {
        x: 60; y: 20; width: 300; height: 60
        visible: pet.bubbleVisible
        color: "#26394d"
        Button {
            id: first
            objectName: "linuxBubbleFirst"
            x: 10; y: 10; width: 135; height: 40
            text: qsTr("Focus action")
            activeFocusOnTab: true
            onClicked: {
                pet.probe.bubbleActivated()
                first.forceActiveFocus()
            }
        }
        Button {
            x: 155; y: 10; width: 135; height: 40
            text: qsTr("Tab / Space")
            activeFocusOnTab: true
            onClicked: pet.probe.bubbleActivated()
        }
    }
}
