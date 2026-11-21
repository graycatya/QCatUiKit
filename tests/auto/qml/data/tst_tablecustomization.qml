// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "TableCustomization"
    when: windowShown
    visible: true

    TableModel {
        id: rows
        columns: ["name", "value"]
        headers: ["Name", "Value"]
        Component.onCompleted: {
            for (let i = 0; i < 4; ++i)
                append({name: "Row " + i, value: i});
        }
    }

    TableModel {
        id: emptyRows
        columns: ["name", "value"]
        headers: ["Name", "Value"]
    }

    TableModel {
        id: longRows
        columns: ["name", "value"]
        headers: ["Name", "Value"]
        Component.onCompleted: {
            for (let i = 0; i < 100; ++i)
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
        id: toolbarComponent
        Rectangle {
            objectName: "testToolbar"
            height: 36
            implicitHeight: 36
            color: "#dddddd"
        }
    }

    Component {
        id: emptyComponent
        Rectangle {
            objectName: "testEmptyState"
            color: "#eeeeee"
        }
    }

    Component {
        id: loadingComponent
        Rectangle {
            objectName: "testLoadingState"
            color: "#cccccc"
        }
    }

    Component {
        id: headerComponent
        CatTableHeaderCell {
            objectName: "customHeader"
            property string marker: "custom"
        }
    }

    function make(properties) {
        const table = createTemporaryObject(tableComponent, test);
        if (properties)
            Object.assign(table, properties);
        return table;
    }

    function test_toolbarLayout() {
        const table = make({toolbar: toolbarComponent});
        verify(table);
        verify(waitForRendering(table));
        const host = findChild(table, "tableToolbarHost");
        const toolbar = findChild(table, "testToolbar");
        const header = findChild(table, "plainHeader");
        verify(host);
        verify(toolbar);
        verify(header);
        compare(host.height, 36);
        compare(host.width, table.width);
        compare(header.mapToItem(table, 0, 0).y, 36);
        table.view.contentY = 100;
        compare(host.y, 0);
        compare(toolbar.height, 36);
    }

    function test_noToolbar() {
        const table = make({});
        verify(table);
        verify(waitForRendering(table));
        compare(findChild(table, "tableToolbarHost").height, 0);
        compare(table.view.y, Theme.tokens.controlHeight);
    }

    function test_verticalWheel() {
        const table = make({model: longRows});
        verify(table);
        verify(waitForRendering(table));
        table.view.contentY = 0;
        mouseWheel(table.view, 40, 80, 0, -120);
        tryVerify(function() { return table.view.contentY > 0; }, 1000);
    }

    function test_verticalWheelWithToolbar() {
        const table = make({model: longRows, toolbar: toolbarComponent});
        verify(table);
        verify(waitForRendering(table));
        table.view.contentY = 0;
        mouseWheel(table.view, 40, 80, 0, -120);
        tryVerify(function() { return table.view.contentY > 0; }, 1000);
    }

    function test_verticalWheelFrozen() {
        const table = make({model: longRows, frozenColumnCount: 1, frozenRowCount: 1});
        verify(table);
        verify(waitForRendering(table));
        table.view.contentY = 0;
        mouseWheel(table.view, 40, 80, 0, -120);
        tryVerify(function() { return table.view.contentY > 0; }, 1000);
    }

    function test_layoutChangesKeepVerticalScroll() {
        const table = make({model: longRows});
        verify(table);
        verify(waitForRendering(table));
        table.toolbar = toolbarComponent;
        wait(50);
        table.toolbar = null;
        wait(50);
        table.headerDelegate = headerComponent;
        wait(50);
        table.headerDelegate = null;
        wait(50);
        table.loadingState = loadingComponent;
        table.loading = true;
        wait(50);
        table.loading = false;
        wait(50);
        table.LayoutMirroring.enabled = true;
        wait(50);
        table.view.contentY = 0;
        mouseWheel(table.view, 40, 80, 0, -120);
        tryVerify(function() { return table.view.contentY > 0; }, 1000);
        table.LayoutMirroring.enabled = false;
        table.height = 420;
        wait(50);
        table.view.contentY = 0;
        mouseWheel(table.view, 40, 80, 0, -120);
        tryVerify(function() { return table.view.contentY > 0; }, 1000);
    }

    function test_emptyState() {
        const table = make({emptyState: emptyComponent, model: emptyRows});
        verify(table);
        verify(waitForRendering(table));
        const overlay = findChild(table, "tableStateOverlay");
        verify(overlay);
        tryVerify(function() { return overlay.visible; }, 1000);
        compare(overlay.y, Theme.tokens.controlHeight);
        compare(overlay.height, table.height - Theme.tokens.controlHeight);
        verify(findChild(table, "testEmptyState"));
        table.model = rows;
        tryVerify(function() { return !overlay.visible; }, 1000);
    }

    function test_loadingState() {
        const table = make({loadingState: loadingComponent, loading: true});
        verify(table);
        verify(waitForRendering(table));
        const overlay = findChild(table, "tableStateOverlay");
        tryVerify(function() { return overlay.visible; }, 1000);
        verify(findChild(table, "testLoadingState"));
        table.loading = false;
        tryVerify(function() { return !overlay.visible; }, 1000);
    }

    function test_dynamicHeaderDelegate() {
        const table = make({});
        verify(table);
        verify(waitForRendering(table));
        table.headerDelegate = headerComponent;
        tryVerify(function() { return findChild(table, "customHeader"); }, 1000);
        table.headerDelegate = null;
        tryVerify(function() { return !findChild(table, "customHeader"); }, 1000);
    }

    function test_headerDelegate() {
        const table = make({headerDelegate: headerComponent});
        verify(table);
        verify(waitForRendering(table));
        const header = findChild(table, "customHeader");
        verify(header);
        compare(header.marker, "custom");
        compare(header.display, "Name");
    }
}
