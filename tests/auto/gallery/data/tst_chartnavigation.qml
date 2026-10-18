// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryChartNavigation"
    when: windowShown
    property var gallery
    property var example
    property int exampleNumber: 0
    readonly property var exampleChart: example ? (example.chart || example) : null
    Component { id: factory; Main { width: 1200; height: 840 } }
    function initTestCase() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        gallery = factory.createObject(test);
        verify(waitForRendering(gallery.contentItem));
    }
    function init() { failOnWarning(/.*/); }
    function cleanup() {
        if (example) example.destroy();
        example = null;
        GalleryLanguage.language = "en";
    }
    function cleanupTestCase() { gallery.close(); gallery.destroy(); }
    function find(name) {
        const item = findChild(gallery.contentItem, name);
        verify(item !== null, name);
        return item;
    }
    function choose(mode) {
        verify(gallery.showChartDemo(mode));
        wait(1);
    }
    function compileExample() {
        const code = find("codeSample").text;
        verify(!code.includes("Gallery"));
        // Distinct snippets are distinct compilation units. Reusing a URL across unrelated
        // roots can retain Qt's inline-component type cache (for example AxisSlider).
        example = Qt.createQmlObject(code, test, "selected_chart_example_" + (++exampleNumber) + ".qml");
        verify(example !== null);
        example.visible = false;
        if (exampleChart.scene3D.enabled) {
            verify(exampleChart.scene3D.series[0].count > 0);
            compare(exampleChart.scene3D.series[0].error, "");
        } else tryVerify(() => exampleChart.series.length > 0 && exampleChart.series[0].count > 0);
        return code;
    }
    function test_catalogAndAliases() {
        const entries = GalleryCatalog.components.filter(entry => entry.category === "charts");
        compare(entries.length, 17);
        compare(new Set(entries.map(entry => entry.id)).size, 17);
        const modes = [].concat(...entries.map(entry => entry.modes)).sort((a, b) => a - b);
        compare(modes.length, 30);
        for (let i = 0; i < 30; ++i) compare(modes[i], i);
        for (const id of ["chart", "chart-axis", "chart-series"]) {
            verify(!entries.some(entry => entry.id === id));
            const entry = GalleryCatalog.componentForId(id);
            verify(entry !== null);
            gallery.showComponent(id);
            compare(gallery.componentId, entry.id);
        }
    }
    function test_groupedBrowserAndSearch() {
        gallery.navigate("components", "charts");
        verify(waitForRendering(gallery.contentItem));
        for (const entry of GalleryChartCatalog.entries) verify(find("chartEntry_" + entry.id));
        find("chartEntry_chart-area").clicked();
        compare(gallery.componentId, "chart-area");
        compare(find("chartDemoPicker").count, 2);
        compare(find("chartDemoPicker").currentValue, 1);
        find("chartDemoPicker").activated(1);
        compare(find("chartDemoPicker").currentValue, 15);
        verify(find("codeSample").text.includes("CatChartSeries.RangeArea"));
        gallery.searchQuery = "CatChartSeries3D";
        compare(gallery.matchingComponents.length, 1);
        compare(gallery.matchingComponents[0].id, "chart-3d");
    }
    function test_selectedExamples_data() {
        return Array.from({length: 30}, (_, mode) => ({tag: "mode_" + mode, mode: mode}));
    }
    function test_selectedExamples(data) {
        choose(data.mode);
        const entry = GalleryChartCatalog.entryForMode(data.mode);
        compare(gallery.componentId, entry.id);
        compare(find("chartDemoPicker").count, entry.modes.length);
        compare(find("chartDemoPicker").currentValue, data.mode);
        compare(find("chartDemoPicker").visible, entry.modes.length > 1);
        compileExample();
    }
    function test_liveStylesAndCopyFeedback() {
        choose(0);
        const before = find("codeSample").text;
        find("copyExampleButton").clicked();
        compare(find("copyExampleButton").copied, true);
        find("chartCurveStyle").activated(5);
        verify(find("codeSample").text !== before);
        verify(find("codeSample").text.includes("CatChartSeries.CatmullRomSpline"));
        compare(find("copyExampleButton").copied, false);
        compileExample();
        compare(exampleChart.series[0].curveMode, CatChartSeries.CatmullRomSpline);
        GalleryLanguage.language = "zh_CN";
        compare(find("chartDemoPicker").currentValue, 0);
        compare(find("chartPreview").curveChoice, 5);
        gallery.width = 420;
        verify(waitForRendering(gallery.contentItem));
        compare(find("chartPreview").curveChoice, 5);
        gallery.width = 1200;
    }
    function test_statisticsControlsAndExamples() {
        choose(20);
        const stack = find("chartStackPreview").dataAdapter;
        tryVerify(() => stack.series[0].count > 0);
        find("stackPercentage").click();
        find("stackSigned").click();
        find("stackGap").click();
        find("stackOpacity").valueEdited(80);
        tryCompare(stack, "percentage", true);
        compare(stack.error, "");
        verify(compileExample().includes("percentage: true"));
        compare(exampleChart.series[0].fillOpacity, .8);
        example.destroy(); example = null;
        stack.series[1].visible = false;
        tryCompare(find("stackSecond"), "checked", false);
        tryCompare(stack.series[1], "count", 0);
        find("stackSecond").click();
        tryVerify(() => stack.series[1].count > 0);

        choose(21);
        const histogram = find("histogramPreview").dataAdapter;
        compare(histogram.sampleCount, 300);
        find("histogramBins").valueEdited(8);
        compare(histogram.series.count, 8);
        find("histogramCustomEdges").click();
        compare(find("histogramBins").enabled, false);
        compare(histogram.series.count, 4);
        find("histogramOutliers").click();
        compare(histogram.sampleCount, 302);
        compare(histogram.includedCount, 300);
        compare(histogram.underflowCount, 1); compare(histogram.overflowCount, 1);
        find("histogramNormalization").activated(2);
        compare(histogram.normalization, CatHistogram.Density);
        find("histogramGap").valueEdited(12);
        const code = compileExample();
        verify(code.includes("CatHistogram.Density"));
        verify(code.includes("[0, 3, 4, 6, 10]"));
        compare(exampleChart.barSpacing, .12);
        compare(exampleChart.series[0].count, 4);
        compare(exampleChart.series[0].at(0).binUpper, 3);
        gallery.width = 420;
        verify(waitForRendering(gallery.contentItem));
        for (const name of ["histogramBins", "histogramNormalization", "histogramGap"])
            verify(find(name).width > 200 && find(name).width < 400, name);
        gallery.width = 1200;
    }
    function test_fitControlsAndExample() {
        choose(22);
        const fit = find("chartFitPreview").dataAdapter;
        tryCompare(fit, "valid", true);
        find("fitNoise").valueEdited(0);
        tryVerify(() => fit.rootMeanSquareError < 1e-12);
        find("fitNoise").valueEdited(4);
        find("fitMultiplier").valueEdited(1.5);
        tryVerify(() => fit.rootMeanSquareError > 1);
        const code = compileExample();
        verify(code.includes("CatChartFit")); verify(code.includes("residualMultiplier: 1.5"));
        example.destroy(); example = null;
        find("fitConstantX").click();
        tryCompare(fit, "valid", false); verify(fit.error.length > 0);
        compare(fit.series.count, 0); compare(fit.band.count, 0); compare(fit.source.count, 40);
        find("fitConstantX").click(); tryCompare(fit, "valid", true);
        find("fitEmpty").click(); tryCompare(fit, "sampleCount", 0); compare(fit.error, "");
        find("fitEmpty").click(); tryCompare(fit, "valid", true);
        find("fitBand").click(); compare(fit.band.visible, false);
        gallery.width = 420; verify(waitForRendering(gallery.contentItem));
        verify(find("fitMultiplier").width < 400); gallery.width = 1200;
    }
    function test_waterfallControlsAndExample() {
        choose(23);
        const waterfall = find("waterfallPreview").dataAdapter;
        compare(waterfall.finalValue, 80);
        find("waterfallCost").valueEdited(-150);
        compare(waterfall.finalValue, -25);
        find("waterfallReset").click(); compare(waterfall.finalValue, 5);
        compare(waterfall.series.at(3).connectorBefore, false);
        find("waterfallInitial").valueEdited(20); compare(waterfall.finalValue, 25);
        find("waterfallConnectors").click(); compare(waterfall.series.at(1).connectorBefore, false);
        find("waterfallLabels").click(); compare(waterfall.series.barLabelsVisible, false);
        find("waterfallWidth").valueEdited(60);
        const code = compileExample();
        verify(code.includes("resetOnSubtotal: true")); verify(code.includes("value:-150"));
        compare(exampleChart.barGroupWidth, .6);
        compare(exampleChart.series[0].at(5).cumulativeAfter, 25);
        gallery.width = 420; verify(waitForRendering(gallery.contentItem));
        verify(find("waterfallCost").width < 400); gallery.width = 1200;
    }
    function test_barExamples_data() {
        return Array.from({length: 6}, (_, variant) => ({tag: "bar_" + variant, variant: variant}));
    }
    function test_barExamples(data) {
        choose(2);
        find("chartBarLayout").activated(data.variant);
        compileExample();
        compare(exampleChart.chartType, find("demoChart").chartType);
    }
    function test_radialExamples_data() {
        return Array.from({length: 4}, (_, variant) => ({tag: "ring_" + variant, variant: variant}));
    }
    function test_radialExamples(data) {
        choose(17);
        find("chartRadialPreview").variant = data.variant;
        compileExample();
        compare(exampleChart.chartType, data.variant === 0 ? CatChartSeries.Pie : CatChartSeries.Doughnut);
        compare(exampleChart.series.length, data.variant < 2 ? 1 : data.variant);
    }
    function test_resetKeepsSelectedVariant() {
        gallery.showComponent("chart-line");
        find("chartDemoPicker").activated(1);
        compare(find("chartDemoPicker").currentValue, 16);
        find("chartAdvancedPreview").curve = 5;
        verify(find("codeSample").text.includes("CatmullRomSpline"));
        find("componentResetButton").clicked();
        compare(find("chartDemoPicker").currentValue, 16);
        verify(find("codeSample").text.includes("LinearCurve"));
        compileExample();
    }
    function test_threeDExamples_data() {
        const rows = [];
        for (let type = 0; type < 4; ++type)
            for (const shape of (type === 3 ? [0] : [0, 1, 2, 3, 4]))
                rows.push({tag: type + "_" + shape, type: type, shape: shape});
        return rows;
    }
    function test_threeDExamples(data) {
        choose(19);
        const preview = find("chart3DPreview");
        preview.type = data.type;
        preview.shape = data.shape;
        compileExample();
        compare(exampleChart.scene3D.series[0].type, data.type);
        if (data.type !== 3) compare(exampleChart.scene3D.series[0].shape, data.shape);
    }
    function test_navigationStopsStress() {
        choose(0);
        const data = find("chartPreview").dataController;
        data.startStress();
        compare(data.running, true);
        gallery.showComponent("chart-axes");
        compare(data.running, false);
        compare(find("chartDemoPicker").currentValue, 18);
        verify(find("codeSample").text.includes("CatChartAxis.FarSide"));
        verify(!gallery.showChartDemo(30));
    }
}
