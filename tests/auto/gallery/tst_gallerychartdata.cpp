// SPDX-License-Identifier: Apache-2.0
#include "gallerychartdata.h"
#include <QtTest>
#include <cmath>

using namespace QCatUiKit;
class GalleryChartDataTests : public QObject
{
    Q_OBJECT
  private slots:
    void millionRecords_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = 0; mode < 15; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void millionRecords()
    {
        QFETCH(int, mode);
        ChartItem chart;
        ChartSeries first, second;
        GalleryChartData demo;
        demo.load(mode, &chart, &first, &second);
        const int initial = demo.sampleCount();
        demo.configureMillionSamples();
        QVERIFY(demo.capacity() <= demo.maximumCapacity());
        QCOMPARE(demo.interval(), 200);
        QVERIFY(demo.batchSize() <= demo.maximumBatchSize());
        demo.clearData();
        for (int i = 0; i < 20; ++i)
            demo.addData();
        QCOMPARE(demo.unitCount(), demo.capacity());
        QCOMPARE(demo.sampleCount(), mode == 14 ? 1000002 : 1000000);
        QVERIFY(demo.aggregated());
        QVERIFY(demo.displayedSampleCount() <= 5120);
        QVERIFY(!demo.sourceAt(0, 0).isEmpty());
        const int sourceCount = mode == 9 ? demo.unitCount() * 40 : demo.unitCount();
        QVERIFY(!demo.sourceAt(0, sourceCount - 1).isEmpty());
        QVERIFY(demo.sourceAt(0, sourceCount).isEmpty());
        QVERIFY(demo.sourceAt(-1, 0).isEmpty());
        const int removedPoints = demo.batchSize() * (mode == 9 ? 40 : 1);
        const auto nextOldest = demo.sourceAt(0, removedPoints);
        demo.addData();
        QCOMPARE(demo.sampleCount(), mode == 14 ? 1000002 : 1000000);
        QCOMPARE(demo.sourceAt(0, 0), nextOldest);
        demo.removeData();
        QCOMPARE(demo.sampleCount(), mode == 14 ? 950002 : 950000);
        demo.setCapacity(3);
        QCOMPARE(demo.unitCount(), 3);
        QVERIFY(!demo.aggregated());
        QCOMPARE(first.count(), mode == 9 ? 120 : 3);
        demo.resetData();
        QCOMPARE(demo.sampleCount(), initial);
        QCOMPARE(demo.sampleCount(), demo.displayedSampleCount());
        QVERIFY(!demo.aggregated());
    }
    void groupedValues_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode : {2, 3, 6, 7, 8, 9, 10, 13})
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void groupedValues()
    {
        QFETCH(int, mode);
        ChartItem chart;
        ChartSeries first, second;
        GalleryChartData demo;
        demo.load(mode, &chart, &first, &second);
        const int units = 1043, perUnit = mode == 9 ? 40 : 1;
        const bool single = mode == 6 || mode == 7 || mode == 9;
        QVector<ChartSeries::Sample> input;
        for (int i = 0; i < units * perUnit; ++i)
            input.append({double(i / perUnit), double(i % perUnit), double(i % 97), {}});
        if (mode != 9)
            for (int i = 0; i < input.size(); ++i)
                input[i].y = (i % 100 + 1) * (mode == 13 && i % 2 ? -1. : 1.);
        first.setCapacity(input.size());
        second.setCapacity(input.size());
        first.replaceSamples(input);
        if (single)
            second.clear();
        else
            second.replaceSamples(input);
        demo.configureMillionSamples();
        QVERIFY(demo.aggregated());
        QCOMPARE(demo.sourceAt(0, input.size() - 1).value("value").toDouble(), input.last().value);
        const int groups = mode == 8 ? 32 : (mode == 6 || mode == 7 || mode == 13) ? 64 : 128;
        for (int group = 0; group < groups; ++group)
        {
            const int from = qint64(units) * group / groups;
            const int to = qint64(units) * (group + 1) / groups;
            for (int row = 0; row < perUnit; ++row)
            {
                double positive = 0, negative = 0;
                for (int i = from; i < to; ++i)
                {
                    const auto &point = input[i * perUnit + row];
                    const double value = mode == 9 ? point.value : point.y;
                    (value >= 0 ? positive : negative) += value;
                }
                if (mode == 13)
                {
                    QCOMPARE(first.sampleAt(group * 2).y, positive);
                    QCOMPARE(first.sampleAt(group * 2 + 1).y, negative);
                }
                else if (mode == 9)
                    QCOMPARE(first.sampleAt(group * 40 + row).value, positive / (to - from));
                else
                    QCOMPARE(first.sampleAt(group).y,
                             mode == 8 ? positive / (to - from) : positive);
            }
        }
    }
    void curvePreservesExtrema()
    {
        ChartItem chart;
        ChartSeries first, second;
        GalleryChartData demo;
        demo.load(0, &chart, &first, &second);
        QVector<ChartSeries::Sample> input;
        for (int i = 0; i < 10043; ++i)
            input.append({double(i), i == 5432 ? 9999. : i == 4321 ? -9999. : 1., 1, {}});
        first.replaceSamples(input);
        second.replaceSamples(input);
        demo.configureMillionSamples();
        QVERIFY(demo.aggregated());
        QCOMPARE(first.sampleAt(0).x, 0.);
        QCOMPARE(first.sampleAt(first.count() - 1).x, 10042.);
        double low = 0, high = 0;
        for (int i = 0; i < first.count(); ++i)
        {
            low = std::min(low, first.sampleAt(i).y);
            high = std::max(high, first.sampleAt(i).y);
        }
        QCOMPARE(low, -9999.);
        QCOMPARE(high, 9999.);
        QCOMPARE(demo.sourceAt(0, 5432).value("y").toDouble(), 9999.);
    }
};
QTEST_MAIN(GalleryChartDataTests)
#include "tst_gallerychartdata.moc"
