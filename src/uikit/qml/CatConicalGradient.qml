// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes

Shape {
    id: control
    property color startColor: "#1677ff"
    property color endColor: "#ffffff"
    property real angle: 0
    implicitWidth: 100
    implicitHeight: 100
    ShapePath {
        strokeWidth: 0
        fillGradient: ConicalGradient {
            centerX: control.width / 2
            centerY: control.height / 2
            angle: control.angle
            GradientStop {
                position: 0
                color: control.startColor
            }
            GradientStop {
                position: 1
                color: control.endColor
            }
        }
        startX: 0
        startY: 0
        PathLine {
            x: control.width
            y: 0
        }
        PathLine {
            x: control.width
            y: control.height
        }
        PathLine {
            x: 0
            y: control.height
        }
        PathLine {
            x: 0
            y: 0
        }
    }
}
