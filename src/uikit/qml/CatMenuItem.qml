// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.MenuItem {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: Math.max(180, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight + topPadding
                             + bottomPadding)
    padding: tokens.padding
    topPadding: 6
    bottomPadding: 6
    leftPadding: padding + (!mirrored && checkable ? 24 : 0)
    rightPadding: padding + (mirrored && checkable ? 24 : subMenu ? 20 : 0)
    font.pixelSize: tokens.fontSize
    hoverEnabled: tokens.hoverEnabled
    Accessible.name: text
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? control.tokens.text : control.tokens.textDisabled
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: control.mirrored ? Text.AlignRight : Text.AlignLeft
        elide: Text.ElideRight
    }
    indicator: CatFontIcon {
        theme: control.theme
        glyph: "✓"
        visible: control.checkable && control.checked
        x: control.mirrored ? control.width - width - control.padding : control.padding
        anchors.verticalCenter: parent.verticalCenter
        width: 20
        height: 20
        color: control.tokens.primary
    }
    arrow: CatFontIcon {
        theme: control.theme
        glyph: control.mirrored ? "‹" : "›"
        visible: control.subMenu !== null
        x: control.mirrored ? control.padding : control.width - width - control.padding
        anchors.verticalCenter: parent.verticalCenter
        width: 16
        height: 20
    }
    background: Rectangle {
        radius: Math.min(4, control.tokens.radius)
        color: control.highlighted || control.hovered || control.down ? control.tokens.surfaceHover :
                                                                        control.tokens.surface
        border.width: control.visualFocus ? 2 : 0
        border.color: control.tokens.primary
    }
}
