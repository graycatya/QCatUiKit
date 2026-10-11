// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import QtQuick.Controls.Basic as B

B.SplitView {
    id: control
    property ThemePalette theme: Theme
    implicitWidth: 360
    implicitHeight: 240
    handle: Rectangle {
        implicitWidth: 8
        implicitHeight: 8
        color: B.SplitHandle.pressed ? (control.theme || Theme).tokens.primary :
                                       B.SplitHandle.hovered ? (control.theme
                                                                || Theme).tokens.surfaceHover : (
                                                                   control.theme
                                                                   || Theme).tokens.border
    }
}
