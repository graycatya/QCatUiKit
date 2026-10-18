// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatchartbubble.h>
#include <QCatUiKit/qcatchartcontour.h>
#include <QCatUiKit/qcatchartgrid.h>
#include <QCatUiKit/qcatchartitem.h>
#include <QtTest>
#include <cmath>
using namespace QCatUiKit;
class ExtendedChartTests : public QObject
{
    Q_OBJECT
  private slots:
    void colorScaleRangeAndAlpha()
    {
        ChartColorScale scale;
        scale.setColors({QColor(255, 0, 0, 0), QColor(0, 0, 255, 255)});
        QVERIFY(scale.setRange(0, 10));
        const auto middle = scale.colorAt(5);
        QVERIFY(middle.blue() > 250);
        QVERIFY(middle.red() < 5);
        QVERIFY(std::abs(middle.alpha() - 128) <= 1);
        QCOMPARE(scale.colorAt(-5).alpha(), 0);
        QCOMPARE(scale.colorAt(20), QColor(Qt::blue));
        QVERIFY(!scale.setRange(5, 1));
        QCOMPARE(scale.maximum(), 10.);
        scale.setDataRange(-1, 1);
        QCOMPARE(scale.maximum(), 10.);
        scale.setAutomatic(true);
        QCOMPARE(scale.minimum(), -1.);
        scale.setDataRange(7, 7);
        QCOMPARE(scale.normalized(7), .5);
        scale.setDataRange(-1e308, 1e308);
        QCOMPARE(scale.normalized(0), .5);
        scale.setDataRange(0, std::numeric_limits<double>::denorm_min());
        QCOMPARE(scale.normalized(0), 0.);
        QCOMPARE(scale.normalized(std::numeric_limits<double>::denorm_min()), 1.);
    }
    void bubbleIndependentEncoding()
    {
        ChartBubble data;
        const QVariantList points{
            QVariantMap{{"x", 0}, {"y", 1}, {"size", 4}, {"colorValue", -10}},
            QVariantMap{{"x", 1}, {"y", 2}, {"size", 16}, {"colorValue", 10}}};
        QVERIFY(data.setData(points));
        QCOMPARE(data.series()->sampleAt(0).value, 4.);
        data.setSizeMode(ChartBubble::Area);
        const double a = data.series()->sampleAt(0).value, b = data.series()->sampleAt(1).value;
        QVERIFY(std::abs(b * b / a / a - 4) < 1e-12);
        QCOMPARE(data.colorScale()->minimum(), -10.);
        QCOMPARE(data.series()->at(1).value("rawSize").toDouble(), 16.);
        data.colorScale()->setRange(-5, 5);
        QCOMPARE(data.series()->at(1).value("mappedColor").value<QColor>(),
                 data.colorScale()->colorAt(5));
        QVERIFY(!data.setData({QVariantMap{{"x", 0}, {"y", 0}, {"size", -1}}}));
        QCOMPARE(data.points(), points);
        data.setColorMapping(false);
        QVERIFY(!data.series()->at(0).contains("mappedColor"));
    }
    void gridMissingUpdateScrollAndPicking()
    {
        ChartGrid grid;
        QVERIFY(grid.setGrid(2, 3, {1, 2, QVariant(), 4, 5, 6}));
        QCOMPARE(grid.series()->count(), 5);
        QCOMPARE(grid.series()->at(2).value("sourceIndex").toInt(), 3);
        QCOMPARE(grid.colorScale()->maximum(), 6.);
        const auto before = grid.values();
        QVERIFY(!grid.setGrid(2, 3, {1, 2}));
        QCOMPARE(grid.values(), before);
        QVERIFY(!grid.setValue(0, 0, INFINITY));
        QCOMPARE(grid.values(), before);
        QVERIFY(grid.setValue(0, 2, 3));
        QCOMPARE(grid.series()->count(), 6);
        QVERIFY(grid.appendRow({7, 8, 9}));
        QCOMPARE(grid.origin(), QPointF(0, 1));
        QCOMPARE(grid.values(), QVariantList({4, 5, 6, 7, 8, 9}));
        grid.colorScale()->setRange(0, 10);
        QVERIFY(grid.setValue(0, 0, 50));
        QCOMPARE(grid.colorScale()->maximum(), 10.);
        ChartItem chart;
        chart.setSize({640, 480});
        chart.addSeries(grid.series());
        const auto hit = chart.dataAt(chart.mapToPosition({0, 1}));
        QCOMPARE(hit.value("value").toDouble(), 50.);
        QCOMPARE(hit.value("row").toInt(), 0);
        QVERIFY(grid.setGrid(0, 0, {}));
        QCOMPARE(grid.series()->count(), 0);
    }
    void contoursSaddlesGapsAndLifetime()
    {
        auto *grid = new ChartGrid;
        QVERIFY(grid->setGrid(2, 2, {-1, 1, 1, -1}));
        ChartContour contour;
        contour.setSource(grid);
        contour.setLevels({-.5, 0, .5});
        contour.setFilled(true);
        QVERIFY(contour.refresh());
        QVERIFY(contour.series()->count() > 0);
        int fills = 0, lines = 0;
        for (int i = 0; i < contour.series()->count(); ++i)
        {
            const auto m = contour.series()->at(i);
            m.value("filled").toBool() ? ++fills : ++lines;
            for (const auto &v : m.value("vertices").toList())
            {
                const auto p = v.toPointF();
                QVERIFY(std::isfinite(p.x()) && p.x() >= 0 && p.x() <= 1 && p.y() >= 0 &&
                        p.y() <= 1);
            }
        }
        QVERIFY(fills > 0 && lines > 0);
        ChartAxis independent;
        grid->series()->setYAxis(&independent);
        QTRY_COMPARE(contour.series()->yAxis(), &independent);
        const auto levels = contour.levels();
        contour.setLevels({1, 0});
        QCOMPARE(contour.levels(), levels);
        QVERIFY(!contour.error().isEmpty());
        QVERIFY(grid->setValue(0, 0, QVariant()));
        QVERIFY(contour.refresh());
        QCOMPARE(contour.series()->count(), 0);
        QVERIFY(grid->setGrid(2, 2, {1, 1, 1, 1}));
        contour.setFilled(false);
        contour.setLevels({1});
        QVERIFY(contour.refresh());
        QCOMPARE(contour.series()->count(), 0);
        // A plateau on a shared band boundary belongs to exactly one band.
        QVERIFY(grid->setGrid(2, 2, {0, 0, 0, 0}));
        contour.setFilled(true);
        contour.setLevels({-1, 0, 1});
        QVERIFY(contour.refresh());
        QCOMPARE(contour.series()->count(), 4);
        for (int i = 0; i < contour.series()->count(); ++i)
            QCOMPARE(contour.series()->at(i).value("lowerLevel").toDouble(), 0.);
        delete grid;
        QCOMPARE(contour.source(), nullptr);
        QCOMPARE(contour.series()->count(), 0);
    }
    void radarDimensionRangesAndRawPicking()
    {
        ChartItem chart;
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Radar);
        ChartSeries series;
        series.replacePoints({{0, 50}, {1, 5}, {2, -5}});
        chart.addSeries(&series);
        const QVariantList ranges{QVariantMap{{"minimum", 0}, {"maximum", 100}},
                                  QVariantMap{{"minimum", 0}, {"maximum", 10}},
                                  QVariantMap{{"minimum", -10}, {"maximum", 0}}};
        chart.setRadarRanges(ranges);
        const auto plot = chart.plotRect();
        const double radius = std::min(plot.width(), plot.height()) * .42;
        const auto hit = chart.dataAt(plot.center() + QPointF(0, -radius * .5));
        QCOMPARE(hit.value("y").toDouble(), 50.);
        chart.setRadarRanges({QVariantMap{{"minimum", 2}, {"maximum", 1}}});
        QCOMPARE(chart.radarRanges(), ranges);
        series.setStrokeStyle(ChartSeries::DashLine);
        series.setFillOpacity(0);
        QCOMPARE(chart.dataAt(plot.center() + QPointF(0, -radius * .5)).value("index").toInt(), 0);
        const double tiny = std::numeric_limits<double>::denorm_min();
        series.replacePoints({{0, tiny}, {1, tiny}, {2, tiny}});
        chart.setRadarRanges({QVariantMap{{"minimum", 0}, {"maximum", tiny}},
                              QVariantMap{{"minimum", 0}, {"maximum", tiny}},
                              QVariantMap{{"minimum", 0}, {"maximum", tiny}}});
        const auto tinyHit = chart.dataAt(plot.center() + QPointF(0, -radius));
        QVERIFY(tinyHit.contains("y"));
        QCOMPARE(tinyHit.value("y").toDouble(), tiny);
    }
};
QTEST_MAIN(ExtendedChartTests)
#include "tst_chartextended.moc"
