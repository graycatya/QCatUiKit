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
        verify(gallery.showChartDemo(mode));
        verify(waitForRendering(gallery.contentItem));
    }
    function test_threeDimensionalControls() {
        choose(19);
        const chart = find("chart3DItem"), series = chart.scene3D.series[0];
        verify(chart.scene3D.enabled); compare(series.count, 108);
        compare(chart.scene3D.lightingModel, CatChartScene3D.BlinnPhong);
        find("chart3DLightingModel").activated(1); compare(chart.scene3D.lightingModel, CatChartScene3D.Phong);
        verify(!find("chart3DGroundFill").enabled); verify(!find("chart3DExposure").enabled);
        find("chart3DLightingModel").activated(0);
        find("chart3DGroundFill").value = 0.3; compare(chart.scene3D.ambientGroundRatio, 0.3);
        find("chart3DExposure").value = 2; compare(chart.scene3D.exposure, 2);
        find("chart3DShininess").value = 64; compare(chart.scene3D.shininess, 64);
        find("chart3DMeshDetail").activated(2); compare(series.meshDetail, CatChartSeries3D.HighDetail);
        find("chart3DShape").activated(4); compare(series.shape, CatChartSeries3D.RoundedCube);
        GalleryLanguage.language = "zh_CN";
        compare(find("chart3DLightingModel").currentValue, CatChartScene3D.BlinnPhong);
        compare(find("chart3DMeshDetail").currentValue, CatChartSeries3D.HighDetail);
        compare(find("chart3DShape").currentValue, CatChartSeries3D.RoundedCube);
        find("chart3DResetLighting").clicked(); compare(chart.scene3D.exposure, 1);
        compare(chart.scene3D.shininess, 32); compare(chart.scene3D.ambientGroundRatio, 0.6);
        find("chart3DShape").activated(0);
        find("chart3DType").activated(3); compare(series.type, CatChartSeries3D.Surface); compare(series.rows, 21);
        find("chart3DPolar").click(); verify(series.polar); verify(series.closed);
        find("chart3DSurfaceMode").activated(1); compare(series.surfaceMode, CatChartSeries3D.Wireframe);
        find("chart3DOrthographic").click(); compare(chart.scene3D.projection, CatChartScene3D.Orthographic);
        find("chart3DSlice").activated(1); compare(chart.scene3D.sliceMode, CatChartScene3D.RowSlice);
        find("chart3DSliceIndex").increase(); find("chart3DSliceIndex").valueModified(); compare(chart.scene3D.sliceIndex, 4);
        verify(chart.scene3D.selectPoint(0, 4)); compare(chart.scene3D.selectedPoint.column, 4);
        GalleryLanguage.language = "zh_CN"; compare(find("chart3DType").currentValue, 3); compare(find("chart3DSurfaceMode").currentValue, 1);
        find("chart3DReset").clicked(); compare(chart.scene3D.projection, CatChartScene3D.Perspective); compare(chart.scene3D.yaw, 45);
        find("chart3DSurfaceInput").activated(1); verify(series.textureSource.toString().endsWith("chart-weave.png"));
        find("chart3DSurfaceInput").activated(2); verify(series.rows > 1); verify(series.columns > 1); verify(!series.polar);
        find("chart3DType").activated(1); compare(chart.scene3D.series[1].count, 108); verify(chart.scene3D.series[1].visible);
        compare(find("chart3DShape").currentValue, CatChartSeries3D.Cube);
        find("chart3DShape").activated(1); compare(series.shape, CatChartSeries3D.Cylinder);
        find("chart3DShape").activated(3); compare(series.shape, CatChartSeries3D.RoundedCube);
        compare(chart.scene3D.series[1].shape, CatChartSeries3D.RoundedCube);
        compare(chart.scene3D.series[1].meshDetail, CatChartSeries3D.HighDetail);
        find("chart3DSecondBars").click(); verify(!chart.scene3D.series[1].visible);
        find("chart3DType").activated(2); compare(series.count, 90); compare(series.rows, 0);
        find("chart3DClosedCurve").click(); verify(series.closed);
        find("chart3DCurveSteps").increase(); find("chart3DCurveSteps").valueModified(); compare(series.subdivisions, 13);
        find("chart3DShape").activated(3); compare(series.shape, CatChartSeries3D.CustomMesh);
        GalleryLanguage.language = "en";
        gallery.width = 420; Theme.density = ThemePalette.Mobile; wait(100);
        verify(find("chart3DItem").width < 420);
        gallery.width = 1200; Theme.density = ThemePalette.Desktop;
    }
    function test_chartMotionControls() {
        choose(4); const chart = find("demoChart");
        find("chartAnimations").click(); verify(chart.animationsEnabled);
        find("chartReducedMotion").click(); verify(Theme.reducedMotion);
        chart.series[0].color = "red"; compare(chart.transitionRunning, false);
        find("chartReducedMotion").click(); verify(!Theme.reducedMotion);
        GalleryLanguage.language = "zh_CN"; verify(find("chartAnimations").checked);
        find("chartRestoreButton").clicked(); compare(chart.animationsEnabled, false);
        GalleryLanguage.language = "en";
    }
    function test_selectionControls() {
        choose(4); const chart = find("demoChart"), picker = find("chartSelectionMode");
        picker.activated(2); compare(chart.selectionMode, CatChart.MultiSelection);
        chart.selectPoint(0, 0); chart.selectPoint(1, 1); compare(chart.selectedPoints.length, 2);
        verify(find("chartClearSelection").enabled); find("chartClearSelection").clicked(); compare(chart.selectedPoints.length, 0);
        GalleryLanguage.language = "zh_CN"; compare(picker.currentValue, 2);
        find("chartRestoreButton").clicked(); compare(chart.selectionMode, CatChart.NoSelection);
        choose(17); const radial = find("radialChart"); find("chartRadialSelectionMode").activated(1);
        radial.selectPoint(0, 0); compare(radial.selectedPoints.length, 1);
        GalleryLanguage.language = "en";
    }
    function test_customDelegatesAndBudget() {
        choose(4); const chart = find("demoChart");
        find("chartPointDelegate").click(); verify(waitForRendering(chart));
        compare(chart.activeDelegateCount, 48); compare(chart.delegateError, "");
        const budget = find("chartDelegateBudget"); budget.decrease(); budget.valueModified();
        verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 47);
        GalleryLanguage.language = "zh_CN"; compare(budget.value, 47); verify(find("chartPointDelegate").checked);
        find("chartRestoreButton").clicked(); verify(waitForRendering(chart)); compare(chart.activeDelegateCount, 0);
        choose(2); find("chartBarDelegate").click(); find("chartBarLabels").click(); find("chartLabelDelegate").click();
        verify(waitForRendering(chart)); verify(chart.activeDelegateCount > 12, "count=" + chart.activeDelegateCount + ", error=" + chart.delegateError + ", labels=" + chart.series[0].barLabelsVisible); compare(chart.delegateError, "");
        choose(17); find("chartLabelDelegate").click(); const radial = find("radialChart");
        verify(waitForRendering(radial)); verify(radial.activeDelegateCount > 0); compare(radial.delegateError, "");
        GalleryLanguage.language = "en";
    }
    function test_textureMarkerAndLabelRotation() {
        choose(4); const chart = find("demoChart"), series = chart.series[0];
        find("chartMarkerShape").activated(4); compare(series.markerShape, CatChartSeries.TextureMarker);
        verify(series.markerTexture.toString().endsWith("chart-marker.png")); compare(series.markerTextureError, "");
        verify(waitForRendering(chart)); find("chartRestoreButton").clicked(); compare(series.markerTexture.toString(), "");
        choose(2); find("chartBarLabels").click();
        const rotation = find("chartBarLabelRotation"); rotation.increase(); rotation.valueModified();
        compare(series.barLabelRotation, 15);
        GalleryLanguage.language = "zh_CN"; compare(rotation.value, 15);
        find("chartRestoreButton").clicked(); compare(series.barLabelRotation, 0); compare(rotation.value, 0);
        GalleryLanguage.language = "en";
    }
    function test_rangeBoundaryInputsAndLargePreset() {
        choose(15); const chart = find("advancedChart"), series = chart.series[0];
        const picker = find("chartBoundaryInput"); picker.activated(1); compare(series.count, 12);
        const before = series.at(1); picker.activated(2); compare(series.at(1), before);
        verify(find("chartBoundaryError").visible);
        picker.activated(3); verify(!find("chartBoundaryError").visible); verify(series.count > 12);
        const aligned = series.at(1); GalleryLanguage.language = "zh_CN";
        compare(picker.currentValue, 3); compare(series.at(1), aligned);
        find("chartRangeLargePreset").clicked(); compare(series.count, 100000); compare(series.capacity, 100000);
        verify(!picker.enabled); verify(waitForRendering(chart));
        verify(chart.renderingStatistics().renderedPointCount < 12000);
        find("chartAdvancedCurve").activated(5); verify(waitForRendering(chart));
        verify(chart.renderingStatistics().curveLodNodesVisited < 6000);
        chart.panBy(Qt.point(20, 0)); verify(waitForRendering(chart));
        compare(series.lowerValue(4321), -3);
        find("chartAdvancedRestore").clicked(); compare(series.count, 12); compare(series.capacity, 512);
        compare(picker.currentValue, 0); verify(picker.enabled);
        GalleryLanguage.language = "en";
    }
    function test_fillMaterials_data() { return [1, 15].map(mode => ({tag: String(mode), mode: mode})); }
    function test_fillMaterials(data) {
        choose(data.mode);
        const root = find(data.mode < 15 ? "chartPreview" : "chartAdvancedPreview");
        const chart = find(data.mode < 15 ? "demoChart" : "advancedChart"), series = chart.series[0];
        const options = findChild(root, "chartAreaOptions");
        findChild(options, "chartAreaFillStyle").activated(2);
        findChild(options, "chartAreaMultiStops").click(); compare(series.fillGradientStops.length, 3);
        findChild(options, "chartAreaFillStyle").activated(3);
        verify(series.fillTexture.toString().endsWith("chart-weave.png")); compare(series.fillTextureError, "");
        findChild(options, "chartAreaTextureMode").activated(1);
        const size = findChild(options, "chartAreaTileSize"); size.increase(); size.valueModified();
        compare(series.fillTextureMode, CatChartSeries.TileTexture); compare(series.fillTextureSize.width, 40);
        const raw = series.at(0); GalleryLanguage.language = "zh_CN";
        compare(series.fillTextureSize.width, 40); compare(series.at(0), raw);
        compare(findChild(options, "chartAreaTextureMode").currentValue, 1);
        find(data.mode < 15 ? "chartRestoreButton" : "chartAdvancedRestore").clicked();
        compare(series.fillStyle, CatChartSeries.SolidFill); compare(series.fillGradientStops.length, 0);
        compare(series.fillTexture.toString(), ""); compare(size.value, 32);
        GalleryLanguage.language = "en";
    }
    function test_strokeControls_data() { return [0, 1, 15, 16].map(mode => ({tag: String(mode), mode: mode})); }
    function test_strokeControls(data) {
        choose(data.mode);
        const root = find(data.mode < 15 ? "chartPreview" : "chartAdvancedPreview");
        const chart = find(data.mode < 15 ? "demoChart" : "advancedChart");
        const cap = findChild(root, "chartLineCap"), join = findChild(root, "chartLineJoin");
        cap.activated(3); join.activated(2);
        const width = findChild(root, "chartLineWidth"); width.increase(); width.valueModified();
        const limit = findChild(root, "chartMiterLimit"); limit.increase(); limit.valueModified();
        compare(chart.series[0].lineCap, CatChartSeries.RoundCap);
        compare(chart.series[0].lineJoin, CatChartSeries.MiterJoin);
        compare(chart.series[0].lineWidth, 3); compare(chart.series[0].miterLimit, 5);
        GalleryLanguage.language = "zh_CN"; compare(cap.currentIndex, 3);
        compare(chart.series[0].lineWidth, 3);
        find(data.mode < 15 ? "chartRestoreButton" : "chartAdvancedRestore").clicked();
        compare(chart.series[0].lineCap, CatChartSeries.AutoCap);
        compare(width.value, 2); compare(limit.value, 4);
        GalleryLanguage.language = "en";
    }
    function test_advancedVariants_data() {
        return [15, 16].map(mode => ({tag: String(mode), mode: mode}));
    }
    function test_advancedVariants(data) {
        choose(data.mode);
        const chart = find("advancedChart"), series = chart.series[0];
        compare(chart.chartType, data.mode === 15 ? CatChartSeries.RangeArea : CatChartSeries.Line);
        compare(series.count, 12); compare(series.breakBefore(6), true);
        if (data.mode === 15) verify(series.at(0).lower !== undefined);
        else { find("chartAdvancedCurve").activated(6); compare(series.curveMode, CatChartSeries.NaturalSpline); }
        const toggle = find("chartAdvancedDashed"); toggle.checked = true; toggle.toggled();
        compare(series.strokeStyle, CatChartSeries.DashLine);
        const gaps = find("chartAdvancedBreaks"); gaps.checked = false; gaps.toggled();
        compare(series.hasBreaks, false);
        find("chartAdvancedAdd").clicked(); compare(series.count, 18);
        const before = series.at(0);
        GalleryLanguage.language = "zh_CN";
        compare(series.at(0), before); compare(series.strokeStyle, CatChartSeries.DashLine);
        find("chartAdvancedRemove").clicked(); compare(series.count, 12);
        find("chartAdvancedClear").clicked(); compare(series.count, 0);
        find("chartAdvancedRestore").clicked(); compare(series.count, 12); compare(series.hasBreaks, true);
        GalleryLanguage.language = "en";
    }
    function test_rangeCurveControls() {
        choose(15);
        const chart = find("advancedChart"), series = chart.series[0];
        const curve = find("chartAdvancedCurve"), tension = find("chartAdvancedTension");
        verify(curve.visible); const raw = series.at(1);
        curve.activated(5); compare(series.curveMode, CatChartSeries.CatmullRomSpline);
        verify(tension.visible); tension.numberBox.commitValue(0.6);
        compare(series.splineTension, 0.6);
        GalleryLanguage.language = "zh_CN";
        compare(curve.currentValue, 5); compare(curve.displayText, "Catmull-Rom 样条");
        compare(series.at(1), raw); compare(series.splineTension, 0.6);
        find("chartAdvancedCrossing").click(); compare(series.at(1).lower, raw.lower);
        curve.activated(6); compare(series.curveMode, CatChartSeries.NaturalSpline);
        verify(!tension.visible); find("chartAdvancedAdd").clicked(); compare(series.count, 18);
        compare(series.curveMode, CatChartSeries.NaturalSpline);
        find("chartAdvancedClear").clicked(); compare(series.count, 0);
        find("chartAdvancedRestore").clicked(); compare(series.curveMode, CatChartSeries.LinearCurve);
        compare(series.splineTension, 0); compare(curve.currentValue, 0);
        GalleryLanguage.language = "en";
    }
    function test_radialVariants() {
        choose(17);
        const chart = find("radialChart");
        compare(chart.multiRing, true); compare(chart.series.length, 3);
        const position = find("chartRadialLabelPosition");
        compare(chart.sliceLabelPosition, CatChart.SliceLabelOutside);
        position.activated(0); compare(chart.sliceLabelPosition, CatChart.SliceLabelInside);
        position.activated(1); compare(chart.sliceLabelPosition, CatChart.SliceLabelOutside);
        compare(chart.series[2].visible, false);
        find("chartRadialVariant").activated(3);
        compare(chart.series[2].visible, true);
        find("chartRadialVariant").activated(2);
        compare(chart.series[2].visible, false);
        compare(chart.series[0].count, 4);
        const start = find("chartRadialStart"); start.numberBox.commitValue(-90);
        const sweep = find("chartRadialSweep"); sweep.numberBox.commitValue(180);
        compare(chart.pieStartAngle, -90); compare(chart.pieEndAngle, 90);
        const separation = find("chartRadialSeparation"); separation.value = 24; separation.valueModified();
        compare(chart.series[0].sliceStyle(0).explodeDistance, 24);
        const radius = find("chartRadialRadius"); radius.numberBox.commitValue(70);
        compare(chart.pieRadius, 0.7);
        const hole = find("chartRadialHole"); hole.numberBox.commitValue(40);
        compare(chart.holeSize, 0.4);
        const gap = find("chartRadialGap"); gap.value = 16; gap.valueModified();
        compare(chart.ringGap, 16);
        verify(chart.setSliceVisible(1, 0, false));
        GalleryLanguage.language = "zh_CN";
        compare(chart.isSliceVisible(1, 0), false);
        compare(chart.series[0].name, "外环");
        compare(position.currentValue, CatChart.SliceLabelOutside);
        compare(position.displayText, "外置引线");
        compare(chart.series[0].at(0).label, "扇区 1");
        compare(chart.pieStartAngle, -90); compare(chart.series[0].sliceStyle(0).explodeDistance, 24);
        find("chartRadialVariant").activated(0);
        compare(chart.chartType, CatChartSeries.Pie); compare(chart.multiRing, false);
        find("chartRadialVariant").activated(1);
        compare(chart.chartType, CatChartSeries.Doughnut); compare(chart.multiRing, false);
        const border = find("chartRadialBorder"); border.checked = false; border.toggled();
        compare(chart.series[0].sliceStyle(0).borderWidth, 0);
        const labels = find("chartRadialLabels"); labels.checked = false; labels.toggled();
        compare(chart.sliceLabelsVisible, false); verify(!position.enabled);
        position.activated(0);
        find("chartRadialAdd").clicked(); compare(chart.series[0].count, 5);
        find("chartRadialRemove").clicked(); compare(chart.series[0].count, 4);
        find("chartRadialClear").clicked(); compare(chart.series[0].count, 0);
        verify(chart.dataAt(Qt.point(chart.width / 2, chart.height / 2)).index === undefined);
        find("chartRadialRestore").clicked(); compare(chart.series[0].count, 4);
        compare(chart.multiRing, true); compare(chart.pieStartAngle, 0); compare(chart.pieEndAngle, 300);
        compare(chart.sliceLabelsVisible, true); verify(position.enabled);
        compare(chart.sliceLabelPosition, CatChart.SliceLabelOutside);
        compare(position.currentValue, CatChart.SliceLabelOutside);
        GalleryLanguage.language = "en";
    }
    function test_dashControls() {
        choose(1);
        const toggle = find("chartDashedStroke"); toggle.checked = true; toggle.toggled();
        const size = find("chartDashParameter0"); size.value = 24; size.valueModified();
        const series = find("demoChart").series[0];
        compare(series.strokeStyle, CatChartSeries.DashLine); compare(series.dashPattern[0], 24);
        find("chartRestoreButton").clicked(); compare(series.strokeStyle, CatChartSeries.SolidLine);
    }
    function test_curveAndMarkerVariants() {
        choose(1);
        const chart = find("demoChart");
        const first = chart.series[0];
        find("chartCurveStyle").activated(5);
        find("chartMarkerShape").activated(3);
        const toggle = find("chartCurveMarkers");
        toggle.checked = true; toggle.toggled();
        const size = find("chartMarkerSize");
        size.numberBox.commitValue(18);
        const tension = find("chartSplineTension");
        tension.numberBox.commitValue(0.5);
        verify(waitForRendering(gallery.contentItem));
        compare(first.curveMode, CatChartSeries.CatmullRomSpline);
        compare(first.markerShape, CatChartSeries.Diamond);
        compare(first.markerSize, 18);
        compare(first.markersVisible, true);
        compare(first.splineTension, 0.5);
        GalleryLanguage.language = "zh_CN";
        compare(find("chartCurveStyle").displayText, "Catmull-Rom 样条");
        compare(first.curveMode, CatChartSeries.CatmullRomSpline);
        find("chartAddButton").clicked();
        compare(first.markerShape, CatChartSeries.Diamond);
        find("chartRestoreButton").clicked();
        compare(first.curveMode, CatChartSeries.LinearCurve);
        compare(first.markersVisible, false);
        GalleryLanguage.language = "en";
    }
    function test_multiAxisControls() {
        choose(18);
        const root = find("chartAxesPreview"), chart = find("multiAxisChart"), series = chart.series[1];
        compare(series.count, 24); compare(series.xAxis.orientation, Qt.Horizontal);
        compare(series.yAxis.side, CatChartAxis.FarSide);
        findChild(root, "chartExtraAxesReversed").click(); compare(series.yAxis.reversed, true);
        findChild(root, "chartExtraAxesFar").click(); compare(series.yAxis.side, CatChartAxis.NearSide);
        findChild(root, "chartExtraAxisLog").click(); compare(series.yAxis.scale, CatChartAxis.Logarithmic);
        const independentY = series.yAxis;
        findChild(root, "chartIndependentAxes").click(); compare(series.yAxis, null);
        findChild(root, "chartIndependentAxes").click(); compare(series.yAxis, independentY);
        findChild(root, "chartExtraAxesVisible").click(); compare(series.yAxis.visible, false);
        const options = findChild(root, "chartInteractionOptions");
        findChild(options, "chartXAxisSide").activated(1); findChild(options, "chartYAxisSide").activated(1);
        compare(chart.xAxis.side, CatChartAxis.FarSide); compare(chart.yAxis.side, CatChartAxis.FarSide);
        GalleryLanguage.language = "zh_CN";
        compare(findChild(root, "chartExtraAxesReversed").text, "反向附加坐标轴");
        compare(findChild(options, "chartYAxisSide").displayText, "右侧");
        findChild(root, "chartAxesAdd").clicked(); compare(series.count, 28);
        findChild(root, "chartAxesClear").clicked(); compare(series.count, 0);
        findChild(root, "chartAxesRestore").clicked(); compare(series.count, 24);
        compare(series.yAxis.reversed, false); compare(series.yAxis.visible, true);
        compare(series.yAxis.side, CatChartAxis.FarSide); compare(chart.yAxis.side, CatChartAxis.NearSide);
        GalleryLanguage.language = "en";
    }
    function test_axisControls_data() { return [0, 11, 12, 15, 16].map(mode => ({tag: String(mode), mode: mode})); }
    function test_axisControls(data) {
        choose(data.mode);
        const root = data.mode < 15 ? find("chartPreview") : find("chartAdvancedPreview");
        const options = findChild(root, "chartInteractionOptions"), chart = options.targetChart;
        const xReverse = findChild(options, "chartXReversed"), yReverse = findChild(options, "chartYReversed");
        const xMinor = findChild(options, "chartXMinorTicks"), yMinor = findChild(options, "chartYMinorTicks");
        xReverse.click(); yReverse.click();
        for (let i = 0; i < 3; ++i) xMinor.increase();
        xMinor.valueModified(); yMinor.increase(); yMinor.increase(); yMinor.valueModified();
        verify(waitForRendering(gallery.contentItem));
        compare(chart.xAxis.reversed, true); compare(chart.yAxis.reversed, true);
        compare(chart.xAxis.minorTickCount, 3); compare(chart.yAxis.minorTickCount, 2);
        const raw = chart.series[0].at(0);
        chart.resetView(); compare(chart.xAxis.reversed, true); compare(chart.xAxis.minorTickCount, 3);
        GalleryLanguage.language = "zh_CN";
        compare(xReverse.text, "反向 X 轴"); compare(yReverse.text, "反向 Y 轴");
        compare(xMinor.value, 3); compare(yMinor.value, 2);
        if (data.mode < 15) find("chartAddButton").clicked(); else find("chartAdvancedAdd").clicked();
        compare(chart.series[0].at(0), raw);
        if (data.mode < 15) find("chartRestoreButton").clicked(); else find("chartAdvancedRestore").clicked();
        compare(chart.xAxis.reversed, false); compare(chart.yAxis.reversed, false);
        compare(chart.xAxis.minorTickCount, 0); compare(chart.yAxis.minorTickCount, 0);
        compare(xMinor.value, 0); compare(yMinor.value, 0);
        compare(xReverse.checked, false); compare(yReverse.checked, false);
        if (data.mode < 15) {
            choose(2); compare(xMinor.enabled, false); compare(yMinor.enabled, true);
        }
        GalleryLanguage.language = "en";
    }
    function test_rectangleZoomControls_data() { return [4, 15, 16].map(mode => ({tag: String(mode), mode: mode})); }
    function test_rectangleZoomControls(data) {
        choose(data.mode);
        const root = data.mode < 15 ? find("chartPreview") : find("chartAdvancedPreview");
        const options = findChild(root, "chartInteractionOptions"); verify(options !== null);
        const chart = options.targetChart;
        const touchOption = findChild(options, "chartTouchNavigation");
        compare(touchOption.checked, true);
        touchOption.click();
        compare(chart.touchNavigationEnabled, false);
        findChild(options, "chartDragMode").activated(1);
        findChild(options, "chartRectangleZoomAxes").activated(2);
        compare(chart.dragMode, CatChart.RectangleZoomDrag); compare(chart.rectangleZoomAxes, CatChart.ZoomYAxis);
        const raw = chart.series[0].at(0);
        GalleryLanguage.language = "zh_CN";
        compare(chart.dragMode, CatChart.RectangleZoomDrag); compare(chart.rectangleZoomAxes, CatChart.ZoomYAxis);
        compare(touchOption.text, "触摸平移与双指缩放");
        compare(chart.touchNavigationEnabled, false);
        const plot = chart.plotRect();
        verify(chart.zoomToRect(Qt.rect(plot.x, plot.y + plot.height / 4, plot.width, plot.height / 2)));
        compare(chart.xAxis.automatic, true); compare(chart.yAxis.automatic, false);
        if (data.mode < 15) find("chartAddButton").clicked(); else find("chartAdvancedAdd").clicked();
        compare(chart.series[0].at(0), raw); compare(chart.yAxis.automatic, false);
        if (data.mode < 15) find("chartRestoreButton").clicked(); else find("chartAdvancedRestore").clicked();
        compare(chart.dragMode, CatChart.PanDrag); compare(chart.rectangleZoomAxes, CatChart.ZoomBothAxes);
        compare(chart.touchNavigationEnabled, true); compare(touchOption.checked, true);
        compare(chart.xAxis.automatic, true); compare(chart.yAxis.automatic, true);
        GalleryLanguage.language = "en";
    }
    function test_areaAppearance_data() { return [1, 15].map(mode => ({tag: String(mode), mode: mode})); }
    function test_areaAppearance(data) {
        choose(data.mode);
        const root = data.mode === 1 ? find("chartPreview") : find("chartAdvancedPreview");
        const options = findChild(root, "chartAreaOptions");
        const chart = data.mode === 1 ? find("demoChart") : find("advancedChart");
        function option(name) { const o = findChild(options, name); verify(o !== null, name); return o; }
        option("chartAreaFillStyle").activated(2); option("chartAreaPalette").activated(2);
        option("chartAreaOutline").activated(1);
        const alpha = option("chartAreaOpacity"); alpha.numberBox.commitValue(60);
        const series = chart.series[0], raw = series.at(0);
        compare(series.fillStyle, CatChartSeries.HorizontalGradient); compare(series.fillColor, "#ff9845");
        compare(series.fillOpacity, 0.6); compare(series.borderColor, Theme.tokens.text);
        if (data.mode === 1) {
            option("chartAreaBaselineMode").activated(1);
            const base = option("chartAreaBaselineValue"); base.numberBox.commitValue(5);
            compare(series.areaBaseline, 5); compare(series.areaBaselineMode, CatChartSeries.CustomBaseline);
        }
        GalleryLanguage.language = "zh_CN";
        compare(series.fillStyle, 2); compare(options.fillPalette, 2); compare(series.at(0), raw);
        const fade = option("chartAreaFade"); fade.checked = true; fade.toggled();
        compare(series.fillEndColor.a, 0);
        if (data.mode === 1) find("chartAddButton").clicked(); else find("chartAdvancedAdd").clicked();
        compare(series.fillStyle, 2);
        if (data.mode === 1) find("chartRestoreButton").clicked(); else find("chartAdvancedRestore").clicked();
        compare(series.fillStyle, CatChartSeries.SolidFill); compare(options.fillPalette, 0);
        compare(series.areaBaselineMode, CatChartSeries.ZeroBaseline);
        GalleryLanguage.language = "en";
    }
    function test_barAppearance() {
        choose(2);
        const chart = find("demoChart"), series = chart.series[0];
        const width = find("chartBarWidth"); width.numberBox.commitValue(60);
        const gap = find("chartBarSpacing"); gap.numberBox.commitValue(25);
        compare(chart.barGroupWidth, 0.6); compare(chart.barSpacing, 0.25);
        const border = find("chartBarBorderWidth"); border.value = 3; border.valueModified();
        find("chartBarBorderColor").activated(1);
        compare(series.barBorderWidth, 3); compare(series.barBorderColor, Theme.tokens.primary);
        const labels = find("chartBarLabels"); labels.click();
        find("chartBarLabelPosition").activated(1); compare(series.barLabelPosition, CatChartSeries.BarLabelInsideEnd);
        const digits = find("chartBarLabelPrecision"); digits.value = 3; digits.valueModified();
        const margin = find("chartBarLabelMargin"); margin.value = 8; margin.valueModified();
        compare(series.barLabelPrecision, 3); compare(series.barLabelMargin, 8);
        find("chartBarLayout").activated(5);
        find("chartBarLabelFormat").activated(3);
        compare(series.barLabelFormat, "@value (@percentage%)");
        const raw = series.at(0);
        GalleryLanguage.language = "zh_CN";
        compare(series.at(0).y, raw.y); compare(chart.barSpacing, 0.25);
        compare(series.barLabelPosition, CatChartSeries.BarLabelInsideEnd, "Position must survive language change");
        tryCompare(find("chartBarLabelPosition"), "currentIndex", 1);
        tryCompare(find("chartBarLabelPosition"), "displayText", "\u67f1\u5185\u672b\u7aef");
        compare(series.barLabelsVisible, true);
        find("chartAddButton").clicked(); compare(series.barBorderWidth, 3);
        find("chartBarLayout").activated(4); compare(series.barLabelFormat, "@value");
        find("chartRestoreButton").clicked();
        compare(chart.barGroupWidth, 0.8); compare(chart.barSpacing, 0);
        compare(series.barLabelsVisible, false); compare(series.barBorderWidth, 0);
        GalleryLanguage.language = "en";
    }
    function test_barVariants_data() {
        return [0,1,2,3,4,5].map(value => ({ tag: String(value), variant: value }));
    }
    function test_barVariants(data) {
        choose(2);
        const chart = find("demoChart");
        const source = chart.series[0].at(0);
        find("chartBarLayout").activated(data.variant);
        const types = [CatChartSeries.Bar, CatChartSeries.StackedBar, CatChartSeries.PercentBar,
            CatChartSeries.HorizontalBar, CatChartSeries.HorizontalStackedBar, CatChartSeries.HorizontalPercentBar];
        compare(chart.chartType, types[data.variant]);
        const categoryAxis = data.variant >= 3 ? chart.yAxis : chart.xAxis;
        compare(categoryAxis.scale, CatChartAxis.Category);
        compare(categoryAxis.categories.length, 6);
        compare(chart.series[0].at(0).y, source.y);
        find("chartAddButton").clicked();
        compare(categoryAxis.categories.length, 7);
        GalleryLanguage.language = "zh_CN";
        compare(chart.chartType, types[data.variant]);
        compare(categoryAxis.categories[0], "一月");
        find("chartClearButton").clicked();
        compare(chart.series[0].count, 0);
        find("chartRestoreButton").clicked();
        compare(chart.chartType, CatChartSeries.Bar);
        GalleryLanguage.language = "en";
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
        compare(find("chartDemoPicker").currentValue, data.mode);
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
