// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.ScrollBar {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    // Views reveal idle overlay scroll bars only while the pointer is inside.
    property bool revealed: true
    readonly property bool engaged: pressed || active || hovered
    readonly property bool shown: policy === T.ScrollBar.AlwaysOn || revealed || engaged
    implicitWidth: horizontal ? 160 : 12
    implicitHeight: horizontal ? 12 : 160
    padding: 3
    minimumSize: 0.05
    visible: policy === T.ScrollBar.AlwaysOn || (policy === T.ScrollBar.AsNeeded && size < 1)
    opacity: shown ? 1 : 0
    hoverEnabled: tokens.hoverEnabled
    Behavior on opacity {
        NumberAnimation {
            duration: control.tokens.motionDuration
            easing.type: Easing.OutCubic
        }
    }
    contentItem: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        radius: 3
        color: control.pressed ? control.tokens.primary : control.tokens.textSecondary
        opacity: control.pressed ? 1 : control.hovered ? 0.9 : 0.35
    }
    background: Rectangle {
        color: control.hovered || control.pressed ? control.tokens.surfaceHover : "transparent"
        radius: 6
    }
}
