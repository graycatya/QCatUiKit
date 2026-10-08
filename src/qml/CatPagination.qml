// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Layouts

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property int total: 0
    property int pageSize: 10
    property int currentPage: 1
    readonly property int pageCount: Math.max(1, Math.ceil(Math.max(0, total) / Math.max(1, pageSize)))
    signal pageRequested(int page)
    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight
    function requestPage(page: int): void {
        const next = Math.max(1, Math.min(pageCount, page));
        if (next !== currentPage) {
            currentPage = next;
            pageRequested(next);
        }
    }
    onPageCountChanged: currentPage = Math.max(1, Math.min(pageCount, currentPage))
    onCurrentPageChanged: {
        const bounded = Math.max(1, Math.min(pageCount, currentPage));
        if (currentPage !== bounded)
            currentPage = bounded;
    }
    RowLayout {
        id: row
        anchors.fill: parent
        spacing: control.tokens.spacing
        layoutDirection: control.LayoutMirroring.enabled ? Qt.RightToLeft : Qt.LeftToRight
        CatButton {
            theme: control.theme
            text: qsTr("Previous")
            enabled: control.currentPage > 1
            onClicked: control.requestPage(control.currentPage - 1)
        }
        CatLabel {
            theme: control.theme
            text: control.currentPage + " / " + control.pageCount
        }
        CatButton {
            theme: control.theme
            text: qsTr("Next")
            enabled: control.currentPage < control.pageCount
            onClicked: control.requestPage(control.currentPage + 1)
        }
    }
}
