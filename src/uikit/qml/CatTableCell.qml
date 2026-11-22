// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QCatUiKit

Rectangle {
    id: cell
    required property int row
    required property int column
    required property var display
    required property bool selected
    required property bool editing
    property ThemePalette theme: Theme
    property Component editDelegate: null
    property real padding: 8
    property color hoverColor: tokens.surfaceHover
    property color selectedColor: Qt.rgba(tokens.primary.r, tokens.primary.g,
                                           tokens.primary.b, 0.16)
    readonly property bool hovered: hoverHandler.hovered
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: 140
    implicitHeight: tokens.controlHeight
    color: selected ? selectedColor : (hovered ? hoverColor : tokens.surface)
    Accessible.role: Accessible.Cell
    Accessible.name: display === undefined ? "" : String(display)
    Accessible.selected: selected

    HoverHandler {
        id: hoverHandler
        enabled: !cell.editing
    }

    CatLabel {
        objectName: "catTableCellLabel"
        theme: cell.theme
        anchors.fill: parent
        anchors.margins: cell.padding
        text: cell.display === undefined ? "" : String(cell.display)
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        visible: !cell.editing
    }

    TableView.editDelegate: cell.editDelegate
}
