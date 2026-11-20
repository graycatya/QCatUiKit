// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableStress"
    when: windowShown
    width: 800
    height: 600
    visible: true

    property int liveCells: 0

    TableModel {
        id: stressRows
        columns: ["a", "b", "c", "d"]
        headers: ["A", "B", "C", "D"]
    }

    Component {
        id: stressCell
        Rectangle {
            required property var display
            required property int row
            required property int column
            implicitWidth: 100
            implicitHeight: 24
            Component.onCompleted: test.liveCells += 1
            Component.onDestruction: test.liveCells -= 1
        }
    }

    Component {
        id: stressTable
        CatTableView {
            width: 800
            height: 600
            model: stressRows
            delegate: stressCell
            frozenColumnCount: 1
            frozenRowCount: 1
        }
    }

    function test_100kRowsWithFreeze() {
        for (let i = 0; i < 100000; ++i)
            stressRows.append({a: i, b: i, c: i, d: i});
        const table = createTemporaryObject(stressTable, test);
        verify(table);
        tryVerify(function() { return table.view.rows === 100000; }, 30000);
        verify(waitForRendering(table));
        verify(test.liveCells > 0);
        verify(test.liveCells < 2000);
        for (let i = 0; i < 100; ++i) {
            table.view.contentY = i * 120;
            wait(5);
        }
        verify(test.liveCells < 2000);
        stressRows.clear();
    }
}
