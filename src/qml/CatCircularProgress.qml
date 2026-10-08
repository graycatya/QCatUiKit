// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Shapes

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property real from: 0
    property real to: 100
    property real value: 0
    property real strokeWidth: 6
    readonly property real position: to === from ? 0 : Math.max(0, Math.min(1, (value - from) / (to - from)))
    implicitWidth: 96
    implicitHeight: 96
    Accessible.role: Accessible.ProgressBar
    Accessible.name: Math.round(position * 100) + "%"
    Shape {
        anchors.fill: parent
        ShapePath {
            fillColor: "transparent"
            strokeColor: control.tokens.disabled
            strokeWidth: control.strokeWidth
            PathAngleArc {
                centerX: control.width / 2
                centerY: control.height / 2
                radiusX: (control.width - control.strokeWidth) / 2
                radiusY: (control.height - control.strokeWidth) / 2
                startAngle: -90
                sweepAngle: 360
            }
        }
        ShapePath {
            fillColor: "transparent"
            strokeColor: control.tokens.primary
            strokeWidth: control.strokeWidth
            capStyle: ShapePath.RoundCap
            PathAngleArc {
                centerX: control.width / 2
                centerY: control.height / 2
                radiusX: (control.width - control.strokeWidth) / 2
                radiusY: (control.height - control.strokeWidth) / 2
                startAngle: -90
                sweepAngle: 360 * control.position
            }
        }
    }
    CatLabel {
        theme: control.theme
        anchors.centerIn: parent
        text: Math.round(control.position * 100) + "%"
    }
}
