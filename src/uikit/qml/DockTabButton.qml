// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import QCatUiKit

CatButton {
    id: tab
    property bool tool: false
    property int symbol: 0 // 1: detach, 2: close, 3: dock back
    readonly property color symbolColor: !tab.enabled ? tab.tokens.textDisabled :
                                                        tab.symbol === 2 && tab.tool
                                                        && (tab.hovered || tab.down)
                                                        ? tab.tokens.danger :
                                                          tab.tokens.textSecondary
    implicitWidth: tool ? tokens.controlHeight : Math.min(240, Math.max(44, implicitContentWidth
                                                                         + leftPadding
                                                                         + rightPadding))
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
                x: 1
                y: 1
                width: 12
                height: 12
                radius: 2
                visible: tab.symbol !== 2
                color: "transparent"
                border.width: 1.2
                border.color: tab.symbolColor
                opacity: 0.8
            }
            Rectangle {
                x: 4
                y: 4
                width: 6
                height: 6
                radius: 1
                visible: tab.symbol !== 2
                color: tab.symbol === 3 ? tab.tokens.primary : "transparent"
                border.width: 1.2
                border.color: tab.symbolColor
            }
            Shape {
                anchors.centerIn: parent
                width: 10
                height: 10
                visible: tab.symbol === 2
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    strokeColor: tab.symbolColor
                    strokeWidth: 1.4
                    capStyle: ShapePath.RoundCap
                    fillColor: "transparent"
                    startX: 1
                    startY: 1
                    PathLine {
                        x: 9
                        y: 9
                    }
                    PathMove {
                        x: 9
                        y: 1
                    }
                    PathLine {
                        x: 1
                        y: 9
                    }
                }
            }
        }
    }
    background: Rectangle {
        color: "transparent"
        Rectangle {
            anchors.fill: parent
            anchors.margins: tab.tool ? Math.min(4, Math.max(2, Math.round((tab.height - 22) / 2))) : 0
            radius: tab.tool ? 4 : tab.tokens.radius
            color: tab.checked ? tab.tokens.surface :
                                 tab.tool && tab.symbol === 2 ? tab.tokens.danger : tab.tokens.surfaceHover
            opacity: tab.tool ? (tab.pressed ? 0.22 : tab.hovered ? 0.12 : 0) :
                                tab.checked || tab.hovered || tab.down ? 1 : 0
            border.width: tab.visualFocus ? 1 : 0
            border.color: tab.tokens.primary
        }
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 2
            visible: tab.checked && !tab.tool
            color: tab.tokens.primary
        }
    }
}
