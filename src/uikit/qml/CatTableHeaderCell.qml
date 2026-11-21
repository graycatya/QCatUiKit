// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QCatUiKit

Rectangle {
    id: cell
    required property var display
    required property int column
    property ThemePalette theme: Theme
    property real padding: 8
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: 140
    implicitHeight: tokens.controlHeight
    color: tokens.surfaceHover

    CatLabel {
        objectName: "catTableHeaderCellLabel"
        theme: cell.theme
        anchors.fill: parent
        anchors.margins: cell.padding
        text: cell.display === undefined ? "" : String(cell.display)
        font.bold: true
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
