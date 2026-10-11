// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Flipable {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool flipped: false
    implicitWidth: 160
    implicitHeight: 120
    transform: Rotation {
        origin.x: control.width / 2
        origin.y: control.height / 2
        axis.x: 0
        axis.y: 1
        axis.z: 0
        angle: control.flipped ? 180 : 0
        Behavior on angle {
            NumberAnimation {
                duration: control.tokens.motionDuration
            }
        }
    }
}
