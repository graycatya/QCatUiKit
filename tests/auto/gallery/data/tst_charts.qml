// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryCharts"
    when: windowShown
    property var gallery
    Component { id: factory; Main { width: 1200; height: 840 } }
    function initTestCase() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        gallery = factory.createObject(test);
        gallery.showComponent("chart");
        verify(waitForRendering(gallery.contentItem));
    }
    function init() { failOnWarning(/.*/); }
    function cleanup() {
        const preview = findChild(gallery.contentItem, "chartPreview");
        if (preview) preview.dataController.stopStress();
    }
    function cleanupTestCase() { gallery.close(); gallery.destroy(); GalleryLanguage.language = "en"; }
    function find(name) {
        const object = findChild(gallery.contentItem, name);
        verify(object !== null, name);
        return object;
    }
    function choose(mode) {
        const picker = find("chartDemoPicker"), previous = picker.currentIndex;
        picker.activated(mode);
        if (previous !== mode) verify(waitForRendering(gallery.contentItem));
        else { find("chartRestoreButton").clicked(); wait(10); }
    }
    function test_modes_data() {
        const rows = [];
        for (const language of ["en", "zh_CN"])
            for (let mode = 0; mode < 15; mode++)
                rows.push({tag: language + "_" + mode, language: language, mode: mode});
        return rows;
    }
    function test_modes(data) {
        GalleryLanguage.language = data.language;
        choose(data.mode);
        const chart = find("demoChart"), first = find("demoChart").series[0];
        verify(first.count > 0);
        compare(chart.antialiasing, true);
        compare(chart.chartType, data.mode <= 9 ? data.mode : ({10: 2, 11: 0, 12: 0, 13: 3, 14: 4})[data.mode]);
        compare(find("chartDemoPicker").currentIndex, data.mode);
        if (data.mode === 0) {
            compare(first.count, 100000);
            compare(first.name, data.language === "en" ? "Signal A" : "信号 A");
        }
        if (data.mode === 11) compare(chart.xAxis.scale, CatChartAxis.DateTime);
        if (data.mode === 12) compare(chart.yAxis.scale, CatChartAxis.Logarithmic);
    }
    function test_languageKeepsDataVisibilityAndView() {
        GalleryLanguage.language = "en";
        choose(0);
        const chart = find("demoChart"), first = find("demoChart").series[0];
        verify(first.append(100000, 3));
        verify(chart.setViewRange(500, 1500, -12, 12));
        first.visible = false;
        GalleryLanguage.language = "zh_CN";
        verify(waitForRendering(gallery.contentItem));
        compare(find("demoChart"), chart);
        compare(first.name, "信号 A");
        compare(first.at(first.count - 1).x, 100000);
        compare(first.visible, false);
        compare(chart.xAxis.automatic, false);
        compare(chart.xAxis.minimum, 500);
        compare(chart.xAxis.maximum, 1500);
        GalleryLanguage.language = "en";
        compare(first.name, "Signal A");
        compare(chart.xAxis.minimum, 500);
        first.visible = true;
        chart.resetView();
    }
    function test_dataEditing_data() { return test_modes_data(); }
    function test_parameterEditing() {
        GalleryLanguage.language = "en";
        choose(2);
        const demo = find("chartPreview").dataController;
        const edits = [
            {name: "chartBatchSize", keys: [Qt.Key_3], property: "batchSize", value: 3},
            {name: "chartCapacity", keys: [Qt.Key_1, Qt.Key_2], property: "capacity", value: 12},
            {name: "chartInterval", keys: [Qt.Key_5, Qt.Key_0], property: "interval", value: 50}
        ];
        for (const edit of edits) {
            const field = find(edit.name);
            field.contentItem.forceActiveFocus();
            field.contentItem.selectAll();
            for (const key of edit.keys) keyClick(key);
            find("chartAddButton").forceActiveFocus();
            compare(demo[edit.property], edit.value);
        }
        find("chartAddButton").clicked();
        compare(demo.unitCount, 9);
        find("chartRemoveButton").clicked();
        compare(demo.unitCount, 6);
    }
    function test_dataEditing(data) {
        GalleryLanguage.language = data.language;
        choose(data.mode);
        const chart = find("demoChart"), demo = find("chartPreview").dataController;
        const initial = demo.unitCount, batch = demo.batchSize;
        const units = data.mode === 9 ? 40 : 1;
        find("chartRemoveButton").clicked();
        compare(demo.unitCount, initial - batch);
        compare(chart.series[0].count, (initial - batch) * units);
        find("chartAddButton").clicked();
        compare(demo.unitCount, initial);
        find("chartClearButton").clicked();
        compare(demo.sampleCount, 0);
        compare(chart.series[0].count, 0);
        compare(chart.series[1].count, 0);
        compare(find("chartRemoveButton").enabled, false);
        demo.batchSize = 3;
        find("chartAddButton").clicked();
        compare(demo.unitCount, 3);
        compare(chart.series[0].count, 3 * units);
        if (![6, 7, 9, 14].includes(data.mode)) compare(chart.series[1].count, 3);
        if (data.mode === 14) compare(chart.series[1].count, 2);
        verify(waitForRendering(chart));
        find("chartRestoreButton").clicked();
        compare(demo.unitCount, initial);
        compare(demo.batchSize, batch);
    }
    function test_capacityAndMetadata_data() {
        return Array.from({length: 15}, (_, mode) => ({tag: String(mode), mode: mode}));
    }
    function test_capacityAndMetadata(data) {
        GalleryLanguage.language = "en";
        choose(data.mode);
        const chart = find("demoChart"), demo = find("chartPreview").dataController;
        demo.capacity = 3;
        demo.batchSize = 2;
        compare(demo.unitCount, 3);
        find("chartAddButton").clicked();
        compare(demo.unitCount, 3);
        const first = chart.series[0];
        if ([2, 3, 8, 10, 13].includes(data.mode)) {
            compare(chart.xAxis.categories.length, 3);
            for (let i = 0; i < 3; i++) compare(first.at(i).x, i);
            verify(chart.xAxis.categories[2].startsWith(data.mode === 8 ? "Dimension" : "Category"));
        }
        if (data.mode === 9) {
            compare(first.count, 120);
            for (let i = 0; i < first.count; i++) compare(first.at(i).y, i % 40);
        }
        if (data.mode === 11) compare(first.at(2).x - first.at(1).x, 900000);
        if (data.mode === 12) {
            for (let i = 0; i < first.count; i++) verify(first.at(i).y > 0);
        }
        if (data.mode === 14) {
            compare(chart.series[1].count, 2);
            compare(chart.series[1].at(0).x, first.at(0).x);
            compare(chart.series[1].at(1).x, first.at(2).x);
        }
        const value = first.at(first.count - 1);
        GalleryLanguage.language = "zh_CN";
        compare(first.at(first.count - 1).x, value.x);
        compare(first.at(first.count - 1).y, value.y);
        compare(demo.capacity, 3);
        if ([2, 3, 8, 10, 13].includes(data.mode))
            verify(chart.xAxis.categories[2].startsWith(data.mode === 8 ? "维度" : "类别"));
        if ([5, 6, 7].includes(data.mode))
            verify(first.at(first.count - 1).label !== value.label);
        find("chartRemoveButton").clicked();
        compare(demo.unitCount, 1);
        find("chartRemoveButton").clicked();
        compare(demo.unitCount, 0);
        compare(demo.sampleCount, 0);
        find("chartAddButton").clicked();
        compare(demo.unitCount, 2);
        verify(waitForRendering(chart));
    }
    function test_stress_data() { return test_capacityAndMetadata_data(); }
    function test_largeCumulativeCounts_data() {
        return [{tag: "en", language: "en"}, {tag: "zh_CN", language: "zh_CN"}];
    }
    function test_largeCumulativeCounts(data) {
        GalleryLanguage.language = data.language;
        choose(4);
        const demo = find("chartPreview").dataController;
        demo.capacity = 900000;
        demo.batchSize = 25000;
        demo.clearData();
        demo.startStress();
        // Exercise the actual qint64 counters without waiting for 66 timer ticks.
        for (let i = 0; i < 66; i++) demo.addData();
        demo.stopStress();
        compare(demo.added, 3300000);
        compare(demo.removed, 1500000);
        compare(demo.sampleCount, 1800000);
        const statistics = find("chartStressStatistics").text;
        verify(statistics.includes("3,300,000"), statistics);
        verify(statistics.includes("1,500,000"), statistics);
        verify(!/[0-9][eE][+-][0-9]/.test(statistics), statistics);
    }
    function test_millionPreset_data() { return test_capacityAndMetadata_data(); }
    function test_millionPreset(data) {
        choose(data.mode);
        const demo = find("chartPreview").dataController;
        const initial = demo.sampleCount;
        find("chartMillionPreset").clicked();
        const samplesPerUnit = data.mode === 9 ? 40 : [6, 7, 14].includes(data.mode) ? 1 : 2;
        compare(demo.capacity * samplesPerUnit, 1000000);
        compare(demo.batchSize * samplesPerUnit, 50000);
        compare(demo.interval, 200);
        compare(demo.running, false);
        compare(demo.sampleCount, initial);
        compare(find("demoChart").antialiasing, true);
        compare(find("chartCapacity").to, demo.maximumCapacity);
        compare(find("chartBatchSize").to, demo.maximumBatchSize);
        verify(demo.maximumCapacity * (data.mode === 9 ? 40 : 1) >= 1000000);
        demo.addData();
        compare(demo.aggregated, true);
        compare(find("demoChart").antialiasing, true);
    }
    function test_millionOverviewAndStop() {
        GalleryLanguage.language = "en";
        choose(6);
        const demo = find("chartPreview").dataController;
        const capacity = find("chartCapacity");
        capacity.contentItem.forceActiveFocus();
        keyClick(Qt.Key_A, Qt.ControlModifier);
        keyClick(Qt.Key_1);
        for (let i = 0; i < 6; i++) keyClick(Qt.Key_0);
        keyClick(Qt.Key_Return);
        compare(demo.capacity, 1000000);
        find("chartMillionPreset").clicked();
        find("chartClearButton").clicked();
        for (let i = 0; i < 20; i++) demo.addData();
        compare(demo.sampleCount, 1000000);
        compare(demo.displayedSampleCount, 64);
        compare(demo.aggregated, true);
        compare(find("demoChart").antialiasing, true);
        verify(demo.sourceAt(0, 999999).y > 0);
        const summary = find("chartOverviewSummary");
        compare(summary.visible, true);
        verify(summary.text.includes("1,000,000"));
        verify(summary.text.includes("sum"));
        verify(waitForRendering(find("demoChart")));
        GalleryLanguage.language = "zh_CN";
        verify(summary.text.includes("求和"));
        compare(demo.sampleCount, 1000000);
        find("chartStressButton").clicked();
        tryVerify(() => demo.updates > 0, 3000);
        compare(find("demoChart").antialiasing, true);
        find("chartStressPanelButton").clicked();
        compare(demo.running, false);
        const stopped = demo.updates;
        wait(250);
        compare(demo.updates, stopped);
        compare(demo.sampleCount, 1000000);
        find("chartRestoreButton").clicked();
        compare(demo.aggregated, false);
        compare(summary.visible, false);
    }
    function test_stress(data) {
        choose(data.mode);
        const demo = find("chartPreview").dataController, button = find("chartStressButton");
        demo.capacity = Math.min(20, demo.maximumCapacity);
        demo.batchSize = 3;
        demo.interval = 16;
        button.clicked();
        compare(demo.running, true);
        compare(find("chartAddButton").enabled, false);
        compare(find("chartRemoveButton").enabled, false);
        compare(find("chartBatchSize").enabled, false);
        tryVerify(() => demo.updates >= 6, 3000);
        verify(demo.added > 0);
        verify(demo.removed > 0);
        verify(demo.unitCount <= demo.capacity);
        verify(demo.elapsedSeconds > 0);
        verify(demo.updatesPerSecond > 0);
        verify(demo.maximumBatchMs > 0);
        button.clicked();
        compare(demo.running, false);
        const stopped = [demo.updates, demo.sampleCount, demo.added, demo.removed, demo.elapsedSeconds];
        wait(80);
        compare([demo.updates, demo.sampleCount, demo.added, demo.removed, demo.elapsedSeconds], stopped);
        compare(find("chartAddButton").enabled, true);
        button.clicked();
        compare(demo.updates, 0);
        find("chartRestoreButton").clicked();
        compare(demo.running, false);
        compare(demo.updates, 0);
    }
    function test_stressStopsOnModeSwitchAndHide() {
        choose(0);
        const demo = find("chartPreview").dataController;
        demo.interval = 16;
        demo.startStress();
        tryVerify(() => demo.updates > 0, 1000);
        choose(2);
        compare(demo.running, false);
        compare(demo.unitCount, 6);
        wait(80);
        compare(demo.unitCount, 6);
        demo.startStress();
        tryVerify(() => demo.updates > 0, 1000);
        gallery.hide();
        tryCompare(demo, "running", false);
        const stopped = demo.updates;
        gallery.show();
        verify(waitForRendering(gallery.contentItem));
        wait(80);
        compare(demo.updates, stopped);
        demo.startStress();
        gallery.showComponent("button");
        verify(waitForRendering(gallery.contentItem));
        gallery.showComponent("chart");
        verify(waitForRendering(gallery.contentItem));
        compare(find("chartPreview").dataController.running, false);
        compare(find("chartPreview").dataController.updates, 0);
    }
}
