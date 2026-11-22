// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QCatUiKit

Rectangle {
    id: cell
    required property var display
    required property int column
    property ThemePalette theme: Theme
    property real padding: 8
    property real rightPadding: 8
    property string accessibleDescription: ""
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: 140
    implicitHeight: tokens.controlHeight
    color: tokens.surfaceHover
    Accessible.role: Accessible.ColumnHeader
    Accessible.name: display === undefined ? "" : String(display)
    Accessible.description: accessibleDescription

    CatLabel {
        objectName: "catTableHeaderCellLabel"
        theme: cell.theme
        anchors.fill: parent
        anchors.leftMargin: cell.padding
        anchors.rightMargin: cell.rightPadding
        text: cell.display === undefined ? "" : String(cell.display)
        font.bold: true
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: cell.tokens.border
    }
}
