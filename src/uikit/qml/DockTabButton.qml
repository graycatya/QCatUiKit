// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

CatButton {
    id: tab
    property bool tool: false
    property int symbol: 0 // 1: detach, 2: close, 3: dock back
    implicitWidth: tool ? tokens.controlHeight : Math.min(220, Math.max(84, implicitContentWidth + 28))
    leftPadding: tool ? 0 : 14
    rightPadding: tool ? 0 : 14
    CatToolTip {
        theme: tab.theme
        text: tab.text
        visible: (tab.tool || label.truncated) && tab.hovered
        delay: 600
    }
    contentItem: Item {
        implicitWidth: label.implicitWidth
        implicitHeight: label.implicitHeight
        Text {
            id: label
            anchors.fill: parent
            visible: tab.symbol === 0
            text: tab.text
            font: tab.font
            color: tab.enabled ? tab.tokens.text : tab.tokens.textDisabled
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: tab.tool ? Text.AlignHCenter : Text.AlignLeft
        }
        Item {
            anchors.centerIn: parent
            width: 14
            height: 14
            visible: tab.symbol > 0
            opacity: tab.enabled ? 1 : 0.35
            Rectangle {
                x: 1; y: 1; width: 12; height: 12
                visible: tab.symbol !== 2
                color: "transparent"
                border.color: tab.tokens.textSecondary
            }
            Rectangle {
                x: 4; y: 4; width: 6; height: 6
                visible: tab.symbol !== 2
                color: tab.symbol === 1 ? tab.tokens.surface : tab.tokens.primary
                border.color: tab.tokens.textSecondary
            }
            Repeater {
                model: tab.symbol === 2 ? 2 : 0
                delegate: Rectangle {
                    required property int index
                    anchors.centerIn: parent
                    width: 12; height: 1
                    rotation: index === 0 ? 45 : -45
                    color: tab.tokens.textSecondary
                }
            }
        }
    }
    background: Rectangle {
        color: tab.checked ? tab.tokens.surface : tab.hovered || tab.down ? tab.tokens.surfaceHover : "transparent"
        border.width: tab.visualFocus ? 1 : 0
        border.color: tab.tokens.primary
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 2
            visible: tab.checked && !tab.tool
            color: tab.tokens.primary
        }
    }
}
