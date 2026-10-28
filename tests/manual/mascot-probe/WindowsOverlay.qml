// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls

Window {
    id: pet
    required property var probe
    width: 420
    height: 320
    color: "transparent"
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    title: "QCat mascot Windows prototype overlay"
    property bool bubbleVisible: true
    Rectangle {
        x: 120
        y: 110
        width: 180
        height: 170
        color: "#e6a64b"
        radius: 22
        Label {
            anchors.centerIn: parent
            text: qsTr("Drag / click\nprototype body\n%1 clicks").arg(pet.probe.bodyClicks)
            horizontalAlignment: Text.AlignHCenter
            color: "#18212b"
        }
        MouseArea {
            anchors.fill: parent
            onPressed: pet.probe.bodyPressed()
            onPositionChanged: if (pressed)
                pet.probe.bodyMoved()
            onReleased: pet.probe.bodyReleased()
        }
    }
    Rectangle {
        x: 60
        y: 20
        width: 300
        height: 60
        visible: pet.bubbleVisible
        color: "#26394d"
        radius: 12
        Button {
            id: focusActionButton
            objectName: "prototypeBubbleButton"
            x: 10
            y: 10
            width: 135
            height: 40
            text: qsTr("Focus action")
            activeFocusOnTab: true
            onClicked: {
                pet.probe.focusBubble();
                pet.probe.bubbleActivated();
                focusActionButton.forceActiveFocus();
            }
            background: Rectangle {
                color: focusActionButton.activeFocus ? "#b5d8f4" : "#edf2f7"
                radius: 5
            }
        }
        Button {
            id: confirm
            objectName: "prototypeConfirmButton"
            x: 155
            y: 10
            width: 135
            height: 40
            text: qsTr("Tab / Space")
            activeFocusOnTab: true
            onClicked: {
                pet.probe.focusBubble();
                pet.probe.bubbleActivated();
            }
            background: Rectangle {
                color: confirm.activeFocus ? "#b5d8f4" : "#edf2f7"
                radius: 5
            }
        }
    }
    Label {
        x: 126
        y: 244
        text: qsTr("Native desktop probe")
        color: "#18212b"
    }
}
