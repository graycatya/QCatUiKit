// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Rectangle {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property Item target
    property real minimumWidth: 80
    property real minimumHeight: 60
    property real maximumWidth: 4096
    property real maximumHeight: 4096
    property size initialSize
    implicitWidth: tokens.controlHeight
    implicitHeight: tokens.controlHeight
    color: tokens.surfaceHover
    radius: tokens.radius
    CatLabel {
        theme: control.theme
        anchors.centerIn: parent
        text: "◢"
    }
    DragHandler {
        target: null
        onActiveChanged: if (active && control.target)
            control.initialSize = Qt.size(control.target.width, control.target.height)
        onActiveTranslationChanged: if (active && control.target) {
            control.target.width = Math.max(control.minimumWidth, Math.min(control.maximumWidth, control.initialSize.width + activeTranslation.x));
            control.target.height = Math.max(control.minimumHeight, Math.min(control.maximumHeight, control.initialSize.height + activeTranslation.y));
        }
    }
    HoverHandler {
        cursorShape: Qt.SizeFDiagCursor
    }
}
