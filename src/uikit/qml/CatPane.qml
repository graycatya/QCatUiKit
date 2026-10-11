// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.Pane {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    padding: tokens.padding
    implicitWidth: Math.max(implicitBackgroundWidth, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight, contentHeight + topPadding + bottomPadding)
    background: SurfaceImpl {
        theme: control.theme
    }
}
