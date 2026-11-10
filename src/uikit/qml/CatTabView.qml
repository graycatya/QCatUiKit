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
                TabCellImpl {
                    required property int index
                    objectName: "tabCell_" + index
                    theme: control.theme
                    tabIndex: index
                    title: control.titles[index] || qsTr("Tab %1").arg(index + 1)
                    selected: control.selectedIndex === index
                    closable: control.tabsClosable
                    onActivated: control.selectedIndex = index
                    onCloseRequested: control.closeRequested(index)
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
