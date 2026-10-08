// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Rectangle {
    id: surface
    required property ThemePalette theme
    readonly property var tokens: (theme || Theme).tokens
    property bool focused: false
    property bool hovered: false
    property bool invalid: false
    color: hovered ? tokens.surfaceHover : tokens.surface
    radius: tokens.radius
    border.width: focused ? 2 : 1
    border.color: invalid ? tokens.danger : focused ? tokens.primary : tokens.border
    Behavior on color {
        ColorAnimation {
            duration: surface.tokens.motionDuration
        }
    }
}
