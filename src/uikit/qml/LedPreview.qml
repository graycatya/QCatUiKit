// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

LedPreviewRenderItem {
    id: control
    implicitWidth: 320
    implicitHeight: 320
    clip: true
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        // Keep camera drags when embedded in a Flickable or a scroll view.
        preventStealing: true
        property point previous
        onPressed: mouse => previous = Qt.point(mouse.x, mouse.y)
        onPositionChanged: mouse => {
            if (pressedButtons & Qt.MiddleButton)
                control.pan(mouse.x - previous.x, mouse.y - previous.y);
            else if (pressedButtons & Qt.LeftButton)
                control.orbit(mouse.x - previous.x, mouse.y - previous.y);
            previous = Qt.point(mouse.x, mouse.y);
        }
        onWheel: wheel => {
            control.zoom(wheel.angleDelta.y / 120);
            wheel.accepted = true;
        }
        onDoubleClicked: control.resetView()
    }
    Text {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 8
        width: Math.max(0, parent.width - 16)
        visible: control.showStatistics
        color: "#d0d9e8"
        font.pixelSize: 11
        wrapMode: Text.Wrap
        text: qsTr("%1 LEDs · color %2 Hz · render %3 FPS\n%4")
              .arg(control.source ? control.source.pointCount : 0)
              .arg(control.colorUpdateHz.toFixed(1)).arg(control.renderFps.toFixed(1))
              .arg(control.renderingInfo)
    }
    Text {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - 24)
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        color: "#e4b0ad"
        visible: text.length > 0
        text: control.renderError || (control.source ? control.source.lastError : "")
    }
}
