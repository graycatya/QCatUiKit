// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryDataExamples"
    when: windowShown
    property var gallery
    Component { id: factory; Main { width: 1200; height: 1000; initialPage: "table" } }
    function initTestCase() {
        failOnWarning(/.*/);
        gallery = factory.createObject(test);
        verify(waitForRendering(gallery.contentItem));
    }
    function init() {
        failOnWarning(/.*/);
        gallery.requestActivate();
        tryCompare(gallery, "active", true);
        tryVerify(() => gallery.activeFocusItem !== null);
    }
    function cleanup() {
        const preview = findChild(gallery.contentItem, "dataPreview");
        if (preview) preview.dataController.stopStress();
    }
    function cleanupTestCase() { gallery.close(); gallery.destroy(); GalleryLanguage.language = "en"; }
    function find(name) {
        const item = findChild(gallery.contentItem, name);
        verify(item !== null, name);
        return item;
    }
    function choose(page) {
        gallery.showComponent(page);
        // Re-selecting the initial page still needs a frame for the render wait.
        gallery.update();
        verify(waitForRendering(gallery.contentItem));
        find("dataRestoreButton").clicked();
        return find("dataPreview").dataController;
    }
    function test_operations_data() {
        const cases = [];
        for (const language of ["en", "zh_CN"])
            for (const page of ["table", "chat", "chatinput"])
                cases.push({tag:language + "_" + page, language:language, page:page});
        return cases;
    }
    function test_parametersWithoutReturn_data() {
        return ["table", "chat", "chatinput"].map(page=>({tag:page,page:page}));
    }
    function test_parametersWithoutReturn(data) {
        const demo=choose(data.page);
        const edits=[{name:"dataBatchSize",property:"batchSize",text:"7",value:7},
            {name:"dataCapacity",property:"capacity",text:"5000",value:5000},
            {name:"dataInterval",property:"interval",text:"50",value:50}];
        for(const edit of edits) {
            const input=find(edit.name).contentItem;
            input.forceActiveFocus();input.selectAll();
            for(const character of edit.text)keyClick(character);
            find("dataStressButton").forceActiveFocus();
            compare(demo[edit.property],edit.value);
        }
    }
    function test_operations(data) {
        GalleryLanguage.language = data.language;
        const demo = choose(data.page);
        const initial = demo.count;
        compare(demo.capacity, data.page === "table" ? 100000 : 10000);
        demo.batchSize = 3;
        if (data.page === "chatinput") {
            const input = find("demoChatInput");
            find("composerWhitespace").clicked();
            input.submit();
            compare(demo.count, initial);
            find("composerLongText").clicked();
            compare(input.text.length, 2048);
            input.submit();
            compare(demo.count, initial + 1);
            compare(demo.chatModel.get(initial).content.length, 2048);
            compare(input.text, "");
        } else {
            find("dataAddButton").clicked();
            compare(demo.count, initial + 3);
            find("dataUpdateButton").clicked();
            compare(demo.changed, 3);
            find("dataRemoveButton").clicked();
            compare(demo.count, initial);
        }
        find("dataClearButton").clicked();
        compare(demo.count, 0);
        find("dataRestoreButton").clicked();
        compare(demo.count, initial);
        find("dataLargePreset").clicked();
        compare(demo.capacity, data.page === "table" ? 100000 : 10000);
        compare(demo.running, false);
        compare(demo.count, initial);
    }
    function test_tableEditingAndLocation() {
        const demo = choose("table");
        verify(waitForRendering(find("demoTableView")));
        mouseClick(find("demoTableView").view, 60, Theme.tokens.controlHeight * 1.5);
        compare(demo.selectedIndex, 1);
        const row = find("tableRowSelector");
        row.value = 2; row.valueModified();
        compare(demo.selectedIndex, 1);
        compare(find("demoTableView").view.currentRow, 1);
        find("tableColumnPicker").currentIndex = 0;
        find("tableCellEditor").text = "Edited cell / 编辑内容";
        find("tableApplyCell").clicked();
        compare(demo.tableModel.get(1).name, "Edited cell / 编辑内容");
        GalleryLanguage.language = "zh_CN";
        compare(demo.tableModel.get(1).name, "Edited cell / 编辑内容");
        find("dataRemoveSelected").clicked();
        compare(demo.count, 2);
        const view = find("demoTableView").view;
        view.setColumnWidth(1, 220);
        compare(view.explicitColumnWidth(1), 220);
    }
    function test_tableFreezeControls() {
        const demo = choose("table");
        verify(waitForRendering(find("demoTableView")));
        const table = find("demoTableView");
        const columns = find("tableFrozenColumns");
        const rows = find("tableFrozenRows");
        columns.value = 1;
        columns.valueModified();
        compare(table.frozenColumnCount, 1);
        rows.value = 2;
        rows.valueModified();
        compare(table.frozenRowCount, 2);
        find("tableClearFreeze").clicked();
        compare(table.frozenColumnCount, 0);
        compare(table.frozenRowCount, 0);
        find("tableFreezeFirst").clicked();
        compare(table.frozenColumnCount, 1);
        compare(table.frozenRowCount, 1);
        compare(columns.value, 1);
        compare(rows.value, 1);
        compare(demo.count, 3);
    }

    function test_tableRtl() {
        choose("table");
        verify(waitForRendering(find("demoTableView")));
        const table = find("demoTableView");
        const toggle = find("tableRtlToggle");
        table.freezeTo(0, 0);
        toggle.checked = true;
        toggle.toggled();
        compare(table.LayoutMirroring.enabled, true);
        verify(table.view.model !== table.model);
        compare(table.view.x, 0);
        const frozen = findChild(table, "frozenBottomLeftView");
        verify(frozen);
        compare(frozen.x, table.width - table.__frozenWidth);
        toggle.checked = false;
        toggle.toggled();
        compare(table.LayoutMirroring.enabled, false);
        verify(table.view.model === table.model);
        table.clearFreeze();
    }

    function test_tableRtlLargePreset() {
        const demo = choose("table");
        verify(waitForRendering(find("demoTableView")));
        const table = find("demoTableView");
        table.LayoutMirroring.enabled = false;
        table.clearFreeze();
        demo.tableModel.clear();
        const preset = find("tableRtlLargePreset");
        verify(preset);
        preset.clicked();
        compare(table.LayoutMirroring.enabled, true);
        compare(find("tableRtlToggle").checked, true);
        tryVerify(function() { return table.view.rows > 0; }, 5000);
        find("dataStressButton").clicked();
        table.LayoutMirroring.enabled = false;
        demo.tableModel.clear();
    }

    function test_tableSortResize() {
        choose("table");
        const table = find("demoTableView");
        const headerToggle = find("tableCustomHeader");
        headerToggle.checked = true;
        headerToggle.toggled();
        wait(50);
        const handle = findChild(table, "tableSortResizeHandle");
        verify(handle);
        const column = handle.parent.column;
        const before = table.view.columnWidth(column);
        mouseDrag(handle, 3, handle.height / 2, 40, 0);
        tryVerify(function() { return table.view.columnWidth(column) > before; }, 1000);
        headerToggle.checked = false;
        headerToggle.toggled();
    }

    function test_tableSorting() {
        const demo = choose("table");
        const table = find("demoTableView");
        const headerToggle = find("tableCustomHeader");
        headerToggle.checked = true;
        headerToggle.toggled();
        const header = find("plainHeader");
        const customHeader = findChild(table, "tableCustomHeaderCell");
        verify(customHeader);
        compare(customHeader.Accessible.role, Accessible.ColumnHeader);
        compare(customHeader.Accessible.name, String(customHeader.display));
        const sortModel = table.model.sourceTableModel;
        verify(sortModel);
        compare(sortModel.sortColumn, -1);
        mouseClick(header, 20, header.height / 2);
        tryVerify(function() { return sortModel.sortColumn === 0; }, 1000);
        compare(sortModel.sortOrder, Qt.AscendingOrder);
        const ascending = sortModel.displayAt(0, 0);
        const indicator = findChild(table, "tableSortIndicator");
        verify(indicator);
        compare(indicator.text, "▲");
        tryVerify(function() {
            return customHeader.Accessible.description.length > 0;
        }, 1000);
        const ascendingDescription = customHeader.Accessible.description;
        mouseClick(header, 20, header.height / 2);
        tryVerify(function() { return sortModel.sortOrder === Qt.DescendingOrder; }, 1000);
        tryVerify(function() {
            return customHeader.Accessible.description !== ascendingDescription;
        }, 1000);
        const descending = sortModel.displayAt(0, 0);
        verify(ascending !== descending);
        sortModel.clearSort();
        table.LayoutMirroring.enabled = true;
        wait(50);
        mouseClick(header, header.width - 20, header.height / 2);
        tryVerify(function() { return sortModel.sortColumn === 0; }, 1000);
        sortModel.clearSort();
        table.LayoutMirroring.enabled = false;
        table.freezeTo(0, 0);
        wait(50);
        const frozenHeader = find("frozenHeader");
        mouseClick(frozenHeader, 70, frozenHeader.height / 2);
        tryVerify(function() { return sortModel.sortColumn === 0; }, 1000);
        table.clearFreeze();
        headerToggle.checked = false;
        headerToggle.toggled();
        demo.resetData();
    }

    function test_tableColumnControls() {
        const demo = choose("table");
        const table = find("demoTableView");
        const columns = table.model;
        verify(columns);
        compare(columns.visibleColumnCount, 4);

        columns.setColumnVisible(0, false);
        tryVerify(function() { return table.view.columns === 3; }, 1000);
        columns.setColumnVisible(0, true);
        tryVerify(function() { return table.view.columns === 4; }, 1000);

        columns.moveColumn(0, 1);
        compare(columns.visibleColumns[0], 1);
        columns.resetColumns();
        compare(columns.visibleColumns[0], 0);

        const headerToggle = find("tableCustomHeader");
        headerToggle.checked = true;
        headerToggle.toggled();
        columns.moveColumn(0, 1);
        wait(50);
        const sortModel = columns.sourceTableModel;
        const header = find("plainHeader");
        mouseClick(header, 20, header.height / 2);
        tryVerify(function() { return sortModel.sortColumn === 1; }, 1000);
        sortModel.clearSort();
        columns.resetColumns();
        headerToggle.checked = false;
        headerToggle.toggled();
        demo.resetData();
    }

    function test_tableCustomization() {
        const demo = choose("table");
        verify(waitForRendering(find("demoTableView")));
        const table = find("demoTableView");
        const headerToggle = find("tableCustomHeader");
        headerToggle.checked = true;
        headerToggle.toggled();
        tryVerify(function() { return find("tableCustomHeaderCell"); }, 1000);
        const toolbarToggle = find("tableToolbar");
        toolbarToggle.checked = true;
        toolbarToggle.toggled();
        tryVerify(function() { return find("tableCustomToolbar"); }, 1000);
        const loadingToggle = find("tableLoading");
        loadingToggle.checked = true;
        loadingToggle.toggled();
        tryVerify(function() {
            return find("tableStateOverlay").visible && find("tableLoadingState");
        }, 1000);
        loadingToggle.checked = false;
        loadingToggle.toggled();
        tryVerify(function() { return !find("tableStateOverlay").visible; }, 1000);
        find("tableEmptyDemo").clicked();
        tryVerify(function() {
            return find("tableStateOverlay").visible && find("tableEmptyState");
        }, 1000);
        find("tableRestoreRows").clicked();
        tryVerify(function() { return table.view.rows > 0; }, 1000);
        headerToggle.checked = false;
        headerToggle.toggled();
        toolbarToggle.checked = false;
        toolbarToggle.toggled();
        demo.resetData();
    }

    function test_chatStatusesAttachmentsAndTail() {
        const demo = choose("chat");
        for (let type = 0; type < 4; type++) {
            find("messageTypePicker").currentIndex = type;
            find("chatReceiveButton").clicked();
            compare(demo.selectedRecord.type, type);
            compare(demo.selectedRecord.sender, "peer");
        }
        find("chatStatusPicker").activated(2);
        compare(demo.selectedRecord.status, ChatModel.Failed);
        const follow = find("chatFollowTail");
        follow.checked = false; follow.toggled();
        const view = find("demoChatView");
        compare(view.followTail, false);
        view.positionViewAtBeginning();
        const historyPosition = view.contentY;
        const draft = find("demoChatInput");
        draft.text = "Unsent draft";
        demo.interval = 16; demo.batchSize = 5;
        find("dataTopStressButton").clicked();
        tryVerify(() => demo.batches >= 3, 2000);
        fuzzyCompare(view.contentY, historyPosition, 1);
        compare(draft.text, "Unsent draft");
        draft.submit();
        compare(demo.chatModel.get(demo.count - 1).content, "Unsent draft");
        find("dataStressButton").clicked();
        compare(demo.running, false);
    }
    function test_continuousAndStop_data() {
        return ["table", "chat", "chatinput"].map(page => ({tag:page,page:page}));
    }
    function test_continuousAndStop(data) {
        const demo = choose(data.page);
        demo.capacity = 30; demo.batchSize = 5; demo.interval = 16; demo.workload = 1;
        const input = find("demoChatInput");
        input.text = "Restore this draft";
        find("dataStressButton").clicked();
        tryVerify(() => demo.batches >= 8, 2500);
        verify(demo.count <= 30);
        if (data.page === "chatinput") verify(demo.rejected > 0);
        else { verify(demo.changed > 0); verify(demo.removed > 0); }
        find("dataTopStressButton").clicked();
        compare(demo.running, false);
        compare(input.text, "Restore this draft");
        const stopped = [demo.count,demo.batches,demo.elapsedSeconds];
        wait(80);
        compare([demo.count,demo.batches,demo.elapsedSeconds], stopped);
        demo.startStress();
        gallery.hide();
        tryCompare(demo,"running",false);
        gallery.show();
        verify(waitForRendering(gallery.contentItem));
        compare(demo.running,false);
        demo.startStress();
        gallery.showComponent("button");
        verify(waitForRendering(gallery.contentItem));
        gallery.showComponent(data.page);
        verify(waitForRendering(gallery.contentItem));
        compare(find("dataPreview").dataController.running,false);
    }
}
