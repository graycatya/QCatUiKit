// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Window

B.ApplicationWindow {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool frameless: false
    width: 960
    height: 640
    minimumWidth: 320
    minimumHeight: 240
    color: tokens.background
    flags: frameless ? Qt.Window | Qt.FramelessWindowHint : Qt.Window
    font.pixelSize: tokens.fontSize
    palette.text: tokens.text
    palette.windowText: tokens.text
    palette.buttonText: tokens.text
    palette.base: tokens.surface
    palette.window: tokens.background
    palette.highlight: tokens.primary
    palette.highlightedText: tokens.onPrimary
    header: CatTitleBar {
        theme: control.theme
        targetWindow: control
        visible: control.frameless
        height: visible ? implicitHeight : 0
    }
    Item {
        parent: B.Overlay.overlay
        anchors.fill: parent
        z: 1000
        visible: control.frameless && control.visibility !== Window.Maximized && control.visibility
                 !== Window.FullScreen
        Repeater {
            model: [
                {
                    edges: Qt.LeftEdge,
                    cursor: Qt.SizeHorCursor
                },
                {
                    edges: Qt.RightEdge,
                    cursor: Qt.SizeHorCursor
                },
                {
                    edges: Qt.TopEdge,
                    cursor: Qt.SizeVerCursor
                },
                {
                    edges: Qt.BottomEdge,
                    cursor: Qt.SizeVerCursor
                },
                {
                    edges: Qt.LeftEdge | Qt.TopEdge,
                    cursor: Qt.SizeFDiagCursor
                },
                {
                    edges: Qt.RightEdge | Qt.BottomEdge,
                    cursor: Qt.SizeFDiagCursor
                },
                {
                    edges: Qt.RightEdge | Qt.TopEdge,
                    cursor: Qt.SizeBDiagCursor
                },
                {
                    edges: Qt.LeftEdge | Qt.BottomEdge,
                    cursor: Qt.SizeBDiagCursor
                }
            ]
            MouseArea {
                required property var modelData
                readonly property bool horizontal: (modelData.edges & (Qt.LeftEdge | Qt.RightEdge))
                                                   !== 0

                readonly property bool vertical: (modelData.edges & (Qt.TopEdge | Qt.BottomEdge))
                                                 !== 0
                x: (modelData.edges & Qt.RightEdge) ? control.width - width : (horizontal ? 0 : 6)
                y: (modelData.edges & Qt.BottomEdge) ? control.height - height : (vertical ? 0 : 6)
                width: horizontal ? 6 : Math.max(0, control.width - 12)
                height: vertical ? 6 : Math.max(0, control.height - 12)
                cursorShape: modelData.cursor
                onPressed: control.startSystemResize(modelData.edges)
            }
        }
    }
}
