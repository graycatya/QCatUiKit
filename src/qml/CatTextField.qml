// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.TextField {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool invalid: false
    implicitWidth: 220
    implicitHeight: Math.max(tokens.controlHeight, contentHeight + 12)
    leftPadding: tokens.padding
    rightPadding: tokens.padding
    topPadding: 6
    bottomPadding: 6
    color: enabled ? tokens.text : tokens.textDisabled
    placeholderTextColor: tokens.textSecondary
    selectionColor: tokens.primary
    selectedTextColor: tokens.onPrimary
    font.pixelSize: tokens.fontSize
    selectByMouse: true
    hoverEnabled: tokens.hoverEnabled
    Text {
        x: control.leftPadding
        y: control.topPadding
        width: control.width - control.leftPadding - control.rightPadding
        height: control.height - control.topPadding - control.bottomPadding
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        visible: control.length === 0 && control.preeditText.length === 0
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: control.effectiveHorizontalAlignment
        elide: Text.ElideRight
        textFormat: Text.PlainText
    }
    background: SurfaceImpl {
        theme: control.theme
        focused: control.activeFocus
        hovered: control.hovered
        invalid: control.invalid
        opacity: control.enabled ? 1 : 0.6
    }
}
