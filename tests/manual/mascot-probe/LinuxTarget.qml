// SPDX-License-Identifier: Apache-2.0
import QtQuick
Window {
    width: 760
    height: 620
    visible: true
    color: "#265d83"
    title: "QCat Linux probe independent input target"
    flags: Qt.Window | Qt.FramelessWindowHint
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20
        text: qsTr("Independent input target — blue background")
        color: "white"
        font.pixelSize: 20
    }
}
