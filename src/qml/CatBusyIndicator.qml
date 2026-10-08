// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Shapes

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool running: true
    implicitWidth: 24
    implicitHeight: 24
    Accessible.role: Accessible.Indicator
    Accessible.name: qsTr("Loading")
    Shape {
        id: ring
        anchors.fill: parent
        ShapePath {
            fillColor: "transparent"
            strokeColor: control.tokens.primary
            strokeWidth: 3
            capStyle: ShapePath.RoundCap
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: Math.max(0, ring.width / 2 - 2)
                radiusY: Math.max(0, ring.height / 2 - 2)
                startAngle: 0
                sweepAngle: 270
            }
        }
        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: 900
            loops: Animation.Infinite
            running: control.running && control.visible && !(control.theme || Theme).reducedMotion
        }
    }
}
