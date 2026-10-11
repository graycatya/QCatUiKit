// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

B.AbstractButton {
    id: control
    required property var entry
    implicitWidth: 240
    implicitHeight: 184
    Layout.minimumWidth: 0
    padding: 20
    hoverEnabled: true
    Accessible.name: entry.title + ", " + entry.name
    background: Rectangle {
        radius: 10
        color: control.hovered ? Theme.tokens.surfaceHover : Theme.tokens.surface
        border.color: control.visualFocus ? Theme.tokens.primary : Theme.tokens.border
        border.width: control.visualFocus ? 2 : 1
    }
    contentItem: ColumnLayout {
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            Rectangle {
                implicitWidth: 36
                implicitHeight: 36
                radius: 8
                color: Theme.tokens.surfaceHover
                GalleryIcon {
                    anchors.centerIn: parent
                    name: control.entry.icon
                    color: Theme.tokens.primary
                }
            }
            Item {
                Layout.fillWidth: true
            }
            GalleryIcon {
                name: "arrow"
                implicitWidth: 16
                implicitHeight: 16
                opacity: control.hovered ? 1 : 0.45
            }
        }
        CatLabel {
            text: control.entry.title
            font.pixelSize: 16
            font.bold: true
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
        CatLabel {
            text: control.entry.description
            font.pixelSize: 12
            color: Theme.tokens.textSecondary
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }
        CatLabel {
            text: control.entry.name
            font.pixelSize: 11
            color: Theme.tokens.textSecondary
        }
    }
}
