// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls.Basic as B
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableFreeze"
    when: windowShown
    width: 640
    height: 480
    visible: true

    TableModel {
        id: rows
        columns: ["name", "state", "value", "note"]
        headers: ["Name", "State", "Value", "Note"]
        Component.onCompleted: {
            for (let i = 0; i < 40; ++i)
                append({name: "Row " + i, state: "Ready", value: i, note: "Note"});
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

    TableModel {
        id: shrinkRows
        columns: ["name"]
        Component.onCompleted: {
            for (let i = 0; i < 6; ++i)
                append({name: "Row " + i});
        }
    }

    Component {
        id: shrinkComponent
        CatTableView {
            width: 480
            height: 320
            model: shrinkRows
            frozenColumnCount: 1
            frozenRowCount: 4
        }
    }

    Component {
        id: customCell
        Rectangle {
            required property var display
            required property int row
            required property int column
            implicitWidth: 120
            implicitHeight: 28
        }
    }

    function init() { failOnWarning(/.*/); }
    function cleanup() { Theme.reducedMotion = false; }
    function make(properties) {
        const table = createTemporaryObject(tableComponent, test, properties || {});
        verify(table);
        verify(waitForRendering(table));
        return table;
    }

    function test_defaultPath() {
        const table = make({});
        compare(table.frozenColumnCount, 0);
        compare(table.frozenRowCount, 0);
        compare(table.view.x, 0);
        compare(table.view.y, Theme.tokens.controlHeight);
        compare(table.view.width, table.width);
        compare(table.view.height, table.height - Theme.tokens.controlHeight);
        compare(table.view.rows, 40);
        compare(table.view.columns, 4);
    }

    function test_frozenSkeleton() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        compare(table.view.rows, 40);
        compare(table.view.columns, 4);
        compare(table.view.x, 140);
        compare(table.view.y, Theme.tokens.controlHeight);
        compare(table.view.width, table.width - 140);
        compare(table.view.height, table.height - Theme.tokens.controlHeight * 2);
    }

    function test_providerWidths() {
        const table = make({
            frozenColumnCount: 2,
            frozenRowCount: 1,
            columnWidthProvider: function(column) { return 100; },
            rowHeightProvider: function(row) { return 30; }
        });
        compare(table.view.x, 201);
        compare(table.view.y, 30);
        compare(table.view.width, table.width - 201);
        compare(table.view.height, table.height - 30 - Theme.tokens.controlHeight);
        const topRight = findChild(table, "frozenTopRightView");
        const bottomLeft = findChild(table, "frozenBottomLeftView");
        verify(topRight);
        verify(bottomLeft);
        compare(bottomLeft.rowHeight(1), 30);
        compare(topRight.rowHeight(0), 30);
        compare(table.view.rowHeight(1), 30);
        compare(bottomLeft.columnWidth(0), 100);
        compare(topRight.columnWidth(2), 100);
    }

    function test_clamps() {
        const table = make({frozenColumnCount: -2, frozenRowCount: -2});
        compare(table.view.x, 0);
        compare(table.view.y, Theme.tokens.controlHeight);

        table.frozenColumnCount = 999;
        table.frozenRowCount = 999;
        compare(table.view.x, 4 * 140 + 3);
        compare(table.view.width, 0);
        compare(table.view.height, 0);
    }

    function test_runtimeToggle() {
        const table = make({});
        compare(table.view.x, 0);
        table.frozenColumnCount = 1;
        compare(table.view.x, 140);
        table.frozenColumnCount = 0;
        compare(table.view.x, 0);
        table.frozenRowCount = 1;
        compare(table.view.height, table.height - Theme.tokens.controlHeight * 2);
        table.frozenRowCount = 0;
        compare(table.view.height, table.height - Theme.tokens.controlHeight);
    }

    function test_modelShrinkClamps() {
        const table = createTemporaryObject(shrinkComponent, test);
        verify(table);
        verify(waitForRendering(table));
        compare(table.view.y, 4 * Theme.tokens.controlHeight + 3);
        for (let i = 0; i < 4; ++i)
            shrinkRows.remove(shrinkRows.count - 1);
        compare(shrinkRows.count, 2);
        tryCompare(table.view, "rows", 2);
        compare(table.view.y, 2 * Theme.tokens.controlHeight + 1);
        compare(table.view.height, table.height - Theme.tokens.controlHeight
                                        - (2 * Theme.tokens.controlHeight + 1));
        shrinkRows.clear();
        tryCompare(table.view, "rows", 0);
        compare(table.view.y, 0);
    }

    function test_fullyFrozenHidesScrollBars() {
        const table = make({frozenColumnCount: 4, frozenRowCount: 40});
        const bar = findChild(table, "frozenVerticalScrollBar");
        verify(bar);
        compare(bar.visible, false);
        compare(table.view.B.ScrollBar.horizontal.policy, B.ScrollBar.AlwaysOff);
    }

    function test_keyboardCrossPanes() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const main = table.view;
        mouseClick(main, 20, Theme.tokens.controlHeight / 2);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 1;
        }, 1000);
        keyClick(Qt.Key_Left);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 0;
        }, 1000);
        keyClick(Qt.Key_Right);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 1;
        }, 1000);
        keyClick(Qt.Key_Up);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 0;
        }, 1000);
        keyClick(Qt.Key_Up);
        compare(main.selectionModel.currentIndex.row, 0);
        keyClick(Qt.Key_Left);
        compare(main.selectionModel.currentIndex.column, 0);
        keyClick(Qt.Key_Left);
        compare(main.selectionModel.currentIndex.column, 0);
        keyClick(Qt.Key_Down);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1;
        }, 1000);
    }

    function test_verticalScrollBarSpansContent() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const bar = findChild(table, "frozenVerticalScrollBar");
        verify(bar);
        compare(bar.visible, true);
        compare(bar.height, table.height - Theme.tokens.controlHeight);
        table.view.contentY = 100;
        tryVerify(function() {
            return Math.abs(bar.position - table.view.visibleArea.yPosition) < 0.001;
        }, 1000);
        compare(bar.size, table.view.visibleArea.heightRatio);
    }

    function test_resizeFrozenColumn() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const header = findChild(table, "frozenHeader");
        const leftPane = findChild(table, "frozenBottomLeftView");
        verify(header);
        verify(leftPane);
        compare(table.view.x, 140);
        mouseDrag(header, 137, header.height / 2, 40, 0);
        tryCompare(table.view, "x", 180);
        tryCompare(header, "width", 180);
        tryCompare(leftPane, "width", 180);
        table.resizableColumns = false;
        mouseDrag(header, 177, header.height / 2, 20, 0);
        compare(table.view.x, 180);
    }

    function test_resizeMainColumn() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const header = findChild(table, "frozenMainHeader");
        const topRight = findChild(table, "frozenTopRightView");
        verify(header);
        verify(topRight);
        compare(table.view.columnWidth(1), 140);
        mouseDrag(header, 137, header.height / 2, 30, 0);
        tryVerify(function() { return table.view.columnWidth(1) === 170; }, 1000);
        tryVerify(function() { return topRight.columnWidth(1) === 170; }, 1000);
    }

    function test_resizePersistsAcrossFreezeToggle() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const header = findChild(table, "frozenHeader");
        verify(header);
        mouseDrag(header, 137, header.height / 2, 40, 0);
        tryCompare(table.view, "x", 180);
        table.frozenColumnCount = 0;
        table.frozenRowCount = 0;
        tryVerify(function() { return table.view.columnWidth(0) === 180; }, 1000);
    }

    function test_selectionSharedAcrossPanes() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const left = findChild(table, "frozenBottomLeftView");
        const main = table.view;
        verify(left);
        mouseClick(left, 20, Theme.tokens.controlHeight / 2);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 1
                && main.selectionModel.currentIndex.column === 0;
        }, 1000);
        mouseClick(main, 20, Theme.tokens.controlHeight * 1.5);
        tryVerify(function() {
            return main.selectionModel.currentIndex.row === 2
                && main.selectionModel.currentIndex.column === 1;
        }, 1000);
    }

    function test_headerSegments() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const frozenHeader = findChild(table, "frozenHeader");
        const mainHeader = findChild(table, "frozenMainHeader");
        verify(frozenHeader);
        verify(mainHeader);
        compare(frozenHeader.contentX, 0);
        compare(frozenHeader.columnWidth(0), 140);
        compare(mainHeader.columnWidth(1), 140);
        table.view.contentX = 40;
        tryCompare(table.view, "contentX", 40);
        tryCompare(mainHeader, "contentX", 40);
        compare(frozenHeader.contentX, 0);
    }

    function test_freezeToAndClear() {
        const table = make({});
        table.freezeTo(2, 1);
        compare(table.frozenRowCount, 3);
        compare(table.frozenColumnCount, 2);
        compare(table.view.x, 2 * 140 + 1);
        compare(table.view.y, Theme.tokens.controlHeight * 3 + 2);
        table.frozenColumnCount = 3;
        compare(table.view.x, 3 * 140 + 2);
        table.freezeTo(0, 0);
        compare(table.frozenRowCount, 1);
        compare(table.frozenColumnCount, 1);
        compare(table.view.x, 140);
        table.clearFreeze();
        compare(table.frozenRowCount, 0);
        compare(table.frozenColumnCount, 0);
        compare(table.view.x, 0);
        compare(table.view.y, Theme.tokens.controlHeight);
        table.freezeTo(-3, -3);
        compare(table.frozenRowCount, 0);
        compare(table.frozenColumnCount, 0);
    }

    function test_cellSpacing() {
        const table = make({frozenColumnCount: 2, frozenRowCount: 2});
        compare(table.view.x, 2 * 140 + 1);
        compare(table.view.y, 2 * Theme.tokens.controlHeight + 1);
        compare(table.view.width, table.width - (2 * 140 + 1));
        compare(table.view.height, table.height - Theme.tokens.controlHeight
                                        - (2 * Theme.tokens.controlHeight + 1));
        compare(findChild(table, "frozenHeader").width, 2 * 140 + 1);
        compare(findChild(table, "frozenBottomLeftView").width, 2 * 140 + 1);
        compare(findChild(table, "frozenTopRightView").height,
                2 * Theme.tokens.controlHeight + 1);
    }

    function test_customDelegateSizes() {
        const table = make({delegate: customCell});
        compare(table.view.columnWidth(0), 120);
        compare(table.view.rowHeight(0), 28);
        table.frozenColumnCount = 1;
        table.frozenRowCount = 1;
        tryCompare(table.view, "x", 120);
        tryCompare(table.view, "y", 28);
        compare(table.view.columnWidth(1), 120);
        compare(table.view.rowHeight(1), 28);
    }

    function test_scrollSync() {
        const table = make({frozenColumnCount: 1, frozenRowCount: 1});
        const bottomLeft = findChild(table, "frozenBottomLeftView");
        const topRight = findChild(table, "frozenTopRightView");
        verify(bottomLeft);
        verify(topRight);
        table.view.contentX = 40;
        table.view.contentY = 60;
        tryCompare(table.view, "contentX", 40);
        tryCompare(table.view, "contentY", 60);
        tryCompare(topRight, "contentX", 40);
        tryCompare(bottomLeft, "contentY", 60);
        compare(bottomLeft.contentX, 0);
        compare(topRight.contentY, 0);
    }
}
