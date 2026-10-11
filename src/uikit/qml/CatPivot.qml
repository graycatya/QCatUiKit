// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatSelectorBar {
    id: control
    contentItem: Flow {
        spacing: control.tokens.spacing
        Repeater {
            model: control.model
            CatButton {
                required property int index
                required property var modelData
                theme: control.theme
                flat: true
                text: String(typeof modelData === "object" ? modelData.text || modelData.title
                                                             || "" : modelData)
                font.bold: control.selectedIndex === index
                onClicked: {
                    control.selectedIndex = index;
                    control.selectionRequested(index);
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 3
                    radius: 1.5
                    color: control.tokens.primary
                    visible: control.selectedIndex === parent.index
                }
            }
        }
    }
    implicitWidth: Math.max(200, implicitContentWidth)
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight)
}
