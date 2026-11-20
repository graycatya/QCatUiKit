// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableRtl"
    when: windowShown
    width: 640
    height: 480
    visible: true

    TableModel {
        id: rows
        columns: ["a", "b", "c", "d"]
        headers: ["A", "B", "C", "D"]
        Component.onCompleted: {
            for (let i = 0; i < 5; ++i)
                append({a: i, b: 10 + i, c: 20 + i, d: 30 + i});
        }
    }

    Component {
        id: ltrTable
        CatTableView {
            width: 480
            height: 320
            model: rows
        }
    }

    Component {
        id: rtlTable
        CatTableView {
            width: 480
            height: 320
            model: rows
            LayoutMirroring.enabled: true
        }
    }

    Component {
        id: rtlFrozenTable
        CatTableView {
            width: 480
            height: 320
            model: rows
            LayoutMirroring.enabled: true
            frozenColumnCount: 1
            frozenRowCount: 1
        }
    }

    function test_ltrIdentity() {
        const table = createTemporaryObject(ltrTable, test);
        verify(table);
        verify(waitForRendering(table));
        compare(table.view.model, table.model);
        const sourceIndex = table.model.index(0, 0);
        compare(table.mapToSource(sourceIndex), sourceIndex);
        compare(table.mapFromSource(sourceIndex), sourceIndex);
    }

    function test_rtlProxyMapping() {
        const table = createTemporaryObject(rtlTable, test);
        verify(table);
        verify(waitForRendering(table));
        verify(table.view.model !== table.model);
        const sourceIndex = table.model.index(0, 0);
        const proxyIndex = table.mapFromSource(sourceIndex);
        compare(proxyIndex.row, 0);
        compare(proxyIndex.column, 3);
        compare(table.mapToSource(proxyIndex), sourceIndex);

        const lastSource = table.model.index(0, 3);
        const firstProxy = table.mapFromSource(lastSource);
        compare(firstProxy.column, 0);
        compare(table.mapToSource(firstProxy), lastSource);

        tryVerify(function() {
            return table.view.contentWidth > table.view.width
                && Math.abs(table.view.contentX
                            - (table.view.contentWidth - table.view.width)) < 0.5;
        }, 1000);
    }

    function test_rtlFrozenGeometry() {
        const table = createTemporaryObject(rtlFrozenTable, test);
        verify(table);
        verify(waitForRendering(table));
        const main = table.view;
        const frozen = findChild(table, "frozenBottomLeftView");
        const header = findChild(table, "frozenHeader");
        verify(frozen);
        verify(header);
        compare(main.x, 0);
        compare(main.width, table.width - 140);
        compare(frozen.x, table.width - 140);
        compare(header.x, table.width - 140);
        const bar = findChild(table, "frozenVerticalScrollBar");
        verify(bar);
        compare(bar.x, 0);
        compare(bar.height, table.height - Theme.tokens.controlHeight);
        tryVerify(function() { return main.columnWidth(0) === 140; }, 1000);
        tryVerify(function() { return frozen.columnWidth(3) === 140; }, 1000);
        verify(main.columnWidth(3) <= 0);
        verify(frozen.columnWidth(0) <= 0);
        tryVerify(function() {
            return main.contentWidth > main.width
                && Math.abs(main.contentX - (main.contentWidth - main.width)) < 0.5;
        }, 1000);
    }

    function test_rtlResizeFrozenColumn() {
        const table = createTemporaryObject(rtlFrozenTable, test);
        verify(table);
        verify(waitForRendering(table));
        const header = findChild(table, "frozenHeader");
        const frozen = findChild(table, "frozenBottomLeftView");
        const main = table.view;
        verify(header);
        verify(frozen);
        compare(header.width, 140);
        mouseDrag(header, 3, header.height / 2, -40, 0);
        tryCompare(header, "width", 180);
        tryVerify(function() { return frozen.columnWidth(3) === 180; }, 1000);
        compare(main.x, 0);
        compare(main.width, table.width - 180);
    }

    function test_rtlKeyboardCrossPanes() {
        const table = createTemporaryObject(rtlFrozenTable, test);
        verify(table);
        verify(waitForRendering(table));
        const main = table.view;
        mouseClick(main, 20, Theme.tokens.controlHeight / 2);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 0;
        }, 1000);
        keyClick(Qt.Key_Right);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 1;
        }, 1000);
        keyClick(Qt.Key_Right);
        compare(main.selectionModel.currentIndex.column, 2);
        keyClick(Qt.Key_Right);
        tryVerify(function() {
            return main.selectionModel.currentIndex.column === 3;
        }, 1000);
        keyClick(Qt.Key_Right);
        compare(main.selectionModel.currentIndex.column, 3);
        keyClick(Qt.Key_Left);
        tryVerify(function() {
            return main.selectionModel.currentIndex.column === 2;
        }, 1000);
        keyClick(Qt.Key_Home);
        tryVerify(function() {
            return main.selectionModel.currentIndex.column === 3;
        }, 1000);
        keyClick(Qt.Key_End);
        tryVerify(function() {
            return main.selectionModel.currentIndex.column === 0;
        }, 1000);
    }

    function test_rtlEditCommit() {
        const table = createTemporaryObject(rtlFrozenTable, test);
        verify(table);
        verify(waitForRendering(table));
        verify(table.edit(1, 1));
        tryVerify(function() { return findChild(table, "tableEditField"); }, 1000);
        verify(table.commitEdit("rtl"));
        tryVerify(function() { return rows.get(1).c === "rtl"; }, 1000);
    }

    function test_rtlSelectionMapping() {
        const table = createTemporaryObject(rtlFrozenTable, test);
        verify(table);
        verify(waitForRendering(table));
        const main = table.view;
        const frozen = findChild(table, "frozenBottomLeftView");
        verify(frozen);
        mouseClick(frozen, 20, Theme.tokens.controlHeight / 2);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 3
                && table.mapToSource(main.selectionModel.currentIndex).column === 0;
        }, 1000);
        mouseClick(main, 20, Theme.tokens.controlHeight / 2);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 0
                && table.mapToSource(main.selectionModel.currentIndex).column === 3;
        }, 1000);
    }
}
