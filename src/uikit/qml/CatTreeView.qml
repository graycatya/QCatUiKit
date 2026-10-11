// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.Basic as B
import QCatUiKit

TreeView {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: 320
    implicitHeight: 240
    clip: true
    delegate: B.TreeViewDelegate {
        id: entry
        implicitWidth: 240
        implicitHeight: control.tokens.controlHeight
        palette.windowText: control.tokens.text
        palette.buttonText: control.tokens.text
        palette.highlight: control.tokens.surfaceHover
        palette.highlightedText: control.tokens.text
        palette.base: control.tokens.surface
        font.pixelSize: control.tokens.fontSize
        background: Rectangle {
            color: entry.highlighted ? control.tokens.surfaceHover : control.tokens.surface
            border.width: entry.current ? 1 : 0
            border.color: control.tokens.primary
        }
    }
    T.ScrollBar.vertical: CatScrollBar {
        theme: control.theme
    }
    T.ScrollBar.horizontal: CatScrollBar {
        theme: control.theme
    }
}
