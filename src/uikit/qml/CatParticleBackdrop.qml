// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Rectangle {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool animationEnabled: true
    property int particleCount: 24
    property real speed: 1
    property real phase: 0
    implicitWidth: 320
    implicitHeight: 200
    color: tokens.background
    clip: true
    Repeater {
        model: Math.max(0, Math.min(500, control.particleCount))
        Rectangle {
            required property int index
            width: 3 + index % 5
            height: width
            radius: width / 2
            antialiasing: true
            color: control.tokens.primary
            opacity: 0.15 + (index % 4) * 0.1
            x: (index * 0.61803398875 % 1) * Math.max(0, control.width - width) + Math.sin(
                   control.phase * Math.PI / 50 + index) * 8
            y: (index * 0.41421356237 + control.phase * (0.02 + index % 3 * 0.01)) % 1
               * control.height

        }
    }
    FrameAnimation {
        id: animation
        running: control.animationEnabled && control.visible && control.enabled && !(control.theme
                                                                                     || Theme).reducedMotion
        onTriggered: control.phase = (control.phase + animation.frameTime * control.speed) % 100
    }
}
