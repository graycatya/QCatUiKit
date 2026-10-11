// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.Label {
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    color: enabled ? tokens.text : tokens.textDisabled
    font.pixelSize: tokens.fontSize
    textFormat: Text.PlainText
}
