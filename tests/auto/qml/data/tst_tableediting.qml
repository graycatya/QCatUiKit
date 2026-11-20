// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableEditing"
    when: windowShown
    width: 640
    height: 480
    visible: true

    TableModel {
        id: rows
        columns: ["name", "value"]
        headers: ["Name", "Value"]
        Component.onCompleted: {
            for (let i = 0; i < 5; ++i)
                append({name: "Row " + i, value: i});
        }
    }

    Component {
        id: tableComponent
        CatTableView {
            width: 480
            height: 320
            model: rows
        }
    }

    Component {
        id: frozenTableComponent
        CatTableView {
            width: 480
            height: 320
            model: rows
            frozenColumnCount: 1
            frozenRowCount: 1
        }
    }

    function test_editCommit() {
        const table = createTemporaryObject(tableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        verify(table.edit(1, 1));
        tryVerify(function() { return findChild(table, "tableEditField"); }, 1000);
        verify(table.commitEdit("changed"));
        tryVerify(function() { return rows.get(1).value === "changed"; }, 1000);
        tryVerify(function() { return !findChild(table, "tableEditField"); }, 1000);
    }

    function test_editCancel() {
        const table = createTemporaryObject(tableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        verify(table.edit(2, 0));
        tryVerify(function() { return findChild(table, "tableEditField"); }, 1000);
        table.cancelEdit();
        tryVerify(function() { return !findChild(table, "tableEditField"); }, 1000);
        compare(rows.get(2).name, "Row 2");
    }

    function test_editFrozenCell() {
        const table = createTemporaryObject(frozenTableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        verify(table.edit(1, 0));
        tryVerify(function() { return findChild(table, "tableEditField"); }, 1000);
        verify(table.commitEdit("frozen"));
        tryVerify(function() { return rows.get(1).name === "frozen"; }, 1000);
        const before = rows.get(1).value;
        verify(table.edit(1, 1));
        table.cancelEdit();
        compare(rows.get(1).value, before);
    }

    function test_editInvalid() {
        const table = createTemporaryObject(tableComponent, test);
        verify(table);
        verify(waitForRendering(table));
        compare(table.edit(-1, 0), false);
        compare(table.edit(0, -1), false);
    }
}
