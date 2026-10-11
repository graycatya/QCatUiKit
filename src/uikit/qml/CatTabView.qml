// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property var titles: []
    property int selectedIndex: 0
    property bool tabsClosable: false
    property bool addTabButtonVisible: false
    default property alias pages: pagesLayout.data
    readonly property int tabCount: pagesLayout.children.length
    signal closeRequested(int index)
    signal addRequested
    implicitWidth: 360
    implicitHeight: 240
    contentItem: ColumnLayout {
        spacing: control.tokens.spacing
        Flow {
            Layout.fillWidth: true
            spacing: 4
            Repeater {
                model: control.tabCount
                Row {
                    required property int index
                    CatToggleButton {
                        theme: control.theme
                        text: control.titles[parent.index] || qsTr("Tab %1").arg(parent.index + 1)
                        checked: control.selectedIndex === parent.index
                        onClicked: control.selectedIndex = parent.index
                    }
                    CatButton {
                        theme: control.theme
                        text: "×"
                        padding: 6
                        flat: true
                        implicitWidth: control.tokens.controlHeight
                        visible: control.tabsClosable
                        Accessible.name: qsTr("Close tab %1").arg(parent.index + 1)
                        onClicked: control.closeRequested(parent.index)
                    }
                }
            }
            CatButton {
                theme: control.theme
                text: "+"
                padding: 6
                implicitWidth: control.tokens.controlHeight
                visible: control.addTabButtonVisible
                Accessible.name: qsTr("Add tab")
                onClicked: control.addRequested()
            }
        }
        StackLayout {
            id: pagesLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: control.selectedIndex
        }
    }
    onTabCountChanged: selectedIndex = tabCount ? Math.max(0, Math.min(selectedIndex, tabCount - 1)) :
                                                  -1
}
