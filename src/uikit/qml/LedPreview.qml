// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

LedPreviewRenderItem {
    id: control
    implicitWidth: 320
    implicitHeight: 320
    clip: true
    function settings(): var { return settingsDialog.snapshot(); }
    function applySettings(settings: var): bool { return settingsDialog.applySettings(settings); }
    function openSettings(): void { settingsDialog.open(); }
    Rectangle { anchors.fill: parent; z: -2; visible: control.backgroundImage.length>0; color: control.backgroundColor }
    Image { anchors.fill: parent; z: -1; source: control.backgroundImage; fillMode: Image.PreserveAspectCrop; asynchronous: true }
    Canvas {
        id: aids
        anchors.fill: parent; visible: control.showGrid || control.showAxes
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx=getContext("2d"); ctx.reset();
            if(control.showGrid) {
                ctx.strokeStyle="rgba(145,167,196,0.18)";ctx.lineWidth=1;
                for(let x=0;x<width;x+=32){ctx.moveTo(x,0);ctx.lineTo(x,height);}
                for(let y=0;y<height;y+=32){ctx.moveTo(0,y);ctx.lineTo(width,y);}ctx.stroke();
            }
            if(control.showAxes) {
                const camera=control.camera(), yaw=(control.viewMode===1?camera.yaw:0)*Math.PI/180;
                const pitch=(control.viewMode===1?camera.pitch:0)*Math.PI/180;
                const vectors=[[Math.cos(yaw),Math.sin(pitch)*Math.sin(yaw)], [0,-Math.cos(pitch)], [Math.sin(yaw),-Math.sin(pitch)*Math.cos(yaw)]];
                const colors=["#ef7e7e","#83d99f","#81b8f1"],labels=["X","Y","Z"];
                for(let i=0;i<3;++i){ctx.beginPath();ctx.lineWidth=2;ctx.strokeStyle=colors[i];ctx.fillStyle=colors[i];ctx.moveTo(45,48);ctx.lineTo(45+vectors[i][0]*26,48+vectors[i][1]*26);ctx.stroke();ctx.fillText(labels[i],48+vectors[i][0]*30,45+vectors[i][1]*30);}
            }
        }
    }
    onCameraChanged: aids.requestPaint()
    onSettingsChanged: aids.requestPaint()
    onViewModeChanged: aids.requestPaint()
    LedPreviewSettings { id: settingsDialog; preview: control }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton | Qt.RightButton
        // Keep camera drags when embedded in a Flickable or a scroll view.
        preventStealing: true
        property point previous
        onPressed: mouse => { previous = Qt.point(mouse.x, mouse.y); if(mouse.button===Qt.RightButton) control.openSettings(); }
        onPositionChanged: mouse => {
            if (pressedButtons & Qt.MiddleButton)
                control.pan(mouse.x - previous.x, mouse.y - previous.y);
            else if (pressedButtons & Qt.LeftButton)
                if(control.viewMode===LedPreviewRenderItem.Plane2D)
                    control.pan(mouse.x - previous.x, mouse.y - previous.y);
                else control.orbit(mouse.x - previous.x, mouse.y - previous.y);
            previous = Qt.point(mouse.x, mouse.y);
        }
        onWheel: wheel => {
            control.zoom(wheel.angleDelta.y / 120);
            wheel.accepted = true;
        }
        onDoubleClicked: control.resetView()
    }
    CatButton {
        anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 6
        text: qsTr("View"); onClicked: control.openSettings()
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
