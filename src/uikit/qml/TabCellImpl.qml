// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

// Internal single-cell tab used by CatTabView: the title and the close target share one
// background so the close target never appears outside the tab boundary.
Rectangle {
    id: cell
    required property ThemePalette theme
    required property int tabIndex
    property string title: ""
    property bool selected: false
    property bool closable: false
    signal activated
    signal closeRequested

    readonly property var tokens: (theme || Theme).tokens
    readonly property bool hovered: hover.hovered || titleButton.hovered
                                     || (closeButton.visible && closeButton.hovered)

    implicitWidth: row.implicitWidth + tokens.padding * 2
    implicitHeight: tokens.controlHeight
    radius: tokens.radius
    color: selected || hovered ? tokens.surfaceHover : tokens.surface
    border.width: 1
    border.color: selected || titleButton.visualFocus ? tokens.primary : tokens.border

    HoverHandler {
        id: hover
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 4

        CatToggleButton {
            id: titleButton
            objectName: "tabTitle_" + cell.tabIndex
            theme: cell.theme
            text: cell.title
            checked: cell.selected
            padding: 0
            implicitWidth: Math.max(44, implicitContentWidth)
            implicitHeight: cell.tokens.controlHeight
            background: Item {}
            onClicked: cell.activated()
        }

        CatButton {
            id: closeButton
            objectName: "tabClose_" + cell.tabIndex
            theme: cell.theme
            visible: cell.closable
            text: "×"
            Accessible.name: qsTr("Close tab %1").arg(cell.tabIndex + 1)
            implicitWidth: 24
            implicitHeight: cell.tokens.controlHeight
            padding: 0
            readonly property color symbolColor: down || hovered ? "#ffffff" : cell.tokens.textSecondary
            // A Row aligns children to the top, so the target fills the cell height and the
            // hover surface is centered inside it. This keeps the symbol on the text baseline.
            background: Item {
                Rectangle {
                    anchors.centerIn: parent
                    width: 20
                    height: 20
                    radius: cell.tokens.radius
                    color: closeButton.down ? Qt.darker(cell.tokens.danger, 1.2)
                         : closeButton.hovered ? cell.tokens.danger : "transparent"
                }
            }
            contentItem: Item {
                implicitWidth: 10
                implicitHeight: 10
                Rectangle {
                    width: 11
                    height: 1.5
                    radius: 0.75
                    anchors.centerIn: parent
                    rotation: 45
                    color: closeButton.symbolColor
                }
                Rectangle {
                    width: 11
                    height: 1.5
                    radius: 0.75
                    anchors.centerIn: parent
                    rotation: -45
                    color: closeButton.symbolColor
                }
            }
            onClicked: cell.closeRequested()
        }
    }
}
