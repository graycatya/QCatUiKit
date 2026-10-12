// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import QCatUiKit

Item {
    id: control
    property string name: "grid"
    property color color: Theme.tokens.textSecondary
    implicitWidth: 20
    implicitHeight: 20
    readonly property var paths: ({
            home: "M3 10 L12 3 L21 10 M5 9 L5 21 L10 21 L10 14 L14 14 L14 21 L19 21 L19 9",
            grid: "M3 3 L9 3 L9 9 L3 9 Z M15 3 L21 3 L21 9 L15 9 Z M3 15 L9 15 L9 21 L3 21 Z M15 15 L21 15 L21 21 L15 21 Z",
            input: "M4 5 L20 5 L20 19 L4 19 Z M8 9 L8 15 M11 12 L16 12",
            feedback: "M12 3 A9 9 0 1 1 11.99 3 M12 7 L12 13 M12 16 L12 17",
            navigation: "M3 6 L21 6 M3 12 L16 12 M3 18 L11 18 M18 14 L22 18 L18 22",
            data: "M3 4 L21 4 L21 20 L3 20 Z M3 10 L21 10 M10 4 L10 20 M3 15 L21 15",
            motion: "M4 8 A9 9 0 0 1 20 8 M4 8 L4 3 M4 8 L9 8 M20 16 A9 9 0 0 1 4 16 M20 16 L20 21 M20 16 L15 16",
            media: "M3 4 L21 4 L21 20 L3 20 Z M3 17 L9 11 L14 16 L17 12 L21 16 M15 8 L17 8",
            tools: "M4 4 L10 4 L10 10 L4 10 Z M14 14 L20 14 L20 20 L14 20 Z M10 7 L20 7 L20 11 M7 10 L7 20 L11 20",
            search: "M10 3 A7 7 0 1 1 9.99 3 M15 15 L21 21",
            sun: "M12 7 A5 5 0 1 1 11.99 7 M12 1 L12 3 M12 21 L12 23 M1 12 L3 12 M21 12 L23 12 M4 4 L6 6 M18 18 L20 20 M4 20 L6 18 M18 6 L20 4",
            moon: "M20 15 A9 9 0 1 1 9 3 A8 8 0 0 0 20 15 Z",
            menu: "M4 6 L20 6 M4 12 L20 12 M4 18 L20 18",
            arrow: "M4 12 L20 12 M14 6 L20 12 L14 18",
            code: "M8 6 L2 12 L8 18 M16 6 L22 12 L16 18 M14 3 L10 21",
            settings: "M12 3 A9 9 0 1 1 11.99 3 M12 8 A4 4 0 1 1 11.99 8",
            close: "M6 6 L18 18 M6 18 L18 6",
            chevron: "M6 9 L12 15 L18 9",
            cat: "M3 10 L3 3 L9 7 Q12 5 15 7 L21 3 L21 10 Q23 21 12 21 Q1 21 3 10 Z M7 12 L9 12 M15 12 L17 12 M10 16 L12 18 L14 16"
        })
    Shape {
        width: 24
        height: 24
        anchors.centerIn: parent
        scale: Math.min(control.width, control.height) / 24
        ShapePath {
            fillColor: "transparent"
            strokeColor: control.color
            strokeWidth: 1.65
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathSvg {
                path: control.paths[control.name] || control.paths.grid
            }
        }
    }
}
