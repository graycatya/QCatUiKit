// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.TextArea {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool invalid: false
    implicitWidth: 320
    implicitHeight: Math.max(120, contentHeight + topPadding + bottomPadding)
    padding: tokens.padding
    color: enabled ? tokens.text : tokens.textDisabled
    font.pixelSize: tokens.fontSize
    placeholderTextColor: enabled ? tokens.textSecondary : tokens.textDisabled
    selectionColor: tokens.primary
    selectedTextColor: tokens.onPrimary
    selectByMouse: true
    wrapMode: TextEdit.Wrap
    hoverEnabled: tokens.hoverEnabled
    Text {
        x: control.leftPadding
        y: control.topPadding
        width: Math.max(0, control.width - control.leftPadding - control.rightPadding)
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        visible: control.length === 0 && control.preeditText.length === 0
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
    }
    background: SurfaceImpl {
        theme: control.theme
        focused: control.enabled && control.activeFocus
        hovered: control.enabled && control.hovered
        invalid: control.invalid
        opacity: control.enabled ? 1 : 0.6
    }
}
