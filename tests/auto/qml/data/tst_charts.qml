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
    Component { id: axisFactory; CatChartAxis {} }
    Component { id: paletteFactory; ThemePalette {} }
    Component {
        id: offsetFactory
        Rectangle { width: 800; height: 600; color: "magenta" }
    }
    SignalSpy { id: clicked; signalName: "pointClicked" }
    property var callbackSeries: null
    property bool mutateFromDelegate: false
    Component {
        id: mutatingDelegate
        Rectangle {
            required property var chartData
            color: chartData.color
            Component.onCompleted: {
                if (test.mutateFromDelegate) {
                    test.mutateFromDelegate = false;
                    test.callbackSeries.clear();
                }
            }
        }
    }
    function test_delegateCallbackCanReplaceData() {
        const chart = createTemporaryObject(factory, test), s = chart.series[0];
        chart.chartType = CatChartSeries.Scatter;
        callbackSeries = s; mutateFromDelegate = true; s.pointDelegate = mutatingDelegate;
        verify(waitForRendering(chart)); tryCompare(chart, "activeDelegateCount", 0);
        compare(s.count, 0); compare(chart.dataAt(Qt.point(100, 100)), {});
        s.append(1, 2); verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 1);
        s.pointDelegate = null; verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0);
        callbackSeries = null;
    }
    property int delegateCreated: 0
    property int delegateDestroyed: 0
    Component {
        id: customDelegate
        Rectangle {
            required property var chartData
            objectName: "delegate-" + chartData.kind + "-" + chartData.index
            property int birth: 0
            color: chartData.color
            border.color: "white"; border.width: 2
            radius: chartData.kind === "point" ? width / 4 : 2
            Component.onCompleted: { birth = ++test.delegateCreated; }
            Component.onDestruction: ++test.delegateDestroyed
            Text { anchors.centerIn: parent; text: parent.chartData.text; color: "white" }
        }
    }
    Component { id: invalidDelegate; QtObject {} }
    function test_transitionsAndReducedMotion() {
        const chart = createTemporaryObject(factory, test), s = chart.series[0];
        const palette = createTemporaryObject(paletteFactory, test);
        palette.overrides = {motionDuration: 300}; chart.theme = palette;
        verify(waitForRendering(chart)); compare(chart.animationsEnabled, false);
        chart.animationsEnabled = true; s.color = "blue"; verify(chart.transitionRunning);
        verify(waitForRendering(chart));
        const revision = chart.renderingStatistics().preparedRevision;
        wait(40); compare(chart.renderingStatistics().preparedRevision, revision);
        chart.selectPoint(0, 1); verify(chart.transitionRunning);
        palette.reducedMotion = true; compare(chart.transitionRunning, false);
        compare(chart.renderingStatistics().appearanceTransitionProgress, 1);
        compare(chart.renderingStatistics().selectionTransitionProgress, 1);
        s.color = "green"; compare(chart.transitionRunning, false);
        const count = s.count; s.append(9, 3); compare(s.count, count + 1);
        chart.animationsEnabled = false; palette.reducedMotion = false;
        s.color = "red"; compare(chart.transitionRunning, false);
        chart.animationsEnabled = true; s.color = "blue"; verify(chart.transitionRunning);
        tryCompare(chart, "transitionRunning", false, 1000);
        chart.panBy(Qt.point(10, 0)); compare(chart.transitionRunning, false);
    }
    function test_selectionClicksAndDelegateState() {
        const chart = createTemporaryObject(factory, test), s = chart.series[0];
        chart.chartType = CatChartSeries.Scatter; s.pointDelegate = customDelegate; s.markerSize = 20;
        chart.selectionMode = CatChart.MultiSelection;
        verify(waitForRendering(chart));
        let at = chart.mapToPosition(Qt.point(1, 4)); mouseClick(chart, at.x, at.y);
        compare(chart.selectedPoints.length, 1); verify(chart.isPointSelected(0, 1));
        compare(findChild(chart, "delegate-point-1").chartData.selected, true);
        at = chart.mapToPosition(Qt.point(2, 1)); mouseClick(chart, at.x, at.y);
        compare(chart.selectedPoints.length, 2); mouseClick(chart, at.x, at.y);
        compare(chart.selectedPoints.length, 1);
        chart.selectionMode = CatChart.SingleSelection; compare(chart.selectedPoints.length, 0);
        mouseClick(chart, at.x, at.y); verify(chart.isPointSelected(0, 2));
        chart.clearSelection(); compare(chart.selectedPoints.length, 0);
        verify(!findChild(chart, "delegate-point-2").chartData.selected);
        chart.selectPoint(0, 0); s.clear(); compare(chart.selectedPoints.length, 0);
    }
    function test_pointDelegateBudgetReuseAndFallback() {
        const chart = createTemporaryObject(factory, test), s = chart.series[0];
        chart.chartType = CatChartSeries.Scatter; s.markerSize = 24;
        chart.delegateBudget = 2; s.pointDelegate = customDelegate;
        verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 2); compare(chart.delegateError, "");
        const item = findChild(chart, "delegate-point-0"); verify(item); const birth = item.birth;
        compare(item.chartData.index, 0); compare(item.chartData.seriesName, "Source");
        compare(item.width, 24); compare(item.chartData.y, 2);
        chart.panBy(Qt.point(2, 0)); verify(waitForRendering(chart));
        compare(findChild(chart, "delegate-point-0").birth, birth);
        s.color = "blue"; verify(waitForRendering(chart)); compare(item.color, "#0000ff");
        chart.delegateBudget = 0; verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0);
        compare(findChild(chart, "delegate-point-0"), null);
        chart.delegateBudget = 4; s.pointDelegate = invalidDelegate;
        verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0); verify(chart.delegateError.length > 0);
        s.pointDelegate = customDelegate; verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 4);
        compare(chart.delegateError, ""); s.visible = false; verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0);
        s.visible = true; verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 4);
        chart.removeSeries(s); verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0);
    }
    function test_barAndLabelDelegates() {
        const chart = createTemporaryObject(factory, test), s = chart.series[0];
        chart.chartType = CatChartSeries.Bar; chart.delegateBudget = 16;
        s.barDelegate = customDelegate; s.labelDelegate = customDelegate; s.barLabelsVisible = true;
        s.barLabelRotation = 90;
        verify(waitForRendering(chart)); verify(chart.activeDelegateCount >= 4);
        const bar = findChild(chart, "delegate-bar-1"), label = findChild(chart, "delegate-label-1");
        verify(bar); verify(label); compare(label.rotation, 90); compare(label.chartData.text, "4");
        compare(bar.chartData.y, 4); verify(bar.width > 20); verify(bar.height > 20);
        s.clear(); verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0);
    }
    function test_sliceLabelDelegates() {
        const chart = createTemporaryObject(factory, test), s = chart.series[0];
        chart.chartType = CatChartSeries.Doughnut;
        chart.sliceLabelsVisible = true; chart.sliceLabelPosition = CatChart.SliceLabelOutside;
        s.labelDelegate = customDelegate;
        verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 4);
        const label = findChild(chart, "delegate-label-0"); verify(label);
        compare(label.chartData.percentage, 20); verify(label.chartData.text.indexOf("20%") >= 0);
        chart.setSliceVisible(0, 0, false); verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 3);
        compare(findChild(chart, "delegate-label-0"), null);
    }
    function init() { failOnWarning(/.*/); }
    function cleanup() {
        clicked.target = null; clicked.clear();
        Theme.colorScheme = ThemePalette.Light;
        Theme.fontSize = 14;
        Theme.reducedMotion = false;
    }
    function test_textureMarkerAndRotationQml() {
        const chart = createTemporaryObject(factory, test), series = chart.series[0];
        series.markerShape = CatChartSeries.TextureMarker; series.markerTexture = "qrc:/missing-chart-marker.png";
        verify(series.markerTextureError.length > 0); series.markerTexture = ""; compare(series.markerTextureError, "");
        chart.chartType = CatChartSeries.Bar; series.barLabelsVisible = true; series.barLabelRotation = 45;
        verify(waitForRendering(chart)); compare(series.barLabelRotation, 45);
    }
    function test_independentBoundariesQml() {
        const chart = createTemporaryObject(factory, test), series = chart.series[0];
        chart.chartType = CatChartSeries.RangeArea;
        const lower = [{x: 0, y: 1}, {x: 2, y: 3}], upper = [{x: 0, y: 4}, {x: 1, y: 5}, {x: 2, y: 6}];
        const oldCount = series.count; verify(!series.setRangeBoundaries(lower, upper)); compare(series.count, oldCount);
        verify(series.setRangeBoundaries(lower, upper, CatChartSeries.LinearAlignment)); compare(series.count, 3);
        compare(series.lowerValue(1), 2); compare(series.at(1).upper, 5);
        verify(waitForRendering(chart));
        verify(series.setRangeBoundaries([], [])); compare(series.count, 0);
    }
    function test_fillMaterialsQml() {
        const chart = createTemporaryObject(factory, test), series = chart.series[0];
        chart.chartType = CatChartSeries.Area;
        series.fillStyle = CatChartSeries.HorizontalGradient;
        series.fillGradientStops = [{position: 0, color: "red"}, {position: 0.5, color: "green"}, {position: 1, color: "blue"}];
        verify(waitForRendering(chart)); compare(series.fillGradientStops.length, 3);
        series.fillGradientStops = [{position: 0.7, color: "red"}, {position: 0.3, color: "blue"}];
        compare(series.fillGradientStops.length, 3);
        series.fillStyle = CatChartSeries.TextureFill; series.fillTextureMode = CatChartSeries.TileTexture;
        series.fillTextureSize = Qt.size(32, 64); series.fillTexture = "qrc:/missing-chart-texture.png";
        verify(series.fillTextureError.length > 0); compare(series.fillTextureSize.height, 64);
        series.fillTexture = ""; compare(series.fillTextureError, "");
    }
    function test_strokeStylesQml() {
        const chart = createTemporaryObject(factory, test), series = chart.series[0];
        series.lineCap = CatChartSeries.RoundCap; series.lineJoin = CatChartSeries.MiterJoin;
        series.miterLimit = 3; series.lineWidth = 12;
        verify(waitForRendering(chart)); compare(series.lineCap, CatChartSeries.RoundCap);
        compare(series.lineJoin, CatChartSeries.MiterJoin); compare(series.miterLimit, 3);
        series.clear(); compare(series.lineCap, CatChartSeries.RoundCap);
    }
    function test_multipleAxisQml() {
        const chart = createTemporaryObject(factory, test);
        const x = createTemporaryObject(axisFactory, test, {orientation: Qt.Horizontal, side: CatChartAxis.FarSide});
        const y = createTemporaryObject(axisFactory, test, {side: CatChartAxis.FarSide});
        const series = chart.series[0]; series.xAxis = x; series.yAxis = y;
        chart.setViewRange(0, 10, 0, 10); x.setRange(0, 4); y.setRange(0, 8);
        verify(waitForRendering(chart));
        compare(chart.axisForSeries(series, Qt.Horizontal), x);
        const plot = chart.plotRect();
        const center = Qt.point(plot.x + plot.width / 2, plot.y + plot.height / 2);
        const point = chart.mapToValue(center, series);
        fuzzyCompare(point.x, 2, 0.0001); fuzzyCompare(point.y, 4, 0.0001);
        y.reversed = true;
        const mapped = chart.mapToPosition(Qt.point(1, 4), series);
        compare(chart.dataAt(mapped).index, 1);
        x.visible = false; y.visible = false; verify(waitForRendering(chart));
        verify(chart.plotRect().width > plot.width); verify(chart.plotRect().height > plot.height);
        y.destroy(); wait(0); compare(series.yAxis, null);
        compare(chart.axisForSeries(series, Qt.Vertical), chart.yAxis);
    }
    function test_minorTickApi() {
        const chart = createTemporaryObject(factory, test);
        compare(chart.xAxis.minorTickCount, 0);
        chart.xAxis.minorTickCount = 4; chart.yAxis.minorTickCount = 100;
        verify(waitForRendering(chart));
        compare(chart.xAxis.minorTickCount, 4); compare(chart.yAxis.minorTickCount, 10);
        chart.resetView(); compare(chart.xAxis.minorTickCount, 4);
        chart.xAxis.minorTickCount = -3; compare(chart.xAxis.minorTickCount, 0);
    }
    function test_areaStyleApi() {
        const chart = createTemporaryObject(factory, test, {chartType: CatChartSeries.Area});
        const series = chart.series[0], before = series.at(1);
        series.areaBaselineMode = CatChartSeries.CustomBaseline; series.areaBaseline = -3.5;
        series.fillColor = "#ccff0000"; series.fillEndColor = "transparent";
        series.borderColor = "white"; series.fillStyle = CatChartSeries.HorizontalGradient;
        series.fillOpacity = 0.6;
        verify(waitForRendering(chart));
        compare(series.areaBaseline, -3.5); compare(series.fillStyle, 2);
        compare(series.borderColor, "#ffffff"); compare(series.at(1), before);
        verify(chart.mapToValue(Qt.point(chart.plotRect().x, chart.plotRect().y + chart.plotRect().height)).y < -3.5);
        series.clear(); series.append(0, 2); compare(series.areaBaseline, -3.5);
    }
    function test_barStyleApi() {
        const chart = createTemporaryObject(factory, test, {chartType: CatChartSeries.Bar,
            barGroupWidth: 0.7, barSpacing: 0.2});
        const series = chart.series[0];
        series.barBorderColor = "#ffffff"; series.barBorderWidth = 3;
        series.barLabelsVisible = true; series.barLabelPosition = CatChartSeries.BarLabelInsideEnd;
        series.barLabelFormat = "@category: @value"; series.barLabelPrecision = 4; series.barLabelMargin = 6;
        verify(waitForRendering(chart));
        compare(series.barLabelPosition, 1); compare(series.barBorderColor, "#ffffff");
        compare(series.barLabelPrecision, 4); compare(chart.barSpacing, 0.2);
        const raw = series.at(1);
        compare(chart.dataAt(chart.mapToPosition(Qt.point(1, 2))).y, raw.y);
        chart.barGroupWidth = 0;
        verify(chart.dataAt(chart.mapToPosition(Qt.point(1, 2))).index === undefined);
    }
    function test_radialPublicApi() {
        const chart = createTemporaryObject(factory, test, {chartType: CatChartSeries.Doughnut,
            multiRing: true, holeSize: 0.3, ringGap: 8, sliceLabelsVisible: false});
        compare(chart.sliceLabelPosition, CatChart.SliceLabelInside);
        chart.sliceLabelPosition = CatChart.SliceLabelOutside;
        compare(chart.sliceLabelPosition, CatChart.SliceLabelOutside);
        chart.sliceLabelPosition = 8;
        compare(chart.sliceLabelPosition, CatChart.SliceLabelOutside);
        verify(chart.setPieAngles(-90, 180));
        const series = chart.series[0];
        verify(series.setData([{x: 0, y: 10, sliceStyle: {color: "#ff0000", explodeDistance: 12,
            borderColor: "#ffffff", borderWidth: 3, labelVisible: false}}, {x: 1, y: 20}]));
        compare(series.sliceStyle(0).color, "#ff0000");
        compare(series.at(0).sliceStyle.explodeDistance, 12);
        verify(!series.setSliceStyle(0, {borderWidth: -1}));
        compare(series.sliceStyle(0).borderWidth, 3);
        verify(chart.setSliceVisible(0, 1, false));
        compare(chart.isSliceVisible(0, 1), false);
        verify(waitForRendering(chart));
        const rect = chart.plotRect();
        verify(chart.dataAt(Qt.point(rect.x + rect.width / 2, rect.y + rect.height / 2)).index === undefined);
        verify(series.append(2, 30));
        compare(chart.isSliceVisible(0, 1), true);
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
    function test_touchPanAndPinch_data() {
        const rows = [];
        for (let scale = 0; scale < 4; ++scale)
            for (let mask = 0; mask < 4; ++mask)
                rows.push({tag: scale + "-" + mask, scale: scale, mask: mask});
        return rows;
    }
    function test_touchPanAndPinch(data) {
        const flickable = createTemporaryObject(flickableFactory, test);
        const chart = createTemporaryObject(factory, flickable.contentItem,
            {x: 30, y: 20, dragMode: CatChart.RectangleZoomDrag});
        chart.xAxis.scale = data.scale; chart.yAxis.scale = data.scale;
        chart.xAxis.reversed = Boolean(data.mask & 1); chart.yAxis.reversed = Boolean(data.mask & 2);
        chart.setViewRange(1, 101, 1, 101);
        clicked.target = chart; verify(waitForRendering(chart));
        const raw = chart.series[0].at(1), touch = touchEvent(chart);
        const center = Qt.point(280, 200), anchor = chart.mapToValue(center);
        // Deliver the initial contact before Qt Quick coalesces later moves for a frame.
        touch.press(4, chart, 280, 200).commit(); wait(20);
        touch.move(4, chart, 310, 220).commit(); wait(20);
        let mapped = chart.mapToPosition(anchor);
        fuzzyCompare(mapped.x, 310, 0.001); fuzzyCompare(mapped.y, 220, 0.001);
        compare(chart.zoomSelecting, false);
        const before = chart.xAxis.minimum;
        touch.stationary(4).press(8, chart, 390, 220).commit(); wait(20);
        compare(chart.xAxis.minimum, before); // Adding a finger must not jump.
        const pinchAnchor = chart.mapToValue(Qt.point(350, 220));
        const span = data.scale === 1 ? Math.log(chart.xAxis.maximum / chart.xAxis.minimum)
                                     : chart.xAxis.maximum - chart.xAxis.minimum;
        touch.move(4, chart, 290, 240).move(8, chart, 450, 240).commit(); wait(20);
        mapped = chart.mapToPosition(pinchAnchor);
        fuzzyCompare(mapped.x, 370, 0.001); fuzzyCompare(mapped.y, 240, 0.001);
        const zoomedSpan = data.scale === 1 ? Math.log(chart.xAxis.maximum / chart.xAxis.minimum)
                                           : chart.xAxis.maximum - chart.xAxis.minimum;
        fuzzyCompare(zoomedSpan, span / 2, 0.001);
        const pinched = chart.xAxis.minimum;
        touch.release(4, chart, 290, 240).stationary(8).commit(); wait(20);
        compare(chart.xAxis.minimum, pinched);
        const remaining = chart.mapToValue(Qt.point(450, 240));
        touch.move(8, chart, 420, 220).commit(); wait(20);
        mapped = chart.mapToPosition(remaining);
        fuzzyCompare(mapped.x, 420, 0.001); fuzzyCompare(mapped.y, 220, 0.001);
        touch.release(8, chart, 420, 220).commit();
        compare(clicked.count, 0); compare(chart.series[0].at(1), raw);
        compare(flickable.contentX, 0); compare(flickable.contentY, 0);
    }
    function test_touchCoincidentPointsAndData() {
        const chart = createTemporaryObject(factory, test);
        chart.setViewRange(0, 10, 0, 10); verify(waitForRendering(chart));
        const touch = touchEvent(chart);
        touch.press(0, chart, 300, 200).press(1, chart, 300, 200).commit();
        touch.move(0, chart, 280, 200).move(1, chart, 320, 200).commit(); wait(20);
        compare(chart.xAxis.minimum, 0); compare(chart.xAxis.maximum, 10);
        chart.series[0].append(100, 100); wait(20); // A fixed view can accept live data during navigation.
        touch.move(0, chart, 260, 200).move(1, chart, 340, 200).commit(); wait(20);
        fuzzyCompare(chart.xAxis.maximum - chart.xAxis.minimum, 5, 0.0001);
        touch.release(0, chart, 260, 200).release(1, chart, 340, 200).commit();
        verify(chart.setViewRange(0, 10, 0, 10));
        touch.press(0, chart, 300, 200).commit();
        // Releasing far from the press without an intermediate update is not a tap.
        clicked.target = chart;
        touch.release(0, chart, 380, 200).commit();
        compare(clicked.count, 0);
    }
    function test_touchTapAndParentScroll_data() {
        return ["enabled", "navigation-off", "interactive-off", "outside", "pie", "radar"].map(mode => ({tag: mode, mode: mode}));
    }
    function test_touchTapAndParentScroll(data) {
        const flickable = createTemporaryObject(flickableFactory, test);
        const chart = createTemporaryObject(factory, flickable.contentItem);
        if (data.mode === "navigation-off") chart.touchNavigationEnabled = false;
        if (data.mode === "interactive-off") chart.interactive = false;
        if (data.mode === "pie") chart.chartType = CatChartSeries.Pie;
        if (data.mode === "radar") chart.chartType = CatChartSeries.Radar;
        clicked.target = chart; verify(waitForRendering(chart));
        const touch = touchEvent(chart);
        if (data.mode === "enabled" || data.mode === "navigation-off") {
            const at = chart.mapToPosition(Qt.point(1, 4));
            touch.press(0, chart, at.x, at.y).commit();
            touch.release(0, chart, at.x, at.y).commit();
            compare(clicked.count, 1); compare(clicked.signalArguments[0][0].index, 1);
            clicked.clear();
        }
        const x = data.mode === "outside" ? 10 : 280;
        touch.press(0, chart, x, 280).commit();
        for (let y = 260; y >= 120; y -= 20) {
            touch.move(0, chart, x, y).commit(); wait(20);
        }
        touch.release(0, chart, x, 120).commit();
        compare(clicked.count, 0);
        if (data.mode === "enabled") {
            compare(flickable.contentY, 0); compare(chart.xAxis.automatic, false);
        } else {
            verify(flickable.contentY > 20, "Parent should receive the drag: " + data.mode);
            compare(chart.xAxis.automatic, true); flickable.cancelFlick();
        }
    }
    function test_touchCancellation_data() {
        return ["escape", "focus", "enabled", "visible", "interactive", "navigation", "resize",
                "axis", "type", "reset", "automatic-data", "three-fingers"].map(reason => ({tag: reason, reason: reason}));
    }
    function test_touchCancellation(data) {
        const chart = createTemporaryObject(factory, test);
        clicked.target = chart; verify(waitForRendering(chart));
        const touch = touchEvent(chart);
        touch.press(0, chart, 260, 200).commit();
        if (data.reason === "escape") keyClick(Qt.Key_Escape);
        if (data.reason === "focus") test.forceActiveFocus();
        if (data.reason === "enabled") chart.enabled = false;
        if (data.reason === "visible") chart.visible = false;
        if (data.reason === "interactive") chart.interactive = false;
        if (data.reason === "navigation") chart.touchNavigationEnabled = false;
        if (data.reason === "resize") chart.width += 20;
        if (data.reason === "axis") chart.setViewRange(0, 10, 0, 10);
        if (data.reason === "type") chart.chartType = CatChartSeries.Pie;
        if (data.reason === "reset") chart.resetView();
        if (data.reason === "automatic-data") { chart.series[0].append(100, 100); wait(30); }
        if (data.reason === "three-fingers") {
            touch.stationary(0).press(1, chart, 300, 200).press(2, chart, 340, 200).commit();
            touch.stationary(0).release(1, chart, 300, 200).release(2, chart, 340, 200).commit();
        }
        const minimum = chart.xAxis.minimum, maximum = chart.xAxis.maximum;
        touch.move(0, chart, 320, 230).commit(); wait(20);
        touch.release(0, chart, 320, 230).commit();
        compare(chart.xAxis.minimum, minimum); compare(chart.xAxis.maximum, maximum);
        compare(clicked.count, 0);
        chart.enabled = true; chart.visible = true; chart.interactive = true;
        chart.touchNavigationEnabled = true; chart.chartType = CatChartSeries.Line;
        chart.setViewRange(0, 4, 0, 5);
        const at = chart.mapToPosition(Qt.point(1, 4));
        touch.press(0, chart, at.x, at.y).commit();
        touch.release(0, chart, at.x, at.y).commit();
        compare(clicked.count, 1); compare(clicked.signalArguments[0][0].index, 1);
    }
    function test_rectangleZoomInsideFlickable_data() {
        return [0, 1, 2].map(axes => ({tag: String(axes), axes: axes}));
    }
    function test_rectangleZoomInsideFlickable(data) {
        const flickable = createTemporaryObject(flickableFactory, test);
        const chart = createTemporaryObject(factory, flickable.contentItem,
            {dragMode: CatChart.RectangleZoomDrag, rectangleZoomAxes: data.axes});
        clicked.target = chart;
        verify(chart.setViewRange(0, 10, 0, 10)); verify(waitForRendering(chart));
        const plot = chart.plotRect();
        const start = Qt.point(Math.round(plot.x + plot.width * 0.2), Math.round(plot.y + plot.height * 0.2));
        const end = Qt.point(start.x + 120, start.y + 80);
        const lower = chart.mapToValue(Qt.point(start.x, end.y)), upper = chart.mapToValue(Qt.point(end.x, start.y));
        mousePress(chart, start.x, start.y);
        try {
            for (let step = 1; step <= 4; ++step) {
                mouseMove(chart, start.x + step * 30, start.y + step * 20, 20, Qt.LeftButton);
                compare(flickable.contentX, 0); compare(flickable.contentY, 0);
                compare(chart.xAxis.minimum, 0); compare(chart.yAxis.maximum, 10);
                compare(chart.zoomSelecting, true);
            }
            if (data.axes === 1) fuzzyCompare(chart.zoomSelection.height, plot.height, 1);
            if (data.axes === 2) fuzzyCompare(chart.zoomSelection.width, plot.width, 1);
        } finally { mouseRelease(chart, end.x, end.y); }
        compare(chart.zoomSelecting, false); compare(chart.zoomSelection.width, 0); compare(clicked.count, 0);
        fuzzyCompare(chart.xAxis.minimum, data.axes === 2 ? 0 : lower.x, 0.0001);
        fuzzyCompare(chart.xAxis.maximum, data.axes === 2 ? 10 : upper.x, 0.0001);
        fuzzyCompare(chart.yAxis.minimum, data.axes === 1 ? 0 : lower.y, 0.0001);
        fuzzyCompare(chart.yAxis.maximum, data.axes === 1 ? 10 : upper.y, 0.0001);
        mouseDrag(flickable, 740, 460, 0, -80); verify(flickable.contentY > 0); flickable.cancelFlick();
    }
    function test_rectangleZoomCancelAndClick_data() {
        return ["escape", "focus", "disabled", "tiny"].map(reason => ({tag: reason, reason: reason}));
    }
    function test_rectangleZoomCancelAndClick(data) {
        const chart = createTemporaryObject(factory, test, {dragMode: CatChart.RectangleZoomDrag});
        clicked.target = chart; verify(waitForRendering(chart));
        const plot = chart.plotRect(), start = Qt.point(plot.x + 40, plot.y + 40);
        mousePress(chart, start.x, start.y);
        mouseMove(chart, start.x + 60, start.y + (data.reason === "tiny" ? 2 : 50), 20, Qt.LeftButton);
        compare(chart.zoomSelecting, true);
        if (data.reason === "escape") keyClick(Qt.Key_Escape);
        if (data.reason === "focus") test.forceActiveFocus();
        if (data.reason === "disabled") chart.enabled = false;
        mouseRelease(chart, start.x + 60, start.y + (data.reason === "tiny" ? 2 : 50));
        compare(chart.zoomSelecting, false); compare(chart.xAxis.automatic, true); compare(chart.yAxis.automatic, true);
        compare(clicked.count, 0);
        chart.enabled = true;
        const at = chart.mapToPosition(Qt.point(1, 4)); mouseClick(chart, at.x, at.y);
        compare(clicked.count, 1); compare(clicked.signalArguments[0][0].index, 1);
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
