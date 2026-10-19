// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/ChartAxis.h>
#include <QCatUiKit/ChartBoxPlot.h>
#include <QCatUiKit/ChartBubble>
#include <QCatUiKit/ChartColorScale.h>
#include <QCatUiKit/ChartContour.h>
#include <QCatUiKit/ChartFit>
#include <QCatUiKit/ChartGrid>
#include <QCatUiKit/ChartHistogram.h>
#include <QCatUiKit/ChartItem>
#include <QCatUiKit/ChartOhlc>
#include <QCatUiKit/ChartScene3D>
#include <QCatUiKit/ChartSeries3D.h>
#include <QCatUiKit/ChartStack>
#include <QCatUiKit/ChartWaterfall.h>
#include <QCatUiKit/qcatchartseries.h>
#include <QDebug>
#include <QGuiApplication>
#include <QEventLoop>
#include <QImage>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTimer>
#include <QQmlComponent>
#include <QQmlEngine>
#include <memory>
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (argc != 2)
        return 1;
    QCatUiKit::ChartItem chart;
    QCatUiKit::ChartHistogram histogram;
    histogram.setBinEdges({0, 1, 3});
    histogram.setNormalization(QCatUiKit::ChartHistogram::Density);
    if (!histogram.setData({0, .5, 1, 3}) || histogram.series()->count() != 2 ||
        histogram.series()->at(1).value("binCount").toInt() != 2)
        return 46;
    QCatUiKit::ChartSeries source;
    source.replacePoints({{0, 2}, {1, 4}});
    QCatUiKit::ChartFit fit;
    fit.setSource(&source);
    if (!fit.refresh() || fit.predict(.5) != 3 || fit.band()->count() != 2 || fit.slope() != 2)
        return 50;
    QCatUiKit::ChartWaterfall waterfall;
    if (!waterfall.setData({QVariantMap{{"value", 10}}, QVariantMap{{"value", -3}},
                            QVariantMap{{"kind", QCatUiKit::ChartWaterfall::Total}}}) ||
        waterfall.finalValue() != 7 || waterfall.series()->sampleAt(1).value != -3 ||
        waterfall.series()->type() != QCatUiKit::ChartSeries::Waterfall)
        return 51;
    QCatUiKit::ChartStack stack;
    stack.addSource(&source);
    stack.setPercentage(true);
    if (!stack.refresh() || stack.seriesAt(0)->sampleAt(1).y != 100 ||
        stack.seriesAt(0)->at(1).value("sourceY").toDouble() != 4)
        return 47;
    if (chart.renderingStatistics().value("backend").toString() != QStringLiteral("uninitialized"))
        return 7;
    QCatUiKit::ChartSeries series;
    series.setCurveMode(QCatUiKit::ChartSeries::NaturalSpline);
    series.setMarkerShape(QCatUiKit::ChartSeries::Diamond);
    series.setMarkerSize(12);
    if (series.curveMode() != QCatUiKit::ChartSeries::NaturalSpline || series.markerSize() != 12)
        return 8;
    series.setLineCap(QCatUiKit::ChartSeries::RoundCap);
    series.setLineJoin(QCatUiKit::ChartSeries::MiterJoin);
    series.setMiterLimit(3);
    if (series.lineCap() != QCatUiKit::ChartSeries::RoundCap || series.miterLimit() != 3)
        return 30;
    series.setFillGradientStops({QVariantMap{{"position", 0}, {"color", QColor(Qt::red)}},
                                 QVariantMap{{"position", 1}, {"color", QColor(Qt::blue)}}});
    series.setFillTextureMode(QCatUiKit::ChartSeries::TileTexture);
    series.setFillTextureSize({32, 64});
    if (series.fillGradientStops().size() != 2 || series.fillTextureSize().height() != 64)
        return 32;
    series.setBarLabelRotation(45);
    series.setMarkerTexture(QUrl("qrc:/missing-marker.png"));
    if (series.barLabelRotation() != 45 || series.markerTextureError().isEmpty())
        return 35;
    series.setCapacity(3);
    chart.setSize({640, 360});
    chart.xAxis()->setReversed(true);
    chart.yAxis()->setMinorTickCount(3);
    if (!chart.xAxis()->reversed() || chart.yAxis()->minorTickCount() != 3)
        return 26;
    QCatUiKit::ChartAxis right;
    right.setSide(QCatUiKit::ChartAxis::FarSide);
    series.setYAxis(&right);
    chart.addSeries(&series);
    if (chart.axisForSeries(&series, Qt::Vertical) != &right)
        return 28;
    series.setYAxis(nullptr);
    if (!series.appendData({QVariantMap{{"x", 0}, {"y", 2}}, QVariantMap{{"x", 1}, {"y", 4}},
                            QVariantMap{{"x", 2}, {"y", 1}}, QVariantMap{{"x", 3}, {"y", 3}}}))
        return 2;
    if (series.count() != 3 || series.sampleAt(0).x != 1)
        return 3;
    if (chart.dataAt(chart.mapToPosition({3, 3})).value("index").toInt() != 2)
        return 4;
    QCatUiKit::ChartSeries band;
    if (!band.replaceRangeSamples({{0, -1, 2, {}, false}, {1, 0, 3, {}, true}}) ||
        !band.breakBefore(1) || band.lowerValue(0) != -1)
        return 10;
    if (!band.setRangeBoundaries({QVariantMap{{"x", 0}, {"y", 1}}, QVariantMap{{"x", 2}, {"y", 3}}},
                                 {QVariantMap{{"x", 0}, {"y", 4}}, QVariantMap{{"x", 1}, {"y", 5}},
                                  QVariantMap{{"x", 2}, {"y", 6}}},
                                 QCatUiKit::ChartSeries::LinearAlignment) ||
        band.count() != 3 || band.lowerValue(1) != 2)
        return 34;
    chart.setChartType(QCatUiKit::ChartSeries::Doughnut);
    chart.setMultiRing(true);
    chart.setHoleSize(.3);
    chart.setRingGap(8);
    chart.setPieRadius(.9);
    chart.setSliceLabelsVisible(false);
    if (!chart.setPieAngles(-90, 180) || chart.pieStartAngle() != -90 ||
        !series.setSliceStyle(0, {{"color", QColor(Qt::red)}, {"explodeDistance", 12.}}) ||
        !chart.setSliceVisible(0, 1, false) || chart.isSliceVisible(0, 1))
        return 12;
    chart.setBarGroupWidth(.6);
    chart.setBarSpacing(.2);
    series.setBarBorderWidth(3);
    series.setBarBorderColor(Qt::white);
    series.setBarLabelsVisible(true);
    series.setBarLabelPosition(QCatUiKit::ChartSeries::BarLabelInsideEnd);
    series.setBarLabelFormat("@value");
    series.setBarLabelPrecision(4);
    series.setBarLabelMargin(8);
    if (chart.barGroupWidth() != .6 || chart.barSpacing() != .2 || series.barBorderWidth() != 3 ||
        !series.barLabelsVisible() ||
        series.barLabelPosition() != QCatUiKit::ChartSeries::BarLabelInsideEnd)
        return 14;
    series.setAreaBaselineMode(QCatUiKit::ChartSeries::CustomBaseline);
    series.setAreaBaseline(-3);
    series.setFillStyle(QCatUiKit::ChartSeries::VerticalGradient);
    series.setFillColor(Qt::red);
    series.setFillEndColor(Qt::transparent);
    series.setBorderColor(Qt::white);
    if (series.areaBaseline() != -3 ||
        series.fillStyle() != QCatUiKit::ChartSeries::VerticalGradient ||
        series.borderColor() != QColor(Qt::white))
        return 16;
    chart.setChartType(QCatUiKit::ChartSeries::Line);
    chart.setDragMode(QCatUiKit::ChartItem::RectangleZoomDrag);
    chart.setRectangleZoomAxes(QCatUiKit::ChartItem::ZoomXAxis);
    if (!chart.zoomToRect(chart.plotRect()) || chart.zoomSelecting() ||
        !chart.zoomSelection().isEmpty())
        return 18;
    chart.cancelZoomSelection();
    if (!chart.touchNavigationEnabled())
        return 20;
    chart.setTouchNavigationEnabled(false);
    if (chart.touchNavigationEnabled())
        return 21;
    QCatUiKit::ChartItem rangeChart;
    rangeChart.setSize({800, 600});
    rangeChart.setChartType(QCatUiKit::ChartSeries::RangeArea);
    rangeChart.setViewRange(-.5, 3.5, -3, 3);
    band.replaceRangeSamples({{0, -1, 1}, {1, -2, 2}, {2, -2, 2}, {3, -1, 1}});
    band.setCurveMode(QCatUiKit::ChartSeries::NaturalSpline);
    rangeChart.addSeries(&band);
    const auto hit = rangeChart.dataAt(rangeChart.mapToPosition({1.5, -2.15}));
    if (hit.value("boundary").toString() != QStringLiteral("lower") ||
        hit.value("lower").toDouble() != -2)
        return 23;
    chart.setSliceLabelPosition(QCatUiKit::ChartItem::SliceLabelOutside);
    if (chart.sliceLabelPosition() != QCatUiKit::ChartItem::SliceLabelOutside)
        return 24;
    QQmlEngine engine;
    engine.addImportPath(QString::fromLocal8Bit(argv[1]));
    QQmlComponent component(&engine);
    component.setData(R"(import QtQuick
import QCatUiKit
CatChart {
    delegateBudget: 12; selectionMode: CatChart.MultiSelection
    selectionColor: "yellow"; selectionWidth: 3; animationsEnabled: true
    width: 640; height: 360
    touchNavigationEnabled: false
    chartType: CatChartSeries.RangeArea
    dragMode: CatChart.RectangleZoomDrag; rectangleZoomAxes: CatChart.ZoomYAxis
    barGroupWidth: 0.6; barSpacing: 0.2
    pieStartAngle: -90; pieEndAngle: 180; pieRadius: 0.9; holeSize: 0.3
    multiRing: true; ringGap: 8; sliceLabelsVisible: false
    sliceLabelPosition: CatChart.SliceLabelOutside
    xAxis.reversed: true; yAxis.minorTickCount: 3
    xAxis.scale: CatChartAxis.Category
    xAxis.categories: ["A", "B"]
    CatChartSeries {
        name: "Orders"
        yAxis: CatChartAxis { side: CatChartAxis.FarSide; visible: false }
        barBorderWidth: 3; barBorderColor: "white"; barLabelsVisible: true
        barLabelPosition: CatChartSeries.BarLabelOutsideEnd; barLabelFormat: "@value"
        barLabelPrecision: 4; barLabelMargin: 8; barLabelRotation: 45
        markerTexture: "qrc:/missing-marker.png"
        curveMode: CatChartSeries.NaturalSpline
        markersVisible: true
        markerShape: CatChartSeries.Diamond
        markerSize: 12
        strokeStyle: CatChartSeries.DashLine
        lineCap: CatChartSeries.FlatCap; lineJoin: CatChartSeries.RoundJoin; miterLimit: 6
        dashPattern: [16, 8]
        fillGradientStops: [{position: 0, color: "red"}, {position: 0.4, color: "green"}, {position: 1, color: "blue"}]
        fillTextureMode: CatChartSeries.TileTexture; fillTextureSize: Qt.size(32, 64)
        fillOpacity: 0.4
        areaBaselineMode: CatChartSeries.PlotBottomBaseline; areaBaseline: 5
        fillStyle: CatChartSeries.HorizontalGradient; fillColor: "red"; fillEndColor: "transparent"; borderColor: "white"
        Component.onCompleted: setRangeData([{x: 0, lower: 0, upper: 2}, {x: 1, lower: 1, upper: 5, breakBefore: true}])
    }
})",
                      QUrl());
    std::unique_ptr<QObject> object(component.create());
    if (!object)
    {
        qWarning() << component.errors();
        return 5;
    }
    auto *qmlChart = qobject_cast<QCatUiKit::ChartItem *>(object.get());
    if (!qmlChart || qmlChart->seriesCount() != 1 || qmlChart->seriesAt(0)->count() != 2)
        return 6;
    if (qmlChart->seriesAt(0)->curveMode() != QCatUiKit::ChartSeries::NaturalSpline ||
        qmlChart->seriesAt(0)->markerShape() != QCatUiKit::ChartSeries::Diamond ||
        !qmlChart->seriesAt(0)->markersVisible())
        return 9;
    if (qmlChart->chartType() != QCatUiKit::ChartSeries::RangeArea ||
        !qmlChart->seriesAt(0)->breakBefore(1) || !qmlChart->seriesAt(0)->hasRangeValue(0) ||
        qmlChart->seriesAt(0)->strokeStyle() != QCatUiKit::ChartSeries::DashLine)
        return 11;
    if (qmlChart->seriesAt(0)->lineCap() != QCatUiKit::ChartSeries::FlatCap ||
        qmlChart->seriesAt(0)->lineJoin() != QCatUiKit::ChartSeries::RoundJoin)
        return 31;
    if (qmlChart->seriesAt(0)->fillGradientStops().size() != 3 ||
        qmlChart->seriesAt(0)->fillTextureMode() != QCatUiKit::ChartSeries::TileTexture)
        return 33;
    if (qmlChart->seriesAt(0)->barLabelRotation() != 45 ||
        qmlChart->seriesAt(0)->markerTextureError().isEmpty())
        return 36;
    qmlChart->setChartType(QCatUiKit::ChartSeries::Doughnut);
    if (!qmlChart->multiRing() || qmlChart->holeSize() != .3 || qmlChart->pieStartAngle() != -90 ||
        !qmlChart->seriesAt(0)->setSliceStyle(1, {{"explodeDistance", 12.}}))
        return 13;
    if (qmlChart->barSpacing() != .2 || qmlChart->seriesAt(0)->barLabelMargin() != 8 ||
        qmlChart->seriesAt(0)->barLabelPosition() != QCatUiKit::ChartSeries::BarLabelOutsideEnd)
        return 15;
    if (qmlChart->seriesAt(0)->areaBaselineMode() != QCatUiKit::ChartSeries::PlotBottomBaseline ||
        qmlChart->seriesAt(0)->fillStyle() != QCatUiKit::ChartSeries::HorizontalGradient ||
        qmlChart->seriesAt(0)->fillEndColor().alpha() != 0)
        return 17;
    if (qmlChart->dragMode() != QCatUiKit::ChartItem::RectangleZoomDrag ||
        qmlChart->rectangleZoomAxes() != QCatUiKit::ChartItem::ZoomYAxis ||
        qmlChart->zoomSelecting())
        return 19;
    if (qmlChart->touchNavigationEnabled())
        return 22;
    if (qmlChart->sliceLabelPosition() != QCatUiKit::ChartItem::SliceLabelOutside)
        return 25;
    if (!qmlChart->xAxis()->reversed() || qmlChart->yAxis()->minorTickCount() != 3)
        return 27;
    if (!qmlChart->seriesAt(0)->yAxis() || qmlChart->seriesAt(0)->yAxis()->isVisible() ||
        qmlChart->seriesAt(0)->yAxis()->side() != QCatUiKit::ChartAxis::FarSide)
        return 29;
    qmlChart->setDelegateBudget(1);
    qmlChart->setSelectionMode(QCatUiKit::ChartItem::MultiSelection);
    qmlChart->setSelectionColor(Qt::yellow);
    qmlChart->setSelectionWidth(3);
    qmlChart->setAnimationsEnabled(true);
    if (!qmlChart->selectPoint(0, 0) || !qmlChart->selectPoint(0, 1) ||
        qmlChart->selectedPoints().size() != 2 || !qmlChart->isPointSelected(0, 0) ||
        !qmlChart->animationsEnabled())
        return 37;
    QQmlComponent marker(&engine);
    marker.setData("import QtQuick; Rectangle { required property var chartData; color: "
                   "chartData.selected ? 'yellow' : chartData.color }",
                   QUrl());
    if (!marker.isReady())
        return 38;
    qmlChart->setChartType(QCatUiKit::ChartSeries::Scatter);
    qmlChart->seriesAt(0)->setPointDelegate(&marker);
    qmlChart->plotRect();
    if (qmlChart->activeDelegateCount() != 1 || !qmlChart->delegateError().isEmpty())
        return 39;
    qmlChart->seriesAt(0)->setPointDelegate(nullptr);
    qmlChart->plotRect();
    if (qmlChart->activeDelegateCount() != 0)
        return 40;
    qmlChart->clearSelection();
    if (!qmlChart->selectedPoints().isEmpty())
        return 41;
    QCatUiKit::ChartSeries3D xyz;
    xyz.setType(QCatUiKit::ChartSeries3D::Surface);
    if (!xyz.setSamples({{0, 0, 0}, {1, 1, 0}, {0, -1, 1}, {1, 0, 1}}, 2, 2))
        return 42;
    auto *scene = chart.scene3D();
    scene->addSeries(&xyz);
    scene->setEnabled(true);
    scene->setAmbient(.3);
    scene->setLightingModel(QCatUiKit::ChartScene3D::BlinnPhong);
    scene->setAmbientGroundRatio(.4);
    scene->setExposure(1.2);
    xyz.setShape(QCatUiKit::ChartSeries3D::RoundedCube);
    xyz.setMeshDetail(QCatUiKit::ChartSeries3D::HighDetail);
    if (scene->ambientGroundRatio() != .4 || scene->exposure() != 1.2 ||
        xyz.meshDetail() != QCatUiKit::ChartSeries3D::HighDetail)
        return 45;
    scene->setProjection(QCatUiKit::ChartScene3D::Orthographic);
    scene->setSelectionMode(QCatUiKit::ChartScene3D::RowSelection);
    scene->setSliceMode(QCatUiKit::ChartScene3D::RowSlice);
    scene->setSliceIndex(1);
    if (!scene->selectPoint(0, 2) || scene->selectedPoint().value("row").toInt() != 1 ||
        scene->ambient() != .3)
        return 43;
    scene->resetView();
    scene->removeSeries(&xyz);
    QQmlComponent three(&engine);
    three.setData(R"(import QtQuick
        import QCatUiKit
        CatChart {
            scene3D.enabled: true
            scene3D.projection: CatChartScene3D.Orthographic
            scene3D.lightingModel: CatChartScene3D.Phong
            scene3D.ambientGroundRatio: 0.5
            scene3D.exposure: 2
            scene3D.series: [CatChartSeries3D {
                shape: CatChartSeries3D.RoundedCube
                meshDetail: CatChartSeries3D.LowDetail
                points: [{x:1, y:2, z:3}]
            }]
        })",
                  QUrl());
    std::unique_ptr<QObject> object3D(three.create());
    auto *chart3D = qobject_cast<QCatUiKit::ChartItem *>(object3D.get());
    if (!chart3D || chart3D->scene3D()->seriesCount() != 1 ||
        chart3D->scene3D()->seriesAt(0)->count() != 1)
        return 44;
    QQmlComponent statistics(&engine);
    statistics.setData(R"(import QCatUiKit
        CatHistogram { values: [0, 1, 2]; binEdges: [0, 1, 2]; normalization: CatHistogram.Frequency })",
                       QUrl());
    std::unique_ptr<QObject> statisticsObject(statistics.create());
    const auto *qmlHistogram = qobject_cast<QCatUiKit::ChartHistogram *>(statisticsObject.get());
    if (!qmlHistogram || qmlHistogram->series()->count() != 2 || qmlHistogram->includedCount() != 3)
        return 48;
    QQmlComponent stackComponent(&engine);
    stackComponent.setData(R"(import QtQuick
        import QCatUiKit
        CatChartStack {
            percentage: true
            CatChartSeries { Component.onCompleted: setData([{x:0,y:2}, {x:1,y:3}]) }
        })",
                           QUrl());
    std::unique_ptr<QObject> stackObject(stackComponent.create());
    auto *qmlStack = qobject_cast<QCatUiKit::ChartStack *>(stackObject.get());
    if (!qmlStack || !qmlStack->refresh() || qmlStack->seriesAt(0)->sampleAt(0).y != 100)
        return 49;
    QQmlComponent fitComponent(&engine);
    fitComponent.setData(R"(import QtQuick
        import QCatUiKit
        CatChartFit { source: CatChartSeries {
            Component.onCompleted: setData([{x:0,y:1},{x:2,y:5}])
        } })",
                         QUrl());
    std::unique_ptr<QObject> fitObject(fitComponent.create());
    auto *qmlFit = qobject_cast<QCatUiKit::ChartFit *>(fitObject.get());
    if (!qmlFit || !qmlFit->refresh() || qmlFit->predict(1) != 3)
        return 52;
    QQmlComponent waterfallComponent(&engine);
    waterfallComponent.setData(R"(import QCatUiKit
        CatWaterfall { steps: [{value:8},{value:-5},{kind:CatWaterfall.Total}]
            increaseColor: "green"; decreaseColor: "red"; connectorsVisible: false
        })",
                               QUrl());
    std::unique_ptr<QObject> waterfallObject(waterfallComponent.create());
    auto *qmlWaterfall = qobject_cast<QCatUiKit::ChartWaterfall *>(waterfallObject.get());
    if (!qmlWaterfall || qmlWaterfall->finalValue() != 3 || qmlWaterfall->connectorsVisible() ||
        qmlWaterfall->series()->at(1).value("barColor").value<QColor>() != QColor(Qt::red))
        return 53;
    QCatUiKit::ChartOhlc prices;
    if (!prices.setData({QVariantMap{
            {"time", 1}, {"open", 2}, {"close", 3}, {"low", 1}, {"high", 4}, {"volume", 5}}}) ||
        prices.series()->type() != QCatUiKit::ChartSeries::Candlestick)
        return 54;
    QCatUiKit::ChartBoxPlot box;
    if (!box.setData({QVariantMap{{"values", QVariantList{1, 2, 3, 4, 10}}}}) ||
        box.outlierSeries()->count() != 1)
        return 55;
    QCatUiKit::ChartGrid grid;
    if (!grid.setGrid(2, 2, {-1, 1, -1, 1}) || !grid.colorScale()->setRange(-1, 1))
        return 56;
    QCatUiKit::ChartContour contours;
    contours.setSource(&grid);
    contours.setLevels({0});
    if (!contours.refresh() || contours.series()->count() == 0)
        return 57;
    QCatUiKit::ChartBubble bubbles;
    bubbles.setSizeMode(QCatUiKit::ChartBubble::Area);
    if (!bubbles.setData({QVariantMap{{"x", 0}, {"y", 1}, {"size", 4}}}))
        return 58;
    chart.setRadarRanges({QVariantMap{{"minimum", 0}, {"maximum", 100}}});
    if (chart.radarRanges().size() != 1)
        return 59;
    QQmlComponent extended(&engine);
    extended.setData(R"(import QtQuick
        import QCatUiKit
        Item {
            CatChartOhlc { records: [{time:1,open:2,high:4,low:1,close:3,volume:5}] }
            CatChartBoxPlot { groups: [{values:[1,2,3,4,10]}] }
            CatChartBubble { points: [{x:0,y:1,size:4,colorValue:1}]; sizeMode:CatChartBubble.Area }
            CatChartColorScale { colors:["red","blue"]; Component.onCompleted:setRange(0,1) }
            CatChartGrid { id:grid; Component.onCompleted:setGrid(2,2,[-1,1,-1,1]) }
            CatChartContour { source:grid; levels:[0]; filled:true }
            CatChart { radarRanges:[{minimum:0,maximum:10}] }
        })",
                     QUrl());
    std::unique_ptr<QObject> extendedObject(extended.create());
    if (!extendedObject)
    {
        qWarning() << extended.errors();
        return 60;
    }
    QTemporaryDir exportDirectory;
    QQuickWindow exportWindow;
    exportWindow.resize(640, 480);
    QCatUiKit::ChartItem exportedChart(exportWindow.contentItem());
    exportedChart.setSize({640, 480});
    exportedChart.setTitle(QStringLiteral("Installed export"));
    exportedChart.addSeries(&source);
    exportWindow.show();
    QEventLoop exportLoop;
    bool exportSucceeded = false;
    const QUrl exportUrl = QUrl::fromLocalFile(exportDirectory.filePath("chart.png"));
    QObject::connect(&exportedChart, &QCatUiKit::ChartItem::imageExportFinished, &exportLoop,
                     [&](QUrl destination, bool success, const QString &error)
                     {
                         exportSucceeded = destination == exportUrl && success && error.isEmpty();
                         exportLoop.quit();
                     });
    QTimer::singleShot(0, &exportLoop, [&]
    {
        if (!exportedChart.exportImage(exportUrl, QSize(320, 240)))
            exportLoop.quit();
    });
    QTimer::singleShot(20000, &exportLoop, &QEventLoop::quit);
    exportLoop.exec();
    if (!exportSucceeded || exportedChart.exportingImage() ||
        QImage(exportUrl.toLocalFile()).size() != QSize(320, 240))
        return 61;
    qInfo() << "Installed chart C++ and QML consumer OK, including PNG export";
    return 0;
}
