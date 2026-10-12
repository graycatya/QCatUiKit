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
    Component {
        id: flickableFactory
        Flickable {
            width: 800; height: 600
            contentWidth: 1200; contentHeight: 1200
            boundsBehavior: Flickable.StopAtBounds
        }
    }
    Component { id: paletteFactory; ThemePalette {} }
    Component {
        id: offsetFactory
        Rectangle { width: 800; height: 600; color: "magenta" }
    }
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
    function test_updatesPreserveChartPlacement() {
        const surface = createTemporaryObject(offsetFactory, test);
        const chart = createTemporaryObject(factory, surface, {x: 100, y: 100});
        verify(waitForRendering(chart));
        compare(grabImage(surface).pixel(25, 25), Qt.rgba(1, 0, 1, 1));
        for (let i = 0; i < 3; i++) {
            verify(chart.series[0].append(4 + i, 2));
            verify(waitForRendering(chart));
            // Rebuilt scene graph children must retain the chart's inherited transform.
            compare(grabImage(surface).pixel(25, 25), Qt.rgba(1, 0, 1, 1));
        }
    }
    function test_dragInsideFlickable() {
        const flickable = createTemporaryObject(flickableFactory, test);
        const chart = createTemporaryObject(factory, flickable.contentItem);
        clicked.target = chart;
        verify(chart.setViewRange(-1, 5, -1, 6));
        verify(waitForRendering(chart));
        const plot = chart.plotRect();
        const start = Qt.point(Math.round(plot.x + plot.width / 2),
                               Math.round(plot.y + plot.height / 2));
        const value = chart.mapToValue(start);
        const sceneStart = chart.mapToItem(test, start.x, start.y);
        mousePress(test, sceneStart.x, sceneStart.y);
        try {
            for (let step = 1; step <= 5; step++) {
                mouseMove(test, sceneStart.x - step * 16, sceneStart.y - step * 12,
                          20, Qt.LeftButton);
                compare(flickable.contentX, 0, "Chart drag must not scroll its parent");
                compare(flickable.contentY, 0, "Chart drag must not scroll its parent");
                const at = chart.mapToPosition(value);
                fuzzyCompare(at.x, start.x - step * 16, 1);
                fuzzyCompare(at.y, start.y - step * 12, 1);
            }
        } finally {
            mouseRelease(test, sceneStart.x - 80, sceneStart.y - 60);
        }
        compare(clicked.count, 0, "Panning must not trigger a point click");
        const at = chart.mapToPosition(Qt.point(1, 4));
        mouseClick(chart, at.x, at.y);
        compare(clicked.count, 1);
        compare(clicked.signalArguments[0][0].index, 1);
        // Releasing the chart must leave scrolling elsewhere available.
        mouseDrag(flickable, 740, 460, 0, -80);
        verify(flickable.contentY > 0);
        flickable.cancelFlick();
    }
    function test_nonPanningGesturesScrollParent_data() {
        return [
            {tag: "outside-plot", type: CatChartSeries.Line, interactive: true, outside: true},
            {tag: "disabled", type: CatChartSeries.Line, interactive: false, outside: false},
            {tag: "pie", type: CatChartSeries.Pie, interactive: true, outside: false},
            {tag: "doughnut", type: CatChartSeries.Doughnut, interactive: true, outside: false},
            {tag: "radar", type: CatChartSeries.Radar, interactive: true, outside: false}
        ];
    }
    function test_nonPanningGesturesScrollParent(data) {
        const flickable = createTemporaryObject(flickableFactory, test);
        const chart = createTemporaryObject(factory, flickable.contentItem,
            {chartType: data.type, interactive: data.interactive});
        clicked.target = chart;
        verify(waitForRendering(chart));
        const plot = chart.plotRect();
        mouseDrag(chart, data.outside ? 10 : plot.x + plot.width / 2,
                  plot.y + plot.height / 2, 0, -80);
        verify(flickable.contentY > 0, "Non-panning gestures must still scroll the parent");
        compare(clicked.count, 0);
        compare(chart.xAxis.automatic, true);
        compare(chart.yAxis.automatic, true);
        flickable.cancelFlick();
    }
}
