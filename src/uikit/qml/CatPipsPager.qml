// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property int numberOfPages: 5
    property int selectedPageIndex: 0
    property int maxVisiblePips: 7
    property int orientation: Qt.Horizontal
    signal pageRequested(int index)
    readonly property var visiblePages: {
        const count = Math.max(0, numberOfPages);
        const visible = Math.max(1, Math.min(maxVisiblePips, count, Math.max(1, Math.floor(width / (
                                                                                               tokens.controlHeight
                                                                                               + 4)) - 2)));
        const start = Math.max(0, Math.min(count - visible, selectedPageIndex - Math.floor(visible
                                                                                           / 2)));
        return count ? Array.from({
                                      length: visible
                                  }, (_, index) => start + index) : [];
    }
    function selectPage(index: int): void {
        const bounded = Math.max(0, Math.min(numberOfPages - 1, index));
        if (numberOfPages > 0 && bounded !== selectedPageIndex) {
            selectedPageIndex = bounded;
            pageRequested(bounded);
        }
    }
    onNumberOfPagesChanged: selectedPageIndex = numberOfPages > 0 ? Math.max(0, Math.min(
                                                                                 numberOfPages - 1,
                                                                                 selectedPageIndex)) :
                                                                    -1
    implicitWidth: (Math.min(Math.max(0, numberOfPages), Math.max(1, maxVisiblePips)) + 2) * (
                       tokens.controlHeight + 4) - 4
    implicitHeight: pips.implicitHeight
    focusPolicy: Qt.StrongFocus
    Keys.onRightPressed: event => {
        selectPage(selectedPageIndex + 1);
        event.accepted = true;
    }
    Keys.onLeftPressed: event => {
        selectPage(selectedPageIndex - 1);
        event.accepted = true;
    }
    contentItem: Grid {
        id: pips
        columns: control.orientation === Qt.Vertical ? 1 : control.visiblePages.length + 2
        spacing: 4
        CatButton {
            theme: control.theme
            implicitWidth: control.tokens.controlHeight
            text: "‹"
            flat: true
            enabled: control.selectedPageIndex > 0
            Accessible.name: qsTr("Previous page")
            onClicked: control.selectPage(control.selectedPageIndex - 1)
        }
        Repeater {
            model: control.visiblePages
            CatButton {
                id: pip
                required property int modelData
                theme: control.theme
                implicitWidth: control.tokens.controlHeight
                flat: true
                Accessible.name: qsTr("Page %1").arg(modelData + 1)
                contentItem: Item {
                    implicitWidth: 8
                    implicitHeight: 8
                    Rectangle {
                        anchors.centerIn: parent
                        width: 8
                        height: 8
                        radius: 4
                        color: control.selectedPageIndex === pip.modelData ? control.tokens.primary :
                                                                             control.tokens.border
                        scale: control.selectedPageIndex === pip.modelData ? 1.4 : 1
                    }
                }
                onClicked: control.selectPage(modelData)
            }
        }
        CatButton {
            theme: control.theme
            implicitWidth: control.tokens.controlHeight
            text: "›"
            flat: true
            enabled: control.selectedPageIndex < control.numberOfPages - 1
            Accessible.name: qsTr("Next page")
            onClicked: control.selectPage(control.selectedPageIndex + 1)
        }
    }
}
