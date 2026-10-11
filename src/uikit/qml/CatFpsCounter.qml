// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

CatLabel {
    id: control
    property int frames: 0
    property real elapsed: 0
    property real fps: 0
    text: Math.round(fps) + " FPS"
    FrameAnimation {
        running: control.visible
        onTriggered: {
            control.frames += 1;
            control.elapsed += frameTime;
            if (control.elapsed >= 1) {
                control.fps = control.frames / control.elapsed;
                control.frames = 0;
                control.elapsed = 0;
            }
        }
    }
}
