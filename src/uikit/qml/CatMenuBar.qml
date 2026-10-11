// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import QtQuick.Controls.Basic as B

B.MenuBar {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    delegate: CatMenuBarItem {
        theme: control.theme
    }
    background: Rectangle {
        color: control.tokens.surface
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: control.tokens.border
        }
    }
}
