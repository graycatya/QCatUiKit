// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.MenuBarItem {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight + topPadding
                             + bottomPadding)
    padding: tokens.padding
    topPadding: 6
    bottomPadding: 6
    font.pixelSize: tokens.fontSize
    hoverEnabled: tokens.hoverEnabled
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? control.tokens.text : control.tokens.textDisabled
        verticalAlignment: Text.AlignVCenter
        textFormat: Text.PlainText
    }
    background: Rectangle {
        radius: control.tokens.radius
        color: control.highlighted || control.hovered || control.down ? control.tokens.surfaceHover :
                                                                        "transparent"
        border.width: control.visualFocus ? 2 : 0
        border.color: control.tokens.primary
    }
}
