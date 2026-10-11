// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.ToolTip {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    delay: 600
    timeout: 3000
    padding: tokens.spacing
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    width: Math.min(implicitWidth, 360)
    contentItem: CatLabel {
        theme: control.theme
        text: control.text
        wrapMode: Text.Wrap
    }
    background: SurfaceImpl {
        theme: control.theme
    }
}
