// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Rectangle {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool active: true
    property int cycleDuration: 1400
    property real shimmerProgress: 0
    implicitWidth: 240
    implicitHeight: 24
    radius: tokens.radius
    color: tokens.surfaceHover
    clip: true
    Rectangle {
        width: parent.width / 2
        height: parent.height
        x: -width + (control.width + width) * control.shimmerProgress
        visible: control.active && !(control.theme || Theme).reducedMotion
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: control.tokens.surfaceHover
            }
            GradientStop {
                position: 0.5
                color: control.tokens.surface
            }
            GradientStop {
                position: 1
                color: control.tokens.surfaceHover
            }
        }
    }
    FrameAnimation {
        id: animation
        running: control.active && control.visible && control.enabled && !(control.theme
                                                                           || Theme).reducedMotion

        onTriggered: control.shimmerProgress = (control.shimmerProgress + animation.frameTime
                                                * 1000 / Math.max(1, control.cycleDuration)) % 1
    }
}
