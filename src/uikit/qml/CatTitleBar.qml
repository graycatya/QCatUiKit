// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import QtQuick.Layouts
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property Window targetWindow
    property string title: targetWindow ? targetWindow.title : ""
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
                                    if (control.targetWindow.visibility === Window.Maximized)
                                        control.targetWindow.showNormal();
                                    else
                                        control.targetWindow.showMaximized();
                                }
            }
        }
        CatButton {
            theme: control.theme
            text: "−"
            flat: true
            Layout.fillHeight: true
            implicitWidth: 44
            enabled: control.targetWindow !== null
            Accessible.name: qsTr("Minimize")
            onClicked: control.targetWindow.showMinimized()
        }
        CatButton {
            theme: control.theme
            text: control.targetWindow && control.targetWindow.visibility === Window.Maximized
                  ? "▣" : "□"
            flat: true
            Layout.fillHeight: true
            implicitWidth: 44
            enabled: control.targetWindow !== null
            Accessible.name: qsTr("Maximize or restore")
            onClicked: if (control.targetWindow.visibility === Window.Maximized)
                           control.targetWindow.showNormal()
                       else
                           control.targetWindow.showMaximized()
        }
        CatButton {
            theme: control.theme
            text: "×"
            flat: true
            danger: true
            Layout.fillHeight: true
            implicitWidth: 44
            enabled: control.targetWindow !== null
            Accessible.name: qsTr("Close window")
            onClicked: control.targetWindow.close()
        }
    }
    background: Rectangle {
        color: control.tokens.surface
    }
}
