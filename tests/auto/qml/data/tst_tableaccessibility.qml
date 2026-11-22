// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableAccessibility"
    when: windowShown
    visible: true

    TableModel {
        id: rows
        columns: ["a", "b", "c", "d"]
        headers: ["A", "B", "C", "D"]
        Component.onCompleted: {
            for (let i = 0; i < 50; ++i)
                append({a: i, b: 10 + i, c: 20 + i, d: 30 + i});
        }
    }

    Component {
        id: tableComponent
        CatTableView {
            width: 480
            height: 240
            model: rows
            accessibleName: "Data table"
        }
    }

    Component {
        id: frozenTableComponent
        CatTableView {
            width: 480
            height: 240
            model: rows
            frozenColumnCount: 1
            frozenRowCount: 1
        }
    }

    Component {
        id: rtlTableComponent
        CatTableView {
            width: 480
            height: 240
            model: rows
            LayoutMirroring.enabled: true
        }
    }

    function selectCell(table, row, column) {
        table.view.selectionModel.setCurrentIndex(
                    table.model.index(row, column),
                    ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Current);
    }

    function focusTable(table) {
        table.view.forceActiveFocus();
        wait(20);
    }

    function test_tableSemantics() {
        const table = createTemporaryObject(tableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        compare(table.Accessible.role, Accessible.Table);
        compare(table.Accessible.name, "Data table");
        verify(table.Accessible.description.indexOf("50") >= 0);
        focusTable(table);
        verify(table.view.activeFocus);
        const ring = findChild(table, "tableFocusRing");
        verify(ring);
        tryVerify(function() { return ring.visible; }, 1000);
    }

    function test_pageNavigation() {
        const table = createTemporaryObject(tableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        focusTable(table);
        selectCell(table, 0, 0);
        keyClick(Qt.Key_PageDown);
        tryVerify(function() {
            return table.view.selectionModel.currentIndex.row > 0;
        }, 1000);
        const down = table.view.selectionModel.currentIndex.row;
        keyClick(Qt.Key_PageUp);
        tryVerify(function() {
            return table.view.selectionModel.currentIndex.row < down;
        }, 1000);
        keyClick(Qt.Key_End, Qt.ControlModifier);
        tryVerify(function() {
            const current = table.view.selectionModel.currentIndex;
            return current.row === 49 && current.column === 3;
        }, 1000);
        keyClick(Qt.Key_Home, Qt.ControlModifier);
        tryVerify(function() {
            const current = table.view.selectionModel.currentIndex;
            return current.row === 0 && current.column === 0;
        }, 1000);
    }

    function test_pageNavigationFrozen() {
        const table = createTemporaryObject(frozenTableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        focusTable(table);
        selectCell(table, 0, 0);
        keyClick(Qt.Key_PageDown);
        tryVerify(function() {
            return table.view.selectionModel.currentIndex.row > 0;
        }, 1000);
        keyClick(Qt.Key_End, Qt.ControlModifier);
        tryVerify(function() {
            const current = table.view.selectionModel.currentIndex;
            return current.row === 49 && current.column === 3;
        }, 1000);
    }

    function test_pageNavigationRtl() {
        const table = createTemporaryObject(rtlTableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        focusTable(table);
        selectCell(table, 1, 0);
        keyClick(Qt.Key_Home, Qt.ControlModifier);
        tryVerify(function() {
            const current = table.view.selectionModel.currentIndex;
            return current.row === 0 && current.column === 3;
        }, 1000);
        keyClick(Qt.Key_End, Qt.ControlModifier);
        tryVerify(function() {
            const current = table.view.selectionModel.currentIndex;
            return current.row === 49 && current.column === 0;
        }, 1000);
    }

    function test_editBlocksPageNavigation() {
        const table = createTemporaryObject(tableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        focusTable(table);
        selectCell(table, 0, 0);
        verify(table.edit(0, 0));
        tryVerify(function() { return findChild(table, "tableEditField"); }, 1000);
        keyClick(Qt.Key_PageDown);
        compare(table.view.selectionModel.currentIndex.row, 0);
        table.cancelEdit();
    }
}
