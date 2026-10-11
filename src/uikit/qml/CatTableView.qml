// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQml.Models
import QtQuick.Controls.Basic as B

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property alias model: table.model
    property alias delegate: table.delegate
    property alias columnWidthProvider: table.columnWidthProvider
    property alias rowHeightProvider: table.rowHeightProvider
    property alias resizableColumns: table.resizableColumns
    property alias view: table
    implicitWidth: 480
    implicitHeight: 280
    B.HorizontalHeaderView {
        id: header
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        syncView: table
        clip: true
        delegate: Rectangle {
            required property var display
            implicitHeight: control.tokens.controlHeight
            implicitWidth: 140
            color: control.tokens.surfaceHover
            CatLabel {
                theme: control.theme
                anchors.fill: parent
                anchors.margins: 8
                text: parent.display === undefined ? "" : String(parent.display)
                font.bold: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
    TableView {
        id: table
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        clip: true
        columnSpacing: 1
        rowSpacing: 1
        resizableColumns: true
        boundsBehavior: Flickable.StopAtBounds
        selectionModel: ItemSelectionModel {
            model: table.model
        }
        delegate: Rectangle {
            required property var display
            required property int row
            required property int column
            required property bool selected
            implicitWidth: 140
            implicitHeight: control.tokens.controlHeight
            color: selected ? control.tokens.surfaceHover : control.tokens.surface
            CatLabel {
                theme: control.theme
                anchors.fill: parent
                anchors.margins: 8
                text: parent.display === undefined ? "" : String(parent.display)
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }
        B.ScrollBar.vertical: B.ScrollBar {}
        B.ScrollBar.horizontal: B.ScrollBar {}
    }
}
