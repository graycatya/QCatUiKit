// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property alias source: picture.source
    property alias sourceSize: picture.sourceSize
    readonly property bool motionEnabled: !(theme || Theme).reducedMotion
    property bool running: true
    property int duration: 1200
    property real fromScale: 1
    property real toScale: 1
    property real fromOpacity: 1
    property real toOpacity: 1
    property point fromOffset: Qt.point(0, 0)
    property point toOffset: Qt.point(0, 0)
    property real phase: 0
    implicitWidth: 120
    implicitHeight: 120
    CatImage {
        id: picture
        theme: control.theme
        width: parent.width
        height: parent.height
        x: control.motionEnabled ? control.fromOffset.x + (control.toOffset.x - control.fromOffset.x) * control.phase : 0
        y: control.motionEnabled ? control.fromOffset.y + (control.toOffset.y - control.fromOffset.y) * control.phase : 0
        scale: control.motionEnabled ? control.fromScale + (control.toScale - control.fromScale) * control.phase : 1
        opacity: control.motionEnabled ? control.fromOpacity + (control.toOpacity - control.fromOpacity) * control.phase : 1
    }
    SequentialAnimation on phase {
        running: control.running && control.visible && control.motionEnabled
        loops: Animation.Infinite
        NumberAnimation {
            from: 0
            to: 1
            duration: Math.max(1, control.duration)
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            from: 1
            to: 0
            duration: Math.max(1, control.duration)
            easing.type: Easing.InOutQuad
        }
    }
}
