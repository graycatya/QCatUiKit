// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableCellBases"
    when: windowShown
    visible: true

    Component {
        id: cellComponent
        CatTableCell {
            width: 140
            height: Theme.tokens.controlHeight
            row: 0
            column: 0
            display: "value"
            selected: false
            editing: false
        }
    }

    Component {
        id: derivedCellComponent
        CatTableCell {
            property string extra: "extra"
            row: 0
            column: 0
            display: "derived"
            selected: false
            editing: false
        }
    }

    Component {
        id: headerComponent
        CatTableHeaderCell {
            width: 140
            height: Theme.tokens.controlHeight
            column: 0
            display: "Header"
        }
    }

    function test_cellContract() {
        const cell = createTemporaryObject(cellComponent, test);
        verify(cell);
        compare(cell.row, 0);
        compare(cell.column, 0);
        compare(cell.display, "value");
        compare(cell.implicitWidth, 140);
        compare(cell.implicitHeight, Theme.tokens.controlHeight);
        compare(cell.color, Theme.tokens.surface);
        compare(cell.Accessible.role, Accessible.Cell);
        compare(cell.Accessible.name, "value");
        compare(cell.Accessible.selected, false);
        const label = findChild(cell, "catTableCellLabel");
        verify(label);
        compare(label.text, "value");
        compare(cell.editing, false);
        verify(label.visible);
        mouseMove(cell, 20, 20);
        tryCompare(cell, "hovered", true);
        compare(cell.color, cell.hoverColor);
        mouseMove(test, 300, 300);
        tryVerify(function() { return !cell.hovered; }, 1000);
        compare(cell.color, Theme.tokens.surface);
        cell.editing = true;
        verify(!label.visible);
        cell.editing = false;
        cell.selected = true;
        compare(cell.color, cell.selectedColor);
        compare(cell.Accessible.selected, true);
        mouseMove(cell, 20, 20);
        tryCompare(cell, "hovered", true);
        compare(cell.color, cell.selectedColor);
        mouseMove(test, 300, 300);
        tryVerify(function() { return !cell.hovered; }, 1000);
    }

    function test_derivedCell() {
        const cell = createTemporaryObject(derivedCellComponent, test);
        verify(cell);
        compare(cell.extra, "extra");
        compare(cell.display, "derived");
        verify(findChild(cell, "catTableCellLabel"));
    }

    function test_headerCellContract() {
        const cell = createTemporaryObject(headerComponent, test);
        verify(cell);
        compare(cell.column, 0);
        compare(cell.implicitWidth, 140);
        compare(cell.color, Theme.tokens.surfaceHover);
        compare(cell.Accessible.role, Accessible.ColumnHeader);
        compare(cell.Accessible.name, "Header");
        compare(cell.Accessible.description, "");
        cell.accessibleDescription = "Sorted ascending";
        compare(cell.Accessible.description, "Sorted ascending");
        mouseMove(cell, 20, 20);
        tryCompare(cell, "hovered", true);
        compare(cell.color, cell.hoverColor);
        mouseMove(test, 300, 300);
        tryVerify(function() { return !cell.hovered; }, 1000);
        compare(cell.color, Theme.tokens.surfaceHover);
        const label = findChild(cell, "catTableHeaderCellLabel");
        verify(label);
        compare(label.text, "Header");
    }
}
