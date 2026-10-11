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
    Component { id: factory; Main { width: 1200; height: 840; font.family: "Segoe UI" } }
    function initTestCase() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        gallery = factory.createObject(test);
        gallery.showComponent("chart");
        verify(waitForRendering(gallery.contentItem));
    }
    function init() { failOnWarning(/.*/); }
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
        else wait(10);
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
    function test_liveAndModeSwitch() {
        choose(1); choose(0);
        const first = find("demoChart").series[0], button = find("chartLiveButton");
        const before = first.at(first.count - 1).x;
        button.clicked();
        tryVerify(() => first.at(first.count - 1).x > before, 1000);
        button.clicked();
        const paused = first.at(first.count - 1).x;
        wait(220);
        compare(first.at(first.count - 1).x, paused);
        button.clicked();
        choose(2);
        compare(first.count, 6);
        wait(220);
        compare(first.count, 6);
    }
}
