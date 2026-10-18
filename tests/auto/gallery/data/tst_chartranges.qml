// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryChartRanges"
    when: windowShown
    property var gallery
    property var preview
    property var chart
    property var xSlider
    property var ySlider
    Component { id: factory; Main { width: 1200; height: 840 } }
    function initTestCase() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        gallery = factory.createObject(test);
    }
    function init() { failOnWarning(/.*/); }
    function cleanupTestCase() { gallery.close(); gallery.destroy(); }
    function find(name, parent) {
        const item = findChild(parent || gallery.contentItem, name);
        verify(item !== null, name);
        return item;
    }
    function choose(mode) {
        gallery.showChartDemo(mode);
        preview = mode === 16 ? find("chartAdvancedPreview") : find("chartPreview");
        chart = find(mode === 16 ? "advancedChart" : "demoChart", preview);
        chart.resetView();
        xSlider = find("chartXRangeSlider", preview);
        ySlider = find("chartYRangeSlider", preview);
        tryVerify(() => xSlider.to > 1);
        wait(10);
    }
    function closeTo(a, b) { verify(Math.abs(a - b) <= Math.max(1, Math.abs(a), Math.abs(b)) * 1e-8, a + " != " + b); }
    function verifySync() {
        const r = chart.plotRect();
        const a = chart.mapToValue(Qt.point(r.x, r.y));
        const b = chart.mapToValue(Qt.point(r.x + r.width, r.y + r.height));
        closeTo(xSlider.first.value, Math.min(a.x, b.x));
        closeTo(xSlider.second.value, Math.max(a.x, b.x));
        closeTo(ySlider.first.value, Math.min(a.y, b.y));
        closeTo(ySlider.second.value, Math.max(a.y, b.y));
    }
    function reveal(item) {
        wait(10);
        const flick = find("galleryContentScroll").contentItem;
        flick.contentY = Math.max(0, Math.min(flick.contentHeight - flick.height,
            flick.contentY + item.mapToItem(flick, 0, 0).y - 100));
        wait(10);
    }
    function test_twoWay_data() { return [{tag: "line", mode: 0}, {tag: "segmented", mode: 16}]; }
    function test_twoWay(data) {
        choose(data.mode);
        verifySync();
        const count = chart.series[0].count;
        const xmin = xSlider.from, xmax = xSlider.to, ymin = ySlider.from, ymax = ySlider.to;
        xSlider.first.value = xmin + (xmax - xmin) * .2;
        xSlider.first.moved();
        compare(chart.xAxis.automatic, false);
        compare(chart.yAxis.automatic, true);
        closeTo(chart.xAxis.minimum, xSlider.first.value);
        ySlider.second.value = ymin + (ymax - ymin) * .7;
        ySlider.second.moved();
        compare(chart.yAxis.automatic, false);
        wait(10); verifySync();
        chart.panBy(Qt.point(40, -20));
        wait(10); verifySync();
        const r = chart.plotRect();
        chart.zoomAt(1.5, Qt.point(r.x + r.width / 2, r.y + r.height / 2));
        wait(10); verifySync();
        chart.xAxis.reversed = true; chart.yAxis.reversed = true;
        wait(10); verifySync();
        chart.setViewRange(xmin - 100, xmax + 200, ymin - 30, ymax + 40);
        wait(10); verifySync();
        verify(xSlider.from <= xmin - 100 && xSlider.to >= xmax + 200);
        find("chartRangeReset", preview).clicked();
        tryCompare(chart.xAxis, "automatic", true);
        tryCompare(chart.yAxis, "automatic", true);
        wait(10); verifySync();
        closeTo(xSlider.from, xmin); closeTo(xSlider.to, xmax);
        compare(chart.series[0].count, count);
        chart.xAxis.reversed = false; chart.yAxis.reversed = false;
    }
    function test_handlesAndKeyboard() {
        choose(0);
        reveal(xSlider);
        const handle = xSlider.first.handle;
        mouseDrag(xSlider, handle.x + handle.width / 2, handle.y + handle.height / 2, xSlider.width * .25, 0);
        verify(chart.xAxis.minimum > xSlider.from);
        compare(chart.yAxis.automatic, true);
        const old = chart.xAxis.minimum;
        xSlider.first.handle.forceActiveFocus();
        keyClick(Qt.Key_Right);
        verify(chart.xAxis.minimum > old);
        wait(10); verifySync();
        ySlider.first.value = ySlider.second.value;
        ySlider.first.moved();
        verify(chart.yAxis.minimum < chart.yAxis.maximum);
        wait(10); verifySync();
        const low = chart.xAxis.minimum;
        GalleryLanguage.language = "zh_CN";
        wait(10);
        closeTo(chart.xAxis.minimum, low);
        verifySync();
        GalleryLanguage.language = "en";
    }
    function test_panKeepsSliderGeometry_data() {
        return [{tag: "desktop", width: 1200, language: "en"},
                {tag: "narrow", width: 420, language: "zh_CN"}];
    }
    function test_panKeepsSliderGeometry(data) {
        gallery.width = data.width;
        GalleryLanguage.language = data.language;
        choose(0);
        find("chartCurveMarkers").click();
        chart.setViewRange(1000, 99000, -11, 11);
        wait(100);
        const geometry = item => {
            const p = item.mapToItem(preview, 0, 0);
            return [p.x, p.y, item.width, item.height];
        };
        const controls = [xSlider, ySlider, find("chartMarkerSize"), chart];
        const before = controls.map(geometry);
        for (let step = 0; step < 24; ++step) {
            chart.panBy(Qt.point(step < 12 ? 7 : -7, step % 2 ? 3 : -3));
            wait(10);
            verifySync();
            for (let i = 0; i < controls.length; ++i) {
                const after = geometry(controls[i]);
                for (let k = 0; k < after.length; ++k)
                    fuzzyCompare(after[k], before[i][k], 0.01, controls[i].objectName + " geometry[" + k + "]");
            }
        }
        gallery.width = 1200; GalleryLanguage.language = "en";
    }
    function test_emptyAndStreaming() {
        choose(0);
        find("chartClearButton").clicked();
        tryCompare(xSlider, "enabled", false);
        tryCompare(ySlider, "enabled", false);
        find("chartAddButton").clicked();
        tryCompare(xSlider, "enabled", true);
        wait(10); verifySync();
        chart.setViewRange(10, 20, -2, 2);
        find("chartAddButton").clicked();
        wait(10);
        compare(chart.xAxis.minimum, 10); compare(chart.xAxis.maximum, 20);
        verifySync();
        find("chartRestoreButton").clicked();
        wait(10); verifySync();
        verify(xSlider.to > 90000);
        chart.series[0].visible = false; chart.series[1].visible = false;
        tryCompare(xSlider, "enabled", false);
        chart.series[0].visible = true;
        tryCompare(xSlider, "enabled", true);
        chart.series[1].visible = true;
    }
    function test_copyableBinding_data() { return test_twoWay_data(); }
    function test_copyableBinding(data) {
        choose(data.mode);
        const example = Qt.createQmlObject(find("codeSample").text, test, "axis_range_example.qml");
        verify(example !== null);
        tryVerify(() => findChild(example, "xRange").to > 1);
        const x = findChild(example, "xRange"), y = findChild(example, "yRange");
        x.first.value = 1; x.first.moved();
        compare(example.chart.xAxis.minimum, 1);
        compare(example.chart.yAxis.automatic, true);
        example.chart.setViewRange(-1, 5, -10, 20);
        tryCompare(x.first, "value", -1);
        compare(x.second.value, 5); compare(y.first.value, -10); compare(y.second.value, 20);
        example.destroy();
    }
}
