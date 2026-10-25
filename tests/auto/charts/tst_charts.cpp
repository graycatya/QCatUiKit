// SPDX-License-Identifier: MIT
#include "qcatchartlabels_p.h"
#include "qcatchartmath_p.h"
#include <QCatUiKit/qcatchartitem.h>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScopeGuard>
#include <QtTest>
#include <limits>
using namespace QCatUiKit;
class TestChart : public ChartItem
{
  public:
    using ChartItem::keyPressEvent;
    using ChartItem::mouseMoveEvent;
    using ChartItem::mousePressEvent;
    using ChartItem::mouseReleaseEvent;
    using ChartItem::mouseUngrabEvent;
    using ChartItem::touchEvent;
    using ChartItem::touchUngrabEvent;
    using ChartItem::updatePolish;
};
class ChartTests : public QObject
{
    Q_OBJECT
  private slots:
    void extremeClippedBarPicking()
    {
        for (bool horizontal : {false, true})
            for (double sign : {-1., 1.})
            {
                TestChart chart;
                chart.setSize({640, 480});
                chart.setChartType(horizontal ? ChartSeries::HorizontalBar : ChartSeries::Bar);
                ChartSeries series;
                series.replacePoints({{5, sign * 1e300}});
                chart.addSeries(&series);
                if (horizontal)
                    chart.setViewRange(-10, 10, 0, 10);
                else
                    chart.setViewRange(0, 10, -10, 10);
                const auto point =
                    chart.mapToPosition(horizontal ? QPointF(sign * 5, 5) : QPointF(5, sign * 5));
                QCOMPARE(chart.dataAt(point).value("index", -1).toInt(), 0);
                QCOMPARE(chart.renderedPointCount(), 1);
            }
    }
    void leaderCrossingPriorityAndOverlap()
    {
        using namespace QCatUiKit::Detail;
        QVERIFY(chartLeaderSegmentsCross({0, 0}, {10, 10}, {0, 10}, {10, 0}));
        QVERIFY(chartLeaderSegmentsCross({0, 0}, {10, 0}, {5, 0}, {15, 0}));
        QVERIFY(!chartLeaderSegmentsCross({0, 0}, {10, 0}, {0, 2}, {10, 2}));
        const auto routes = nonCrossingChartLeaders(
            {{{{0, 0}, {10, 10}}, 0, 1}, {{{0, 10}, {10, 0}}, 1, 2}, {{{0, 20}, {10, 20}}, 2, 1}});
        QCOMPARE(routes.size(), 2);
        QCOMPARE(routes[0].index, 1);
        QCOMPARE(routes[1].index, 2);
        // A route cannot fold back across itself.
        QVERIFY(nonCrossingChartLeaders({{{{0, 0}, {10, 10}, {0, 10}, {10, 0}}, 0, 1}}).isEmpty());
    }
    void selectionContractAndLifetime()
    {
        TestChart chart;
        chart.setSize({640, 480});
        ChartSeries first, second;
        first.replacePoints({{1, 1}, {2, 2}});
        second.replacePoints({{1, 3}});
        chart.addSeries(&first);
        chart.addSeries(&second);
        QSignalSpy changed(&chart, &ChartItem::selectionChanged);
        QCOMPARE(chart.selectionMode(), ChartItem::NoSelection);
        QVERIFY(!chart.selectPoint(-1, 0));
        QVERIFY(!chart.selectPoint(0, 2));
        chart.setSelectionMode(ChartItem::MultiSelection);
        QVERIFY(chart.selectPoint(0, 1));
        QVERIFY(chart.selectPoint(1, 0));
        QCOMPARE(chart.selectedPoints().size(), 2);
        QCOMPARE(changed.size(), 2);
        QVERIFY(chart.selectPoint(0, 1));
        QCOMPARE(changed.size(), 2);
        QCOMPARE(chart.dataAt(chart.mapToPosition({2, 2})).value("selected").toBool(), true);
        first.setColor(Qt::red);
        QVERIFY(chart.isPointSelected(0, 1));
        first.setVisible(false);
        QVERIFY(chart.isPointSelected(0, 1));
        first.append(3, 3);
        QVERIFY(!chart.isPointSelected(0, 1));
        QVERIFY(chart.isPointSelected(1, 0));
        chart.removeSeries(&second);
        QVERIFY(chart.selectedPoints().isEmpty());
        chart.setSelectionMode(ChartItem::SingleSelection);
        QVERIFY(chart.selectPoint(0, 0));
        QVERIFY(chart.selectPoint(0, 1));
        QCOMPARE(chart.selectedPoints().size(), 1);
        QVERIFY(chart.isPointSelected(0, 1));
        chart.setSelectionWidth(4);
        chart.setSelectionWidth(NAN);
        chart.setSelectionWidth(9);
        QCOMPARE(chart.selectionWidth(), 4.);
        chart.setSelectionColor(Qt::magenta);
        QCOMPARE(chart.selectionColor(), QColor(Qt::magenta));
        chart.clearSeries();
        QVERIFY(chart.selectedPoints().isEmpty());
    }
    void spatialPickingLimitsCandidates()
    {
        TestChart chart;
        chart.setSize({1200, 800});
        chart.setChartType(ChartSeries::Scatter);
        ChartSeries series;
        series.setMarkerSize(3);
        QVector<QPointF> points;
        for (int y = 0; y < 100; ++y)
            for (int x = 0; x < 100; ++x)
                points.append({double(x), double(y)});
        series.replacePoints(points);
        chart.addSeries(&series);
        chart.setViewRange(-1, 100, -1, 100);
        QCOMPARE(chart.dataAt(chart.mapToPosition({50, 50})).value("index").toInt(), 5050);
        QVERIFY(chart.renderingStatistics().value("pickCandidates").toInt() < 10);
        QVERIFY(chart.renderingStatistics().value("pickNodesVisited").toInt() < 100);
        chart.panBy({20, 10});
        QCOMPARE(chart.dataAt(chart.mapToPosition({50, 50})).value("index").toInt(), 5050);
        series.clear();
        QVERIFY(chart.dataAt(chart.mapToPosition({50, 50})).isEmpty());
    }
    void delegateLifecycleAndBudget()
    {
        TestChart chart;
        ChartSeries series;
        QCOMPARE(chart.delegateBudget(), 128);
        chart.setDelegateBudget(8);
        chart.setDelegateBudget(-1);
        chart.setDelegateBudget(1025);
        QCOMPARE(chart.delegateBudget(), 8);
        QQmlEngine engine;
        auto *component = new QQmlComponent(&engine);
        component->setData(
            "import QtQuick; Rectangle { required property var chartData; color: chartData.color }",
            QUrl());
        QVERIFY2(component->isReady(), qPrintable(component->errorString()));
        series.setPointDelegate(component);
        series.setBarDelegate(component);
        series.setLabelDelegate(component);
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Scatter);
        series.replacePoints({{1, 1}, {2, 2}});
        chart.addSeries(&series);
        chart.plotRect();
        QCOMPARE(chart.activeDelegateCount(), 2);
        delete component;
        QCOMPARE(series.pointDelegate(), nullptr);
        QCOMPARE(series.barDelegate(), nullptr);
        QCOMPARE(series.labelDelegate(), nullptr);
        chart.plotRect();
        QCOMPARE(chart.activeDelegateCount(), 0);
        QVERIFY(chart.delegateError().isEmpty());
    }
    void textureMarkerAndLabelContract()
    {
        ChartSeries series;
        series.setMarkerShape(ChartSeries::TextureMarker);
        QCOMPARE(series.markerShape(), ChartSeries::TextureMarker);
        series.setMarkerTexture(QUrl("https://invalid.example/marker.png"));
        QVERIFY(!series.markerTextureError().isEmpty());
        series.setMarkerTexture({});
        QVERIFY(series.markerTextureError().isEmpty());
        series.setBarLabelRotation(90);
        series.setBarLabelRotation(NAN);
        series.setBarLabelRotation(181);
        QCOMPARE(series.barLabelRotation(), 90.);
        series.clear();
        QCOMPARE(series.barLabelRotation(), 90.);
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::Scatter);
        series.setMarkerSize(40);
        series.replacePoints({{5, 5}});
        chart.addSeries(&series);
        chart.setViewRange(0, 10, 0, 10);
        const auto center = chart.mapToPosition({5, 5});
        QCOMPARE(chart.dataAt(center + QPointF(17, -17)).value("index", -1).toInt(), 0);
    }
    void independentRangeBoundaries()
    {
        ChartSeries series;
        QSignalSpy changed(&series, &ChartSeries::dataChanged);
        const QVariantList low{QVariantMap{{"x", 0}, {"y", 0}}, QVariantMap{{"x", 2}, {"y", 2}},
                               QVariantMap{{"x", 4}, {"y", 0}}};
        const QVariantList high{QVariantMap{{"x", 0}, {"y", 4}}, QVariantMap{{"x", 1}, {"y", 2}},
                                QVariantMap{{"x", 4}, {"y", 6}}};
        QVERIFY(!series.setRangeBoundaries(low, high));
        QCOMPARE(series.count(), 0);
        QCOMPARE(changed.size(), 0);
        QVERIFY(series.setRangeBoundaries(low, high, ChartSeries::LinearAlignment));
        QCOMPARE(series.count(), 4);
        QCOMPARE(changed.size(), 1);
        QCOMPARE(series.sampleAt(1).x, 1.);
        QCOMPARE(series.lowerValue(1), 1.);
        QCOMPARE(series.sampleAt(2).x, 2.);
        QVERIFY(std::abs(series.sampleAt(2).y - 10. / 3) < 1e-12);
        const auto before = series.at(2);
        QVERIFY(!series.setRangeBoundaries({low[1], low[0]}, high, ChartSeries::LinearAlignment));
        QVERIFY(!series.setRangeBoundaries({low[0], low[0]}, high, ChartSeries::LinearAlignment));
        QVERIFY(!series.setRangeBoundaries(low, {}, ChartSeries::LinearAlignment));
        QCOMPARE(series.at(2), before);
        QCOMPARE(changed.size(), 1);
        QVERIFY(series.setRangeBoundaries(low, low));
        QCOMPARE(series.count(), 3);
        const QVariantList broken{QVariantMap{{"x", 0}, {"y", 0}},
                                  QVariantMap{{"x", 3}, {"y", 3}, {"breakBefore", true}},
                                  QVariantMap{{"x", 4}, {"y", 4}}};
        QVERIFY(series.setRangeBoundaries(broken, high, ChartSeries::LinearAlignment));
        QCOMPARE(series.count(), 3);
        QCOMPARE(series.sampleAt(1).x, 3.);
        QVERIFY(series.breakBefore(1));
        // Only the shared domain is aligned; no extrapolated point appears at zero or four.
        QVERIFY(series.setRangeBoundaries(
            low, {QVariantMap{{"x", 1}, {"y", 5}}, QVariantMap{{"x", 3}, {"y", 5}}},
            ChartSeries::LinearAlignment));
        QCOMPARE(series.count(), 3);
        QCOMPARE(series.sampleAt(0).x, 1.);
        QCOMPARE(series.sampleAt(2).x, 3.);
        QVERIFY(series.setRangeBoundaries({}, {}));
        QCOMPARE(series.count(), 0);
    }
    void denseRangePan_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = 0; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void denseRangePan()
    {
        QFETCH(int, mode);
        TestChart chart;
        chart.setSize({1440, 400});
        chart.setChartType(ChartSeries::RangeArea);
        ChartSeries series;
        series.setCapacity(100000);
        series.setCurveMode(ChartSeries::CurveMode(mode));
        QVector<ChartSeries::RangeSample> samples;
        for (int i = 0; i < 100000; ++i)
            samples.append({double(i),
                            i == 4321 ? -10. : -2 + std::cos(i * .01),
                            i == 1234 ? 10. : 2 + std::sin(i * .01),
                            {},
                            i == 50000});
        series.replaceRangeSamples(samples);
        chart.addSeries(&series);
        chart.updatePolish();
        const auto builds = chart.renderingStatistics().value("curveCacheBuilds").toULongLong();
        QCOMPARE(builds, 1ULL);
        QVERIFY(chart.renderedPointCount() < 12000);
        for (int i = 0; i < 3; ++i)
        {
            chart.panBy({4, 2});
            chart.updatePolish();
            QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
            QVERIFY(chart.renderingStatistics().value("curveLodNodesVisited").toInt() < 6000);
            QVERIFY(chart.renderedPointCount() < 12000);
        }
        const auto peak = chart.dataAt(chart.mapToPosition({4321, -10}));
        QCOMPARE(peak.value("index").toInt(), 4321);
        QCOMPARE(peak.value("boundary").toString(), QString("lower"));
        QCOMPARE(peak.value("y").toDouble(), -10.);
        chart.setViewRange(4318, 4324, -12, 12);
        chart.updatePolish();
        QVERIFY(chart.renderingStatistics().value("curveLodNodesVisited").toInt() < 150);
        QCOMPARE(series.count(), 100000);
        QCOMPARE(series.lowerValue(4321), -10.);
        chart.xAxis()->setReversed(true);
        chart.yAxis()->setReversed(true);
        chart.updatePolish();
        QCOMPARE(chart.dataAt(chart.mapToPosition({4321, -10})).value("index").toInt(), 4321);
        chart.xAxis()->setScale(ChartAxis::Logarithmic);
        chart.updatePolish();
        QCOMPARE(chart.dataAt(chart.mapToPosition({4321, -10})).value("index").toInt(), 4321);
        series.setFillColor(Qt::red);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds + 1);
    }
    void gradientStopsContract()
    {
        ChartSeries series;
        QVERIFY(series.fillGradientStops().isEmpty());
        const QVariantList stops{QVariantMap{{"position", 0.2}, {"color", QColor(Qt::red)}},
                                 QVariantMap{{"position", 0.8}, {"color", QColor(Qt::blue)}}};
        QSignalSpy changed(&series, &ChartSeries::styleChanged);
        series.setFillGradientStops(stops);
        QCOMPARE(series.fillGradientStops(), stops);
        QCOMPARE(changed.size(), 1);
        series.setFillGradientStops({stops.first()});
        series.setFillGradientStops({stops.last(), stops.first()});
        series.setFillGradientStops({stops.first(), stops.first()});
        series.setFillGradientStops(
            {QVariantMap{{"position", NAN}, {"color", QColor(Qt::red)}}, stops.last()});
        QCOMPARE(series.fillGradientStops(), stops);
        QCOMPARE(changed.size(), 1);
        series.clear();
        QCOMPARE(series.fillGradientStops(), stops);
        series.setFillGradientStops({});
        QVERIFY(series.fillGradientStops().isEmpty());
        QCOMPARE(series.fillTextureMode(), ChartSeries::StretchTexture);
        series.setFillTextureSize({64, 16});
        series.setFillTextureSize({0, 10});
        QCOMPARE(series.fillTextureSize(), QSizeF(64, 16));
        series.setFillTexture(QUrl("https://invalid.example/texture.png"));
        QVERIFY(!series.fillTextureError().isEmpty());
        series.setFillTexture({});
        QVERIFY(series.fillTextureError().isEmpty());
        series.setFillStyle(ChartSeries::TextureFill);
        QCOMPARE(series.fillStyle(), ChartSeries::TextureFill);
    }
    void strokeStylesContract()
    {
        ChartSeries series;
        QSignalSpy changed(&series, &ChartSeries::styleChanged);
        QCOMPARE(series.lineCap(), ChartSeries::AutoCap);
        QCOMPARE(series.lineJoin(), ChartSeries::AutoJoin);
        QCOMPARE(series.miterLimit(), 4.);
        series.setLineCap(ChartSeries::RoundCap);
        series.setLineJoin(ChartSeries::MiterJoin);
        series.setMiterLimit(8);
        QCOMPARE(changed.count(), 3);
        series.setLineCap(ChartSeries::LineCapStyle(-1));
        series.setLineJoin(ChartSeries::LineJoinStyle(99));
        series.setMiterLimit(NAN);
        QCOMPARE(changed.count(), 3);
        series.setMiterLimit(100);
        QCOMPARE(series.miterLimit(), 16.);
        series.setMiterLimit(-1);
        QCOMPARE(series.miterLimit(), 1.);
        series.replacePoints({{0, 0}, {1, 1}});
        series.clear();
        QCOMPARE(series.lineCap(), ChartSeries::RoundCap);
    }
    void multiAxisMappingAndLifetime()
    {
        TestChart chart;
        chart.setSize({900, 560});
        chart.setLegendVisible(false);
        ChartSeries a, b;
        a.replacePoints({{0, 0}, {5, 5}, {10, 10}});
        b.replacePoints({{100, 1000}, {150, 2000}, {200, 3000}});
        ChartAxis extraX;
        extraX.setOrientation(Qt::Horizontal);
        extraX.setSide(ChartAxis::FarSide);
        auto extraY = std::make_unique<ChartAxis>();
        extraY->setSide(ChartAxis::FarSide);
        b.setXAxis(&extraX);
        b.setYAxis(extraY.get());
        chart.addSeries(&a);
        chart.addSeries(&b);
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        extraX.setRange(100, 200);
        extraY->setRange(1000, 3000);
        auto plot = chart.plotRect();
        QVERIFY(plot.left() > 50 && chart.width() - plot.right() > 50);
        QVERIFY(plot.top() > 70 && chart.height() - plot.bottom() > 50);
        QCOMPARE(chart.mapToPosition({5, 5}, &a), chart.mapToPosition({150, 2000}, &b));
        QCOMPARE(chart.mapToValue(plot.center(), &b), QPointF(150, 2000));
        QCOMPARE(chart.axisForSeries(&b, Qt::Horizontal), &extraX);
        QCOMPARE(chart.axisForSeries(&b, Qt::Vertical), extraY.get());
        extraY->setReversed(true);
        extraX.setReversed(true);
        const auto anchor = chart.mapToPosition({130, 2400}, &b);
        chart.zoomAt(2, anchor);
        QVERIFY(QLineF(chart.mapToPosition({130, 2400}, &b), anchor).length() < 1e-8);
        chart.panBy({12, 8});
        QVERIFY(QLineF(chart.mapToPosition({130, 2400}, &b), anchor + QPointF(12, 8)).length() <
                1e-8);
        const double untouched = chart.yAxis()->minimum(), extraUntouched = extraY->minimum();
        chart.setRectangleZoomAxes(ChartItem::ZoomXAxis);
        plot = chart.plotRect();
        QVERIFY(chart.zoomToRect(
            {plot.left() + plot.width() / 4, plot.top(), plot.width() / 2, plot.height()}));
        QCOMPARE(chart.yAxis()->minimum(), untouched);
        QCOMPARE(extraY->minimum(), extraUntouched);
        chart.resetView();
        QVERIFY(extraX.automatic());
        QVERIFY(extraY->automatic());
        chart.yAxis()->setVisible(false);
        chart.xAxis()->setVisible(false);
        extraY->setSide(ChartAxis::NearSide);
        extraX.setSide(ChartAxis::NearSide);
        plot = chart.plotRect();
        QVERIFY(plot.left() >= 76);
        QCOMPARE(chart.width() - plot.right(), 24.);
        // Destroying an associated axis returns to the chart default without retaining pointers.
        extraY.reset();
        QCOMPARE(b.yAxis(), nullptr);
        QCOMPARE(chart.axisForSeries(&b, Qt::Vertical), chart.yAxis());
        const auto mapped = chart.mapToPosition({150, 2000}, &b);
        QVERIFY(std::isfinite(mapped.x()) && std::isfinite(mapped.y()));
        chart.removeSeries(&b);
        const auto previous = chart.renderingStatistics().value("preparedRevision");
        chart.plotRect();
        const auto stable = chart.renderingStatistics().value("preparedRevision");
        extraX.setRange(1, 2);
        chart.plotRect();
        QCOMPARE(chart.renderingStatistics().value("preparedRevision"), stable);
        Q_UNUSED(previous);
    }
    void multiAxisRangesStacksAndPicking()
    {
        TestChart chart;
        chart.setSize({900, 600});
        ChartAxis small, large;
        large.setSide(ChartAxis::FarSide);
        ChartSeries a, b, c;
        for (auto s : {&a, &b, &c})
        {
            s->setType(ChartSeries::StackedBar);
            chart.addSeries(s);
        }
        a.setYAxis(&small);
        b.setYAxis(&small);
        c.setYAxis(&large);
        a.replacePoints({{0, 3}});
        b.replacePoints({{0, 2}});
        c.replacePoints({{0, 1000}});
        auto plot = chart.plotRect();
        QVERIFY(chart.mapToValue(plot.topLeft(), &a).y() < 6);
        QVERIFY(chart.mapToValue(plot.topLeft(), &c).y() > 1000);
        c.setVisible(false);
        QCOMPARE(chart.dataAt(chart.mapToPosition({0, 4}, &a)).value("seriesIndex").toInt(), 1);
        c.setVisible(true);
        a.setVisible(false);
        b.setVisible(false);
        QCOMPARE(chart.dataAt(chart.mapToPosition({0, 500}, &c)).value("y").toDouble(), 1000.);
        c.setType(ChartSeries::Line);
        c.setCurveMode(ChartSeries::NaturalSpline);
        c.replacePoints({{0, 1000}, {1, 2000}, {2, 1500}});
        large.setScale(ChartAxis::Logarithmic);
        large.setReversed(true);
        QCOMPARE(chart.dataAt(chart.mapToPosition({1, 2000}, &c)).value("index").toInt(), 1);
        // An incompatible orientation is observable through the effective-axis query.
        large.setOrientation(Qt::Horizontal);
        QCOMPARE(chart.axisForSeries(&c, Qt::Vertical), chart.yAxis());
        chart.plotRect();
        large.setOrientation(Qt::Vertical);
        QCOMPARE(chart.axisForSeries(&c, Qt::Vertical), &large);
        QCOMPARE(chart.dataAt(chart.mapToPosition({1, 2000}, &c)).value("index").toInt(), 1);
        c.setYAxis(nullptr);
        QCOMPARE(c.yAxis(), nullptr);
        QCOMPARE(chart.axisForSeries(&c, Qt::Vertical), chart.yAxis());
    }
    void dateTimeNavigationKeepsPlot_data()
    {
        QTest::addColumn<double>("span");
        QTest::addColumn<bool>("farSide");
        QTest::addColumn<int>("fontSize");
        for (double span : {100., 600000., 864000000.})
            for (bool farSide : {false, true})
                for (int fontSize : {12, 18})
                    QTest::newRow(qPrintable(QString("%1-%2-%3").arg(span).arg(farSide).arg(fontSize)))
                        << span << farSide << fontSize;
    }
    void dateTimeNavigationKeepsPlot()
    {
        QFETCH(double, span);
        QFETCH(bool, farSide);
        QFETCH(int, fontSize);
        TestChart chart;
        chart.setSize({800, 600});
        auto font = chart.font();
        font.setPixelSize(fontSize);
        chart.setFont(font);
        chart.yAxis()->setScale(ChartAxis::DateTime);
        chart.yAxis()->setSide(farSide ? ChartAxis::FarSide : ChartAxis::NearSide);
        QVERIFY(chart.setViewRange(1, 101, 1, 1 + span));
        const auto plot = chart.plotRect();
        const QPointF value(51, 1 + span * .8);
        const auto anchor = chart.mapToPosition(value);
        chart.zoomAt(2, anchor);
        QCOMPARE(chart.plotRect(), plot);
        QVERIFY(QLineF(chart.mapToPosition(value), anchor).length() < 1e-8);
        chart.panBy({21, -13});
        QCOMPARE(chart.plotRect(), plot);
        QVERIFY(QLineF(chart.mapToPosition(value), anchor + QPointF(21, -13)).length() < 1e-8);
    }
    void reversedAxisMapping_data()
    {
        QTest::addColumn<int>("scale");
        QTest::addColumn<int>("mask");
        for (int scale = 0; scale < 4; ++scale)
            for (int mask = 0; mask < 4; ++mask)
                QTest::newRow(qPrintable(QString("%1-%2").arg(scale).arg(mask))) << scale << mask;
    }
    void reversedAxisMapping()
    {
        QFETCH(int, scale);
        QFETCH(int, mask);
        TestChart chart;
        chart.setSize({800, 600});
        auto x = chart.xAxis(), y = chart.yAxis();
        QVERIFY(!x->reversed());
        QSignalSpy changed(x, &ChartAxis::changed);
        x->setReversed(mask & 1);
        y->setReversed(mask & 2);
        x->setReversed(mask & 1);
        QCOMPARE(changed.count(), mask & 1 ? 1 : 0);
        x->setScale(ChartAxis::Scale(scale));
        y->setScale(ChartAxis::Scale(scale));
        QVERIFY(chart.setViewRange(1, 101, 1, 101));
        ChartSeries series;
        series.replacePoints({{11, 21}, {51, 81}, {91, 31}});
        chart.addSeries(&series);
        auto plot = chart.plotRect();
        const auto minimum = chart.mapToPosition({1, 1}), maximum = chart.mapToPosition({101, 101});
        QCOMPARE(minimum.x(), mask & 1 ? plot.right() : plot.left());
        QCOMPARE(minimum.y(), mask & 2 ? plot.top() : plot.bottom());
        QCOMPARE(maximum.x(), mask & 1 ? plot.left() : plot.right());
        QCOMPARE(maximum.y(), mask & 2 ? plot.bottom() : plot.top());
        const auto point = chart.mapToPosition({51, 81});
        QVERIFY(QLineF(chart.mapToValue(point), {51, 81}).length() < 1e-9);
        QCOMPARE(chart.dataAt(point).value("index").toInt(), 1);
        chart.zoomAt(2, point);
        QVERIFY(QLineF(chart.mapToPosition({51, 81}), point).length() < 1e-8);
        chart.panBy({21, -13});
        QVERIFY(QLineF(chart.mapToPosition({51, 81}), point + QPointF(21, -13)).length() < 1e-8);
        plot = chart.plotRect();
        const QRectF selection(plot.left() + plot.width() * .1, plot.top() + plot.height() * .2,
                               plot.width() * .6, plot.height() * .4);
        const auto a = chart.mapToValue(selection.topLeft()),
                   b = chart.mapToValue(selection.bottomRight());
        QVERIFY(chart.zoomToRect(selection));
        QCOMPARE(x->minimum(), std::min(a.x(), b.x()));
        QCOMPARE(x->maximum(), std::max(a.x(), b.x()));
        QCOMPARE(y->minimum(), std::min(a.y(), b.y()));
        QCOMPARE(y->maximum(), std::max(a.y(), b.y()));
        chart.resetView();
        QCOMPARE(x->reversed(), bool(mask & 1));
        QCOMPARE(y->reversed(), bool(mask & 2));
        QCOMPARE(series.sampleAt(1).x, 51.);
        QCOMPARE(series.sampleAt(1).y, 81.);
    }
    void minorTickContract()
    {
        ChartAxis axis;
        QSignalSpy changed(&axis, &ChartAxis::changed);
        QCOMPARE(axis.minorTickCount(), 0);
        axis.setMinorTickCount(-1);
        QCOMPARE(changed.count(), 0);
        axis.setMinorTickCount(3);
        axis.setMinorTickCount(3);
        QCOMPARE(changed.count(), 1);
        axis.setMinorTickCount(100);
        QCOMPARE(axis.minorTickCount(), 10);
        QCOMPARE(changed.count(), 2);
        using namespace QCatUiKit::Detail;
        QCOMPARE(minorTicks({0, 10}, ChartAxis::Linear, 3, 1), QVector<double>({2.5, 7.5}));
        QCOMPARE(minorTicks({1, 9}, ChartAxis::Linear, 3, 1), QVector<double>({2.5, 7.5}));
        QCOMPARE(minorTicks({1.1, 1.9}, ChartAxis::Linear, 2, 1), QVector<double>({1.5}));
        QCOMPARE(minorTicks({0, 10}, ChartAxis::Category, 3, 3).size(), 0);
        QCOMPARE(minorTicks({0, 10}, ChartAxis::Linear, 3, 0).size(), 0);
        const auto log = minorTicks({1, 100}, ChartAxis::Logarithmic, 3, 1);
        QCOMPARE(log.size(), 2);
        QVERIFY(std::abs(log[0] - std::sqrt(10.)) < 1e-10);
        QVERIFY(std::abs(log[1] - std::sqrt(1000.)) < 1e-10);
        const double base = 1700000000000.;
        QCOMPARE(minorTicks({base, base + 10}, ChartAxis::DateTime, 3, 1),
                 QVector<double>({base + 2.5, base + 7.5}));
    }
    void touchLifecycle_data()
    {
        QTest::addColumn<QString>("reason");
        for (const auto *reason : {"cancel", "ungrab", "window", "observer"})
            QTest::newRow(reason) << QString(reason);
    }
    void touchLifecycle()
    {
        QFETCH(QString, reason);
        QQuickWindow first, second;
        first.resize(640, 400);
        TestChart chart;
        chart.setParentItem(first.contentItem());
        chart.setSize({640, 400});
        chart.setViewRange(0, 10, 0, 10);
        first.show();
        QVERIFY(QTest::qWaitForWindowExposed(&first));
        std::unique_ptr<QPointingDevice> device(QTest::createTouchDevice());
        auto touch = QTest::touchEvent(&first, device.get(), false);
        QSignalSpy clicked(&chart, &ChartItem::pointClicked);
        touch.press(0, {250, 180}).commit();
        QTest::qWait(20);
        QVERIFY(chart.keepTouchGrab());
        if (reason == "cancel")
        {
            QTouchEvent cancel(QEvent::TouchCancel, device.get());
            chart.touchEvent(&cancel);
        }
        else if (reason == "ungrab")
            chart.touchUngrabEvent();
        else if (reason == "window")
            chart.setParentItem(second.contentItem());
        else
        {
            connect(&chart, &ChartItem::viewChanged, &chart, [&] { chart.setInteractive(false); });
            touch.move(0, {280, 200}).commit();
            QTest::qWait(20);
        }
        QVERIFY(!chart.keepTouchGrab());
        const double minimum = chart.xAxis()->minimum();
        touch.move(0, {320, 230}).commit();
        QTest::qWait(20);
        touch.release(0, {320, 230}).commit();
        QCOMPARE(chart.xAxis()->minimum(), minimum);
        QCOMPARE(clicked.count(), 0);
    }
    void rectangleZoomMapping_data()
    {
        QTest::addColumn<int>("axes");
        QTest::addColumn<int>("scale");
        for (int axes = 0; axes < 3; ++axes)
            for (int scale = 0; scale < 4; ++scale)
                QTest::newRow(qPrintable(QString("%1-%2").arg(axes).arg(scale))) << axes << scale;
    }
    void rectangleZoomMapping()
    {
        QFETCH(int, axes);
        QFETCH(int, scale);
        TestChart chart;
        chart.setSize({800, 600});
        chart.xAxis()->setScale(ChartAxis::Scale(scale));
        chart.yAxis()->setScale(ChartAxis::Scale(scale));
        QVERIFY(chart.setViewRange(1, 101, 1, 101));
        chart.setRectangleZoomAxes(ChartItem::ZoomAxes(axes));
        const auto plot = chart.plotRect();
        const QPointF a(plot.left() + plot.width() * .2, plot.top() + plot.height() * .25);
        const QPointF b(plot.left() + plot.width() * .7, plot.top() + plot.height() * .75);
        const auto low = chart.mapToValue({a.x(), b.y()}), high = chart.mapToValue({b.x(), a.y()});
        QVERIFY(chart.zoomToRect(QRectF(b, a))); // Reverse dragging is normalized.
        QCOMPARE(chart.xAxis()->minimum(), axes == 2 ? 1. : low.x());
        QCOMPARE(chart.xAxis()->maximum(), axes == 2 ? 101. : high.x());
        QCOMPARE(chart.yAxis()->minimum(), axes == 1 ? 1. : low.y());
        QCOMPARE(chart.yAxis()->maximum(), axes == 1 ? 101. : high.y());
    }
    void rectangleZoomValidation()
    {
        TestChart chart;
        chart.setSize({800, 600});
        ChartSeries s;
        s.replacePoints({{0, 0}, {10, 10}});
        chart.addSeries(&s);
        QSignalSpy changed(&chart, &ChartItem::appearanceChanged);
        QCOMPARE(chart.dragMode(), ChartItem::PanDrag);
        chart.setDragMode(ChartItem::RectangleZoomDrag);
        chart.setRectangleZoomAxes(ChartItem::ZoomXAxis);
        chart.setDragMode(ChartItem::DragMode(99));
        chart.setRectangleZoomAxes(ChartItem::ZoomAxes(-1));
        QCOMPARE(changed.count(), 2);
        const auto plot = chart.plotRect();
        QVERIFY(chart.zoomToRect({plot.left() + 20, -100, plot.width() / 2, 0}));
        QVERIFY(chart.yAxis()->automatic());
        QVERIFY(!chart.xAxis()->automatic());
        const double before = chart.xAxis()->minimum();
        for (auto rect : {QRectF(0, 0, 1, 100), QRectF(NAN, 0, 10, 10), QRectF(0, 0, INFINITY, 10),
                          QRectF(-100, -100, 10, 10), QRectF(plot.left(), plot.top(), 5, 50)})
            QVERIFY(!chart.zoomToRect(rect));
        QCOMPARE(chart.xAxis()->minimum(), before);
        chart.setRectangleZoomAxes(ChartItem::ZoomBothAxes);
        const auto expected = chart.mapToValue(plot.center());
        QVERIFY(chart.zoomToRect(
            {plot.left() - 50, plot.top() - 50, plot.width() / 2 + 50, plot.height() / 2 + 50}));
        QCOMPARE(chart.xAxis()->maximum(), expected.x());
        QCOMPARE(chart.yAxis()->minimum(), expected.y());
        chart.setChartType(ChartSeries::Pie);
        QVERIFY(!chart.zoomToRect(plot));
    }
    void rectangleSelectionSurvivesDataButNotWindowChange()
    {
        QQuickWindow first, second;
        TestChart chart;
        chart.setParentItem(first.contentItem());
        chart.setSize({640, 400});
        chart.setViewRange(0, 10, 0, 10);
        chart.setDragMode(ChartItem::RectangleZoomDrag);
        ChartSeries series;
        series.replacePoints({{1, 2}, {3, 4}});
        chart.addSeries(&series);
        const auto a = chart.plotRect().center(), b = a + QPointF(50, 40);
        QMouseEvent press(QEvent::MouseButtonPress, a, a, Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        QMouseEvent move(QEvent::MouseMove, b, b, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        chart.mousePressEvent(&press);
        chart.mouseMoveEvent(&move);
        const auto selection = chart.zoomSelection();
        series.append(100, 100);
        chart.updatePolish();
        QVERIFY(chart.zoomSelecting());
        QCOMPARE(chart.zoomSelection(), selection);
        chart.setParentItem(second.contentItem());
        QVERIFY(!chart.zoomSelecting());
        QVERIFY(chart.zoomSelection().isEmpty());
        QVERIFY(!chart.keepMouseGrab());
        QCOMPARE(chart.xAxis()->maximum(), 10.);
    }
    void selectionObserverCanCancelImmediately()
    {
        TestChart chart;
        chart.setSize({640, 400});
        chart.setDragMode(ChartItem::RectangleZoomDrag);
        connect(&chart, &ChartItem::zoomSelectionChanged, &chart,
                [&]
                {
                    if (chart.zoomSelecting())
                        chart.cancelZoomSelection();
                });
        const auto point = chart.plotRect().center();
        QMouseEvent press(QEvent::MouseButtonPress, point, point, Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        chart.mousePressEvent(&press);
        QVERIFY(!chart.zoomSelecting());
        QVERIFY(!chart.keepMouseGrab());
    }
    void rectangleZoomCancellation_data()
    {
        QTest::addColumn<QString>("reason");
        for (auto reason : {"escape", "ungrab", "interactive", "enabled", "visible", "resize",
                            "mode", "axes", "range", "type", "reset", "api", "automatic-data"})
            QTest::newRow(reason) << QString(reason);
    }
    void rectangleZoomCancellation()
    {
        QFETCH(QString, reason);
        TestChart chart;
        chart.setSize({640, 400});
        ChartSeries s;
        s.replacePoints({{0, 0}, {10, 10}});
        chart.addSeries(&s);
        chart.setDragMode(ChartItem::RectangleZoomDrag);
        const auto start = chart.plotRect().center(), end = start + QPointF(50, 40);
        QSignalSpy clicked(&chart, &ChartItem::pointClicked);
        QMouseEvent press(QEvent::MouseButtonPress, start, start, Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        QMouseEvent move(QEvent::MouseMove, end, end, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, end, end, Qt::LeftButton, Qt::NoButton,
                            Qt::NoModifier);
        chart.mousePressEvent(&press);
        chart.mouseMoveEvent(&move);
        QVERIFY(chart.zoomSelecting());
        QVERIFY(!chart.zoomSelection().isEmpty());
        QVERIFY(chart.keepMouseGrab());
        QVERIFY(chart.xAxis()->automatic());
        QVERIFY(chart.yAxis()->automatic());
        if (reason == "escape")
        {
            QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            chart.keyPressEvent(&event);
        }
        else if (reason == "ungrab")
            chart.mouseUngrabEvent();
        else if (reason == "interactive")
            chart.setInteractive(false);
        else if (reason == "enabled")
            chart.setEnabled(false);
        else if (reason == "visible")
            chart.setVisible(false);
        else if (reason == "resize")
            chart.setWidth(700);
        else if (reason == "mode")
            chart.setDragMode(ChartItem::PanDrag);
        else if (reason == "axes")
            chart.setRectangleZoomAxes(ChartItem::ZoomXAxis);
        else if (reason == "range")
            chart.xAxis()->setRange(0, 20);
        else if (reason == "type")
            chart.setChartType(ChartSeries::Pie);
        else if (reason == "reset")
            chart.resetView();
        else if (reason == "api")
            chart.cancelZoomSelection();
        else
        {
            s.append(20, 20);
            chart.updatePolish();
        }
        QVERIFY(!chart.zoomSelecting());
        QVERIFY(chart.zoomSelection().isEmpty());
        QVERIFY(!chart.keepMouseGrab());
        const auto before = chart.mapToValue(chart.plotRect().center());
        chart.mouseMoveEvent(&move);
        chart.mouseReleaseEvent(&release);
        QCOMPARE(chart.mapToValue(chart.plotRect().center()), before);
        QCOMPARE(clicked.count(), 0);
    }
    void areaStyleProperties()
    {
        ChartSeries s;
        QCOMPARE(s.areaBaselineMode(), ChartSeries::ZeroBaseline);
        QCOMPARE(s.areaBaseline(), 0.);
        QCOMPARE(s.fillStyle(), ChartSeries::SolidFill);
        QVERIFY(!s.fillColor().isValid() && !s.fillEndColor().isValid() &&
                !s.borderColor().isValid());
        QSignalSpy style(&s, &ChartSeries::styleChanged);
        s.setAreaBaselineMode(ChartSeries::CustomBaseline);
        s.setAreaBaseline(-12.5);
        s.setFillStyle(ChartSeries::VerticalGradient);
        s.setFillColor(Qt::red);
        s.setFillEndColor(Qt::transparent);
        s.setBorderColor(Qt::green);
        QCOMPARE(style.count(), 6);
        s.setAreaBaselineMode(ChartSeries::AreaBaselineMode(99));
        s.setAreaBaseline(NAN);
        s.setAreaBaseline(INFINITY);
        s.setFillStyle(ChartSeries::FillStyle(-1));
        s.setFillColor(Qt::red);
        QCOMPARE(style.count(), 6);
        QCOMPARE(s.areaBaseline(), -12.5);
        s.setFillColor({});
        s.setFillEndColor({});
        s.setBorderColor({});
        QCOMPARE(style.count(), 9);
        QVERIFY(!s.fillColor().isValid() && !s.fillEndColor().isValid() &&
                !s.borderColor().isValid());
    }
    void areaBaselineRanges()
    {
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::Area);
        ChartSeries s;
        s.replacePoints({{1, 10}, {2, 20}});
        chart.addSeries(&s);
        const auto low = [&]
        {
            chart.updatePolish();
            return chart.mapToValue(chart.plotRect().bottomLeft()).y();
        };
        const auto high = [&]
        {
            chart.updatePolish();
            return chart.mapToValue(chart.plotRect().topLeft()).y();
        };
        QVERIFY(low() < 0);
        s.setAreaBaselineMode(ChartSeries::PlotBottomBaseline);
        QVERIFY(low() > 0 && low() < 10);
        s.setAreaBaselineMode(ChartSeries::CustomBaseline);
        s.setAreaBaseline(-20);
        QVERIFY(low() < -20);
        s.setAreaBaseline(40);
        QVERIFY(high() > 40);
        chart.setViewRange(0, 3, 5, 25);
        QCOMPARE(low(), 5.);
        QCOMPARE(high(), 25.); // Baseline never changes a manual view.
        chart.resetView();
        chart.yAxis()->setScale(ChartAxis::Logarithmic);
        s.setAreaBaseline(1);
        QVERIFY(low() <= 1);
        s.setAreaBaseline(-100);
        QVERIFY(low() > 0);
        s.setAreaBaselineMode(ChartSeries::PlotBottomBaseline);
        const double bottom = low();
        s.setAreaBaselineMode(ChartSeries::ZeroBaseline);
        QCOMPARE(low(), bottom);
        s.setAreaBaselineMode(ChartSeries::CustomBaseline);
        s.setAreaBaseline(10000);
        s.setType(ChartSeries::RangeArea);
        s.replaceRangeSamples({{1, 10, 20}, {2, 12, 24}});
        QVERIFY(high() < 10000); // Range bands have their own lower boundary.
        ChartSeries empty;
        empty.setType(ChartSeries::Area);
        empty.setAreaBaselineMode(ChartSeries::CustomBaseline);
        empty.setAreaBaseline(1e6);
        chart.addSeries(&empty);
        QVERIFY(high() < 10000);
    }
    void barStyleProperties()
    {
        ChartSeries s;
        TestChart chart;
        QSignalSpy style(&s, &ChartSeries::styleChanged);
        QSignalSpy appearance(&chart, &ChartItem::appearanceChanged);
        QCOMPARE(chart.barGroupWidth(), .8);
        QCOMPARE(chart.barSpacing(), 0.);
        QCOMPARE(s.barBorderWidth(), 0.);
        QVERIFY(!s.barBorderColor().isValid());
        QVERIFY(!s.barLabelsVisible());
        QCOMPARE(s.barLabelFormat(), QString("@value"));
        QCOMPARE(s.barLabelPrecision(), 6);
        QCOMPARE(s.barLabelMargin(), 4.);
        chart.setBarGroupWidth(.6);
        chart.setBarSpacing(.25);
        chart.setBarGroupWidth(NAN);
        chart.setBarGroupWidth(2);
        chart.setBarSpacing(-1);
        QCOMPARE(chart.barGroupWidth(), .6);
        QCOMPARE(chart.barSpacing(), .25);
        QCOMPARE(appearance.count(), 2);
        s.setBarBorderWidth(3);
        s.setBarBorderWidth(17);
        s.setBarBorderWidth(NAN);
        QCOMPARE(s.barBorderWidth(), 3.);
        s.setBarBorderColor(Qt::green);
        QCOMPARE(s.barBorderColor(), QColor(Qt::green));
        s.setBarBorderColor({});
        QVERIFY(!s.barBorderColor().isValid());
        s.setBarLabelPosition(ChartSeries::BarLabelOutsideEnd);
        s.setBarLabelPosition(ChartSeries::BarLabelPosition(99));
        QCOMPARE(s.barLabelPosition(), ChartSeries::BarLabelOutsideEnd);
        s.setBarLabelFormat("@value (@percentage%)");
        s.setBarLabelFormat(QString(257, 'x'));
        QCOMPARE(s.barLabelFormat(), QString("@value (@percentage%)"));
        s.setBarLabelPrecision(3);
        s.setBarLabelPrecision(0);
        s.setBarLabelPrecision(16);
        QCOMPARE(s.barLabelPrecision(), 3);
        s.setBarLabelMargin(8);
        s.setBarLabelMargin(INFINITY);
        s.setBarLabelMargin(-1);
        QCOMPARE(s.barLabelMargin(), 8.);
        s.setBarLabelsVisible(true);
        QVERIFY(s.barLabelsVisible());
        QCOMPARE(style.count(), 8);
    }
    void barWidthAndSpacing_data()
    {
        QTest::addColumn<int>("type");
        for (auto type : {ChartSeries::Bar, ChartSeries::StackedBar, ChartSeries::PercentBar,
                          ChartSeries::HorizontalBar, ChartSeries::HorizontalStackedBar,
                          ChartSeries::HorizontalPercentBar})
            QTest::newRow(qPrintable(QString::number(type))) << int(type);
    }
    void barWidthAndSpacing()
    {
        QFETCH(int, type);
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::Type(type));
        ChartSeries a, b;
        a.replacePoints({{0, 10}, {1, -10}, {2, 0}});
        b.replacePoints({{0, 20}, {1, -20}, {2, 0}});
        chart.addSeries(&a);
        chart.addSeries(&b);
        const bool horizontal = type >= ChartSeries::HorizontalBar;
        const bool grouped = type == ChartSeries::Bar || type == ChartSeries::HorizontalBar;
        const bool percent =
            type == ChartSeries::PercentBar || type == ChartSeries::HorizontalPercentBar;
        if (horizontal)
            chart.setViewRange(-120, 120, -1, 3);
        else
            chart.setViewRange(-1, 3, -120, 120);
        chart.setBarGroupWidth(.6);
        chart.setBarSpacing(.5);
        const auto at = [&](double category, double value)
        {
            return chart.dataAt(chart.mapToPosition(horizontal ? QPointF(value, category)
                                                               : QPointF(category, value)));
        };
        auto hit = at(grouped ? -.15 : 0, percent ? 20 : 5);
        QVERIFY(!hit.isEmpty());
        QCOMPARE(hit.value("seriesIndex").toInt(), 0);
        hit = at(grouped ? .15 : 0, percent ? 80 : grouped ? 15 : 25);
        QCOMPARE(hit.value("seriesIndex").toInt(), 1);
        QCOMPARE(hit.value("y").toDouble(), 20.);
        QVERIFY(at(.28, 5).isEmpty());
        if (grouped)
            QVERIFY(at(0, 5).isEmpty());
        QVERIFY(at(2, 5).isEmpty());
        hit = at(1 + (grouped ? .15 : 0), percent ? -80 : grouped ? -15 : -25);
        QCOMPARE(hit.value("y").toDouble(), -20.);
        if (percent)
            QVERIFY(qAbs(hit.value("percentage").toDouble() + 200. / 3) < 1e-9);
        a.setBarLabelsVisible(true);
        a.setBarBorderWidth(16);
        QVERIFY(!at(grouped ? -.15 : 0, 5).isEmpty());
        b.setVisible(false);
        QCOMPARE(at(0, 5).value("seriesIndex").toInt(), 0);
        chart.setBarSpacing(1);
        QVERIFY(at(0, 5).isEmpty());
        chart.setBarSpacing(0);
        chart.setBarGroupWidth(0);
        QVERIFY(at(0, 5).isEmpty());
        chart.setBarGroupWidth(1);
        QVERIFY(!at(.45, 5).isEmpty());
        a.clear();
        QVERIFY(at(0, 5).isEmpty());
    }
    void sliceStylesAndRingStorage()
    {
        ChartSeries s;
        s.setCapacity(2);
        QVERIFY(s.replacePoints({{0, 1}, {1, 2}}));
        QSignalSpy changed(&s, &ChartSeries::styleChanged);
        const QVariantMap style{{"color", QColor(Qt::red)},
                                {"borderWidth", 2.},
                                {"borderColor", QColor(Qt::white)},
                                {"explodeDistance", 12.},
                                {"labelVisible", false}};
        QVERIFY(s.setSliceStyle(1, style));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(s.sliceStyle(1), style);
        QVERIFY(!s.setSliceStyle(1, {{"color", "invalid"}}));
        QVERIFY(!s.setSliceStyle(1, {{"explodeDistance", NAN}}));
        QVERIFY(!s.setSliceStyle(1, {{"borderWidth", 17}}));
        QVERIFY(!s.setSliceStyle(1, {{"labelVisible", "false"}}));
        QVERIFY(!s.setSliceStyle(1, {{"unknown", 1}}));
        QVERIFY(!s.setSliceStyle(-1, {}));
        QCOMPARE(s.sliceStyle(1), style);
        QVERIFY(s.append(2, 3));
        QCOMPARE(s.sliceStyle(0), style);
        QCOMPARE(s.at(0).value("sliceStyle").toMap(), style);
        s.setCapacity(1);
        QVERIFY(s.sliceStyle(0).isEmpty());
        QVERIFY(s.setData({QVariantMap{{"x", 0}, {"y", 1}, {"sliceStyle", style}}}));
        QVERIFY(!s.appendData(
            {QVariantMap{{"x", 1}, {"y", 2}, {"sliceStyle", QVariantMap{{"color", "bad"}}}}}));
        QCOMPARE(s.sliceStyle(0), style);
        QVERIFY(s.setSliceStyle(0, {}));
        QVERIFY(s.sliceStyle(0).isEmpty());
        QVERIFY(s.setSliceStyle(0, style));
        s.replacePoints({{0, 1}});
        QVERIFY(s.sliceStyle(0).isEmpty());
    }
    void outsideLabelColumnLayout()
    {
        using namespace QCatUiKit::Detail;
        const QVector<ChartLabelSlot> crowded{
            {-20, 1, 0}, {50, 8, 1}, {50, 6, 2}, {60, 7, 3}, {200, 2, 4}};
        const auto entries = layoutChartLabelColumn(crowded, 10, 80, 20, 3);
        QCOMPARE(entries.size(), 3);
        QCOMPARE(entries[0].index, 1);
        QCOMPARE(entries[1].index, 2);
        QCOMPARE(entries[2].index, 3);
        QVERIFY(entries.front().y - 10 >= 10);
        QVERIFY(entries.back().y + 10 <= 80);
        for (int i = 1; i < entries.size(); ++i)
            QVERIFY(entries[i].y - entries[i - 1].y >= 23);
        const auto roomy = layoutChartLabelColumn(crowded, 0, 300, 20, 3);
        QCOMPARE(roomy.size(), 5);
        QCOMPARE(roomy[2].index, 2); // Equal anchor heights retain source order.
        QCOMPARE(roomy.back().y, 200.);
        QVERIFY(layoutChartLabelColumn(crowded, 0, 19, 20, 3).isEmpty());
        const auto tied = layoutChartLabelColumn({{50, 1, 0}, {30, 1, 1}}, 0, 20, 20, 3);
        QCOMPARE(tied.size(), 1);
        QCOMPARE(tied[0].index, 0); // Equal weights favor the earlier record.
    }
    void outsideLabelGeometry()
    {
        TestChart chart;
        chart.setSize({600, 500});
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Pie);
        ChartSeries s;
        s.replacePoints({{0, 1}, {1, 1}});
        chart.addSeries(&s);
        const auto center = chart.plotRect().center();
        const auto extent = [&]
        {
            int result = 0;
            for (int x = 1; x < 280; ++x)
                if (!chart.dataAt(center + QPointF(x, 0)).isEmpty())
                    result = x;
            return result;
        };
        const int original = extent();
        QSignalSpy changed(&chart, &ChartItem::appearanceChanged);
        QCOMPARE(chart.sliceLabelPosition(), ChartItem::SliceLabelInside);
        chart.setSliceLabelPosition(ChartItem::SliceLabelOutside);
        QCOMPARE(changed.size(), 1);
        chart.setSliceLabelPosition(ChartItem::SliceLabelOutside);
        chart.setSliceLabelPosition(static_cast<ChartItem::SliceLabelPosition>(-1));
        chart.setSliceLabelPosition(static_cast<ChartItem::SliceLabelPosition>(2));
        QCOMPARE(changed.size(), 1);
        const int outside = extent();
        QVERIFY(outside > 40 && outside < original);
        const auto hit = chart.dataAt(center + QPointF(outside - 10, 0));
        QCOMPARE(hit.value("index").toInt(), 0);
        QCOMPARE(hit.value("y").toDouble(), 1.);
        QCOMPARE(hit.value("percentage").toDouble(), 50.);
        QVERIFY(chart.dataAt({chart.plotRect().right() - 20, center.y()}).isEmpty());
        QVERIFY(s.setSliceStyle(0, {{"explodeDistance", 20}}));
        QVERIFY(chart.dataAt(center + QPointF(10, 0)).isEmpty());
        QCOMPARE(chart.dataAt(center + QPointF(outside - 10, 0)).value("index").toInt(), 0);
        s.setSliceStyle(0, {});
        chart.setSliceLabelsVisible(false);
        QCOMPARE(extent(), original);
        chart.setSliceLabelsVisible(true);
        QCOMPARE(extent(), outside);
        chart.setSize({160, 200}); // Omit labels, retain the actual slices on a tiny plot.
        QVERIFY(!chart.dataAt(chart.plotRect().center() + QPointF(20, 0)).isEmpty());
        QCOMPARE(chart.renderedPointCount(), 2);
        chart.setSize({600, 500});
        chart.setSliceLabelPosition(ChartItem::SliceLabelInside);
        QCOMPARE(extent(), original);
    }
    void radialAnglesAndLimits()
    {
        TestChart chart;
        chart.setSize({600, 500});
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Pie);
        ChartSeries s;
        s.replacePoints({{0, 1}, {1, 1}});
        chart.addSeries(&s);
        QCOMPARE(chart.pieRadius(), .84);
        QCOMPARE(chart.holeSize(), .58);
        QVERIFY(chart.setPieAngles(0, 180));
        auto c = chart.plotRect().center();
        QCOMPARE(chart.dataAt(c + QPointF(60, -60)).value("index").toInt(), 0);
        QCOMPARE(chart.dataAt(c + QPointF(60, 60)).value("index").toInt(), 1);
        QVERIFY(chart.dataAt(c + QPointF(-60, 0)).isEmpty());
        QVERIFY(!chart.setPieAngles(0, 361));
        QVERIFY(!chart.setPieAngles(NAN, 90));
        QCOMPARE(chart.pieEndAngle(), 180.);
        QVERIFY(chart.setPieAngles(-90, 90));
        QVERIFY(!chart.dataAt(c + QPointF(-60, -60)).isEmpty());
        QVERIFY(chart.dataAt(c + QPointF(0, 60)).isEmpty());
        chart.setPieStartAngle(180); // invalid pair draws no geometry
        QVERIFY(chart.dataAt(c).isEmpty());
        QCOMPARE(chart.renderedPointCount(), 0);
        chart.setPieEndAngle(540);
        QVERIFY(!chart.dataAt(c + QPointF(60, 0)).isEmpty());
        chart.setPieRadius(0);
        QVERIFY(chart.dataAt(c).isEmpty());
        chart.setPieRadius(2);
        QCOMPARE(chart.pieRadius(), 0.);
        chart.setPieRadius(.84);
        chart.setChartType(ChartSeries::Doughnut);
        chart.setHoleSize(1);
        QVERIFY(chart.dataAt(c + QPointF(60, 0)).isEmpty());
        chart.setHoleSize(0);
        QVERIFY(!chart.dataAt(c + QPointF(60, 0)).isEmpty());
        chart.setRingGap(65);
        QCOMPARE(chart.ringGap(), 4.);
    }
    void explodedSlicesAndLargeValues()
    {
        TestChart chart;
        chart.setSize({600, 500});
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Pie);
        ChartSeries s;
        chart.addSeries(&s);
        s.replacePoints({{0, 1e308}, {1, 1e308}, {2, -1}, {3, 0}});
        QVERIFY(s.setSliceStyle(0, {{"explodeDistance", 30}}));
        const auto c = chart.plotRect().center();
        const auto hit = chart.dataAt(c + QPointF(100, 0));
        QCOMPARE(hit.value("index").toInt(), 0);
        QCOMPARE(hit.value("percentage").toDouble(), 50.);
        QVERIFY(chart.dataAt(c + QPointF(15, 0)).isEmpty());
        QCOMPARE(chart.dataAt(c + QPointF(-100, 0)).value("index").toInt(), 1);
        QCOMPARE(chart.renderedPointCount(), 2);
        QVERIFY(chart.setSliceVisible(0, 0, false));
        QCOMPARE(chart.dataAt(c + QPointF(100, 0)).value("index").toInt(), 1);
        QCOMPARE(chart.dataAt(c + QPointF(100, 0)).value("percentage").toDouble(), 100.);
        s.replacePoints({{0, -1}, {1, 0}});
        QVERIFY(chart.dataAt(c).isEmpty());
        QCOMPARE(chart.renderedPointCount(), 0);
    }
    void concentricRingsAndVisibility()
    {
        TestChart chart;
        chart.setSize({600, 500});
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Doughnut);
        chart.setMultiRing(true);
        chart.setHoleSize(.2);
        chart.setRingGap(12);
        ChartSeries a, b;
        a.replacePoints({{0, 1}, {1, 1}});
        b.replacePoints({{0, 2}, {1, 2}});
        chart.addSeries(&a);
        chart.addSeries(&b);
        const auto c = chart.plotRect().center();
        const double r = chart.plotRect().height() * .42;
        auto hit = chart.dataAt(c + QPointF(.8 * r, 0));
        QCOMPARE(hit.value("seriesIndex").toInt(), 0);
        QCOMPARE(hit.value("ringIndex").toInt(), 0);
        hit = chart.dataAt(c + QPointF(.4 * r, 0));
        QCOMPARE(hit.value("seriesIndex").toInt(), 1);
        QCOMPARE(hit.value("ringIndex").toInt(), 1);
        QVERIFY(chart.dataAt(c + QPointF(.6 * r, 0)).isEmpty());
        QVERIFY(chart.dataAt(c).isEmpty());
        QVERIFY(chart.setSliceVisible(1, 0, false));
        QVERIFY(chart.isSliceVisible(0, 0));
        QVERIFY(!chart.isSliceVisible(1, 0));
        QCOMPARE(chart.dataAt(c + QPointF(.4 * r, 0)).value("index").toInt(), 1);
        b.setSliceStyle(1, {{"color", QColor(Qt::green)}});
        QVERIFY(!chart.isSliceVisible(1, 0));
        a.append(2, 1);
        QVERIFY(!chart.isSliceVisible(1, 0));
        b.append(2, 1);
        QVERIFY(chart.isSliceVisible(1, 0));
        b.setVisible(false);
        QCOMPARE(chart.dataAt(c + QPointF(.4 * r, 0)).value("seriesIndex").toInt(), 0);
        b.setVisible(true);
        chart.removeSeries(&a);
        QCOMPARE(chart.dataAt(c + QPointF(.4 * r, 0)).value("seriesIndex").toInt(), 0);
        auto *temporary = new ChartSeries;
        temporary->append(0, 1);
        chart.addSeries(temporary);
        QVERIFY(chart.setSliceVisible(1, 0, false));
        delete temporary;
        QCOMPARE(chart.seriesCount(), 1);
        QVERIFY(!chart.setSliceVisible(1, 0, true));
        b.clear();
        QVERIFY(chart.dataAt(c + QPointF(.4 * r, 0)).isEmpty());
    }
    void rangeAndBreakRecords()
    {
        ChartSeries s;
        s.setCapacity(3);
        QVERIFY(s.replaceRangeSamples(
            {{0, -1, 2, "A", false}, {1, 0, 3, "B", true}, {2, 1, 4, "C", false}}));
        QVERIFY(s.hasBreaks());
        QVERIFY(s.breakBefore(1));
        QCOMPARE(s.at(1).value("lower").toDouble(), 0.);
        QVERIFY(!s.appendRangeSamples({{3, NAN, 5, {}, false}}));
        QCOMPARE(s.count(), 3);
        QVERIFY(s.appendRangeSamples({{3, 2, 5, {}, false}}));
        QVERIFY(s.breakBefore(0));
        QCOMPARE(s.lowerValue(0), 0.);
        s.setCapacity(2);
        QVERIFY(!s.hasBreaks());
        QCOMPARE(s.lowerValue(0), 1.);
        QVERIFY(s.setBreakBefore(1, true));
        QVERIFY(!s.setBreakBefore(2, true));
        QVERIFY(!s.setRangeData({QVariantMap{{"x", 9}, {"lower", 1}}}));
        QCOMPARE(s.count(), 2);
        QVERIFY(!s.setData({QVariantMap{{"x", 9}, {"y", 1}, {"breakBefore", "true"}}}));
        QVERIFY(s.setData({QVariantMap{{"x", 0}, {"y", 1}},
                           QVariantMap{{"x", 1}, {"y", 2}, {"breakBefore", true}}}));
        QVERIFY(s.breakBefore(1));
        QVERIFY(!s.hasRangeValue(0));
        QVERIFY(!s.appendSamples({{2, NAN, 1, {}}}));
        s.clear();
        QVERIFY(!s.hasBreaks());
        QVERIFY(!s.hasRangeValue(0));
        QVERIFY(s.setRangeData(
            {QVariantMap{{"x", 1}, {"lower", -2}, {"upper", 4}, {"breakBefore", true}}}));
        QCOMPARE(s.at(0).value("upper").toDouble(), 4.);
        bool accepted = true;
        QScopedPointer<QThread> worker(QThread::create(
            [&] {
                accepted = s.appendRangeSamples({{2, 1, 3, {}, false}});
            }));
        worker->start();
        QVERIFY(worker->wait(5000));
        QVERIFY(!accepted);
        QCOMPARE(s.count(), 1);
    }
    void explicitBreaks_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = ChartSeries::LinearCurve; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void explicitBreaks()
    {
        QFETCH(int, mode);
        TestChart chart;
        chart.setSize({800, 600});
        ChartSeries s;
        chart.addSeries(&s);
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replacePoints({{0, 1}, {2, 1}, {8, 8}, {10, 8}});
        s.setBreakBefore(2, true);
        chart.setViewRange(-1, 11, 0, 10);
        QVERIFY(chart.dataAt(chart.mapToPosition({5, 4.5})).isEmpty());
        QCOMPARE(chart.dataAt(chart.mapToPosition({2, 1})).value("index").toInt(), 1);
        chart.setChartType(ChartSeries::Area);
        QVERIFY(chart.dataAt(chart.mapToPosition({5, 4.5})).isEmpty());
        QVector<ChartSeries::Sample> many;
        for (int i = 0; i < 100; ++i)
            many.append({double(i), double(i % 7), 1, {}});
        s.replaceSamples(many);
        s.setBreakBefore(51, true);
        const auto indices = Detail::visibleIndices(&s, {0, 99}, 8);
        QVERIFY(indices.contains(50));
        QVERIFY(indices.contains(51));
    }
    void rangeCurveMapping_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = 0; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void rangeCurveMapping()
    {
        QFETCH(int, mode);
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::RangeArea);
        chart.setViewRange(-.5, 4.5, -3, 5);
        ChartSeries s;
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replaceRangeSamples({{0, -1, 1}, {1, -2, 2}, {2, -2, 2}, {3, -1, 1}});
        chart.addSeries(&s);
        const auto raw = s.at(1);
        // All three splines bulge away from the straight middle plateau, except monotone.
        const double bound = mode == ChartSeries::NaturalSpline      ? 2.15
                             : mode == ChartSeries::CatmullRomSpline ? 2.125
                                                                     : 2.;
        auto hit = chart.dataAt(chart.mapToPosition({1.5, bound}));
        QCOMPARE(hit.value("boundary").toString(), QString("upper"));
        QVERIFY(hit.value("index").toInt() == 1 || hit.value("index").toInt() == 2);
        QCOMPARE(hit.value("upper").toDouble(), 2.);
        hit = chart.dataAt(chart.mapToPosition({1.5, -bound}));
        QCOMPARE(hit.value("boundary").toString(), QString("lower"));
        QCOMPARE(hit.value("y").toDouble(), -2.);
        QVERIFY(chart.dataAt(chart.mapToPosition({1.5, bound + .4})).isEmpty());
        if (mode == ChartSeries::StepLeft || mode == ChartSeries::StepRight ||
            mode == ChartSeries::StepCenter)
        {
            const bool leftOwner = mode != ChartSeries::StepLeft;
            hit = chart.dataAt(chart.mapToPosition({.25, leftOwner ? .9 : 1.9}));
            QCOMPARE(hit.value("index").toInt(), leftOwner ? 0 : 1);
        }
        if (mode == ChartSeries::LinearCurve || mode >= ChartSeries::MonotoneSpline)
        {
            QCOMPARE(chart.dataAt(chart.mapToPosition({1.2, 0})).value("index").toInt(), 1);
            QCOMPARE(chart.dataAt(chart.mapToPosition({1.8, 0})).value("index").toInt(), 2);
        }
        s.setBreakBefore(2, true);
        QVERIFY(chart.dataAt(chart.mapToPosition({1.5, 0})).isEmpty());
        s.setBreakBefore(2, false);
        QCOMPARE(s.at(1), raw);
        chart.xAxis()->setScale(ChartAxis::DateTime);
        s.replaceRangeSamples({{1700000000000., -1, 1},
                               {1700000001000., -2, 2},
                               {1700000002000., -2, 2},
                               {1700000003000., -1, 1}});
        chart.setViewRange(1700000000000., 1700000003000., -3, 3);
        QVERIFY(!chart.dataAt(chart.mapToPosition({1700000001500., 0})).isEmpty());
        s.clear();
        QCOMPARE(chart.dataAt(chart.plotRect().center()), QVariantMap());
    }
    void rangeCurvesOnLogAxes_data()
    {
        rangeCurveMapping_data();
    }
    void rangeCurvesOnLogAxes()
    {
        QFETCH(int, mode);
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::RangeArea);
        chart.xAxis()->setScale(ChartAxis::Logarithmic);
        chart.yAxis()->setScale(ChartAxis::Logarithmic);
        chart.setViewRange(1, 1000, .01, 10000);
        ChartSeries s;
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replaceRangeSamples({{1, 1, 100}, {10, .1, 1000}, {100, .1, 1000}, {1000, 1, 100}});
        chart.addSeries(&s);
        const double delta = mode == ChartSeries::NaturalSpline      ? .15
                             : mode == ChartSeries::CatmullRomSpline ? .125
                                                                     : 0.;
        auto hit = chart.dataAt(chart.mapToPosition({std::sqrt(1000.), std::pow(10., 3 + delta)}));
        QCOMPARE(hit.value("boundary").toString(), QString("upper"));
        QCOMPARE(hit.value("upper").toDouble(), 1000.);
        hit = chart.dataAt(chart.mapToPosition({std::sqrt(1000.), std::pow(10., -1 - delta)}));
        QCOMPARE(hit.value("boundary").toString(), QString("lower"));
        QCOMPARE(hit.value("lower").toDouble(), .1);
        if (mode == ChartSeries::StepCenter)
        {
            hit = chart.dataAt(chart.mapToPosition({2., 90.}));
            QCOMPARE(hit.value("index").toInt(), 0);
            hit = chart.dataAt(chart.mapToPosition({5., 900.}));
            QCOMPARE(hit.value("index").toInt(), 1);
        }
    }
    void rangeSplineExtremaAndOvershoot_data()
    {
        QTest::addColumn<int>("mode");
        QTest::newRow("natural") << int(ChartSeries::NaturalSpline);
        QTest::newRow("catmull") << int(ChartSeries::CatmullRomSpline);
    }
    void rangeSplineExtremaAndOvershoot()
    {
        QFETCH(int, mode);
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::RangeArea);
        ChartSeries s;
        chart.addSeries(&s);
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replaceRangeSamples({{0, 0, 0}, {1, -1, 1}, {2, -1, 1}, {3, 0, 0}});
        QVERIFY(chart.mapToValue(chart.plotRect().topRight()).y() > 1.12);
        QVERIFY(chart.mapToValue(chart.plotRect().bottomLeft()).y() < -1.12);
        s.replaceRangeSamples({{0, 0, 2}, {0, 0, 2}, {0, 0, 2}, {10, 0, 2}});
        QVERIFY(chart.mapToValue(chart.plotRect().topLeft()).x() < -.1);
        chart.setViewRange(-.4, -.1, -.5, 2.5);
        QVERIFY(!chart.dataAt(chart.mapToPosition({-.2, 1})).isEmpty());
        // One missing lower boundary must also isolate the upper boundary's spline neighbors.
        s.replaceRangeSamples({{0, 1, 4}, {1, 1, 4}});
        s.append(2, 100);
        s.appendRangeSamples({{3, 1, 4}, {4, 1, 4}});
        chart.setViewRange(0, 4, 0, 5);
        QVERIFY(chart.dataAt(chart.mapToPosition({2, 2})).isEmpty());
        QVERIFY(!chart.dataAt(chart.mapToPosition({.5, 3.9})).isEmpty());
        chart.yAxis()->setScale(ChartAxis::Logarithmic);
        s.replaceRangeSamples({{1, 1, 10}, {2, 0, 100}, {3, 1, 10}, {4, 1, 10}});
        chart.setViewRange(1, 4, 1, 100);
        QVERIFY(chart.dataAt(chart.mapToPosition({2, 5})).isEmpty());
        QVERIFY(!chart.dataAt(chart.mapToPosition({3.5, 9.9})).isEmpty());
    }
    void rangeAreaPickingAndCrossing()
    {
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::RangeArea);
        ChartSeries s;
        chart.addSeries(&s);
        QVERIFY(s.replaceRangeSamples({{0, -3, 4, {}, false},
                                       {2, -3, 4, {}, false},
                                       {8, -1, 2, {}, true},
                                       {10, -1, 2, {}, false}}));
        chart.updatePolish();
        QVERIFY(chart.mapToValue(chart.plotRect().bottomLeft()).y() < -3);
        auto hit = chart.dataAt(chart.mapToPosition({.2, -2.9}));
        QCOMPARE(hit.value("boundary").toString(), QString("lower"));
        QCOMPARE(hit.value("y").toDouble(), -3.);
        QCOMPARE(hit.value("upper").toDouble(), 4.);
        QCOMPARE(chart.dataAt(chart.mapToPosition({.2, 3.9})).value("boundary").toString(),
                 QString("upper"));
        QVERIFY(chart.dataAt(chart.mapToPosition({5, 1})).isEmpty());
        QVERIFY(s.replaceRangeSamples({{0, 0, 4, {}, false}, {10, 4, 0, {}, false}}));
        chart.setViewRange(-1, 11, -1, 5);
        QVERIFY(!chart.dataAt(chart.mapToPosition({2, 2})).isEmpty());
        QVERIFY(!chart.dataAt(chart.mapToPosition({8, 2})).isEmpty());
        QVERIFY(chart.dataAt(chart.mapToPosition({5, 3})).isEmpty());
        s.setVisible(false);
        QVERIFY(chart.dataAt(chart.mapToPosition({2, 2})).isEmpty());
        s.setVisible(true);
        s.replacePoints({{0, 1}, {10, 2}});
        chart.updatePolish();
        QCOMPARE(chart.renderedPointCount(), 0); // No fabricated lower boundary.
    }
    void strokeAndFillProperties()
    {
        ChartSeries s;
        QCOMPARE(s.strokeStyle(), ChartSeries::SolidLine);
        QCOMPARE(s.dashPattern(), QList<qreal>({8, 4}));
        s.setStrokeStyle(ChartSeries::DashLine);
        s.setDashPattern({12, 6, 2, 6});
        s.setDashOffset(-3);
        s.setFillOpacity(.5);
        QSignalSpy changed(&s, &ChartSeries::styleChanged);
        s.setDashPattern({1, 0});
        s.setDashPattern({1, 2, 3});
        s.setDashPattern({NAN, 1});
        s.setDashOffset(INFINITY);
        s.setFillOpacity(2);
        s.setStrokeStyle(ChartSeries::StrokeStyle(99));
        QCOMPARE(changed.count(), 0);
        QCOMPARE(s.dashPattern(), QList<qreal>({12, 6, 2, 6}));
        QCOMPARE(s.dashOffset(), -3.);
        QCOMPARE(s.fillOpacity(), .5);
    }
    void variantProperties()
    {
        ChartSeries series;
        QSignalSpy changed(&series, &ChartSeries::styleChanged);
        QCOMPARE(series.curveMode(), ChartSeries::LinearCurve);
        QCOMPARE(series.markerSize(), 8.);
        QCOMPARE(series.markerShape(), ChartSeries::Circle);
        QVERIFY(!series.markersVisible());
        series.setCurveMode(ChartSeries::CatmullRomSpline);
        series.setSplineTension(.5);
        series.setMarkerShape(ChartSeries::Diamond);
        series.setMarkerSize(20);
        series.setMarkersVisible(true);
        QCOMPARE(changed.size(), 5);
        series.setCurveMode(ChartSeries::CurveMode(42));
        series.setMarkerShape(ChartSeries::MarkerShape(-1));
        series.setMarkerSize(0);
        series.setMarkerSize(INFINITY);
        series.setSplineTension(-.1);
        series.setSplineTension(NAN);
        QCOMPARE(changed.size(), 5);
        QCOMPARE(series.curveMode(), ChartSeries::CatmullRomSpline);
        QCOMPARE(series.splineTension(), .5);
    }
    void stepOwnershipAndDuplicateX()
    {
        TestChart chart;
        chart.setSize({800, 600});
        ChartSeries series;
        chart.addSeries(&series);
        QVERIFY(chart.setViewRange(-1, 11, -1, 10));
        QVERIFY(series.replacePoints({{0, 1}, {10, 8}}));
        const auto hit = [&](double x, double y) {
            return chart.dataAt(chart.mapToPosition({x, y}));
        };
        series.setCurveMode(ChartSeries::StepLeft);
        QCOMPARE(hit(3, 8).value("index").toInt(), 1);
        QCOMPARE(hit(0, 1).value("index").toInt(), 0);
        series.setCurveMode(ChartSeries::StepRight);
        QCOMPARE(hit(7, 1).value("index").toInt(), 0);
        series.setCurveMode(ChartSeries::StepCenter);
        QCOMPARE(hit(3, 1).value("index").toInt(), 0);
        QCOMPARE(hit(7, 8).value("index").toInt(), 1);
        QVERIFY(hit(3, 5).isEmpty());
        QVERIFY(series.replacePoints({{10, 1}, {0, 8}}));
        QCOMPARE(hit(7, 1).value("index").toInt(), 0);
        QVERIFY(series.replacePoints({{0, 1}, {10, 1}, {10, 8}}));
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        QVERIFY(!hit(10, 5).isEmpty());
        QCOMPARE(series.count(), 3);
    }
    void denseCurvePan_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = ChartSeries::LinearCurve; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void linearOverviewReusesSourceIndex()
    {
        TestChart chart;
        chart.setSize({1440, 400});
        ChartSeries series;
        series.setCapacity(100000);
        series.setMarkersVisible(true);
        QVector<ChartSeries::Sample> samples;
        for (int i = 0; i < 100000; ++i)
            samples.append({double(i), i == 4242 ? 20. : std::sin(i * .01), 1, {}});
        QVERIFY(series.replaceSamples(samples));
        chart.addSeries(&series);
        QVERIFY(chart.setViewRange(1000, 99000, -2, 22));
        chart.updatePolish();
        const auto builds = chart.renderingStatistics().value("curveCacheBuilds").toULongLong();
        QVERIFY(builds > 0);
        for (int i = 0; i < 24; ++i)
        {
            chart.panBy({i < 12 ? 5. : -5., 0});
            chart.updatePolish();
            QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
            QVERIFY(chart.renderingStatistics().value("curveLodNodesVisited").toInt() < 10000);
            QVERIFY(chart.renderedPointCount() < 12000);
            QCOMPARE(chart.dataAt(chart.mapToPosition({4242, 20})).value("index").toInt(), 4242);
        }
        QCOMPARE(series.count(), 100000);
        QVERIFY(chart.setViewRange(4240, 4244, -2, 22));
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
        QVERIFY(chart.renderedPointCount() < 30);
    }
    void denseCurvePan()
    {
        QFETCH(int, mode);
        TestChart chart;
        chart.setSize({1440, 400});
        ChartSeries a, b;
        a.setCapacity(100000);
        b.setCapacity(100000);
        QVector<ChartSeries::Sample> av, bv;
        for (int i = 0; i < 100000; ++i)
        {
            av.append({double(i), std::sin(i * .007) * 8 + std::sin(i * .057) * 2, 1, {}});
            bv.append({double(i), std::cos(i * .008) * 5, 1, {}});
        }
        a.replaceSamples(av);
        b.replaceSamples(bv);
        a.setBreakBefore(70000, true);
        b.setBreakBefore(70000, true);
        a.setCurveMode(ChartSeries::CurveMode(mode));
        b.setCurveMode(ChartSeries::CurveMode(mode));
        chart.addSeries(&a);
        chart.addSeries(&b);
        chart.updatePolish();
        const auto builds = chart.renderingStatistics().value("curveCacheBuilds").toULongLong();
        chart.xAxis()->setReversed(true);
        chart.yAxis()->setReversed(true);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
        QVERIFY(chart.renderingStatistics().value("curveLodNodesVisited").toInt() < 10000);
        chart.panBy({4, 2});
        chart.updatePolish();
        const auto reversedSample = a.sampleAt(4242);
        QCOMPARE(chart.dataAt(chart.mapToPosition({reversedSample.x, reversedSample.y}))
                     .value("index")
                     .toInt(),
                 4242);
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
        QVERIFY(chart.renderedPointCount() < 12000);
        chart.panBy({-4, -2});
        chart.xAxis()->setReversed(false);
        chart.yAxis()->setReversed(false);
        QCOMPARE(builds, 2ULL);
        for (int i = 0; i < 4; ++i)
        {
            chart.panBy({i % 2 ? 4. : -4., 2});
            chart.updatePolish();
            QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
            QVERIFY(chart.renderingStatistics().value("curveLodNodesVisited").toInt() < 10000);
            QVERIFY(chart.renderedPointCount() < 12000);
        }
        const auto sample = a.sampleAt(4242);
        const auto hit = chart.dataAt(chart.mapToPosition({sample.x, sample.y}));
        QCOMPARE(hit.value("index").toInt(), 4242);
        QCOMPARE(hit.value("y").toDouble(), sample.y);
        a.setColor(Qt::red);
        a.setLineWidth(3);
        a.setStrokeStyle(ChartSeries::DashLine);
        a.setLineCap(ChartSeries::RoundCap);
        a.setLineJoin(ChartSeries::RoundJoin);
        chart.setSize({800, 480});
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds);
        QVERIFY(chart.setViewRange(4238, 4246, -12, 12));
        chart.updatePolish();
        QVERIFY(chart.renderingStatistics().value("curveLodNodesVisited").toInt() < 150);
        QCOMPARE(a.count(), 100000);
        QCOMPARE(b.count(), 100000);
        QCOMPARE(a.sampleAt(4242).y, sample.y);
        // Data and geometric style invalidate just the affected series.
        a.append(100000, 4);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds + 1);
        a.setSplineTension(.4);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds + 2);
        a.setBreakBefore(4240, true);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds + 3);
        a.clear();
        b.clear();
        chart.updatePolish();
        QCOMPARE(chart.renderedPointCount(), 0);
        QVERIFY(chart.dataAt(chart.plotRect().center()).isEmpty());
    }
    void curveCacheMappingAndLifetime()
    {
        TestChart chart;
        chart.setSize({800, 500});
        chart.setDisplayBudget(8);
        auto *series = new ChartSeries;
        series->setCurveMode(ChartSeries::NaturalSpline);
        QVector<QPointF> points;
        for (int i = 1; i <= 20; ++i)
            points.append({double(i), double(i * i)});
        series->replacePoints(points);
        chart.addSeries(series);
        chart.updatePolish();
        auto builds = chart.renderingStatistics().value("curveCacheBuilds").toULongLong();
        chart.xAxis()->setScale(ChartAxis::Logarithmic);
        chart.yAxis()->setScale(ChartAxis::Logarithmic);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds + 1);
        const auto hit = chart.dataAt(chart.mapToPosition({10, 100}));
        QCOMPARE(hit.value("index").toInt(), 9);
        QCOMPARE(hit.value("y").toDouble(), 100.);
        chart.setChartType(ChartSeries::Area);
        chart.updatePolish();
        QCOMPARE(chart.renderingStatistics().value("curveCacheBuilds").toULongLong(), builds + 2);
        series->setCapacity(9);
        chart.updatePolish();
        QCOMPARE(chart.dataAt(chart.mapToPosition({20, 400})).value("index").toInt(), 8);
        chart.removeSeries(series);
        chart.addSeries(series);
        chart.updatePolish();
        QVERIFY(chart.renderingStatistics().value("curveCacheBuilds").toULongLong() > builds + 3);
        delete series;
        chart.updatePolish();
        QCOMPARE(chart.seriesCount(), 0);
        QCOMPARE(chart.renderedPointCount(), 0);
    }
    void lodCurvePickingRefinesSource_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = ChartSeries::StepLeft; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void lodCurvePickingRefinesSource()
    {
        QFETCH(int, mode);
        TestChart chart, reference;
        chart.setSize({800, 500});
        reference.setSize({800, 500});
        chart.setLegendVisible(false);
        reference.setLegendVisible(false);
        chart.setDisplayBudget(8);
        reference.setDisplayBudget(0);
        ChartSeries s;
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replacePoints({{0, 0},
                         {1, 3},
                         {2, 3},
                         {3, 0},
                         {4, 2},
                         {5, 0},
                         {6, 2},
                         {7, 2},
                         {8, 0},
                         {9, 1},
                         {10, 1},
                         {11, 0}});
        chart.addSeries(&s);
        reference.addSeries(&s);
        chart.setViewRange(-500, 500, -1, 4);
        reference.setViewRange(-500, 500, -1, 4);
        for (double y = .25; y < 3.4; y += .2)
        {
            const auto point = chart.mapToPosition({1.5, y});
            const auto actual = chart.dataAt(point), expected = reference.dataAt(point);
            QCOMPARE(actual.isEmpty(), expected.isEmpty());
            if (!actual.isEmpty())
            {
                QCOMPARE(actual.value("y").toDouble(), s.sampleAt(actual.value("index").toInt()).y);
                const auto chosen = s.sampleAt(actual.value("index").toInt());
                QVERIFY(chosen.x >= 0 && chosen.x <= 11);
            }
        }
        // A zoomed-in leaf must exactly match the unsampled source curve.
        chart.setViewRange(0, 3, -1, 4);
        reference.setViewRange(0, 3, -1, 4);
        const double y = mode == ChartSeries::NaturalSpline      ? 3.4
                         : mode == ChartSeries::CatmullRomSpline ? 3.375
                                                                 : 3.;
        const auto point = chart.mapToPosition({1.5, y});
        const auto actual = chart.dataAt(point), expected = reference.dataAt(point);
        QCOMPARE(actual, expected);
    }
    void splineExtentsAndSourcePicking()
    {
        TestChart chart;
        chart.setSize({800, 600});
        ChartSeries series;
        chart.addSeries(&series);
        QVERIFY(series.replacePoints({{0, 0}, {1, 1}, {2, 1}, {3, 0}}));
        series.setCurveMode(ChartSeries::CatmullRomSpline);
        chart.updatePolish();
        const double top = chart.mapToValue(chart.plotRect().topLeft()).y();
        QVERIFY(top > 1.125 && top < 1.2);
        const auto hit = chart.dataAt(chart.mapToPosition({1.5, 1.125}));
        QVERIFY(!hit.isEmpty());
        QCOMPARE(hit.value("y").toDouble(), 1.); // Original endpoint, never the interpolated value.
        QVERIFY(hit.value("index").toInt() == 1 || hit.value("index").toInt() == 2);
        series.setCurveMode(ChartSeries::MonotoneSpline);
        chart.updatePolish();
        QVERIFY(chart.mapToValue(chart.plotRect().topLeft()).y() < 1.1);
        QVERIFY(!chart.dataAt(chart.mapToPosition({1.5, 1})).isEmpty());
        series.setCurveMode(ChartSeries::CatmullRomSpline);
        series.setSplineTension(1);
        chart.updatePolish();
        QVERIFY(chart.mapToValue(chart.plotRect().topLeft()).y() < 1.1);
        series.setCurveMode(ChartSeries::NaturalSpline);
        chart.updatePolish();
        QVERIFY(chart.mapToValue(chart.plotRect().topLeft()).y() > 1.15);
        QVERIFY(!chart.dataAt(chart.mapToPosition({1.5, 1.15})).isEmpty());
        // Natural fitting stays stable at a large absolute timestamp.
        QVERIFY(series.replacePoints({{1e12, 0}, {1e12 + 1, 1}, {1e12 + 2, 1}, {1e12 + 3, 0}}));
        QVERIFY(!chart.dataAt(chart.mapToPosition({1e12 + 1.5, 1.15})).isEmpty());
        // A repeated X does not divide by zero in the monotone interpolator.
        series.setCurveMode(ChartSeries::MonotoneSpline);
        QVERIFY(series.replacePoints({{0, 0}, {1, 1}, {1, 2}, {2, 3}}));
        chart.updatePolish();
        QVERIFY(!chart.dataAt(chart.mapToPosition({1, 1.5})).isEmpty());
        chart.xAxis()->setScale(ChartAxis::Logarithmic);
        chart.yAxis()->setScale(ChartAxis::Logarithmic);
        QVERIFY(series.replacePoints({{1, 1}, {10, 10}, {100, 100}}));
        QVERIFY(!chart.dataAt(chart.mapToPosition({std::sqrt(10.), std::sqrt(10.)})).isEmpty());
        series.clear();
        chart.updatePolish();
        QVERIFY(chart.dataAt(chart.plotRect().center()).isEmpty());
    }
    void markerGeometryPicking()
    {
        TestChart chart;
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::Scatter);
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        ChartSeries series;
        chart.addSeries(&series);
        series.append(5, 5, 10);
        series.setMarkerSize(40);
        for (auto shape : {ChartSeries::Circle, ChartSeries::Square, ChartSeries::Triangle,
                           ChartSeries::Diamond})
        {
            series.setMarkerShape(shape);
            const auto center = chart.mapToPosition({5, 5});
            QVERIFY(!chart.dataAt(center).isEmpty());
            QCOMPARE(!chart.dataAt(center + QPointF(16, -16)).isEmpty(),
                     shape == ChartSeries::Square);
            QVERIFY(chart.dataAt(center + QPointF(25, 0)).isEmpty());
        }
        chart.setChartType(ChartSeries::Bubble);
        series.setMarkerShape(ChartSeries::Circle);
        const auto center = chart.mapToPosition({5, 5});
        QVERIFY(chart.dataAt(center + QPointF(15, 0)).isEmpty()); // value remains the radius.
    }
    void percentAndHorizontalBars()
    {
        TestChart chart;
        chart.setSize({800, 600});
        ChartSeries first, second;
        chart.addSeries(&first);
        chart.addSeries(&second);
        first.replacePoints({{0, 1e308}, {1, -1}, {2, 0}});
        second.replacePoints({{0, 1e308}, {1, -3}, {2, 0}});
        for (auto type : {ChartSeries::PercentBar, ChartSeries::HorizontalPercentBar})
        {
            chart.setChartType(type);
            chart.resetView();
            const bool horizontal = type == ChartSeries::HorizontalPercentBar;
            const auto position = [&](double category, double value) {
                return chart.mapToPosition(horizontal ? QPointF(value, category)
                                                      : QPointF(category, value));
            };
            auto hit = chart.dataAt(position(0, 75));
            QCOMPARE(hit.value("seriesIndex").toInt(), 1);
            QCOMPARE(hit.value("y").toDouble(), 1e308);
            QCOMPARE(hit.value("percentage").toDouble(), 50.);
            hit = chart.dataAt(position(1, -60));
            QCOMPARE(hit.value("percentage").toDouble(), -75.);
            QCOMPARE(hit.value("y").toDouble(), -3.);
            QVERIFY(chart.dataAt(position(2, 50)).isEmpty());
            second.setVisible(false);
            QCOMPARE(chart.dataAt(position(0, 75)).value("percentage").toDouble(), 100.);
            second.setVisible(true);
        }
        first.replacePoints({{0, 10}, {1, -3}});
        second.replacePoints({{0, 20}, {1, -8}});
        chart.setChartType(ChartSeries::HorizontalStackedBar);
        auto hit = chart.dataAt(chart.mapToPosition({25, 0}));
        QCOMPARE(hit.value("seriesIndex").toInt(), 1);
        QCOMPARE(hit.value("y").toDouble(), 20.);
        QVERIFY(!hit.contains("percentage"));
        chart.setChartType(ChartSeries::HorizontalBar);
        QCOMPARE(chart.dataAt(chart.mapToPosition({5, -.2})).value("seriesIndex").toInt(), 0);
        QCOMPARE(chart.dataAt(chart.mapToPosition({15, .2})).value("seriesIndex").toInt(), 1);
    }
    void destructionWhileAttachedToWindow()
    {
        QQuickWindow window;
        auto *chart = new ChartItem(window.contentItem());
        chart->setSize({640, 400});
        delete chart;
        QVERIFY(window.contentItem()->childItems().isEmpty());
    }
    void canceledDrag_data()
    {
        QTest::addColumn<bool>("disableInteraction");
        QTest::addColumn<bool>("moveBeforeCancel");
        QTest::newRow("ungrab-press") << false << false;
        QTest::newRow("ungrab-drag") << false << true;
        QTest::newRow("disabled-press") << true << false;
        QTest::newRow("disabled-drag") << true << true;
    }
    void canceledDrag()
    {
        QFETCH(bool, disableInteraction);
        QFETCH(bool, moveBeforeCancel);
        TestChart chart;
        chart.setSize({640, 400});
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        const auto start = chart.plotRect().center();
        QSignalSpy clicked(&chart, &ChartItem::pointClicked);
        QMouseEvent press(QEvent::MouseButtonPress, start, start, Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        chart.mousePressEvent(&press);
        const auto end = start + QPointF(30, 20);
        QMouseEvent move(QEvent::MouseMove, end, end, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        if (moveBeforeCancel)
            chart.mouseMoveEvent(&move);
        if (disableInteraction)
        {
            chart.setInteractive(false);
            chart.setInteractive(true);
        }
        else
            chart.mouseUngrabEvent();
        QVERIFY(!chart.keepMouseGrab());
        const auto before = chart.mapToValue(start);
        const auto next = end + QPointF(30, 20);
        QMouseEvent furtherMove(QEvent::MouseMove, next, next, Qt::NoButton, Qt::LeftButton,
                                Qt::NoModifier);
        chart.mouseMoveEvent(&furtherMove);
        QCOMPARE(chart.mapToValue(start), before);
        QMouseEvent release(QEvent::MouseButtonRelease, end, end, Qt::LeftButton, Qt::NoButton,
                            Qt::NoModifier);
        chart.mouseReleaseEvent(&release);
        QCOMPARE(clicked.size(), 0);
        chart.mousePressEvent(&press);
        chart.mouseReleaseEvent(&release);
        QCOMPARE(clicked.size(), 1);
        QVERIFY(!chart.keepMouseGrab());
    }
    void themeAndOwnership()
    {
        TestChart chart;
        chart.setSize({800, 500});
        auto *custom = new ThemePalette;
        custom->setColorScheme(ThemePalette::Dark);
        chart.setTheme(custom);
        QCOMPARE(chart.backgroundColor(), custom->tokens().value("surface").value<QColor>());
        custom->setFontSize(20);
        QCOMPARE(chart.font().pixelSize(), 20);
        chart.setBackgroundColor(Qt::red);
        custom->setColorScheme(ThemePalette::Light);
        QCOMPARE(chart.backgroundColor(), QColor(Qt::red));
        chart.resetBackgroundColor();
        QCOMPARE(chart.backgroundColor(), QColor(Qt::white));
        delete custom;
        QVERIFY(chart.theme());
        QCOMPARE(chart.backgroundColor(), chart.tokens().value("surface").value<QColor>());
        ChartSeries external;
        chart.addSeries(&external);
        QVERIFY(!external.parent());
        chart.clearSeries();
        QCOMPARE(chart.seriesCount(), 0);
        QVERIFY(external.append(1, 2));
    }
    void workerBatchIsRejected()
    {
        ChartSeries series;
        QVERIFY(series.append(1, 2));
        bool accepted = true;
        QScopedPointer<QThread> worker(QThread::create([&] { accepted = series.append(2, 3); }));
        worker->start();
        QVERIFY(worker->wait(5000));
        QVERIFY(!accepted);
        QCOMPARE(series.count(), 1);
        QCOMPARE(series.sampleAt(0).x, 1.);
    }
    void ringAndValidation()
    {
        ChartSeries s;
        s.setCapacity(5);
        QSignalSpy changed(&s, &ChartSeries::dataChanged);
        QVector<ChartSeries::Sample> input;
        for (int i = 0; i < 13; ++i)
            input.append({double(i), double(i * i), 1, {}});
        QVERIFY(s.appendSamples(input));
        QCOMPARE(changed.size(), 1);
        QCOMPARE(s.count(), 5);
        QCOMPARE(s.sampleAt(0).x, 8.);
        QCOMPARE(s.sampleAt(4).x, 12.);
        QVERIFY(!s.appendSamples(
            {{13, 1, 1, {}}, {14, std::numeric_limits<double>::quiet_NaN(), 1, {}}}));
        QCOMPARE(s.sampleAt(4).x, 12.);
        s.setCapacity(3);
        QCOMPARE(s.sampleAt(0).x, 10.);
        s.setCapacity(7);
        s.append(13, 3);
        QCOMPARE(s.sampleAt(0).x, 10.);
        QCOMPARE(s.lowerBound(11.5), 2);
        QVERIFY(!s.setData({QVariantMap{{"x", 1}}}));
        QCOMPARE(s.count(), 4);
        s.clear();
        QCOMPARE(s.count(), 0);
        QVERIFY(s.sortedX());
    }
    void extremaAndRawQuery()
    {
        ChartSeries s;
        QVector<ChartSeries::Sample> data;
        for (int i = 0; i < 100000; ++i)
            data.append({double(i), i == 51111 ? 999. : std::sin(i * .01), 1, {}});
        s.replaceSamples(data);
        auto selected = Detail::visibleIndices(&s, {50000, 52000}, 64);
        QVERIFY(selected.size() <= 64);
        QVERIFY(selected.contains(51111));
        QCOMPARE(s.count(), 100000);
        QCOMPARE(s.sampleAt(51111).y, 999.);
        QVERIFY(selected.front() <= 50000);
        QVERIFY(selected.back() >= 52000);
    }
    void axisAndTransforms()
    {
        ChartAxis axis;
        QVERIFY(!axis.setRange(1, 1));
        QVERIFY(!axis.setRange(2, 1));
        axis.setScale(ChartAxis::Logarithmic);
        QVERIFY(!axis.setRange(0, 10));
        QVERIFY(axis.setRange(1, 1000));
        QCOMPARE(Detail::fraction(10, {1, 1000}, axis.scale()), 1. / 3.);
        QVERIFY(std::abs(Detail::valueAt(.5, {1, 1000}, axis.scale()) - std::sqrt(1000.)) < 1e-9);
        auto ticks = Detail::ticks({1, 1000}, axis.scale(), 6);
        QCOMPARE(ticks, QVector<double>({1, 10, 100, 1000}));
        ticks = Detail::ticks({-3, 7}, ChartAxis::Linear, 6);
        QVERIFY(ticks.contains(0));
    }
    void largeAxisLabels_data()
    {
        QTest::addColumn<QString>("localeName");
        QTest::newRow("english") << QStringLiteral("en_US");
        QTest::newRow("chinese") << QStringLiteral("zh_CN");
    }
    void largeAxisLabels()
    {
        QFETCH(QString, localeName);
        const QLocale previous;
        const auto restore = qScopeGuard([previous] { QLocale::setDefault(previous); });
        QLocale::setDefault(QLocale(localeName));
        ChartAxis axis;
        QCOMPARE(Detail::label(3300000, &axis, {0, 5000000}), QString("3,300,000"));
        QCOMPARE(Detail::label(-1500000, &axis, {-2000000, 0}), QString("-1,500,000"));
        QCOMPARE(Detail::label(1000002, &axis, {1000000, 1000010}), QString("1,000,002"));
        QCOMPARE(Detail::label(1000004, &axis, {1000000, 1000010}), QString("1,000,004"));
        const Detail::Range fractional{1000000, 1000000.01};
        const auto values = Detail::ticks(fractional, axis.scale(), axis.tickCount());
        QSet<QString> labels;
        for (double value : values)
        {
            const auto label = Detail::label(value, &axis, fractional);
            bool ok = false;
            const double parsed = QLocale().toDouble(label, &ok);
            QVERIFY(ok);
            QVERIFY(std::abs(parsed - value) < .0001);
            QVERIFY(!label.contains('e', Qt::CaseInsensitive));
            labels.insert(label);
        }
        QCOMPARE(labels.size(), values.size());
        axis.setScale(ChartAxis::Logarithmic);
        QCOMPARE(Detail::label(1000000, &axis, {1, 100000000}), QString("1,000,000"));
        QCOMPARE(Detail::label(.000001, &axis, {.000001, .00001}), QString("0.000001"));
        axis.setScale(ChartAxis::Category);
        QCOMPARE(Detail::label(1000000, &axis, {0, 2000000}), QString("1,000,000"));
        axis.setCategories({"Original category"});
        QCOMPARE(Detail::label(0, &axis, {0, 1}), QString("Original category"));
    }
    void chartNavigationAndLookup()
    {
        TestChart c;
        c.setSize({800, 600});
        ChartSeries s;
        s.setName("source");
        s.replacePoints({{0, 0}, {5, 10}, {10, 0}});
        c.addSeries(&s);
        c.updatePolish();
        auto at = c.mapToPosition({5, 10});
        QCOMPARE(c.dataAt(at).value("index").toInt(), 1);
        QVERIFY((c.mapToValue(at) - QPointF(5, 10)).manhattanLength() < 1e-8);
        c.zoomAt(2, c.plotRect().center());
        c.updatePolish();
        QVERIFY(!c.xAxis()->automatic());
        QCOMPARE(c.xAxis()->maximum() - c.xAxis()->minimum(), 5.);
        const double old = c.xAxis()->minimum();
        c.panBy({80, 0});
        c.updatePolish();
        QVERIFY(c.xAxis()->minimum() < old);
        QVERIFY(!c.setViewRange(3, 1, 0, 1));
        c.resetView();
        QVERIFY(c.xAxis()->automatic());
        auto *temporary = new ChartSeries;
        c.addSeries(temporary);
        QCOMPARE(c.seriesCount(), 2);
        delete temporary;
        QCOMPARE(c.seriesCount(), 1);
        c.removeSeries(&s);
        QCOMPARE(c.seriesCount(), 0);
        s.append(20, 1);
        c.updatePolish();
    }
    void allChartTypes_data()
    {
        QTest::addColumn<int>("type");
        for (int t = 0; t <= ChartSeries::RangeArea; ++t)
            QTest::newRow(qPrintable(QString::number(t))) << t;
    }
    void allChartTypes()
    {
        QFETCH(int, type);
        TestChart c;
        c.setSize({800, 600});
        c.setChartType(ChartSeries::Type(type));
        ChartSeries a, b;
        a.replaceSamples({{0, 2, 1, "A"}, {1, 4, 3, "B"}, {2, 1, 2, "C"}, {3, 3, 4, "D"}});
        b.replacePoints({{0, 1}, {1, 2}, {2, 3}, {3, 1}});
        if (type == ChartSeries::RangeArea)
            a.replaceRangeSamples(
                {{0, 0, 2, {}, false}, {1, 1, 4, {}, false}, {2, -1, 1, {}, false}});
        c.addSeries(&a);
        c.addSeries(&b);
        c.updatePolish();
        QVERIFY(c.renderedPointCount() > 0);
        QVERIFY(c.plotRect().width() > 0);
        QVERIFY(c.preparationNanoseconds() > 0);
        if (type <= ChartSeries::Bubble || type == ChartSeries::Heatmap)
        {
            const auto point = c.mapToPosition({1, 4});
            QVERIFY(!c.dataAt(point).isEmpty() || type == ChartSeries::Bar ||
                    type == ChartSeries::StackedBar);
        }
        b.setVisible(false);
        c.updatePolish();
        QVERIFY(c.renderedPointCount() > 0);
    }
    void stackedExtentsAndMixed()
    {
        TestChart c;
        c.setSize({800, 600});
        c.setChartType(ChartSeries::StackedBar);
        ChartSeries a, b;
        a.replacePoints({{0, 10}, {1, -3}});
        b.replacePoints({{0, 20}, {1, -8}});
        c.addSeries(&a);
        c.addSeries(&b);
        c.updatePolish();
        const auto top = c.mapToValue(c.plotRect().topLeft());
        const auto bottom = c.mapToValue(c.plotRect().bottomLeft());
        QVERIFY(top.y() > 30);
        QVERIFY(bottom.y() < -11);
        auto point = c.dataAt(c.mapToPosition({0, 25}));
        QCOMPARE(point.value("seriesIndex").toInt(), 1);
        QCOMPARE(point.value("y").toDouble(), 20.);
        c.setChartType(ChartSeries::Bar);
        b.setType(ChartSeries::Line);
        c.updatePolish();
        QVERIFY(!c.dataAt(c.mapToPosition({0, 20})).isEmpty());
    }
    void logarithmicChart()
    {
        TestChart c;
        c.setSize({800, 600});
        ChartSeries s;
        s.replacePoints({{1, 1}, {10, 10}, {100, 100}});
        c.addSeries(&s);
        c.xAxis()->setScale(ChartAxis::Logarithmic);
        c.yAxis()->setScale(ChartAxis::Logarithmic);
        c.updatePolish();
        const auto at = c.mapToPosition({10, 10});
        QVERIFY((at - c.plotRect().center()).manhattanLength() < 1e-6);
        c.zoomAt(2, at);
        c.updatePolish();
        QVERIFY(c.xAxis()->minimum() > 0);
        QVERIFY((c.mapToValue(at) - QPointF(10, 10)).manhattanLength() < 1e-6);
    }
    void singletonDestructionFallsBackSafely()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent component(&engine);
        component.setData("import QCatUiKit\nCatChart {}", QUrl());
        QScopedPointer<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *chart = qobject_cast<ChartItem *>(object.data());
        QVERIFY(chart && chart->theme());
        engine.clearSingletons();
        QVERIFY(chart->theme());
        QCOMPARE(chart->backgroundColor(), chart->tokens().value("surface").value<QColor>());
    }
    void qmlApi()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QCatUiKit\nCatChart { width: 800; height: 500; "
                          "title: 'Native'; "
                          "chartType: CatChartSeries.Bar; xAxis.title: 'Time'; CatChartSeries { "
                          "name: 'A'; color: 'orange' } }",
                          QUrl());
        QScopedPointer<QObject> obj(component.create());
        QVERIFY2(obj, qPrintable(component.errorString()));
        auto *c = qobject_cast<ChartItem *>(obj.data());
        QVERIFY(c);
        QCOMPARE(c->seriesCount(), 1);
        QCOMPARE(c->seriesAt(0)->name(), QString("A"));
    }
    void legendClickAndPointClick()
    {
        TestChart c;
        c.setSize({800, 600});
        ChartSeries s;
        s.setName("Sample");
        s.replacePoints({{0, 1}, {1, 2}, {2, 3}});
        c.addSeries(&s);
        c.updatePolish();
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(35, 582), QPointF(35, 582),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        c.mousePressEvent(&press);
        QVERIFY(!s.isVisible());
        c.mousePressEvent(&press);
        QVERIFY(s.isVisible());
        c.updatePolish();
        QSignalSpy clicked(&c, &ChartItem::pointClicked);
        const auto at = c.mapToPosition({1, 2});
        QMouseEvent down(QEvent::MouseButtonPress, at, at, Qt::LeftButton, Qt::LeftButton,
                         Qt::NoModifier);
        QMouseEvent up(QEvent::MouseButtonRelease, at, at, Qt::LeftButton, Qt::NoButton,
                       Qt::NoModifier);
        c.mousePressEvent(&down);
        c.mouseReleaseEvent(&up);
        QCOMPARE(clicked.size(), 1);
        QCOMPARE(clicked[0][0].toMap().value("index").toInt(), 1);
    }
    void heatmapAndSliceLookup()
    {
        TestChart c;
        c.setSize({800, 600});
        ChartSeries s;
        s.replaceSamples({{0, 0, 11, "A"}, {1, 0, 22, "B"}, {0, 1, 33, "C"}, {1, 1, 44, "D"}});
        c.addSeries(&s);
        c.setChartType(ChartSeries::Heatmap);
        c.updatePolish();
        QCOMPARE(c.dataAt(c.mapToPosition({1, 1})).value("value").toDouble(), 44.);
        c.zoomAt(2, c.plotRect().center());
        c.updatePolish();
        QCOMPARE(c.dataAt(c.mapToPosition({1, 1})).value("value").toDouble(), 44.);
        c.setChartType(ChartSeries::Doughnut);
        s.replaceSamples({{0, 1, 1, "A"}, {1, 1, 1, "B"}});
        c.resetView();
        c.updatePolish();
        QVERIFY(c.dataAt(c.plotRect().center()).isEmpty());
        const auto r = c.plotRect();
        const auto at = r.center() + QPointF(std::min(r.width(), r.height()) * .35, 0);
        QCOMPARE(c.dataAt(at).value("index").toInt(), 0);
    }
    void rawTooltipSurvivesSampling()
    {
        TestChart c;
        c.setSize({800, 600});
        c.setDisplayBudget(8);
        ChartSeries s;
        QVector<ChartSeries::Sample> data;
        for (int i = 0; i < 1000; ++i)
            data.append({double(i), double(i % 7), 1, {}});
        s.replaceSamples(data);
        c.addSeries(&s);
        c.setViewRange(490, 510, -1, 8);
        c.updatePolish();
        const auto p = s.sampleAt(501);
        QCOMPARE(c.dataAt(c.mapToPosition({p.x, p.y})).value("index").toInt(), 501);
        for (int batch = 0; batch < 10; ++batch)
        {
            s.append(1000 + batch, 2);
            c.updatePolish();
            QCOMPARE(c.xAxis()->minimum(), 490.);
        }
    }
};
QTEST_MAIN(ChartTests)
#include "tst_charts.moc"
