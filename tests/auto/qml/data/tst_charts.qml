// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "ChartContracts"
    when: windowShown
    width: 800; height: 600; visible: true
    Component {
        id: factory
        CatChart {
            width: 640; height: 400
            legendVisible: false
            gridVisible: false
            xAxis.tickCount: 2; yAxis.tickCount: 2
            CatChartSeries {
                name: "Source"; color: "red"
                Component.onCompleted: setData([{x: 0, y: 2, value: 7, label: "A"},
                    {x: 1, y: 4, value: 12, label: "B"}, {x: 2, y: 1, value: 9, label: "C"},
                    {x: 3, y: 3, value: 4, label: "D"}])
            }
        }
    }
    Component { id: paletteFactory; ThemePalette {} }
    SignalSpy { id: clicked; signalName: "pointClicked" }
    function init() { failOnWarning(/.*/); }
    function cleanup() {
        clicked.target = null; clicked.clear();
        Theme.colorScheme = ThemePalette.Light;
        Theme.fontSize = 14;
        Theme.reducedMotion = false;
    }
    function test_globalAndCustomTheme() {
        const chart = createTemporaryObject(factory, test);
        compare(chart.theme, Theme);
        compare(chart.backgroundColor, Theme.tokens.surface);
        Theme.colorScheme = ThemePalette.Dark;
        compare(chart.backgroundColor, Theme.tokens.surface);
        Theme.fontSize = 19;
        compare(chart.font.pixelSize, 19);
        const custom = createTemporaryObject(paletteFactory, test);
        custom.primaryColor = "purple";
        chart.theme = custom;
        compare(chart.tokens.primary, custom.tokens.primary);
        chart.theme = null;
        compare(chart.theme, Theme);
    }
    function test_allTypesRender_data() {
        return Array.from({length: 10}, (_, type) => ({tag: String(type), type: type}));
    }
    function test_allTypesRender(data) {
        const chart = createTemporaryObject(factory, test, {chartType: data.type});
        verify(waitForRendering(chart));
        const image = grabImage(chart), plot = chart.plotRect();
        let colored = 0;
        for (let y = Math.ceil(plot.y); y < plot.y + plot.height; y += 5)
            for (let x = Math.ceil(plot.x); x < plot.x + plot.width; x += 5) {
                const c = image.pixel(x, y);
                if (Math.max(c.r, c.g, c.b) - Math.min(c.r, c.g, c.b) > 0.2)
                    colored++;
            }
        verify(colored > 0, "Chart shapes must actually render, type " + data.type);
    }
    function test_keyboardNavigationAndDisabledInput() {
        const chart = createTemporaryObject(factory, test);
        verify(waitForRendering(chart));
        chart.forceActiveFocus();
        keyClick(Qt.Key_Plus);
        compare(chart.xAxis.automatic, false);
        const before = chart.xAxis.minimum;
        keyClick(Qt.Key_Right);
        verify(chart.xAxis.minimum > before);
        keyClick(Qt.Key_Home);
        compare(chart.xAxis.automatic, true);
        clicked.target = chart;
        chart.interactive = false;
        const at = chart.mapToPosition(Qt.point(1, 4));
        mouseClick(chart, at.x, at.y);
        keyClick(Qt.Key_Plus);
        compare(clicked.count, 0);
        compare(chart.xAxis.automatic, true);
        chart.interactive = true;
        mouseClick(chart, at.x, at.y);
        compare(clicked.count, 1);
        compare(clicked.signalArguments[0][0].index, 1);
    }
    function test_batchesPreserveViewAndRawValues() {
        const chart = createTemporaryObject(factory, test);
        const series = chart.series[0];
        verify(chart.setViewRange(0, 3, -1, 5));
        const before = series.count;
        verify(!series.appendData([{x: 4, y: 2}, {x: 5, y: NaN}]));
        compare(series.count, before);
        Theme.reducedMotion = true;
        verify(series.appendData([{x: 4, y: 2}, {x: 5, y: 3}]));
        compare(series.count, before + 2);
        compare(chart.xAxis.maximum, 3);
        compare(series.at(5).x, 5);
        chart.resetView();
        compare(chart.xAxis.automatic, true);
    }
}
