// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryChartExport"
    when: windowShown
    property var gallery
    Component { id: factory; Main { width: 1200; height: 900 } }
    Image { id: exported; visible: false; cache: false }
    function init() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        gallery = factory.createObject(test);
    }
    function cleanup() {
        exported.source = "";
        gallery.close(); gallery.destroy();
        Theme.density = ThemePalette.Desktop;
    }
    function test_chartTargets_data() {
        return [0, 15, 17, 18, 19, 20, 21, 22, 23, 26].map(mode => ({tag: String(mode), mode: mode}));
    }
    function test_chartTargets(data) {
        verify(gallery.showChartDemo(data.mode));
        const controls = findChild(gallery.contentItem, "chartExportControls");
        verify(controls !== null);
        tryVerify(() => controls.chart !== null && controls.chart.width > 0);
        verify(waitForRendering(gallery.contentItem));
        const preview = findChild(gallery.contentItem, "chartPreview");
        compare(controls.chart, preview.exportChart);
        verify(!controls.saveImage("https://example.org/chart.png"));
        verify(controls.failed);
        verify(controls.statusText.indexOf("Export failed:") === 0);
        if (data.mode !== 19) {
            controls.doubleSize = true;
            const destination = chartExportTestDirectory + "gallery-" + data.mode + ".png";
            const expectedWidth = Math.ceil(controls.chart.width * 2);
            const expectedHeight = Math.ceil(controls.chart.height * 2);
            verify(controls.saveImage(destination));
            verify(!findChild(controls, "chartExportButton").enabled);
            tryVerify(() => !controls.chart.exportingImage);
            verify(!controls.failed, controls.statusText);
            exported.source = destination;
            tryCompare(exported, "status", Image.Ready);
            compare(exported.sourceSize.width, expectedWidth);
            compare(exported.sourceSize.height, expectedHeight);
        }
        GalleryLanguage.language = "zh_CN";
        gallery.width = 420;
        Theme.density = ThemePalette.Mobile;
        wait(30);
        verify(controls.width <= preview.width);
        verify(findChild(controls, "chartExportButton").text.indexOf("PNG") >= 0);
        verify(!controls.saveImage("https://example.org/chart.png"));
        verify(controls.statusText.indexOf("导出失败") === 0);
    }
}
