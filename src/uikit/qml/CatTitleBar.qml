// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Shapes
import QtQuick.Templates as T
import QtQuick.Layouts
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property Window targetWindow
    property string title: targetWindow ? targetWindow.title : ""
    readonly property bool maximized: targetWindow !== null
                                          && targetWindow.visibility === Window.Maximized
    implicitWidth: 400
    implicitHeight: Math.max(40, tokens.controlHeight)
    contentItem: RowLayout {
        spacing: 0
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            CatLabel {
                theme: control.theme
                anchors.fill: parent
                anchors.leftMargin: control.tokens.padding
                anchors.rightMargin: control.tokens.padding
                text: control.title
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            DragHandler {
                target: null
                onActiveChanged: if (active && control.targetWindow)
                                     control.targetWindow.startSystemMove()
            }
            TapHandler {
                onDoubleTapped: if (control.targetWindow) {
                                    if (control.maximized)
                                        control.targetWindow.showNormal();
                                    else
                                        control.targetWindow.showMaximized();
                                }
            }
        }
        CatButton {
            id: minimizeButton
            objectName: "titleBarMinimize"
            theme: control.theme
            flat: true
            text: ""
            Layout.fillHeight: true
            implicitWidth: 46
            enabled: control.targetWindow !== null
            hoverEnabled: true
            Accessible.name: qsTr("Minimize")
            onClicked: control.targetWindow.showMinimized()
            readonly property color faceColor: minimizeButton.pressed ? control.tokens.disabled :
                                                                        minimizeButton.hovered
                                                                        ? control.tokens.surfaceHover :
                                                                          control.tokens.surface
            contentItem: Item {
                implicitWidth: 14
                implicitHeight: 14
                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 1.5
                    radius: 0.75
                    color: minimizeButton.enabled ? control.tokens.textSecondary :
                                                    control.tokens.textDisabled
                }
            }
            background: Rectangle {
                color: minimizeButton.faceColor
                border.width: minimizeButton.visualFocus ? 2 : 0
                border.color: control.tokens.primary
            }
        }
        CatButton {
            id: maximizeButton
            objectName: "titleBarMaximize"
            theme: control.theme
            flat: true
            text: ""
            Layout.fillHeight: true
            implicitWidth: 46
            enabled: control.targetWindow !== null
            hoverEnabled: true
            Accessible.name: qsTr("Maximize or restore")
            onClicked: if (control.maximized)
                           control.targetWindow.showNormal();
                       else
                           control.targetWindow.showMaximized()
            readonly property color faceColor: maximizeButton.pressed ? control.tokens.disabled :
                                                                        maximizeButton.hovered
                                                                        ? control.tokens.surfaceHover :
                                                                          control.tokens.surface
            readonly property color iconColor: maximizeButton.enabled ? control.tokens.textSecondary :
                                                                         control.tokens.textDisabled
            contentItem: Item {
                implicitWidth: 14
                implicitHeight: 14
                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    radius: 2
                    visible: !control.maximized
                    color: "transparent"
                    border.width: 1.2
                    border.color: maximizeButton.iconColor
                }
                Item {
                    anchors.centerIn: parent
                    width: 13
                    height: 13
                    visible: control.maximized
                    Rectangle {
                        x: 3
                        y: 0
                        width: 10
                        height: 10
                        radius: 2
                        color: "transparent"
                        border.width: 1.2
                        border.color: maximizeButton.iconColor
                    }
                    Rectangle {
                        x: 0
                        y: 3
                        width: 10
                        height: 10
                        radius: 2
                        color: maximizeButton.faceColor
                        border.width: 1.2
                        border.color: maximizeButton.iconColor
                    }
                }
            }
            background: Rectangle {
                color: maximizeButton.faceColor
                border.width: maximizeButton.visualFocus ? 2 : 0
                border.color: control.tokens.primary
            }
        }
        CatButton {
            id: closeButton
            objectName: "titleBarClose"
            theme: control.theme
            flat: true
            text: ""
            Layout.fillHeight: true
            implicitWidth: 46
            enabled: control.targetWindow !== null
            hoverEnabled: true
            Accessible.name: qsTr("Close window")
            onClicked: control.targetWindow.close()
            readonly property bool hot: closeButton.hovered || closeButton.pressed
            readonly property color faceColor: closeButton.pressed ? Qt.darker(control.tokens.danger, 1.1) :
                                                                     closeButton.hovered
                                                                     ? control.tokens.danger :
                                                                       control.tokens.surface
            readonly property color iconColor: !closeButton.enabled ? control.tokens.textDisabled :
                                                                      closeButton.hot
                                                                      ? control.tokens.background :
                                                                        control.tokens.textSecondary
            contentItem: Item {
                implicitWidth: 14
                implicitHeight: 14
                Shape {
                    anchors.centerIn: parent
                    width: 12
                    height: 12
                    preferredRendererType: Shape.CurveRenderer
                    ShapePath {
                        strokeColor: closeButton.iconColor
                        strokeWidth: 1.4
                        capStyle: ShapePath.RoundCap
                        fillColor: "transparent"
                        startX: 1.4
                        startY: 1.4
                        PathLine {
                            x: 10.6
                            y: 10.6
                        }
                        PathMove {
                            x: 10.6
                            y: 1.4
                        }
                        PathLine {
                            x: 1.4
                            y: 10.6
                        }
                    }
                }
            }
            background: Rectangle {
                color: closeButton.faceColor
                border.width: closeButton.visualFocus ? 2 : 0
                border.color: control.tokens.primary
            }
        }
    }
    background: Rectangle {
        color: control.tokens.surface
    }
}
