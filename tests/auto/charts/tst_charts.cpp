// SPDX-License-Identifier: MIT
#include "qcatchartmath_p.h"
#include <QCatUiKit/qcatchartitem.h>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtTest>
#include <limits>
using namespace QCatUiKit;
class TestChart : public ChartItem
{
  public:
    using ChartItem::mousePressEvent;
    using ChartItem::mouseReleaseEvent;
    using ChartItem::updatePolish;
};
class ChartTests : public QObject
{
    Q_OBJECT
  private slots:
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
        for (int t = 0; t <= 9; ++t)
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
