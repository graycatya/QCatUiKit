// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.ScrollBar {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: horizontal ? 160 : 12
    implicitHeight: horizontal ? 12 : 160
    padding: 3
    minimumSize: 0.05
    visible: policy === T.ScrollBar.AlwaysOn || (policy === T.ScrollBar.AsNeeded && size < 1)
    hoverEnabled: tokens.hoverEnabled
    contentItem: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        radius: 3
        color: control.pressed ? control.tokens.primary : control.tokens.textSecondary
        opacity: control.policy === T.ScrollBar.AlwaysOn || control.active || control.hovered ? 0.8 :
                                                                                                0.3
        Behavior on opacity {
            NumberAnimation {
                duration: control.tokens.motionDuration
            }
        }
    }
    background: Rectangle {
        color: control.hovered ? control.tokens.surfaceHover : "transparent"
        radius: 6
    }
}
