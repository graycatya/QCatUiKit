// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatScrollView {
    id: control
    property var model: []
    implicitWidth: 320
    implicitHeight: 260
    contentWidth: availableWidth
    Column {
        width: control.availableWidth
        Repeater {
            model: control.model
            Item {
                id: entry
                required property var modelData
                required property int index
                width: parent.width
                height: details.implicitHeight + 20
                Rectangle {
                    x: 9
                    y: 8
                    width: 2
                    height: parent.height
                    color: (control.theme || Theme).tokens.border
                    visible: entry.index < control.model.length - 1
                }
                Rectangle {
                    x: 4
                    y: 8
                    width: 12
                    height: 12
                    radius: 6
                    antialiasing: true
                    color: (control.theme || Theme).tokens.primary
                }
                Column {
                    id: details
                    x: 32
                    width: Math.max(0, parent.width - x)
                    spacing: 6
                    CatLabel {
                        theme: control.theme
                        width: parent.width
                        text: entry.modelData.title || ""
                        font.bold: true
                        wrapMode: Text.Wrap
                    }
                    CatLabel {
                        theme: control.theme
                        width: parent.width
                        text: entry.modelData.description || ""
                        wrapMode: Text.Wrap
                    }
                    CatLabel {
                        theme: control.theme
                        width: parent.width
                        text: entry.modelData.timestamp || ""
                        color: (control.theme || Theme).tokens.textSecondary
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}
