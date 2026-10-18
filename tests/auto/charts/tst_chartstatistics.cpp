// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatchartboxplot.h>
#include <QCatUiKit/qcatchartfit.h>
#include <QCatUiKit/qcatcharthistogram.h>
#include <QCatUiKit/qcatchartitem.h>
#include <QCatUiKit/qcatchartohlc.h>
#include <QCatUiKit/qcatchartstack.h>
#include <QCatUiKit/qcatwaterfall.h>
#include <QtTest>
using namespace QCatUiKit;
class ChartStatisticsTests : public QObject
{
    Q_OBJECT
  private slots:
    void ohlcContractAndIntervals()
    {
        ChartOhlc data;
        const QVariantList records{QVariantMap{{"time", 1000},
                                               {"open", 10},
                                               {"high", 15},
                                               {"low", 8},
                                               {"close", 12},
                                               {"volume", 30}},
                                   QVariantMap{{"time", 4000},
                                               {"open", 12},
                                               {"high", 13},
                                               {"low", 6},
                                               {"close", 7},
                                               {"volume", 40}}};
        QVERIFY(data.setData(records));
        QCOMPARE(data.series()->sampleAt(1).x, 1.);
        QCOMPARE(data.series()->at(1).value("close").toDouble(), 7.);
        QCOMPARE(data.volumeSeries()->sampleAt(1).y, 40.);
        data.setCompressedTime(false);
        QCOMPARE(data.series()->sampleAt(1).x, 4000.);
        QCOMPARE(data.series()->at(0).value("categoryWidth").toDouble(), 3000.);
        data.setCandlesticks(false);
        QCOMPARE(data.series()->type(), ChartSeries::Ohlc);
        auto invalid = records;
        auto row = records[1].toMap();
        row["high"] = 6;
        invalid[1] = row;
        QVERIFY(!data.setData(invalid));
        QCOMPARE(data.records(), records);
        row = records[1].toMap();
        row["time"] = 1000;
        invalid[1] = row;
        QVERIFY(!data.setData(invalid));
        QVERIFY(data.setData({}));
        QCOMPARE(data.series()->count(), 0);
        QVERIFY(data.error().isEmpty());
    }
    void boxQuantilesOutliersAndSummary()
    {
        ChartBoxPlot data;
        QVERIFY(data.setData({QVariantMap{{"values", QVariantList{1, 2, 3, 4, 100}}}}));
        auto p = data.series()->at(0);
        QCOMPARE(p.value("q1").toDouble(), 2.);
        QCOMPARE(p.value("median").toDouble(), 3.);
        QCOMPARE(p.value("q3").toDouble(), 4.);
        QCOMPARE(p.value("maximum").toDouble(), 4.);
        QCOMPARE(data.outlierSeries()->sampleAt(0).y, 100.);
        QCOMPARE(data.outlierSeries()->at(0).value("sourceIndex").toInt(), 4);
        const auto before = data.groups();
        QVERIFY(!data.setData({QVariantMap{{"values", QVariantList{INFINITY}}}}));
        QCOMPARE(data.groups(), before);
        QVERIFY(data.setData({QVariantMap{{"minimum", 1},
                                          {"q1", 2},
                                          {"median", 3},
                                          {"q3", 4},
                                          {"maximum", 5},
                                          {"outliers", QVariantList{0, 6}}}}));
        QCOMPARE(data.series()->at(0).value("source").toString(), QString("summary"));
        QCOMPARE(data.outlierSeries()->count(), 2);
        QVERIFY(data.setData({QVariantMap{{"values", QVariantList{1, 2, 3, 4}}}}));
        data.setWhiskerFactor(0);
        QVERIFY(data.error().isEmpty());
        QCOMPARE(data.series()->at(0).value("q1").toDouble(), 1.75);
        QVERIFY(data.setData({QVariantMap{{"values", QVariantList{5}}}}));
        QCOMPARE(data.series()->sampleAt(0).value, 5.);
    }
    void waterfallTotalsResetAndProvenance()
    {
        ChartWaterfall waterfall;
        const QVariantList steps{
            QVariantMap{{"value", 10}, {"label", "A"}}, QVariantMap{{"value", -4}},
            QVariantMap{{"kind", ChartWaterfall::Subtotal}}, QVariantMap{{"value", -8}},
            QVariantMap{{"kind", ChartWaterfall::Total}}};
        QVERIFY(waterfall.setData(steps));
        auto *s = waterfall.series();
        QCOMPARE(s->type(), ChartSeries::Waterfall);
        QCOMPARE(s->count(), 5);
        QCOMPARE(s->lowerValue(1), 10.);
        QCOMPARE(s->sampleAt(1).y, 6.);
        QCOMPARE(s->lowerValue(2), 0.);
        QCOMPARE(s->sampleAt(2).y, 6.);
        QCOMPARE(s->lowerValue(3), 6.);
        QCOMPARE(s->sampleAt(3).y, -2.);
        QCOMPARE(s->sampleAt(3).value, -8.);
        QCOMPARE(waterfall.finalValue(), -2.);
        QCOMPARE(waterfall.steps(), steps);
        QCOMPARE(waterfall.categories().first(), QString("A"));
        waterfall.setResetOnSubtotal(true);
        QCOMPARE(s->lowerValue(3), 0.);
        QCOMPARE(s->sampleAt(3).y, -8.);
        QVERIFY(!s->at(3).value("connectorBefore").toBool());
        QVERIFY(s->at(2).value("resetAfter").toBool());
        waterfall.setInitialValue(2);
        QCOMPARE(s->lowerValue(0), 2.);
        QCOMPARE(waterfall.finalValue(), -6.);
        waterfall.setDecreaseColor(Qt::red);
        QCOMPARE(s->at(1).value("barColor").value<QColor>(), QColor(Qt::red));
        waterfall.setConnectorsVisible(false);
        QVERIFY(!s->at(1).value("connectorBefore").toBool());
        ChartItem chart;
        chart.setSize({640, 480});
        chart.addSeries(s);
        QVERIFY(chart.setViewRange(-.5, 4.5, -10, 15));
        const auto hit = chart.dataAt(chart.mapToPosition({1, 10}));
        QCOMPARE(hit.value("sourceIndex").toInt(), 1);
        QCOMPARE(hit.value("delta").toDouble(), -4.);
        QCOMPARE(hit.value("cumulativeAfter").toDouble(), 8.);
    }
    void waterfallRejectsInvalidBatches()
    {
        ChartWaterfall waterfall;
        QVERIFY(waterfall.setData({QVariantMap{{"value", 5}}}));
        const auto before = waterfall.series()->at(0);
        for (const QVariantList steps :
             {QVariantList{QVariantMap{{"value", INFINITY}}},
              QVariantList{QVariantMap{{"kind", 1.5}}},
              QVariantList{QVariantMap{{"kind", ChartWaterfall::Total}, {"value", 10}}},
              QVariantList{QVariantMap{{"value", 1e308}}, QVariantMap{{"value", 1e308}}}})
        {
            QVERIFY(!waterfall.setData(steps));
            QVERIFY(!waterfall.error().isEmpty());
            QCOMPARE(waterfall.series()->at(0), before);
            QCOMPARE(waterfall.finalValue(), 5.);
        }
        waterfall.setInitialValue(INFINITY);
        QCOMPARE(waterfall.initialValue(), 0.);
        QVERIFY(waterfall.setData({}));
        QCOMPARE(waterfall.series()->count(), 0);
        QVERIFY(waterfall.categories().isEmpty());
        QVERIFY(waterfall.error().isEmpty());
    }
    void linearFitAndResidualBand()
    {
        ChartSeries source;
        QVERIFY(source.replacePoints({{0, 1}, {1, 2}, {2, 2}}));
        ChartFit fit;
        fit.setSource(&source);
        QVERIFY(fit.refresh());
        QCOMPARE(fit.sampleCount(), 3);
        QVERIFY(fit.isValid());
        QCOMPARE(fit.slope(), .5);
        QVERIFY(std::abs(fit.intercept() - 7. / 6) < 1e-12);
        QVERIFY(std::abs(fit.rSquared() - .75) < 1e-12);
        QVERIFY(std::abs(fit.rootMeanSquareError() - std::sqrt(1. / 18)) < 1e-12);
        QCOMPARE(fit.series()->count(), 2);
        QVERIFY(std::abs(fit.predict(1) - 5. / 3) < 1e-12);
        QVERIFY(std::abs(fit.band()->sampleAt(0).y - fit.band()->lowerValue(0) -
                         4 * fit.rootMeanSquareError()) < 1e-12);
        QCOMPARE(source.sampleAt(2).y, 2.);
        QVERIFY(fit.series()->at(0).value("fitted").toBool());
        QCOMPARE(fit.series()->at(0).value("observationCount").toInt(), 3);
        fit.setResidualMultiplier(0);
        QVERIFY(fit.refresh());
        QCOMPARE(fit.band()->sampleAt(0).y, fit.band()->lowerValue(0));
    }
    void fitPrecisionDegeneracyAndLifecycle()
    {
        ChartFit fit;
        auto *source = new ChartSeries;
        fit.setSource(source);
        QVERIFY(!fit.refresh());
        QVERIFY(fit.error().isEmpty());
        source->replacePoints({{1e15 + 2, 5}, {1e15, 1}, {1e15 + 1, 3}});
        QVERIFY(fit.refresh());
        QCOMPARE(fit.slope(), 2.);
        QCOMPARE(fit.predict(1e15 + .5), 2.);
        QCOMPARE(fit.series()->sampleAt(0).x, 1e15);
        source->replacePoints({{-1e308, -1e308}, {0, 0}, {1e308, 1e308}});
        QVERIFY(fit.refresh());
        QCOMPARE(fit.slope(), 1.);
        QCOMPARE(fit.intercept(), 0.);
        source->replacePoints({{0, 5}, {1, 5}});
        QVERIFY(fit.refresh());
        QCOMPARE(fit.slope(), 0.);
        QCOMPARE(fit.rSquared(), 1.);
        QCOMPARE(fit.rootMeanSquareError(), 0.);
        source->replacePoints({{1, 5}, {1, 6}});
        QVERIFY(!fit.refresh());
        QVERIFY(!fit.error().isEmpty());
        QCOMPARE(fit.series()->count(), 0);
        QVERIFY(std::isnan(fit.predict(1)));
        source->replacePoints({{0, 1}, {1, 3}});
        QTRY_VERIFY(fit.isValid());
        QCOMPARE(fit.predict(.5), 2.);
        QSignalSpy updates(&fit, &ChartFit::statisticsChanged);
        source->setColor(Qt::red);
        QCoreApplication::processEvents();
        QCOMPARE(updates.count(), 0);
        ChartAxis axis;
        source->setYAxis(&axis);
        QTRY_COMPARE(fit.series()->yAxis(), &axis);
        delete source;
        QCOMPARE(fit.source(), nullptr);
        QVERIFY(!fit.isValid());
        QCOMPARE(fit.band()->count(), 0);
    }
    void percentageCrossingLimitsAndSeparateScales()
    {
        ChartSeries a, b;
        a.replacePoints({{0, 2}, {1, -2}});
        b.replacePoints({{0, -3}, {1, 3}});
        ChartStack stack;
        stack.addSource(&a);
        stack.addSource(&b);
        stack.setPercentage(true);
        QVERIFY(stack.refresh());
        const auto *out = stack.seriesAt(0);
        QCOMPARE(out->count(), 4);
        QCOMPARE(out->sampleAt(1).x, .5);
        QCOMPARE(out->sampleAt(1).y, 100.);
        QCOMPARE(out->sampleAt(2).x, .5);
        QCOMPARE(out->sampleAt(2).y, -100.);
        QVERIFY(out->breakBefore(2));
        QVERIFY(out->at(1).value("percentageLimit").toBool());
        QCOMPARE(out->at(1).value("stackValue").toDouble(), 0.);
        a.replacePoints({{0, 1e308}, {1, 1e308}});
        b.replacePoints({{0, -1e-300}, {1, -1e-300}});
        QVERIFY(stack.refresh());
        QCOMPARE(stack.seriesAt(1)->sampleAt(0).y, -100.);
        a.replacePoints({{0, 2}});
        b.replacePoints({{0, 3}});
        QVERIFY(stack.refresh());
        QCOMPARE(stack.seriesAt(0)->count(), 1);
    }
    void histogramBinsAndNormalization()
    {
        ChartHistogram histogram;
        histogram.setBinEdges({0, 1, 3, 6});
        const QVariantList values{-1, 0, .5, 1, 2, 3, 6, 7};
        QVERIFY(histogram.setData(values));
        QCOMPARE(histogram.values(), values);
        QCOMPARE(histogram.sampleCount(), 8);
        QCOMPARE(histogram.includedCount(), 6);
        QCOMPARE(histogram.underflowCount(), 1);
        QCOMPARE(histogram.overflowCount(), 1);
        auto *series = histogram.series();
        QCOMPARE(series->type(), ChartSeries::Histogram);
        QCOMPARE(series->count(), 3);
        for (int i = 0; i < 3; ++i)
        {
            QCOMPARE(series->sampleAt(i).y, 2.);
            QCOMPARE(series->at(i).value("binCount").toInt(), 2);
            QCOMPARE(series->at(i).value("upperInclusive").toBool(), i == 2);
        }
        histogram.setNormalization(ChartHistogram::Frequency);
        double sum = 0;
        for (int i = 0; i < 3; ++i)
            sum += series->sampleAt(i).y;
        QCOMPARE(sum, 1.);
        histogram.setNormalization(ChartHistogram::Density);
        double integral = 0;
        for (int i = 0; i < 3; ++i)
            integral += series->sampleAt(i).y * (i + 1);
        QCOMPARE(integral, 1.);
        QCOMPARE(series->sampleAt(0).y, 1. / 3);
        QCOMPARE(series->sampleAt(2).y, 1. / 9);
        ChartItem chart;
        chart.setSize({640, 480});
        chart.addSeries(series);
        chart.setBarSpacing(0);
        QVERIFY(chart.setViewRange(0, 6, 0, .5));
        for (const QPointF point : {QPointF(.1, .1), QPointF(2.9, .1), QPointF(5.9, .1)})
        {
            const auto hit = chart.dataAt(chart.mapToPosition(point));
            QVERIFY(!hit.isEmpty());
            QCOMPARE(hit.value("binCount").toInt(), 2);
        }
        QVERIFY(histogram.setData({-5, 8}));
        QCOMPARE(histogram.includedCount(), 0);
        for (int i = 0; i < 3; ++i)
            QCOMPARE(series->sampleAt(i).y, 0.);
    }
    void histogramInvalidInputPreservesResult()
    {
        ChartHistogram histogram;
        histogram.setBinCount(2);
        QVERIFY(histogram.setData({0, 1, 2}));
        const auto before = histogram.series()->at(0);
        QVERIFY(!histogram.setData({0, INFINITY}));
        QVERIFY(!histogram.error().isEmpty());
        QCOMPARE(histogram.sampleCount(), 3);
        QCOMPARE(histogram.series()->at(0), before);
        histogram.setBinEdges({0, 0, 2});
        QCOMPARE(histogram.binEdges(), QList<double>());
        histogram.setBinCount(0);
        QCOMPARE(histogram.binCount(), 2);
        histogram.setNormalization(ChartHistogram::Normalization(99));
        QCOMPARE(histogram.normalization(), ChartHistogram::Count);
        histogram.setBinEdges({-1e308, 1e308});
        QVERIFY(!histogram.error().isEmpty());
        QCOMPARE(histogram.series()->at(0), before);
        QVERIFY(histogram.setData({0, 1, 2}));
        QVERIFY(histogram.error().isEmpty());
    }
    void histogramEmptyConstantAndExtreme()
    {
        ChartHistogram histogram;
        QVERIFY(histogram.setData({}));
        QCOMPARE(histogram.series()->count(), 0);
        QVERIFY(histogram.setData({5, 5, 5}));
        QCOMPARE(histogram.includedCount(), 3);
        int total = 0;
        for (int i = 0; i < histogram.series()->count(); ++i)
            total += histogram.series()->at(i).value("binCount").toInt();
        QCOMPARE(total, 3);
        QVERIFY(histogram.setData({1e308, 1e308}));
        QCOMPARE(histogram.includedCount(), 2);
        histogram.setBinCount(1);
        QVERIFY(!histogram.setData({-1e308, 1e308}));
        histogram.setBinCount(4);
        QVERIFY(histogram.setData({-1e308, 1e308}));
        QCOMPARE(histogram.includedCount(), 2);
        QVERIFY(histogram.setData({}));
        QCOMPARE(histogram.series()->count(), 0);
        histogram.setBinEdges({0, 1, 2});
        QCOMPARE(histogram.series()->count(), 2);
        QCOMPARE(histogram.series()->sampleAt(0).y, 0.);
    }
    void stackValuesAndVisibility()
    {
        ChartSeries a, b;
        a.replacePoints({{0, 2}, {1, 4}, {2, 6}});
        b.replacePoints({{0, 3}, {1, 6}, {2, 9}});
        ChartStack stack;
        stack.addSource(&a);
        stack.addSource(&b);
        QVERIFY(stack.refresh());
        auto *out = stack.seriesAt(1);
        QCOMPARE(out->count(), 3);
        QCOMPARE(out->lowerValue(1), 4.);
        QCOMPARE(out->sampleAt(1).y, 10.);
        QCOMPARE(out->at(1).value("sourceY").toDouble(), 6.);
        QCOMPARE(out->at(1).value("percentage").toDouble(), 60.);
        QCOMPARE(b.sampleAt(1).y, 6.);
        stack.setPercentage(true);
        QVERIFY(stack.refresh());
        QCOMPARE(out->lowerValue(1), 40.);
        QCOMPARE(out->sampleAt(1).y, 100.);
        a.setVisible(false);
        QVERIFY(stack.refresh());
        QCOMPARE(out->lowerValue(1), 0.);
        QCOMPARE(out->sampleAt(1).y, 100.);
        QCOMPARE(stack.seriesAt(0)->count(), 0);
        a.setVisible(true);
        QVERIFY(stack.refresh());
        QCOMPARE(out->lowerValue(1), 40.);
        ChartItem chart;
        chart.setSize({640, 480});
        chart.addSeries(out);
        chart.setViewRange(0, 2, 0, 110);
        const auto hit = chart.dataAt(chart.mapToPosition({1, 70}));
        QCOMPARE(hit.value("sourceIndex").toInt(), 1);
        QCOMPARE(hit.value("sourceY").toDouble(), 6.);
        stack.seriesAt(0)->setVisible(false); // The chart legend operates on derived series.
        QCOMPARE(a.isVisible(), false);
        QTRY_COMPARE(out->lowerValue(1), 0.);
        QCOMPARE(out->sampleAt(1).y, 100.);
        a.setVisible(true);
        QTRY_COMPARE(out->lowerValue(1), 40.);
    }
    void signedCrossingsAndGaps()
    {
        ChartSeries a, b;
        a.replacePoints({{0, 2}, {1, -2}, {2, -2}});
        b.replacePoints({{0, 4}, {1, 4}, {2, 4}});
        ChartStack stack;
        stack.addSource(&a);
        stack.addSource(&b);
        QVERIFY(stack.refresh());
        auto *out = stack.seriesAt(1);
        QCOMPARE(out->sampleAt(1).x, .5);
        QCOMPARE(out->lowerValue(1), 0.);
        QCOMPARE(out->sampleAt(0).y, 6.);
        QCOMPARE(out->sampleAt(2).y, 4.);
        a.setBreakBefore(1, true);
        QVERIFY(stack.refresh());
        QCOMPARE(out->sampleAt(0).x, 1.); // A gap in any source interrupts the stack.
        QCOMPARE(out->count(), 2);
        a.clear();
        b.clear();
        QVERIFY(stack.refresh());
        QCOMPARE(out->count(), 0);
    }
    void alignmentErrorsAndLifetime()
    {
        ChartStack stack;
        ChartSeries a;
        auto *b = new ChartSeries;
        a.replacePoints({{0, 1}, {1, 2}});
        b->replacePoints({{0, 3}, {2, 4}});
        stack.addSource(&a);
        stack.addSource(b);
        QVERIFY(!stack.refresh());
        QVERIFY(!stack.error().isEmpty());
        QCOMPARE(a.count(), 2);
        QCOMPARE(stack.seriesAt(0)->count(), 0);
        b->replacePoints({{0, 3}, {1, 4}});
        QVERIFY(stack.refresh());
        QVERIFY(stack.error().isEmpty());
        QPointer<ChartSeries> output = stack.seriesAt(1);
        delete b;
        QCOMPARE(stack.count(), 1);
        QVERIFY(output.isNull());
        QVERIFY(stack.refresh());
        a.replacePoints({{0, 1}, {0, 2}});
        QVERIFY(!stack.refresh());
        stack.clearSources();
        QVERIFY(stack.refresh());
        QCOMPARE(stack.count(), 0);
    }
    void percentagesAvoidOverflow()
    {
        ChartStack stack;
        ChartSeries a, b;
        a.replacePoints({{0, 1e308}, {1, 1e308}});
        b.replacePoints({{0, 1e308}, {1, 1e308}});
        stack.addSource(&a);
        stack.addSource(&b);
        QVERIFY(!stack.refresh());
        stack.setPercentage(true);
        QVERIFY(stack.refresh());
        QCOMPARE(stack.seriesAt(1)->lowerValue(0), 50.);
        QCOMPARE(stack.seriesAt(1)->sampleAt(0).y, 100.);
        a.replacePoints({{0, 0}, {1, 0}});
        b.replacePoints({{0, 0}, {1, 0}});
        QVERIFY(stack.refresh());
        QCOMPARE(stack.seriesAt(0)->count(), 0);
    }
    void stackAxisChangesAndPan()
    {
        ChartAxis axis;
        ChartSeries a;
        a.setYAxis(&axis);
        a.replacePoints({{0, 2}, {1, 3}});
        ChartStack stack;
        stack.addSource(&a);
        QVERIFY(stack.refresh());
        QSignalSpy changed(stack.seriesAt(0), &ChartSeries::dataChanged);
        QVERIFY(axis.setRange(-10, 10));
        QCoreApplication::processEvents();
        QCOMPARE(changed.count(), 0); // A viewport update must not rebuild stack data.
        axis.setScale(ChartAxis::Logarithmic);
        QTRY_VERIFY(!stack.error().isEmpty());
        QCOMPARE(stack.seriesAt(0)->count(), 0);
        axis.setScale(ChartAxis::Linear);
        QTRY_VERIFY(stack.error().isEmpty());
        QCOMPARE(stack.seriesAt(0)->count(), 2);
    }
};
QTEST_MAIN(ChartStatisticsTests)
#include "tst_chartstatistics.moc"
