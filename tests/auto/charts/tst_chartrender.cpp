// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatchartfit.h>
#include <QCatUiKit/qcatchartohlc.h>
#include <QCatUiKit/qcatchartboxplot.h>
#include <QCatUiKit/qcatchartbubble.h>
#include <QCatUiKit/qcatchartgrid.h>
#include <QCatUiKit/qcatchartcontour.h>
#include <QCatUiKit/qcatcharthistogram.h>
#include <QCatUiKit/qcatchartitem.h>
#include <QCatUiKit/qcatchartstack.h>
#include <QCatUiKit/qcatwaterfall.h>
#include <QHoverEvent>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QtTest>

using namespace QCatUiKit;
namespace
{
class InteractiveChart : public ChartItem
{
  public:
    using ChartItem::ChartItem;
    using ChartItem::mouseMoveEvent;
    using ChartItem::mousePressEvent;
    using ChartItem::mouseReleaseEvent;
    void hover(QPointF point)
    {
        QHoverEvent event(QEvent::HoverMove, point, point, QPointF());
        hoverMoveEvent(&event);
    }
};
quint64 stat(const ChartItem &chart, const char *name)
{
    return chart.renderingStatistics().value(QLatin1String(name)).toULongLong();
}
bool current(const ChartItem &chart)
{
    return stat(chart, "frames") > 0 && stat(chart, "preparedRevision") > 0 &&
           stat(chart, "renderedRevision") == stat(chart, "preparedRevision");
}
bool areaReady(const ChartItem &chart)
{
    return chart.renderingStatistics().value("backend").toString() == QStringLiteral("software")
               ? stat(chart, "preparedRevision") > 0
               : current(chart);
}
int differences(const QImage &a, const QImage &b, QRectF logical)
{
    if (a.size() != b.size())
        return -1;
    const qreal dpr = a.devicePixelRatio();
    const QRect bounds(QPoint(qCeil(logical.left() * dpr), qCeil(logical.top() * dpr)),
                       QPoint(qFloor(logical.right() * dpr), qFloor(logical.bottom() * dpr)));
    int n = 0;
    for (int y = std::max(0, bounds.top()); y <= std::min(a.height() - 1, bounds.bottom()); ++y)
        for (int x = std::max(0, bounds.left()); x <= std::min(a.width() - 1, bounds.right()); ++x)
            n += a.pixel(x, y) != b.pixel(x, y);
    return n;
}
QColor pixel(const QImage &image, QPointF logical)
{
    const auto point = logical * image.devicePixelRatio();
    return image.pixelColor(qRound(point.x()), qRound(point.y()));
}
ChartItem *findChart(QQuickItem *item, const QString &name)
{
    if (item->objectName() == name)
        return qobject_cast<ChartItem *>(item);
    for (auto *child : item->childItems())
        if (auto *chart = findChart(child, name))
            return chart;
    return nullptr;
}
} // namespace

class ChartRenderTests : public QObject
{
    Q_OBJECT
  private slots:
    void radarFillStrokeAndNormalization()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Radar);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        ChartSeries s;
        s.setColor(Qt::red);
        s.setFillOpacity(1);
        s.setLineWidth(4);
        s.replacePoints({{0, 1}, {1, 1}, {2, 1}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto plot = chart.plotRect();
        const double radius = std::min(plot.width(), plot.height()) * .42;
        auto image = window.grabWindow();
        QVERIFY(pixel(image, plot.center()).red() > 250);
        auto revision = stat(chart, "preparedRevision");
        s.setFillOpacity(0);
        s.setStrokeStyle(ChartSeries::DashLine);
        s.setDashPattern({10, 10});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QCOMPARE(pixel(image, plot.center()), QColor(Qt::black));
        const auto a = plot.center() + QPointF(0, -radius),
                   b = plot.center() + QPointF(std::sqrt(3.) / 2 * radius, radius / 2);
        int colored = 0, gaps = 0;
        for (int i = 20; i < 80; ++i)
        {
            const auto p = a * (1 - i / 100.) + b * (i / 100.);
            pixel(image, p).red() > 128 ? ++colored : ++gaps;
        }
        QVERIFY(colored > 5 && gaps > 5);
        revision = stat(chart, "preparedRevision");
        chart.setRadarRanges({QVariantMap{{"minimum", 0}, {"maximum", 2}},
                              QVariantMap{{"minimum", 0}, {"maximum", 2}},
                              QVariantMap{{"minimum", 0}, {"maximum", 2}}});
        s.setStrokeStyle(ChartSeries::SolidLine);
        s.setMarkersVisible(true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, plot.center() + QPointF(0, -radius / 2)).red() > 250);
        QCOMPARE(chart.dataAt(plot.center() + QPointF(0, -radius / 2)).value("y").toDouble(), 1.);
    }
    void financialBodiesWicksAndOutliers()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        ChartOhlc prices;
        prices.setRisingColor(Qt::green);
        prices.setFallingColor(Qt::red);
        QVERIFY(prices.setData(
            {QVariantMap{{"time", 1}, {"open", 3}, {"close", 7}, {"low", 1}, {"high", 9}},
             QVariantMap{{"time", 2}, {"open", 7}, {"close", 3}, {"low", 2}, {"high", 8}}}));
        prices.series()->setLineWidth(3);
        chart.addSeries(prices.series());
        QVERIFY(chart.setViewRange(-1, 2, 0, 10));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 5})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({1, 5})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({0, 8})).green() > 200);
        QCOMPARE(chart.dataAt(chart.mapToPosition({0, 8})).value("low").toDouble(), 1.);
        auto revision = stat(chart, "preparedRevision");
        prices.setCandlesticks(false);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.2, 5})).green() < 20);
        // A large price offset must not turn a nonzero body into a doji.
        const double offset = 1e15;
        revision = stat(chart, "preparedRevision");
        prices.setCandlesticks(true);
        QVERIFY(prices.setData({QVariantMap{{"time", 1},
                                            {"open", offset + 2},
                                            {"close", offset + 8},
                                            {"low", offset + 1},
                                            {"high", offset + 9}}}));
        QVERIFY(chart.setViewRange(-1, 2, offset, offset + 10));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.2, offset + 5})).green() > 200);
        QVERIFY(chart.setViewRange(-1, 2, 0, 10));
        chart.removeSeries(prices.series());
        ChartBoxPlot box;
        QVERIFY(box.setData({QVariantMap{{"values", QVariantList{1, 2, 3, 4, 9}}}}));
        box.series()->setColor(Qt::green);
        box.series()->setBorderColor(Qt::red);
        box.series()->setLineWidth(3);
        box.outlierSeries()->setColor(Qt::blue);
        revision = stat(chart, "preparedRevision");
        chart.addSeries(box.series());
        chart.addSeries(box.outlierSeries());
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.15, 2.5})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({0, 2.5})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({.15, 3})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({0, 9})).blue() > 200);
        QVERIFY(chart.dataAt(chart.mapToPosition({0, 9})).value("outlier").toBool());
    }
    void scalarGridSparseAndPartialUpload()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        ChartGrid grid;
        grid.colorScale()->setColors({QColor(Qt::red), QColor(Qt::blue)});
        grid.colorScale()->setRange(0, 10);
        grid.colorScale()->setLegendVisible(false);
        QVERIFY(grid.setGrid(2, 3, {0, 5, QVariant(), 10, 0, 5}));
        chart.addSeries(grid.series());
        QVERIFY(chart.setViewRange(-.5, 2.5, -.5, 1.5));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 0})).red() > 250);
        QVERIFY(pixel(image, chart.mapToPosition({0, 1})).blue() > 250);
        QCOMPARE(pixel(image, chart.mapToPosition({2, 0})), QColor(Qt::black));
        QVERIFY(chart.dataAt(chart.mapToPosition({2, 0})).isEmpty());
        const bool gpu = chart.renderingStatistics().value("backend").toString() != "software";
        const auto bytes = stat(chart, "textureUploadedBytes"),
                   allocations = stat(chart, "textureAllocations");
        auto revision = stat(chart, "preparedRevision");
        QVERIFY(grid.setValue(0, 0, 10));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 0})).blue() > 250);
        if (gpu)
        {
            QCOMPARE(stat(chart, "textureUploadedBytes") - bytes, quint64(4));
            QCOMPARE(stat(chart, "textureAllocations"), allocations);
        }
        revision = stat(chart, "preparedRevision");
        grid.setDense(false);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto sparse = window.grabWindow();
        for (const QPointF p : {QPointF(0, 0), QPointF(1, 0), QPointF(2, 0), QPointF(0, 1)})
            QCOMPARE(pixel(sparse, chart.mapToPosition(p)), pixel(image, chart.mapToPosition(p)));
        revision = stat(chart, "preparedRevision");
        grid.setDense(true);
        chart.xAxis()->setReversed(true);
        chart.yAxis()->setReversed(true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 0})).blue() > 250);
        QCOMPARE(pixel(image, chart.mapToPosition({2, 0})), QColor(Qt::black));
    }
    void bubbleAreaColorAndContourBands()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        ChartBubble bubble;
        bubble.colorScale()->setColors({QColor(Qt::red), QColor(Qt::blue)});
        bubble.colorScale()->setLegendVisible(false);
        bubble.setSizeMode(ChartBubble::Area);
        QVERIFY(bubble.setData(
            {QVariantMap{{"x", 0}, {"y", 0}, {"size", 314.1592653589793}, {"colorValue", 0}},
             QVariantMap{{"x", 1}, {"y", 0}, {"size", 1256.637061435917}, {"colorValue", 1}}}));
        chart.addSeries(bubble.series());
        QVERIFY(chart.setViewRange(-1, 2, -1, 1));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 0})).red() > 250);
        QVERIFY(pixel(image, chart.mapToPosition({1, 0}) + QPointF(15, 0)).blue() > 250);
        QCOMPARE(pixel(image, chart.mapToPosition({0, 0}) + QPointF(15, 0)), QColor(Qt::black));
        chart.removeSeries(bubble.series());
        ChartGrid grid;
        grid.colorScale()->setColors({QColor(Qt::green), QColor(Qt::green)});
        grid.colorScale()->setLegendVisible(false);
        QVERIFY(grid.setGrid(2, 2, {0, 1, 0, 1}));
        ChartContour contour;
        contour.setSource(&grid);
        contour.setLevels({.25, .75});
        contour.setFilled(true);
        contour.setLabelsVisible(false);
        QVERIFY(contour.refresh());
        auto revision = stat(chart, "preparedRevision");
        chart.addSeries(contour.series());
        chart.setViewRange(0, 1, 0, 1);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.5, .4})).green() > 200);
        QCOMPARE(pixel(image, chart.mapToPosition({.1, .4})), QColor(Qt::black));
        const auto hit = chart.dataAt(chart.mapToPosition({.5, .4}));
        QVERIFY(hit.value("filled").toBool());
        QCOMPARE(hit.value("lowerLevel").toDouble(), .25);
        revision = stat(chart, "preparedRevision");
        QVERIFY(grid.setValue(0, 0, QVariant()));
        QVERIFY(contour.refresh());
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(pixel(window.grabWindow(), chart.mapToPosition({.5, .4})), QColor(Qt::black));
    }
    void waterfallColorsConnectionsAndReset()
    {
        QQuickWindow window;
        window.resize(800, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({800, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        ChartWaterfall waterfall;
        waterfall.setIncreaseColor(Qt::green);
        waterfall.setDecreaseColor(Qt::red);
        waterfall.setSubtotalColor(Qt::yellow);
        waterfall.setTotalColor(Qt::blue);
        waterfall.series()->setBorderColor(Qt::white);
        waterfall.series()->setLineWidth(3);
        QVERIFY(waterfall.setData({QVariantMap{{"value", 10}}, QVariantMap{{"value", -4}},
                                   QVariantMap{{"kind", ChartWaterfall::Subtotal}},
                                   QVariantMap{{"value", -8}},
                                   QVariantMap{{"kind", ChartWaterfall::Total}}}));
        chart.addSeries(waterfall.series());
        QVERIFY(chart.setViewRange(-.5, 4.5, -10, 12));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 5})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({1, 8})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2, 4})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2, 4})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({4, -1})).blue() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({.5, 10})).green() > 200);
        const auto hit = chart.dataAt(chart.mapToPosition({1, 8}));
        QCOMPARE(hit.value("delta").toDouble(), -4.);
        QCOMPARE(hit.value("cumulativeAfter").toDouble(), 6.);
        const auto revision = stat(chart, "preparedRevision");
        waterfall.setResetOnSubtotal(true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({3, -6})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({3, 4})).red() < 30);
        QVERIFY(pixel(image, chart.mapToPosition({2.5, 6})).green() < 30);
        const auto colorRevision = stat(chart, "preparedRevision");
        waterfall.setIncreaseColor(QColor(0, 255, 0, 128));
        QTRY_VERIFY(stat(chart, "preparedRevision") > colorRevision && areaReady(chart));
        image = window.grabWindow();
        const int green = pixel(image, chart.mapToPosition({0, 5})).green();
        QVERIFY(green >= 126 && green <= 130);
    }
    void fittedLineAndBand()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        ChartSeries source;
        source.replacePoints({{-1, 0}, {0, 2}, {1, 0}});
        ChartFit fit;
        fit.setSource(&source);
        fit.setResidualMultiplier(1);
        QVERIFY(fit.refresh());
        fit.band()->setColor(Qt::blue);
        fit.band()->setFillOpacity(1);
        fit.series()->setColor(Qt::red);
        fit.series()->setLineWidth(4);
        chart.addSeries(fit.band());
        chart.addSeries(fit.series());
        QVERIFY(chart.setViewRange(-1.5, 1.5, -1, 3));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({0, 2. / 3})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({0, 1})).blue() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({0, -.5})).blue() < 30);
        const auto hit = chart.dataAt(chart.mapToPosition({0, 1}));
        QVERIFY(hit.value("fitted").toBool());
        QCOMPARE(hit.value("observationCount").toInt(), 3);
    }
    void histogramUnequalWidths()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        chart.setBarSpacing(0);
        ChartHistogram histogram;
        histogram.setBinEdges({0, 1, 3, 6});
        QVERIFY(histogram.setData({0, .5, 1, 2, 3, 6}));
        histogram.series()->setColor(Qt::red);
        chart.addSeries(histogram.series());
        QVERIFY(chart.setViewRange(0, 6, 0, 3));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto image = window.grabWindow();
        for (double x : {.1, .9, 1.1, 2.9, 3.1, 5.9})
            QVERIFY(pixel(image, chart.mapToPosition({x, 1})).red() > 200);
        const auto hit = chart.dataAt(chart.mapToPosition({5.9, 1}));
        QCOMPARE(hit.value("binLower").toDouble(), 3.);
        QCOMPARE(hit.value("binUpper").toDouble(), 6.);
        QCOMPARE(hit.value("binCount").toInt(), 2);
        const auto revision = stat(chart, "preparedRevision");
        histogram.setNormalization(ChartHistogram::Density);
        QVERIFY(chart.setViewRange(0, 6, 0, .4));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.5, .3})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2, .15})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({5, .1})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2, .3})).red() < 30);
        QVERIFY(pixel(image, chart.mapToPosition({5, .15})).red() < 30);
    }
    void stackedAreaBands()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        ChartSeries a, b;
        a.setColor(Qt::red);
        b.setColor(Qt::green);
        a.setFillOpacity(1);
        b.setFillOpacity(1);
        a.replacePoints({{0, 2}, {1, 2}, {2, -2}, {3, -2}});
        b.replacePoints({{0, 3}, {1, 3}, {2, 3}, {3, 3}});
        ChartStack stack;
        stack.addSource(&a);
        stack.addSource(&b);
        QVERIFY(stack.refresh());
        chart.addSeries(stack.seriesAt(0));
        chart.addSeries(stack.seriesAt(1));
        QVERIFY(chart.setViewRange(0, 3, -6, 6));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.5, 1})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({.5, 4})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2.5, -1})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2.5, 1})).green() > 200);
        auto hit = chart.dataAt(chart.mapToPosition({.5, 4}));
        QCOMPARE(hit.value("sourceSeriesIndex").toInt(), 1);
        QCOMPARE(hit.value("sourceY").toDouble(), 3.);
        const auto revision = stat(chart, "preparedRevision");
        stack.setPercentage(true);
        QVERIFY(stack.refresh());
        QVERIFY(chart.setViewRange(0, 3, -110, 110));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({.5, 20})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({.5, 70})).green() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2.5, -60})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({1.75, -80})).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({2.5, 60})).green() > 200);
    }
    void denseMarkersTranslateWithPan_data()
    {
        QTest::addColumn<int>("mode");
        QTest::addColumn<bool>("reversed");
        for (int mode : {int(ChartSeries::LinearCurve), int(ChartSeries::CatmullRomSpline)})
            for (bool reversed : {false, true})
                QTest::newRow(qPrintable(QString("%1-%2").arg(mode).arg(reversed)))
                    << mode << reversed;
    }
    void denseMarkersTranslateWithPan()
    {
        QFETCH(int, mode);
        QFETCH(bool, reversed);
        QQuickWindow window;
        window.resize(800, 480);
        InteractiveChart chart(window.contentItem());
        chart.setSize({800, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        chart.setCrosshairEnabled(false);
        chart.setTooltipsEnabled(false);
        chart.setDisplayBudget(128);
        ChartSeries series;
        series.setCapacity(100000);
        series.setColor(Qt::red);
        series.setMarkersVisible(true);
        series.setMarkerSize(10);
        series.setLineWidth(1);
        series.setCurveMode(ChartSeries::CurveMode(mode));
        QVector<ChartSeries::Sample> samples;
        for (int i = 0; i < 100000; ++i)
            samples.append({double(i), std::sin(i * .00073), 1, {}});
        QVERIFY(series.replaceSamples(samples));
        chart.addSeries(&series);
        QVERIFY(chart.setViewRange(1000, 99000, -2, 2));
        chart.xAxis()->setReversed(reversed);
        chart.yAxis()->setReversed(reversed);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto before = window.grabWindow();
        const QRectF plot = chart.plotRect();
        const quint64 revision = stat(chart, "preparedRevision");
        chart.panBy({5, 0});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto after = window.grabWindow();
        QCOMPARE(chart.plotRect(), plot);
        const qreal dpr = before.devicePixelRatio();
        const int shift = qRound(5 * dpr);
        const QRect region = QRectF(plot.adjusted(30, 20, -30, -20).topLeft() * dpr,
                                    plot.adjusted(30, 20, -30, -20).size() * dpr).toAlignedRect();
        int changed = 0, colored = 0;
        for (int y = region.top(); y < region.bottom(); ++y)
            for (int x = region.left(); x < region.right(); ++x)
            {
                const int a = before.pixelColor(x, y).red();
                const int b = after.pixelColor(x + shift, y).red();
                colored += a > 64 || b > 64;
                changed += std::abs(a - b) > 32;
            }
        QVERIFY(colored > 500);
        QVERIFY2(changed < colored / 20,
                 qPrintable(QString("%1 of %2 pixels changed after translation")
                                .arg(changed).arg(colored)));
    }
    void selectionClipsExtremeGeometry_data()
    {
        QTest::addColumn<int>("type");
        QTest::newRow("bar") << int(ChartSeries::Bar);
        QTest::newRow("band") << int(ChartSeries::RangeArea);
    }
    void selectionClipsExtremeGeometry()
    {
        QFETCH(int, type);
        QQuickWindow window;
        window.resize(640, 480);
        InteractiveChart chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Type(type));
        chart.setSelectionColor(Qt::yellow);
        chart.setSelectionWidth(3);
        ChartSeries series;
        series.setColor(Qt::red);
        if (type == ChartSeries::RangeArea)
            series.replaceRangeSamples({{5, -1e100, 1e100}});
        else
            series.replacePoints({{5, 1e100}});
        chart.addSeries(&series);
        chart.setViewRange(0, 10, 0, 10);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto before = window.grabWindow();
        QVERIFY(chart.selectPoint(0, 0));
        QTRY_VERIFY(differences(before, window.grabWindow(), chart.plotRect()) > 100);
        if (type == ChartSeries::RangeArea)
            QCOMPARE(pixel(window.grabWindow(), chart.mapToPosition({5, 5})), QColor(Qt::yellow));
    }
    void appearanceFadeReusesGeometry()
    {
        ThemePalette palette;
        palette.setOverrides({{"motionDuration", 600}});
        QQuickWindow window;
        window.resize(640, 480);
        InteractiveChart chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setTheme(&palette);
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Scatter);
        ChartSeries series;
        series.setMarkerShape(ChartSeries::Square);
        series.setMarkerSize(40);
        series.setColor(Qt::red);
        series.replacePoints({{5, 5}});
        chart.addSeries(&series);
        chart.setViewRange(0, 10, 0, 10);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        chart.setAnimationsEnabled(true);
        series.setColor(Qt::blue);
        QVERIFY(chart.transitionRunning());
        QTest::qWait(80);
        QTRY_VERIFY(areaReady(chart));
        const auto center = chart.mapToPosition({5, 5});
        const auto partial = pixel(window.grabWindow(), center);
        QVERIFY(partial.blue() > 0 && partial.blue() < 255);
        QCOMPARE(partial.red(), 0);
        const auto uploaded = stat(chart, "uploadedBytes"),
                   revision = stat(chart, "preparedRevision");
        QTRY_VERIFY(!chart.transitionRunning());
        QCOMPARE(pixel(window.grabWindow(), center), QColor(Qt::blue));
        QCOMPARE(stat(chart, "uploadedBytes"), uploaded);
        QCOMPARE(stat(chart, "preparedRevision"), revision);
        series.setColor(Qt::red);
        QVERIFY(chart.transitionRunning());
        palette.setReducedMotion(true);
        QVERIFY(!chart.transitionRunning());
        QTRY_VERIFY(areaReady(chart));
        QCOMPARE(pixel(window.grabWindow(), center), QColor(Qt::red));
        series.append(8, 8);
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(!chart.transitionRunning());
        QCOMPARE(series.count(), 2);
    }
    void selectionOverlayDoesNotUploadData()
    {
        QQuickWindow window;
        window.resize(640, 480);
        InteractiveChart chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setGridVisible(false);
        chart.setLegendVisible(false);
        chart.setChartType(ChartSeries::Scatter);
        chart.setSelectionColor(Qt::yellow);
        chart.setSelectionWidth(3);
        ChartSeries series;
        series.setMarkerSize(24);
        series.setMarkerShape(ChartSeries::Square);
        series.setColor(Qt::red);
        series.replacePoints({{5, 5}});
        chart.addSeries(&series);
        chart.setViewRange(0, 10, 0, 10);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto before = window.grabWindow();
        const auto uploaded = stat(chart, "uploadedBytes"),
                   revision = stat(chart, "preparedRevision");
        const auto center = chart.mapToPosition({5, 5});
        QVERIFY(chart.selectPoint(0, 0));
        QTRY_COMPARE(pixel(window.grabWindow(), center + QPointF(15, 0)), QColor(Qt::yellow));
        QCOMPARE(stat(chart, "uploadedBytes"), uploaded);
        QCOMPARE(stat(chart, "preparedRevision"), revision);
        chart.clearSelection();
        QTRY_COMPARE(pixel(window.grabWindow(), center + QPointF(15, 0)), QColor(Qt::black));
        QCOMPARE(stat(chart, "uploadedBytes"), uploaded);
    }
    void delegateRenderingAndFallback()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick; Rectangle { required property var chartData; color: 'lime' }", QUrl());
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QQuickWindow window;
        window.resize(640, 480);
        InteractiveChart chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setChartType(ChartSeries::Scatter);
        chart.setDelegateBudget(1);
        ChartSeries series;
        series.setColor(Qt::red);
        series.setMarkerSize(32);
        series.setMarkerShape(ChartSeries::Square);
        series.setPointDelegate(&component);
        series.replacePoints({{3, 5}, {7, 5}});
        chart.addSeries(&series);
        chart.setViewRange(0, 10, 0, 10);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        QTRY_COMPARE(chart.activeDelegateCount(), 1);
        auto image = window.grabWindow();
        QCOMPARE(pixel(image, chart.mapToPosition({3, 5})), QColor(Qt::green));
        QCOMPARE(pixel(image, chart.mapToPosition({7, 5})), QColor(Qt::red));
        chart.setDelegateBudget(0);
        QTRY_COMPARE(chart.activeDelegateCount(), 0);
        QTRY_VERIFY(areaReady(chart));
        image = window.grabWindow();
        QCOMPARE(pixel(image, chart.mapToPosition({3, 5})), QColor(Qt::red));
        chart.setDelegateBudget(2);
        QTRY_COMPARE(chart.activeDelegateCount(), 2);
        QTRY_VERIFY(areaReady(chart));
        image = window.grabWindow();
        QCOMPARE(pixel(image, chart.mapToPosition({7, 5})), QColor(Qt::green));
        chart.removeSeries(&series);
        QTRY_COMPARE(chart.activeDelegateCount(), 0);
        QTRY_VERIFY(areaReady(chart));
        image = window.grabWindow();
        QCOMPARE(pixel(image, chart.mapToPosition({7, 5})), QColor(Qt::black));
    }
    void textureMarkers_data()
    {
        QTest::addColumn<int>("type");
        for (int type : {int(ChartSeries::Line), int(ChartSeries::Area), int(ChartSeries::Scatter),
                         int(ChartSeries::Bubble)})
            QTest::newRow(qPrintable(QString::number(type))) << type;
    }
    void textureMarkers()
    {
        QFETCH(int, type);
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QImage image(16, 16, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        for (int y = 4; y < 12; ++y)
            for (int x = 4; x < 12; ++x)
                image.setPixelColor(x, y, QColor(255, 0, 0, 128));
        const auto file = directory.filePath("marker.png");
        QVERIFY(image.save(file));
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Type(type));
        chart.setViewRange(0, 10, 0, 10);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        ChartSeries series;
        series.setColor(Qt::blue);
        series.setMarkerShape(ChartSeries::TextureMarker);
        series.setMarkerTexture(QUrl::fromLocalFile(file));
        series.setMarkerSize(40);
        series.setMarkersVisible(true);
        series.replaceSamples({{5, 5, 20, {}}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto center = chart.mapToPosition({5, 5});
        const auto frame = window.grabWindow();
        const auto middle = pixel(frame, center);
        QVERIFY(std::abs(middle.red() - 128) <= 2 && middle.blue() < 3);
        QCOMPARE(pixel(frame, center + QPointF(17, -17)).red(), 0);
        QCOMPARE(chart.dataAt(center + QPointF(17, -17)).value("index", -1).toInt(), 0);
        const auto uploads = stat(chart, "textureUploads");
        auto revision = stat(chart, "preparedRevision");
        series.setMarkerSize(48);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(stat(chart, "textureUploads"), uploads);
        revision = stat(chart, "preparedRevision");
        series.setMarkerTexture(QUrl::fromLocalFile(directory.filePath("missing.png")));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QVERIFY(!series.markerTextureError().isEmpty());
        QVERIFY(pixel(window.grabWindow(), center).blue() > 250);
    }
    void rotatedBarLabel()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Bar);
        chart.setViewRange(0, 10, 0, 10);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        ChartSeries series;
        series.setColor(QColor("#003366"));
        series.setBarLabelsVisible(true);
        series.setBarLabelFormat("123456789");
        series.replacePoints({{5, 8}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto center = chart.mapToPosition({5, 4});
        const auto whiteBounds = [&](const QImage &image)
        {
            QRect bounds;
            for (int y = -60; y <= 60; ++y)
                for (int x = -18; x <= 18; ++x)
                {
                    const auto color = pixel(image, center + QPointF(x, y));
                    if (color.red() > 180 && color.green() > 180 && color.blue() > 180)
                        bounds |= QRect(x, y, 1, 1);
                }
            return bounds;
        };
        QVERIFY(whiteBounds(window.grabWindow()).isEmpty());
        auto revision = stat(chart, "preparedRevision");
        series.setBarLabelRotation(90);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto bounds = whiteBounds(window.grabWindow());
        QVERIFY(bounds.height() > 40);
        QVERIFY(bounds.width() < 20);
        revision = stat(chart, "preparedRevision");
        series.setBarLabelRotation(-90);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QVERIFY(whiteBounds(window.grabWindow()).height() > 40);
    }
    void instanceStorageAndTransparency()
    {
        QQuickWindow window;
        window.resize(800, 600);
        InteractiveChart chart(window.contentItem());
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::Scatter);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 64, 0, 64);
        chart.setDisplayBudget(0);
        ChartSeries series;
        series.setColor(QColor(255, 0, 0, 128));
        series.setMarkerShape(ChartSeries::Square);
        series.setMarkerSize(4);
        QVector<QPointF> samples;
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 64; ++x)
                samples.append({x + .5, y + .5});
        series.replacePoints(samples);
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        QCOMPARE(stat(chart, "instanceCount"), 4096ULL);
        QCOMPARE(stat(chart, "instanceUploads"), 1ULL);
        QCOMPARE(stat(chart, "vertexUploads"), 0ULL);
        QCOMPARE(stat(chart, "uploadedBytes"), 4096ULL * 28);
        const auto allocations = stat(chart, "bufferAllocations"),
                   bytes = stat(chart, "uploadedBytes");
        chart.hover(chart.plotRect().center());
        QTest::qWait(60);
        QCOMPARE(stat(chart, "uploadedBytes"), bytes);
        auto revision = stat(chart, "preparedRevision");
        samples[2048].ry() += .2;
        series.replacePoints(samples);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(stat(chart, "uploadedBytes") - bytes, 28ULL);
        QCOMPARE(stat(chart, "bufferAllocations"), allocations);
        revision = stat(chart, "preparedRevision");
        series.clear();
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(stat(chart, "instanceCount"), 0ULL);
        chart.setAcceptHoverEvents(false);
        chart.hover({-1, -1});
        series.setMarkerSize(24);
        revision = stat(chart, "preparedRevision");
        series.replacePoints({{5, 5}, {5, 5}});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(stat(chart, "instanceCount"), 2ULL);
        QVERIFY(std::abs(pixel(window.grabWindow(), chart.mapToPosition({5, 5})).red() - 192) <= 2);
    }
    void denseRangeLod_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = 0; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void denseRangeLod()
    {
        QFETCH(int, mode);
        QQuickWindow window;
        window.resize(800, 500);
        ChartItem chart(window.contentItem());
        chart.setSize({800, 500});
        chart.setChartType(ChartSeries::RangeArea);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 25000, -10, 10);
        ChartSeries series;
        series.setCapacity(20000);
        series.setColor(Qt::red);
        series.setFillOpacity(.6);
        series.setCurveMode(ChartSeries::CurveMode(mode));
        series.setLineWidth(3);
        QVector<ChartSeries::RangeSample> samples;
        for (int i = 0; i < 20000; ++i)
            samples.append({double(i < 10000 ? i : i + 5000),
                            i == 4321 ? -8. : 4 * std::cos(i * .02),
                            i == 1234 ? 8. : 4 * std::sin(i * .02),
                            {},
                            i == 10000});
        series.replaceRangeSamples(samples);
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(chart.renderedPointCount() < 12000);
        const auto image = window.grabWindow();
        for (const auto &point : {QPointF(1234, 8), QPointF(4321, -8)})
        {
            const auto at = chart.mapToPosition(point);
            bool found = false;
            for (int dy = -2; dy <= 2; ++dy)
                for (int dx = -2; dx <= 2; ++dx)
                    found |= pixel(image, at + QPointF(dx, dy)).red() > 140;
            QVERIFY(found);
        }
        QCOMPARE(chart.dataAt(chart.mapToPosition({4321, -8})).value("index").toInt(), 4321);
        QVERIFY(pixel(image, chart.mapToPosition({12500, 0})).red() < 5);
        const auto builds = stat(chart, "curveCacheBuilds");
        auto revision = stat(chart, "preparedRevision");
        chart.panBy({15, 0});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(stat(chart, "curveCacheBuilds"), builds);
        QVERIFY(stat(chart, "curveLodNodesVisited") < 5000);
        revision = stat(chart, "preparedRevision");
        chart.setViewRange(4318, 4324, -10, 10);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto detailed = window.grabWindow();
        revision = stat(chart, "preparedRevision");
        chart.setDisplayBudget(0);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(
            differences(detailed, window.grabWindow(), chart.plotRect().adjusted(5, 5, -5, -5)), 0);
        QCOMPARE(series.count(), 20000);
    }
    void multiStopFill_data()
    {
        QTest::addColumn<int>("type");
        QTest::addColumn<int>("style");
        for (int type : {int(ChartSeries::Area), int(ChartSeries::RangeArea)})
            for (int style :
                 {int(ChartSeries::HorizontalGradient), int(ChartSeries::VerticalGradient)})
                QTest::newRow(qPrintable(QString("%1-%2").arg(type).arg(style))) << type << style;
    }
    void multiStopFill()
    {
        QFETCH(int, type);
        QFETCH(int, style);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries series;
        series.setBorderColor(Qt::transparent);
        series.setFillOpacity(1);
        series.setFillStyle(ChartSeries::FillStyle(style));
        series.setFillGradientStops({QVariantMap{{"position", 0.0}, {"color", QColor(Qt::red)}},
                                     QVariantMap{{"position", 0.5}, {"color", QColor(Qt::green)}},
                                     QVariantMap{{"position", 1.0}, {"color", QColor(Qt::blue)}}});
        if (type == ChartSeries::RangeArea)
            series.replaceRangeSamples({{-1, -1, 11}, {11, -1, 11}});
        else
            series.replacePoints({{-1, 11}, {11, 11}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto plot = chart.plotRect();
        const auto at = [&](double t)
        {
            return style == ChartSeries::HorizontalGradient
                       ? QPointF(plot.left() + t * plot.width(), plot.center().y())
                       : QPointF(plot.center().x(), plot.top() + t * plot.height());
        };
        const auto image = window.grabWindow();
        const auto first = pixel(image, at(.25)), middle = pixel(image, at(.5)),
                   last = pixel(image, at(.75));
        QVERIFY(std::abs(first.red() - 128) < 4 && std::abs(first.green() - 128) < 4 &&
                first.blue() < 3);
        QVERIFY(middle.green() > 250 && middle.red() < 4 && middle.blue() < 4);
        QVERIFY(std::abs(last.blue() - 128) < 4 && std::abs(last.green() - 128) < 4 &&
                last.red() < 3);
        auto revision = stat(chart, "preparedRevision");
        series.setFillGradientStops(
            {QVariantMap{{"position", 0.2}, {"color", QColor(Qt::red)}},
             QVariantMap{{"position", 0.5}, {"color", QColor(0, 255, 0, 0)}},
             QVariantMap{{"position", 0.8}, {"color", QColor(Qt::blue)}}});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto transparent = window.grabWindow();
        QVERIFY(pixel(transparent, at(.5)).green() < 3);
        QVERIFY(pixel(transparent, at(.1)).red() > 250);
        QVERIFY(pixel(transparent, at(.9)).blue() > 250);
    }
    void textureFillAndReuse()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QImage tile(4, 4, QImage::Format_ARGB32);
        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x)
                tile.setPixelColor(
                    x, y, x < 2 ? (y < 2 ? Qt::red : Qt::blue) : (y < 2 ? Qt::green : Qt::yellow));
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                tile.setPixelColor(x, y, QColor(255, 0, 0, 128));
        const auto path = directory.filePath("tile.png");
        QVERIFY(tile.save(path));
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::RangeArea);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries series;
        series.setBorderColor(Qt::transparent);
        series.setFillOpacity(.5);
        series.setFillColor(Qt::magenta);
        series.setFillStyle(ChartSeries::TextureFill);
        series.setFillTexture(QUrl::fromLocalFile(path));
        QVERIFY(series.fillTextureError().isEmpty());
        series.replaceRangeSamples({{-10, -10, 20}, {20, -10, 20}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto plot = chart.plotRect();
        const auto point = [&](double x, double y)
        { return plot.topLeft() + QPointF(x * plot.width(), y * plot.height()); };
        auto image = window.grabWindow();
        QVERIFY(std::abs(pixel(image, point(.25, .25)).red() - 64) < 3);
        QVERIFY(std::abs(pixel(image, point(.75, .25)).green() - 128) < 3);
        QVERIFY(std::abs(pixel(image, point(.25, .75)).blue() - 128) < 3);
        const bool gpu = chart.renderingStatistics().value("backend").toString() != "software";
        auto uploads = stat(chart, "textureUploads");
        if (gpu)
            QCOMPARE(uploads, 1ULL);
        auto revision = stat(chart, "preparedRevision");
        chart.panBy({5, 0});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(stat(chart, "textureUploads"), uploads);
        revision = stat(chart, "preparedRevision");
        series.setFillTextureMode(ChartSeries::TileTexture);
        series.setFillTextureSize({64, 64});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        image = window.grabWindow();
        QVERIFY(std::abs(pixel(image, plot.topLeft() + QPointF(16, 16)).red() - 64) < 3);
        QVERIFY(std::abs(pixel(image, plot.topLeft() + QPointF(80, 16)).red() - 64) < 3);
        QVERIFY(std::abs(pixel(image, plot.topLeft() + QPointF(48, 16)).green() - 128) < 3);
        uploads = stat(chart, "textureUploads");
        revision = stat(chart, "preparedRevision");
        series.setFillTextureSize({96, 96});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(stat(chart, "textureUploads"), uploads);
        revision = stat(chart, "preparedRevision");
        series.setFillTexture(QUrl::fromLocalFile(directory.filePath("missing.png")));
        QVERIFY(!series.fillTextureError().isEmpty());
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto fallback = pixel(window.grabWindow(), plot.center());
        QVERIFY(std::abs(fallback.red() - 128) < 3 && std::abs(fallback.blue() - 128) < 3);
    }
    void lineCaps_data()
    {
        QTest::addColumn<int>("cap");
        QTest::addColumn<bool>("dashed");
        QTest::addColumn<int>("type");
        for (int cap = 1; cap <= 3; ++cap)
            for (int dashed = 0; dashed < 2; ++dashed)
                for (int type :
                     {int(ChartSeries::Line), int(ChartSeries::Area), int(ChartSeries::RangeArea)})
                    QTest::newRow(qPrintable(QString("%1-%2-%3").arg(cap).arg(dashed).arg(type)))
                        << cap << bool(dashed) << type;
    }
    void lineCaps()
    {
        QFETCH(int, cap);
        QFETCH(bool, dashed);
        QFETCH(int, type);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAntialiasing(false);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries series;
        series.setColor(Qt::red);
        series.setLineWidth(20);
        series.setFillOpacity(0);
        series.setLineCap(ChartSeries::LineCapStyle(cap));
        series.setLineJoin(ChartSeries::BevelJoin);
        if (dashed)
        {
            series.setStrokeStyle(ChartSeries::DashLine);
            series.setDashPattern({40, 30});
        }
        if (type == ChartSeries::RangeArea)
            series.replaceRangeSamples({{2, 2, 5}, {8, 2, 5}});
        else
            series.replacePoints({{2, 5}, {8, 5}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto image = window.grabWindow();
        const auto start = chart.mapToPosition({2, 5});
        QVERIFY(pixel(image, start + QPointF(15, 0)).red() > 200);
        QCOMPARE(pixel(image, start - QPointF(7, 0)).red() > 200, cap != ChartSeries::FlatCap);
        QCOMPARE(pixel(image, start + QPointF(-8, 8)).red() > 200, cap == ChartSeries::SquareCap);
        if (dashed)
            QCOMPARE(pixel(image, start + QPointF(55, 0)).red(), 0);
    }
    void lineJoinsAndAlpha_data()
    {
        QTest::addColumn<int>("join");
        for (int join = 1; join <= 3; ++join)
            QTest::newRow(qPrintable(QString::number(join))) << join;
    }
    void lineJoinsAndAlpha()
    {
        QFETCH(int, join);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAntialiasing(false);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries series;
        series.setColor(QColor(255, 0, 0, 128));
        series.setLineWidth(24);
        series.setLineCap(ChartSeries::FlatCap);
        series.setLineJoin(ChartSeries::LineJoinStyle(join));
        series.replacePoints({{2, 3}, {5, 3}, {5, 8}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto image = window.grabWindow();
        const auto at = chart.mapToPosition({5, 3});
        QVERIFY(std::abs(pixel(image, at + QPointF(-4, -4)).red() - 128) <= 2);
        const int corner = pixel(image, at + QPointF(10, 10)).red();
        QCOMPARE(corner > 100, join == ChartSeries::MiterJoin);
        // The stroke interior is triangulated once, including joins, so alpha never doubles.
        for (int y = -10; y <= 10; ++y)
            for (int x = -10; x <= 10; ++x)
                QVERIFY(pixel(image, at + QPointF(x, y)).red() <= 130);
    }
    void strokeLimitsContinuityAndAa()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAntialiasing(false);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries series;
        series.setColor(Qt::red);
        series.setLineWidth(20);
        series.setLineCap(ChartSeries::RoundCap);
        series.setLineJoin(ChartSeries::MiterJoin);
        series.replacePoints({{4.5, 2}, {5, 6}, {5.5, 2}});
        series.setMiterLimit(1);
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto shortMiter = window.grabWindow();
        auto revision = stat(chart, "preparedRevision");
        series.setMiterLimit(12);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto longMiter = window.grabWindow();
        QVERIFY(differences(shortMiter, longMiter, chart.plotRect()) > 10);
        // A redundant collinear source vertex must not restart a dash or create an extra cap.
        series.setStrokeStyle(ChartSeries::DashLine);
        series.setDashPattern({36, 20});
        series.setDashOffset(7);
        series.replacePoints({{2, 5}, {8, 5}});
        revision = stat(chart, "preparedRevision");
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto continuous = window.grabWindow();
        series.replacePoints({{2, 5}, {4, 5}, {8, 5}});
        revision = stat(chart, "preparedRevision");
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(differences(continuous, window.grabWindow(), chart.plotRect()), 0);
        series.setStrokeStyle(ChartSeries::SolidLine);
        series.setLineJoin(ChartSeries::RoundJoin);
        series.replacePoints({{2, 2}, {5, 8}, {8, 3}});
        chart.setAntialiasing(true);
        revision = stat(chart, "preparedRevision");
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto smooth = window.grabWindow();
        int partial = 0;
        const auto rect = chart.plotRect().adjusted(20, 20, -20, -20);
        for (int y = qCeil(rect.top()); y < rect.bottom(); ++y)
            for (int x = qCeil(rect.left()); x < rect.right(); ++x)
            {
                const int red = pixel(smooth, {qreal(x), qreal(y)}).red();
                partial += red > 0 && red < 255;
            }
        QVERIFY(partial > 20);
    }
    void multiAxisRenders_data()
    {
        QTest::addColumn<int>("mask");
        for (int mask = 0; mask < 4; ++mask)
            QTest::newRow(qPrintable(QString::number(mask))) << mask;
    }
    void multiAxisRenders()
    {
        QFETCH(int, mask);
        QQuickWindow window;
        window.resize(800, 600);
        ChartItem chart(window.contentItem());
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::Scatter);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        ChartAxis extraX, extraY;
        extraX.setOrientation(Qt::Horizontal);
        extraX.setSide(ChartAxis::FarSide);
        extraY.setSide(ChartAxis::FarSide);
        extraX.setRange(100, 200);
        extraY.setRange(1000, 2000);
        extraX.setReversed(mask & 1);
        extraY.setReversed(mask & 2);
        ChartSeries a, b;
        a.setColor(Qt::red);
        b.setColor(Qt::green);
        a.setMarkerSize(16);
        b.setMarkerSize(16);
        a.replacePoints({{2, 3}});
        b.replacePoints({{120, 1800}});
        b.setXAxis(&extraX);
        b.setYAxis(&extraY);
        chart.addSeries(&a);
        chart.addSeries(&b);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto image = window.grabWindow();
        QVERIFY(pixel(image, chart.mapToPosition({2, 3}, &a)).red() > 200);
        QVERIFY(pixel(image, chart.mapToPosition({120, 1800}, &b)).green() > 200);
        QCOMPARE(chart.dataAt(chart.mapToPosition({120, 1800}, &b)).value("seriesIndex").toInt(),
                 1);
        std::unique_ptr<QPointingDevice> device(QTest::createTouchDevice());
        auto touch = QTest::touchEvent(&window, device.get(), false);
        const auto oldCenter = chart.plotRect().center().toPoint();
        const auto anchorA = chart.mapToValue(oldCenter, &a),
                   anchorB = chart.mapToValue(oldCenter, &b);
        touch.press(0, oldCenter - QPoint(60, 0)).press(1, oldCenter + QPoint(60, 0)).commit();
        QTest::qWait(20);
        touch.move(0, oldCenter - QPoint(70, -20)).move(1, oldCenter + QPoint(110, 20)).commit();
        QTRY_VERIFY(!extraX.automatic() && extraX.maximum() - extraX.minimum() < 80);
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(QLineF(chart.mapToPosition(anchorA, &a), oldCenter + QPoint(20, 20)).length() <
                .01);
        QVERIFY(QLineF(chart.mapToPosition(anchorB, &b), oldCenter + QPoint(20, 20)).length() <
                .01);
        touch.release(0, oldCenter - QPoint(70, -20))
            .release(1, oldCenter + QPoint(110, 20))
            .commit();
        const auto before = chart.plotRect();
        const auto revision = stat(chart, "preparedRevision");
        extraX.setVisible(false);
        extraY.setVisible(false);
        QTRY_VERIFY(areaReady(chart) && stat(chart, "preparedRevision") > revision);
        QVERIFY(chart.plotRect().width() > before.width());
        QVERIFY(chart.plotRect().height() > before.height());
        QCOMPARE(chart.axisForSeries(&b, Qt::Vertical), &extraY);
        QCOMPARE(b.sampleAt(0).y, 1800.);
    }
    void reversedGeometry_data()
    {
        QTest::addColumn<int>("type");
        QTest::addColumn<int>("mode");
        for (int type :
             {int(ChartSeries::Line), int(ChartSeries::Area), int(ChartSeries::RangeArea)})
            for (int mode = 0; mode <= 6; ++mode)
                QTest::newRow(qPrintable(QString("curve-%1-%2").arg(type).arg(mode)))
                    << type << mode;
        for (int type :
             {int(ChartSeries::Bar), int(ChartSeries::StackedBar), int(ChartSeries::PercentBar),
              int(ChartSeries::HorizontalBar), int(ChartSeries::HorizontalStackedBar),
              int(ChartSeries::HorizontalPercentBar), int(ChartSeries::Scatter),
              int(ChartSeries::Bubble), int(ChartSeries::Heatmap)})
            QTest::newRow(qPrintable(QString("shape-%1").arg(type))) << type << 0;
    }
    void reversedGeometry()
    {
        QFETCH(int, type);
        QFETCH(int, mode);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAntialiasing(false);
        chart.setViewRange(-1, type == ChartSeries::HorizontalPercentBar ? 120 : 10, -1,
                           type == ChartSeries::PercentBar ? 120 : 10);
        ChartSeries series;
        series.setColor(Qt::red);
        series.setMarkerSize(14);
        series.setLineWidth(4);
        series.setCurveMode(ChartSeries::CurveMode(mode));
        if (type == ChartSeries::RangeArea)
            series.replaceRangeSamples({{1, 1, 2, {}, false},
                                        {3, 2, 7, {}, false},
                                        {5, 3, 8, {}, false},
                                        {8, 1, 4, {}, false}});
        else
            series.replaceSamples({{1, 2, 8, {}}, {3, 7, 9, {}}, {5, 8, 10, {}}, {8, 4, 11, {}}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto normal = window.grabWindow();
        const auto normalBandHit = chart.dataAt(chart.mapToPosition({3.25, 4}));
        const auto plot = chart.plotRect();
        auto revision = stat(chart, "preparedRevision");
        chart.xAxis()->setReversed(true);
        chart.yAxis()->setReversed(true);
        QTRY_VERIFY(areaReady(chart) && stat(chart, "preparedRevision") > revision);
        const auto reversed = window.grabWindow();
        const double dpr = normal.devicePixelRatio();
        int colored = 0, missed = 0;
        // Mirror data pixels across both axes; allow one physical pixel for raster edge rounding.
        // The color legend for heat maps is an overlay, so ignore its lower strip.
        for (int py = qCeil((plot.top() + 4) * dpr); py < (plot.bottom() - 30) * dpr; ++py)
            for (int px = qCeil((plot.left() + 4) * dpr); px < (plot.right() - 4) * dpr; ++px)
            {
                const auto color = normal.pixelColor(px, py);
                if (color.red() < 80 || color.red() < color.green() * 1.3)
                    continue;
                const int mx = qRound((plot.left() + plot.right()) * dpr - px - 1);
                const int my = qRound((plot.top() + plot.bottom()) * dpr - py - 1);
                bool found = false;
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        const auto candidate = reversed.pixelColor(mx + dx, my + dy);
                        found |= candidate.red() > 80 && candidate.red() > candidate.green() * 1.3;
                    }
                ++colored;
                missed += !found;
            }
        QVERIFY(colored > 20);
        QVERIFY2(missed < colored * .02,
                 qPrintable(QString("%1 / %2 mirrored pixels missing").arg(missed).arg(colored)));
        if (type == ChartSeries::Line || type == ChartSeries::Area ||
            type == ChartSeries::Scatter || type == ChartSeries::Bubble)
            QCOMPARE(chart.dataAt(chart.mapToPosition({3, 7})).value("index").toInt(), 1);
        if (type == ChartSeries::RangeArea)
        {
            QVERIFY(!normalBandHit.isEmpty());
            QCOMPARE(chart.dataAt(chart.mapToPosition({3.25, 4})), normalBandHit);
        }
    }
    void reversedPlotBottomBaseline()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Area);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        chart.yAxis()->setReversed(true);
        ChartSeries series;
        series.setColor(Qt::red);
        series.setFillOpacity(1);
        series.setAreaBaselineMode(ChartSeries::PlotBottomBaseline);
        series.replacePoints({{1, 3}, {9, 3}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto result = window.grabWindow();
        QVERIFY(pixel(result, chart.mapToPosition({5, 8})).red() > 200);
        QCOMPARE(pixel(result, chart.mapToPosition({5, 1})), QColor(Qt::black));
    }
    void minorGridRenders()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setLegendVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        chart.xAxis()->setTickCount(3);
        chart.yAxis()->setTickCount(3);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto baseline = window.grabWindow();
        auto revision = stat(chart, "preparedRevision");
        chart.xAxis()->setMinorTickCount(1);
        chart.yAxis()->setMinorTickCount(1);
        QTRY_VERIFY(areaReady(chart) && stat(chart, "preparedRevision") > revision);
        const auto divided = window.grabWindow();
        const auto center = chart.mapToPosition({2.5, 3.5});
        QVERIFY(pixel(divided, center).red() > pixel(baseline, center).red() + 10);
        QVERIFY(differences(baseline, divided, chart.plotRect().adjusted(4, 4, -4, -4)) > 100);
        revision = stat(chart, "preparedRevision");
        chart.setGridVisible(false);
        QTRY_VERIFY(areaReady(chart) && stat(chart, "preparedRevision") > revision);
        QCOMPARE(pixel(window.grabWindow(), center), QColor(Qt::black));
        revision = stat(chart, "preparedRevision");
        chart.setGridVisible(true);
        chart.xAxis()->setScale(ChartAxis::Category);
        chart.yAxis()->setScale(ChartAxis::Category);
        QTRY_VERIFY(areaReady(chart) && stat(chart, "preparedRevision") > revision);
        QCOMPARE(pixel(window.grabWindow(), center), QColor(Qt::black));
    }
    void touchNavigationRenders_data()
    {
        QTest::addColumn<int>("mask");
        for (int mask = 0; mask < 4; ++mask)
            QTest::newRow(qPrintable(QString::number(mask))) << mask;
    }
    void touchNavigationRenders()
    {
        QFETCH(int, mask);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Scatter);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        chart.xAxis()->setReversed(mask & 1);
        chart.yAxis()->setReversed(mask & 2);
        ChartSeries s;
        s.setColor(Qt::red);
        s.setMarkerSize(16);
        s.replacePoints({{5, 5}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto before = window.grabWindow();
        const auto revision = stat(chart, "preparedRevision");
        const auto uploads = stat(chart, "vertexUploads");
        std::unique_ptr<QPointingDevice> device(QTest::createTouchDevice());
        auto touch = QTest::touchEvent(&window, device.get(), false);
        const QPoint a(260, 220), b(380, 220);
        touch.press(0, a).press(1, b).commit();
        QTest::qWait(30);
        QCOMPARE(stat(chart, "preparedRevision"), revision);
        QCOMPARE(stat(chart, "vertexUploads"), uploads);
        const auto anchor = chart.mapToValue({320, 220});
        touch.move(0, {240, 240}).move(1, {440, 240}).commit();
        QTRY_VERIFY(chart.xAxis()->maximum() - chart.xAxis()->minimum() < 7);
        QTRY_VERIFY(areaReady(chart) && stat(chart, "preparedRevision") > revision);
        const auto anchored = chart.mapToPosition(anchor);
        QVERIFY(QLineF(anchored, QPointF(340, 240)).length() < .01);
        const auto after = window.grabWindow();
        QVERIFY(differences(before, after, chart.plotRect()) > 100);
        const auto color = pixel(after, chart.mapToPosition({5, 5}));
        QVERIFY(color.red() > 200 && color.green() < 40);
        touch.release(0, {240, 240}).release(1, {440, 240}).commit();
        QCOMPARE(s.sampleAt(0).x, 5.);
        QCOMPARE(s.sampleAt(0).y, 5.);
        QVERIFY(!chart.keepTouchGrab());
    }
    void rectangleSelectionOverlay_data()
    {
        QTest::addColumn<int>("axes");
        for (int axes = 0; axes < 3; ++axes)
            QTest::newRow(qPrintable(QString::number(axes))) << axes;
    }
    void rectangleSelectionOverlay()
    {
        QFETCH(int, axes);
        QQuickWindow window;
        window.resize(640, 480);
        InteractiveChart chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Area);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setDragMode(ChartItem::RectangleZoomDrag);
        chart.setRectangleZoomAxes(ChartItem::ZoomAxes(axes));
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        ChartSeries s;
        s.setColor(Qt::red);
        s.setFillOpacity(1);
        s.replacePoints({{0, 8}, {10, 8}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto before = window.grabWindow();
        const auto plot = chart.plotRect();
        const auto revision = stat(chart, "preparedRevision"),
                   uploads = stat(chart, "vertexUploads");
        const QPointF a(plot.left() + plot.width() * .2, plot.top() + plot.height() * .3);
        const QPointF b(plot.left() + plot.width() * .7, plot.top() + plot.height() * .8);
        QMouseEvent press(QEvent::MouseButtonPress, a, a, Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        QMouseEvent move(QEvent::MouseMove, b, b, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        chart.mousePressEvent(&press);
        chart.mouseMoveEvent(&move);
        QVERIFY(chart.zoomSelecting());
        QTest::qWait(60);
        const auto selected = window.grabWindow();
        QVERIFY(pixel(selected, chart.zoomSelection().center()).blue() >
                pixel(before, chart.zoomSelection().center()).blue() + 20);
        QCOMPARE(stat(chart, "preparedRevision"), revision);
        QCOMPARE(stat(chart, "vertexUploads"), uploads);
        const auto outside = axes == 2 ? QPointF(plot.left() + plot.width() / 2, plot.top() + 8)
                                       : QPointF(plot.left() + 8, plot.top() + plot.height() / 2);
        QCOMPARE(pixel(selected, outside), pixel(before, outside));
        chart.cancelZoomSelection();
        QTest::qWait(60);
        QCOMPARE(differences(before, window.grabWindow(), plot.adjusted(2, 2, -2, -2)), 0);
        QCOMPARE(stat(chart, "vertexUploads"), uploads);
        chart.mousePressEvent(&press);
        chart.mouseMoveEvent(&move);
        QMouseEvent release(QEvent::MouseButtonRelease, b, b, Qt::LeftButton, Qt::NoButton,
                            Qt::NoModifier);
        chart.mouseReleaseEvent(&release);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QVERIFY(!chart.zoomSelecting());
        if (axes != 2)
            QVERIFY(chart.xAxis()->minimum() > 0);
        if (axes != 1)
            QVERIFY(chart.yAxis()->maximum() < 10);
    }
    void areaFillStyles_data()
    {
        QTest::addColumn<int>("type");
        QTest::addColumn<int>("style");
        for (int type : {int(ChartSeries::Area), int(ChartSeries::RangeArea)})
            for (int style = 0; style < 3; ++style)
                QTest::newRow(qPrintable(QString("%1-%2").arg(type).arg(style))) << type << style;
    }
    void areaFillStyles()
    {
        QFETCH(int, type);
        QFETCH(int, style);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries s;
        s.setColor(Qt::green);
        s.setBorderColor(Qt::transparent);
        s.setFillColor(QColor::fromRgbF(1, 0, 0, .8));
        s.setFillEndColor(QColor::fromRgbF(0, 0, 1, .2));
        s.setFillOpacity(.5);
        s.setFillStyle(ChartSeries::FillStyle(style));
        s.setAreaBaselineMode(ChartSeries::CustomBaseline);
        s.setAreaBaseline(-10);
        if (type == ChartSeries::RangeArea)
            s.replaceRangeSamples({{-5, -10, 20}, {15, -10, 20}});
        else
            s.replacePoints({{-5, 20}, {15, 20}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto check = [&](const QImage &frame, bool transparentEnd)
        {
            const auto rect = chart.plotRect();
            for (double fraction : {.2, .5, .8})
            {
                const QPointF point(rect.left() + fraction * rect.width(),
                                    rect.top() + (1 - fraction) * rect.height());
                const auto color = pixel(frame, point);
                const double t = style == 0 ? 0 : style == 1 ? 1 - fraction : fraction;
                QVERIFY2(qAbs(color.red() - qRound(102 * (1 - t))) <= 3, qPrintable(color.name()));
                QVERIFY(color.green() <= 2);
                QVERIFY(qAbs(color.blue() - qRound(transparentEnd ? 0 : 25.5 * t)) <= 3);
            }
        };
        check(window.grabWindow(), false);
        const auto revision = stat(chart, "preparedRevision");
        s.setFillEndColor(QColor::fromRgbF(0, 1, 0, 0));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        check(window.grabWindow(), true);
    }
    void areaCrossingBaselineAndGaps()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Area);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, -10, 10);
        ChartSeries s;
        s.setColor(Qt::green);
        s.setFillColor(Qt::red);
        s.setFillOpacity(.5);
        s.setBorderColor(Qt::white);
        s.setLineWidth(6);
        s.replacePoints({{1, 8}, {9, -8}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        auto frame = window.grabWindow();
        const auto at = [&](double x, double y) { return chart.mapToPosition({x, y}); };
        for (auto p : {QPointF(2, 2), QPointF(8, -2)})
            QVERIFY(qAbs(pixel(frame, chart.mapToPosition(p)).red() - 128) <= 2);
        QCOMPARE(pixel(frame, at(8, 2)), QColor(Qt::black));
        QVERIFY(pixel(frame, at(2, 6)).green() > 240); // Independent opaque boundary.
        auto revision = stat(chart, "preparedRevision");
        s.setAreaBaselineMode(ChartSeries::CustomBaseline);
        s.setAreaBaseline(5);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        QVERIFY(pixel(frame, at(8, 4)).red() > 120);
        QCOMPARE(pixel(frame, at(2, 2)), QColor(Qt::black));
        revision = stat(chart, "preparedRevision");
        s.setBreakBefore(1, true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(pixel(window.grabWindow(), at(8, 4)), QColor(Qt::black));
        revision = stat(chart, "preparedRevision");
        s.setBreakBefore(1, false);
        s.setAreaBaselineMode(ChartSeries::PlotBottomBaseline);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QVERIFY(pixel(window.grabWindow(), at(8, -9)).red() > 120);
        revision = stat(chart, "preparedRevision");
        s.setAreaBaselineMode(ChartSeries::CustomBaseline);
        s.setAreaBaseline(100);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        QVERIFY(pixel(frame, at(8, 9)).red() > 120);
        QCOMPARE(pixel(frame, at(8, -9)), QColor(Qt::black));
        // Transparent fill does not suppress the independent boundary.
        revision = stat(chart, "preparedRevision");
        s.setFillOpacity(0);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        QCOMPARE(pixel(frame, at(8, 9)), QColor(Qt::black));
        QVERIFY(pixel(frame, at(2, 6)).green() > 240);
    }
    void areaOverlapAndMarkers()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Area);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries a, b;
        for (auto *s : {&a, &b})
        {
            s->replacePoints({{2, 8}, {8, 8}});
            s->setFillOpacity(.5);
            s->setBorderColor(Qt::white);
            s->setLineWidth(4);
            chart.addSeries(s);
        }
        a.setColor(Qt::green);
        a.setFillColor(Qt::red);
        a.setMarkersVisible(true);
        a.setMarkerSize(16);
        b.setFillColor(Qt::blue);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto center = chart.mapToPosition({5, 4});
        auto color = pixel(window.grabWindow(), center);
        QVERIFY(qAbs(color.red() - 64) <= 2 && qAbs(color.blue() - 128) <= 2 && color.green() <= 2);
        const auto revision = stat(chart, "preparedRevision");
        b.setVisible(false);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        auto frame = window.grabWindow();
        QCOMPARE(pixel(frame, chart.mapToPosition({2, 8})), QColor(Qt::green));
        QVERIFY(pixel(frame, chart.mapToPosition({5, 8})).green() > 240);
        QVERIFY(qAbs(pixel(frame, center).red() - 128) <= 2);
    }
    void styledBars_data()
    {
        QTest::addColumn<int>("type");
        for (auto type : {ChartSeries::Bar, ChartSeries::StackedBar, ChartSeries::PercentBar,
                          ChartSeries::HorizontalBar, ChartSeries::HorizontalStackedBar,
                          ChartSeries::HorizontalPercentBar})
            QTest::newRow(qPrintable(QString::number(type))) << int(type);
    }
    void styledBars()
    {
        QFETCH(int, type);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setBarSpacing(.25);
        const bool horizontal = type >= ChartSeries::HorizontalBar;
        const bool percent =
            type == ChartSeries::PercentBar || type == ChartSeries::HorizontalPercentBar;
        const double top = percent ? 100 : 8;
        if (horizontal)
            chart.setViewRange(-2, top + 2, -1, 1);
        else
            chart.setViewRange(-1, 1, -2, top + 2);
        const auto at = [&](double category, double value) {
            return chart.mapToPosition(horizontal ? QPointF(value, category)
                                                  : QPointF(category, value));
        };
        ChartSeries s;
        s.setColor(Qt::red);
        s.setBarBorderColor(Qt::white);
        s.setBarBorderWidth(4);
        s.append(0, 8);
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        auto frame = window.grabWindow();
        QCOMPARE(pixel(frame, at(0, top / 2)), QColor(Qt::red));
        const auto edge = at(-.3, top / 2) + (horizontal ? QPointF(0, -2) : QPointF(2, 0));
        QVERIFY(pixel(frame, edge).green() > 200);
        QCOMPARE(pixel(frame, at(.35, top / 2)), QColor(Qt::black));
        QVERIFY(chart.dataAt(at(.35, top / 2)).isEmpty());
        auto revision = stat(chart, "preparedRevision");
        chart.setBarGroupWidth(.4);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        frame = window.grabWindow();
        QCOMPARE(pixel(frame, at(.2, top / 2)), QColor(Qt::black));
        revision = stat(chart, "preparedRevision");
        s.clear();
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(pixel(window.grabWindow(), at(0, top / 2)), QColor(Qt::black));
    }
    void barLabelPositions_data()
    {
        QTest::addColumn<bool>("horizontal");
        QTest::addColumn<int>("position");
        QTest::addColumn<bool>("reversed");
        for (int horizontal = 0; horizontal < 2; ++horizontal)
            for (int position = 0; position < 4; ++position)
                for (int reversed = 0; reversed < 2; ++reversed)
                    QTest::newRow(
                        qPrintable(QString("%1-%2-%3").arg(horizontal).arg(position).arg(reversed)))
                        << bool(horizontal) << position << bool(reversed);
    }
    void barLabelPositions()
    {
        QFETCH(bool, horizontal);
        QFETCH(int, position);
        QFETCH(bool, reversed);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(horizontal ? ChartSeries::HorizontalBar : ChartSeries::Bar);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setTextColor(Qt::white);
        chart.xAxis()->setReversed(reversed);
        chart.yAxis()->setReversed(reversed);
        if (horizontal)
            chart.setViewRange(-12, 12, -1, 2);
        else
            chart.setViewRange(-1, 2, -12, 12);
        ChartSeries s;
        s.setColor(Qt::red);
        s.replacePoints({{0, 8}, {1, -8}});
        s.setBarLabelFormat("88");
        s.setBarLabelMargin(5);
        s.setBarLabelPosition(ChartSeries::BarLabelPosition(position));
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        const auto plain = window.grabWindow();
        auto revision = stat(chart, "preparedRevision");
        s.setBarLabelsVisible(true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        const auto labeled = window.grabWindow();
        for (int index = 0; index < 2; ++index)
        {
            const double value = index == 0 ? 8 : -8;
            const auto point = [&](double v)
            { return chart.mapToPosition(horizontal ? QPointF(v, index) : QPointF(index, v)); };
            const auto base = point(0), end = point(value);
            const QPointF direction = (end - base) / QLineF(base, end).length();
            const double half = horizontal ? 12 : 11;
            QPointF center = (base + end) / 2;
            if (position == ChartSeries::BarLabelInsideEnd)
                center = end - direction * (5 + half);
            if (position == ChartSeries::BarLabelInsideBase)
                center = base + direction * (5 + half);
            if (position == ChartSeries::BarLabelOutsideEnd)
                center = end + direction * (5 + half);
            QVERIFY(differences(plain, labeled, QRectF(center - QPointF(22, 18), QSizeF(44, 36))) >
                    12);
        }
        revision = stat(chart, "preparedRevision");
        s.setBarLabelsVisible(false);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(differences(plain, window.grabWindow(), chart.plotRect()), 0);
    }
    void barLabelFormatAndClippedBorder()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(ChartSeries::PercentBar);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(-1, 1, 0, 120);
        ChartSeries s;
        s.setName("Group");
        s.setColor(Qt::red);
        s.setBarLabelsVisible(true);
        s.setBarLabelPrecision(3);
        s.setBarLabelFormat("@value / @percentage / @category / @series");
        s.replaceSamples({{0, 5.1234, 1, "Label"}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        const auto formatted = window.grabWindow();
        auto revision = stat(chart, "preparedRevision");
        s.setBarLabelFormat("5.12 / 100 / Label / Group");
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(differences(formatted, window.grabWindow(), chart.plotRect()), 0);
        revision = stat(chart, "preparedRevision");
        s.setBarLabelsVisible(false);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QVERIFY(differences(formatted, window.grabWindow(), chart.plotRect()) > 30);
        revision = stat(chart, "preparedRevision");
        chart.setChartType(ChartSeries::Bar);
        chart.setViewRange(-1, 1, 0, 5);
        s.replacePoints({{0, 10}});
        s.setBarBorderColor(Qt::white);
        s.setBarBorderWidth(6);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        const auto clipped = window.grabWindow();
        QCOMPARE(pixel(clipped, chart.mapToPosition({0, 5}) + QPointF(0, 3)), QColor(Qt::red));
        QVERIFY(chart.renderingStatistics().value("error").toString().isEmpty());
    }
    void outsideSliceLabels_data()
    {
        QTest::addColumn<int>("rings");
        QTest::newRow("pie") << 0;
        QTest::newRow("doughnut") << 1;
        QTest::newRow("concentric") << 2;
    }
    void outsideSliceLabels()
    {
        QFETCH(int, rings);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setChartType(rings ? ChartSeries::Doughnut : ChartSeries::Pie);
        chart.setMultiRing(rings == 2);
        chart.setHoleSize(.2);
        chart.setLegendVisible(false);
        chart.setBackgroundColor(Qt::black);
        chart.setTextColor(Qt::white);
        chart.setAcceptHoverEvents(false);
        QFont font;
        font.setPixelSize(14);
        chart.setFont(font);
        chart.setPieAngles(0, 180);
        ChartSeries a, b;
        a.setName("Outer");
        b.setName("Inner");
        a.setData({QVariantMap{{"x", 0}, {"y", 1}, {"label", "A very long slice name"}}});
        b.setData({QVariantMap{{"x", 0}, {"y", 2}, {"label", "Second"}}});
        a.setSliceStyle(0, {{"color", QColor(Qt::red)}, {"explodeDistance", 12}});
        b.setSliceStyle(0, {{"color", QColor(Qt::red)}});
        chart.addSeries(&a);
        if (rings == 2)
            chart.addSeries(&b);
        chart.setSliceLabelPosition(ChartItem::SliceLabelOutside);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const auto plot = chart.plotRect();
        const QRectF column(plot.right() - 138, plot.top(), 138, plot.height());
        const auto whitePixels = [&](const QImage &image)
        {
            int n = 0;
            for (int y = qCeil(column.top()); y < column.bottom(); ++y)
                for (int x = qCeil(column.left()); x < column.right(); ++x)
                {
                    const QColor color = pixel(image, {qreal(x), qreal(y)});
                    n += color.red() > 150 && color.green() > 150 && color.blue() > 150;
                }
            return n;
        };
        const auto labeled = window.grabWindow();
        QVERIFY(whitePixels(labeled) > 20);
        QVERIFY(chart.dataAt(column.center()).isEmpty());
        const auto revision = stat(chart, "preparedRevision");
        a.setSliceStyle(
            0, {{"color", QColor(Qt::red)}, {"explodeDistance", 12}, {"labelVisible", false}});
        b.setSliceStyle(0, {{"color", QColor(Qt::red)}, {"labelVisible", false}});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto hidden = window.grabWindow();
        QCOMPARE(whitePixels(hidden), 0);
        // The only changed pixels between the ring and column are the leader lines.
        QVERIFY(differences(labeled, hidden, QRectF(458, plot.top(), 22, plot.height())) > 5);
        QCOMPARE(chart.renderedPointCount(), rings == 2 ? 2 : 1);
        a.clear();
        b.clear();
        QTRY_VERIFY(chart.renderedPointCount() == 0 && areaReady(chart));
        QCOMPARE(whitePixels(window.grabWindow()), 0);
        a.append(0, 1);
        a.setSliceStyle(0, {{"color", QColor(Qt::red)}});
        chart.setSliceLabelPosition(ChartItem::SliceLabelInside);
        QTRY_VERIFY(chart.renderedPointCount() == 1 && areaReady(chart));
        const auto inside = window.grabWindow();
        QCOMPARE(whitePixels(inside), 0);
        chart.setSliceLabelPosition(ChartItem::SliceLabelOutside);
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(whitePixels(window.grabWindow()) > 10);
    }
    void radialGeometryAndPicking()
    {
        QQuickWindow window;
        window.resize(600, 500);
        ChartItem chart(window.contentItem());
        chart.setSize({600, 500});
        chart.setChartType(ChartSeries::Doughnut);
        chart.setLegendVisible(false);
        chart.setSliceLabelsVisible(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAcceptHoverEvents(false);
        chart.setMultiRing(true);
        chart.setHoleSize(.2);
        chart.setRingGap(12);
        QVERIFY(chart.setPieAngles(0, 180));
        ChartSeries outer, inner;
        outer.replacePoints({{0, 1}});
        inner.replacePoints({{0, 1}});
        outer.setSliceStyle(
            0,
            {{"color", QColor(Qt::red)}, {"borderColor", QColor(Qt::white)}, {"borderWidth", 4.}});
        inner.setSliceStyle(0, {{"color", QColor(Qt::green)}});
        chart.addSeries(&outer);
        chart.addSeries(&inner);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        auto frame = window.grabWindow();
        const auto c = chart.plotRect().center();
        const double r = chart.plotRect().height() * .42;
        QCOMPARE(pixel(frame, c + QPointF(.8 * r, 0)), QColor(Qt::red));
        QCOMPARE(pixel(frame, c + QPointF(.4 * r, 0)), QColor(Qt::green));
        QCOMPARE(pixel(frame, c + QPointF(.6 * r, 0)), QColor(Qt::black));
        QCOMPARE(pixel(frame, c), QColor(Qt::black));
        QCOMPARE(pixel(frame, c - QPointF(.8 * r, 0)), QColor(Qt::black));
        QVERIFY(pixel(frame, c + QPointF(r - 2, 0)).green() > 180);
        QCOMPARE(chart.dataAt(c + QPointF(.4 * r, 0)).value("seriesIndex").toInt(), 1);
        auto revision = stat(chart, "preparedRevision");
        inner.clear();
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        frame = window.grabWindow();
        QCOMPARE(pixel(frame, c + QPointF(.4 * r, 0)), QColor(Qt::red));
        revision = stat(chart, "preparedRevision");
        chart.setChartType(ChartSeries::Pie);
        outer.replacePoints({{0, 1}, {1, 1}});
        chart.setPieAngles(0, 360);
        outer.setSliceStyle(0, {{"color", QColor(Qt::red)}, {"explodeDistance", 30.}});
        outer.setSliceStyle(1, {{"color", QColor(Qt::blue)}});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        frame = window.grabWindow();
        QCOMPARE(pixel(frame, c + QPointF(15, 0)), QColor(Qt::black));
        QCOMPARE(pixel(frame, c + QPointF(100, 0)), QColor(Qt::red));
        QCOMPARE(pixel(frame, c - QPointF(100, 0)), QColor(Qt::blue));
        QVERIFY(chart.dataAt(c + QPointF(15, 0)).isEmpty());
        revision = stat(chart, "preparedRevision");
        outer.clear();
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        frame = window.grabWindow();
        QCOMPARE(pixel(frame, c + QPointF(100, 0)), QColor(Qt::black));
        QVERIFY(chart.renderingStatistics().value("error").toString().isEmpty());
    }
    void allTypes_data()
    {
        QTest::addColumn<int>("type");
        for (int type = ChartSeries::Line; type <= ChartSeries::RangeArea; ++type)
            QTest::newRow(qPrintable(QString::number(type))) << type;
    }
    void allTypes()
    {
        QFETCH(int, type);
        QQuickWindow window;
        window.resize(600, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({600, 400});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        ChartSeries series;
        series.setColor(Qt::red);
        QVERIFY(series.replaceSamples(
            {{0, 2, 10, {}}, {1, 5, 18, {}}, {2, 3, 24, {}}, {3, 7, 32, {}}}));
        if (type == ChartSeries::RangeArea)
            series.replaceRangeSamples({{0, 0, 2, {}, false},
                                        {1, 1, 5, {}, false},
                                        {2, 0, 3, {}, false},
                                        {3, 1, 7, {}, false}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.rendererInterface()->graphicsApi() != QSGRendererInterface::Software);
        QTRY_VERIFY(current(chart));
        QCOMPARE(chart.renderingStatistics().value("backend").toString(), QStringLiteral("rhi"));
        QVERIFY2(chart.renderingStatistics().value("error").toString().isEmpty(),
                 qPrintable(chart.renderingStatistics().value("error").toString()));
        QVERIFY(stat(chart, "vertexUploads") + stat(chart, "instanceUploads") > 0);
        const auto visible = window.grabWindow();
        QVERIFY(!visible.isNull());
        const QRectF plot = chart.plotRect();
        const auto revision = stat(chart, "preparedRevision");
        series.clear();
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        const auto empty = window.grabWindow();
        QVERIFY2(differences(visible, empty, plot.adjusted(5, 5, -5, -5)) > 50,
                 "Clearing a series must remove its visible GPU geometry.");
    }
    void rangeCurvePixels_data()
    {
        QTest::addColumn<int>("mode");
        for (int mode = 0; mode <= ChartSeries::NaturalSpline; ++mode)
            QTest::newRow(qPrintable(QString::number(mode))) << mode;
    }
    void rangeCurvePixels()
    {
        QFETCH(int, mode);
        QQuickWindow window;
        window.resize(800, 600);
        ChartItem chart(window.contentItem());
        chart.setSize({800, 600});
        chart.setChartType(ChartSeries::RangeArea);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(-.5, 3.5, -1, 4);
        ChartSeries s;
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.setFillColor(Qt::red);
        s.setFillOpacity(.5);
        s.setBorderColor(Qt::white);
        s.setLineWidth(4);
        // One straight boundary and one curved boundary require shared subdivision.
        s.replaceRangeSamples({{0, 2, 3}, {1, 0, 3}, {2, 0, 3}, {3, 2, 3}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        const double lower = mode == ChartSeries::NaturalSpline      ? -.3
                             : mode == ChartSeries::CatmullRomSpline ? -.25
                                                                     : 0.;
        auto frame = window.grabWindow();
        QVERIFY(pixel(frame, chart.mapToPosition({1.5, lower})).green() > 180);
        QVERIFY(pixel(frame, chart.mapToPosition({1.5, 3})).green() > 180);
        QVERIFY(pixel(frame, chart.mapToPosition({1.5, lower - .25})).red() < 10);
        // No cracks or double alpha along the triangulated strip.
        for (double x = .8; x < 2.8; x += .13)
        {
            const auto color = pixel(frame, chart.mapToPosition({x, 2.5}));
            QVERIFY(color.red() > 110 && color.red() < 145);
            QVERIFY(color.green() < 10);
        }
        auto revision = stat(chart, "preparedRevision");
        s.setStrokeStyle(ChartSeries::DashLine);
        s.setDashPattern({18, 18});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        int ink = 0, gaps = 0;
        for (double x = .5; x < 2.5; x += .025)
        {
            const auto c = pixel(frame, chart.mapToPosition({x, 3}));
            ink += c.green() > 180;
            gaps += c.green() < 20;
        }
        QVERIFY(ink > 10 && gaps > 10);
        revision = stat(chart, "preparedRevision");
        s.setStrokeStyle(ChartSeries::SolidLine);
        s.setBreakBefore(2, true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        QVERIFY(pixel(frame, chart.mapToPosition({1.5, 2})).red() < 10);
        QVERIFY(pixel(frame, chart.mapToPosition({1.5, 3})).red() < 10);
        revision = stat(chart, "preparedRevision");
        s.replaceRangeSamples({{0, -2, 2}, {3, 2, -2}});
        chart.setViewRange(-.5, 3.5, -3, 3);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        for (double x : {.5, 2.5})
        {
            const auto color = pixel(frame, chart.mapToPosition({x, 0}));
            QVERIFY(color.red() > 110 && color.red() < 145);
        }
        if (mode == 0 || mode >= ChartSeries::MonotoneSpline)
            QVERIFY(pixel(frame, chart.mapToPosition({1.5, 1})).red() < 10);
        revision = stat(chart, "preparedRevision");
        s.replaceRangeSamples({{0, 0, -1}, {1, 0, 1}, {2, 0, -1}, {3, 0, 1}});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        const double leftY = mode == ChartSeries::StepLeft ? -.2 : .2;
        const double rightY = mode == ChartSeries::StepRight ? .2 : -.2;
        for (QPointF p : {QPointF(1.25, leftY), QPointF(1.75, rightY)})
        {
            const auto c = pixel(frame, chart.mapToPosition(p));
            QVERIFY(c.red() > 110 && c.red() < 145 && c.green() < 10);
        }
        if (mode == 0 || mode >= ChartSeries::MonotoneSpline)
            QVERIFY(pixel(frame, chart.mapToPosition({1.5, .4})).red() < 10);
        revision = stat(chart, "preparedRevision");
        s.replaceRangeSamples({{0, -2, 2}, {3, 2, -2}});
        s.setFillStyle(ChartSeries::HorizontalGradient);
        s.setFillEndColor(Qt::blue);
        chart.setViewRange(.2, 2.8, -.7, .7);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        frame = window.grabWindow();
        const auto left = pixel(frame, chart.mapToPosition({.4, 0}));
        const auto right = pixel(frame, chart.mapToPosition({2.6, 0}));
        QVERIFY(left.red() > left.blue());
        QVERIFY(right.blue() > right.red());
        QVERIFY(pixel(frame, chart.plotRect().topLeft() - QPointF(8, 8)).red() < 10);
        revision = stat(chart, "preparedRevision");
        s.clear();
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QVERIFY(pixel(window.grabWindow(), chart.plotRect().center()).red() < 10);
    }
    void rangeSplineOffscreenNodes_data()
    {
        QTest::addColumn<int>("mode");
        QTest::newRow("natural") << int(ChartSeries::NaturalSpline);
        QTest::newRow("catmull") << int(ChartSeries::CatmullRomSpline);
    }
    void rangeSplineOffscreenNodes()
    {
        QFETCH(int, mode);
        QQuickWindow window;
        window.resize(640, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 400});
        chart.setChartType(ChartSeries::RangeArea);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(-.4, -.1, -.5, 2.5);
        ChartSeries s;
        s.setColor(Qt::red);
        s.setFillOpacity(1);
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replaceRangeSamples({{0, 0, 2}, {0, 0, 2}, {0, 0, 2}, {10, 0, 2}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(pixel(window.grabWindow(), chart.mapToPosition({-.2, 1})).red() > 240);
    }
    void lodSplineOffscreenControls_data()
    {
        QTest::addColumn<int>("mode");
        QTest::newRow("catmull") << int(ChartSeries::CatmullRomSpline);
        QTest::newRow("natural") << int(ChartSeries::NaturalSpline);
    }
    void lodSplineOffscreenControls()
    {
        QFETCH(int, mode);
        QQuickWindow window;
        window.resize(640, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 400});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setDisplayBudget(8);
        chart.setViewRange(-.4, -.1, 0, 2);
        ChartSeries s;
        s.setColor(Qt::red);
        s.setLineWidth(4);
        s.setCurveMode(ChartSeries::CurveMode(mode));
        s.replacePoints({{0, 1},
                         {0, 1},
                         {0, 1},
                         {10, 1},
                         {20, 1},
                         {30, 1},
                         {40, 1},
                         {50, 1},
                         {60, 1},
                         {70, 1}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(stat(chart, "curveLodNodesVisited") > 0);
        const auto at = chart.mapToPosition({-.2, 1});
        QVERIFY(pixel(window.grabWindow(), at).red() > 200);
        QVERIFY(!chart.dataAt(at).isEmpty());
    }
    void denseCurveLod_data()
    {
        QTest::addColumn<int>("mode");
        QTest::addColumn<int>("type");
        for (int mode = ChartSeries::LinearCurve; mode <= ChartSeries::NaturalSpline; ++mode)
            for (int type : {int(ChartSeries::Line), int(ChartSeries::Area)})
                QTest::newRow(qPrintable(QString("%1-%2").arg(mode).arg(type))) << mode << type;
    }
    void denseCurveLod()
    {
        QFETCH(int, mode);
        QFETCH(int, type);
        QQuickWindow window;
        window.resize(800, 500);
        ChartItem chart(window.contentItem());
        chart.setSize({800, 500});
        chart.setChartType(ChartSeries::Type(type));
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        chart.setBackgroundColor(Qt::black);
        chart.setViewRange(0, 25000, -1, 10);
        ChartSeries s;
        s.setCapacity(20000);
        s.setColor(Qt::red);
        s.setFillOpacity(1);
        s.setLineWidth(3);
        s.setCurveMode(ChartSeries::CurveMode(mode));
        QVector<QPointF> points;
        for (int i = 0; i < 20000; ++i)
            points.append(
                {double(i < 10000 ? i : i + 5000), i == 1234 ? 8. : 2. + std::sin(i * .02)});
        s.replacePoints(points);
        s.setBreakBefore(10000, true);
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(areaReady(chart));
        QVERIFY(chart.renderedPointCount() < 12000);
        const auto overview = window.grabWindow();
        const auto peak = chart.mapToPosition({1234, 8});
        bool red = false;
        for (int dy = -2; dy <= 2; ++dy)
            for (int dx = -2; dx <= 2; ++dx)
                red = red || pixel(overview, peak + QPointF(dx, dy)).red() > 140;
        QVERIFY(red); // The isolated peak survives overview LOD.
        QVERIFY(pixel(overview, chart.mapToPosition({12500, 2})).red() < 30);
        const auto builds = stat(chart, "curveCacheBuilds");
        auto revision = stat(chart, "preparedRevision");
        chart.panBy({15, 0});
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(stat(chart, "curveCacheBuilds"), builds);
        QVERIFY(stat(chart, "curveLodNodesVisited") < 5000);
        revision = stat(chart, "preparedRevision");
        chart.setViewRange(1230, 1238, -1, 10);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        const auto detailed = window.grabWindow();
        revision = stat(chart, "preparedRevision");
        chart.setDisplayBudget(0);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && areaReady(chart));
        QCOMPARE(
            differences(detailed, window.grabWindow(), chart.plotRect().adjusted(5, 5, -5, -5)), 0);
        QCOMPARE(s.count(), 20000);
        QCOMPARE(s.sampleAt(1234).y, 8.);
    }
    void curveVariants_data()
    {
        QTest::addColumn<int>("mode");
        QTest::addColumn<QPointF>("expected");
        QTest::newRow("step-left") << int(ChartSeries::StepLeft) << QPointF(.5, 1);
        QTest::newRow("step-right") << int(ChartSeries::StepRight) << QPointF(.5, 0);
        QTest::newRow("step-center") << int(ChartSeries::StepCenter) << QPointF(.75, 1);
        QTest::newRow("natural") << int(ChartSeries::NaturalSpline) << QPointF(1.5, 1.15);
        QTest::newRow("monotone") << int(ChartSeries::MonotoneSpline) << QPointF(1.5, 1);
        QTest::newRow("catmull-rom") << int(ChartSeries::CatmullRomSpline) << QPointF(1.5, 1.125);
    }
    void curveVariants()
    {
        QFETCH(int, mode);
        QFETCH(QPointF, expected);
        QQuickWindow window;
        window.resize(640, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 400});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAcceptHoverEvents(false);
        QVERIFY(chart.setViewRange(-.5, 3.5, -.5, 1.5));
        ChartSeries series;
        series.setColor(Qt::red);
        series.setLineWidth(6);
        series.setCurveMode(ChartSeries::CurveMode(mode));
        series.replacePoints({{0, 0}, {1, 1}, {2, 1}, {3, 0}});
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        auto frame = window.grabWindow();
        QVERIFY(pixel(frame, chart.mapToPosition(expected)).red() > 180);
        const auto revision = stat(chart, "preparedRevision");
        chart.setChartType(ChartSeries::Area);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        frame = window.grabWindow();
        QVERIFY(pixel(frame, chart.mapToPosition({1.5, .5})).red() > 40);
        QVERIFY(chart.renderingStatistics().value("error").toString().isEmpty());
    }
    void markerVariants_data()
    {
        QTest::addColumn<int>("shape");
        for (int shape = ChartSeries::Circle; shape <= ChartSeries::Diamond; ++shape)
            QTest::newRow(qPrintable(QString::number(shape))) << shape;
    }
    void markerVariants()
    {
        QFETCH(int, shape);
        QQuickWindow window;
        window.resize(640, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 400});
        chart.setChartType(ChartSeries::Scatter);
        chart.setBackgroundColor(Qt::black);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setAcceptHoverEvents(false);
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        ChartSeries series;
        series.setColor(Qt::red);
        series.setMarkerShape(ChartSeries::MarkerShape(shape));
        series.setMarkerSize(40);
        series.append(5, 5);
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        const auto center = chart.mapToPosition({5, 5});
        const auto frame = window.grabWindow();
        QVERIFY(pixel(frame, center).red() > 245);
        QCOMPARE(pixel(frame, center + QPointF(16, -16)).red() > 180, shape == ChartSeries::Square);
        QVERIFY(pixel(frame, center + QPointF(25, 0)).red() < 10);
        // An isolated Line point becomes visible only when data markers are requested.
        const auto revision = stat(chart, "preparedRevision");
        chart.setChartType(ChartSeries::Line);
        series.setMarkersVisible(true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QVERIFY(pixel(window.grabWindow(), center).red() > 245);
    }
    void dashPixelsAndPhase()
    {
        QQuickWindow window;
        window.resize(640, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 400});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAcceptHoverEvents(false);
        chart.setViewRange(0, 10, 0, 10);
        ChartSeries s;
        s.setColor(Qt::red);
        s.setLineWidth(4);
        s.setStrokeStyle(ChartSeries::DashLine);
        s.setDashPattern({20, 20});
        s.replacePoints({{1, 5}, {4, 5}, {9, 5}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        const auto start = chart.mapToPosition({1, 5});
        auto frame = window.grabWindow();
        QVERIFY(pixel(frame, start + QPointF(10, 0)).red() > 180);
        QVERIFY(pixel(frame, start + QPointF(30, 0)).red() < 10);
        // A source node must not restart the dash pattern.
        QVERIFY(pixel(frame, start + QPointF(190, 0)).red() < 10);
        auto revision = stat(chart, "preparedRevision");
        s.setDashOffset(20);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        frame = window.grabWindow();
        QVERIFY(pixel(frame, start + QPointF(10, 0)).red() < 10);
        QVERIFY(pixel(frame, start + QPointF(30, 0)).red() > 180);
    }
    void gapsAndRangeFill()
    {
        QQuickWindow window;
        window.resize(640, 400);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 400});
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setBackgroundColor(Qt::black);
        chart.setAcceptHoverEvents(false);
        chart.setChartType(ChartSeries::RangeArea);
        chart.setViewRange(-1, 11, -1, 5);
        ChartSeries s;
        s.setColor(Qt::red);
        s.setFillOpacity(.5);
        s.replaceRangeSamples({{0, 0, 4, {}, false}, {10, 4, 0, {}, false}});
        chart.addSeries(&s);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        auto frame = window.grabWindow();
        QVERIFY(pixel(frame, chart.mapToPosition({2, 2})).red() > 100);
        QVERIFY(pixel(frame, chart.mapToPosition({8, 2})).red() > 100);
        QVERIFY(pixel(frame, chart.mapToPosition({5, 3})).red() < 10);
        auto revision = stat(chart, "preparedRevision");
        s.setBreakBefore(1, true);
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QVERIFY(pixel(window.grabWindow(), chart.mapToPosition({2, 2})).red() < 10);
        for (int mode = ChartSeries::LinearCurve; mode <= ChartSeries::NaturalSpline; ++mode)
        {
            revision = stat(chart, "preparedRevision");
            chart.setChartType(ChartSeries::Area);
            s.setCurveMode(ChartSeries::CurveMode(mode));
            s.replacePoints({{0, 3}, {2, 3}, {8, 3}, {10, 3}});
            s.setBreakBefore(2, true);
            QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
            frame = window.grabWindow();
            QVERIFY(pixel(frame, chart.mapToPosition({1, 2})).red() > 100);
            QVERIFY(pixel(frame, chart.mapToPosition({5, 2})).red() < 10);
            QVERIFY(pixel(frame, chart.mapToPosition({5, 3})).red() < 10);
        }
    }
    void overlayAndIncrementalUploads()
    {
        QQuickWindow window;
        window.resize(600, 400);
        InteractiveChart chart(window.contentItem());
        chart.setSize({600, 400});
        chart.setChartType(ChartSeries::Bar);
        chart.setLegendVisible(false);
        chart.setTooltipsEnabled(false);
        chart.setAcceptHoverEvents(false);
        chart.setGridVisible(false);
        QVERIFY(chart.setViewRange(-1, 4, -1, 10));
        ChartSeries series;
        series.setColor(Qt::red);
        QVERIFY(series.replaceSamples({{0, 8, 1, {}}, {1, 8, 1, {}}, {2, 8, 1, {}}}));
        chart.addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        const auto before = window.grabWindow();
        const auto revision = stat(chart, "preparedRevision"),
                   uploads = stat(chart, "instanceUploads");
        const auto allocations = stat(chart, "bufferAllocations");
        const QPointF center = chart.mapToPosition({1, 4});
        chart.hover(center);
        QTest::qWait(80);
        const auto hovered = window.grabWindow();
        QVERIFY(differences(before, hovered, QRectF(center - QPointF(4, 15), QSizeF(8, 30))) > 10);
        QCOMPARE(stat(chart, "preparedRevision"), revision);
        QCOMPARE(stat(chart, "instanceUploads"), uploads);
        const auto bytes = stat(chart, "uploadedBytes");
        QVERIFY(series.replaceSamples({{0, 8, 1, {}}, {1, 6, 1, {}}, {2, 8, 1, {}}}));
        QTRY_VERIFY(stat(chart, "preparedRevision") > revision && current(chart));
        QCOMPARE(stat(chart, "bufferAllocations"), allocations);
        QVERIFY(stat(chart, "uploadedBytes") > bytes);
        // Only the middle bar changed; unchanged prefix/suffix stay in the GPU buffer.
        QVERIFY(stat(chart, "uploadedBytes") - bytes < 3 * 28);
        const auto changed = window.grabWindow();
        QVERIFY(differences(hovered, changed, chart.plotRect()) > 100);
        window.resize(720, 480);
        chart.setSize({720, 480});
        QTRY_VERIFY(current(chart) && stat(chart, "preparedRevision") > revision + 1);
        QCOMPARE(stat(chart, "bufferAllocations"), allocations);
    }
    void transparencyClippingAndWindowMigration()
    {
        auto first = std::make_unique<QQuickWindow>();
        first->setColor(Qt::blue);
        first->resize(600, 400);
        ChartItem chart(first->contentItem());
        chart.setSize({600, 400});
        chart.setChartType(ChartSeries::Bubble);
        chart.setBackgroundColor(Qt::transparent);
        chart.setLegendVisible(false);
        chart.setGridVisible(false);
        chart.setOpacity(.5);
        // Pointer hover depends on window placement and can paint over the sampled pixel.
        chart.setAcceptHoverEvents(false);
        QVERIFY(chart.setViewRange(0, 10, 0, 10));
        ChartSeries series;
        series.setColor(Qt::red);
        QVERIFY(series.replaceSamples({{0, 5, 24, {}}, {5, 5, 24, {}}}));
        chart.addSeries(&series);
        first->show();
        QVERIFY(QTest::qWaitForWindowExposed(first.get()));
        QTRY_VERIFY(current(chart));
        const auto original = first->grabWindow();
        const auto color = pixel(original, chart.mapToPosition({5, 5}));
        QVERIFY(qAbs(color.red() - 128) < 8);
        QVERIFY(color.green() < 8);
        QVERIFY(qAbs(color.blue() - 127) < 8);
        const auto outside = pixel(original, chart.mapToPosition({0, 5}) - QPointF(12, 0));
        QVERIFY(outside.blue() > 245 && outside.red() < 8);
        QQuickWindow second;
        // Keep the stack-owned chart alive even if a migration assertion returns early.
        const auto detach = qScopeGuard(
            [&]
            {
                chart.setParentItem(nullptr);
                chart.setParent(nullptr);
            });
        second.setColor(Qt::blue);
        second.resize(600, 400);
        const auto frames = stat(chart, "frames");
        chart.setParentItem(second.contentItem());
        chart.setParent(second.contentItem());
        first.reset();
        second.show();
        QVERIFY(QTest::qWaitForWindowExposed(&second));
        // Exposure may already have rendered the migrated frame; do not require a redundant one.
        QTRY_VERIFY(stat(chart, "frames") > frames && current(chart));
        QVERIFY(chart.renderingStatistics().value("error").toString().isEmpty());
        const auto migrated = pixel(second.grabWindow(), chart.mapToPosition({5, 5}));
        QVERIFY2(qAbs(migrated.red() - color.red()) < 8,
                 qPrintable(QString("Original %1, migrated %2")
                                .arg(color.name(QColor::HexArgb), migrated.name(QColor::HexArgb))));
    }
    void multipleChartsInTransformedLayer()
    {
        QQuickWindow window;
        window.resize(640, 400);
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QCatUiKit
Item {
    width: 640; height: 400
    Rectangle { anchors.fill: parent; color: "blue" }
    Item {
        x: 20; y: 15; width: 600; height: 360
        rotation: 5; opacity: 0.5; clip: true; layer.enabled: true
        Repeater {
            model: 2
            CatChart {
                objectName: "layerChart" + index
                required property int index
                x: index * 300; width: 300; height: 360
                chartType: CatChartSeries.Bubble
                backgroundColor: "transparent"
                legendVisible: false; gridVisible: false
                CatChartSeries {
                    color: "red"
                    Component.onCompleted: setData([{x: 5, y: 5, value: 24}])
                }
                Component.onCompleted: setViewRange(0, 10, 0, 10)
            }
        }
    }
})",
                          QUrl());
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *root = qobject_cast<QQuickItem *>(object.get());
        QVERIFY(root);
        root->setParentItem(window.contentItem());
        auto *a = findChart(root, QStringLiteral("layerChart0"));
        auto *b = findChart(root, QStringLiteral("layerChart1"));
        QVERIFY(a && b);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(*a) && current(*b));
        const auto frame = window.grabWindow();
        for (auto *chart : {a, b})
        {
            const auto point = chart->mapToScene(chart->mapToPosition({5, 5}));
            const auto color = pixel(frame, point);
            QVERIFY(qAbs(color.red() - 128) < 8 && qAbs(color.blue() - 127) < 8);
        }
        const auto bUploads = stat(*b, "vertexUploads");
        const auto revision = stat(*a, "preparedRevision");
        a->seriesAt(0)->setColor(Qt::green);
        QTRY_VERIFY(stat(*a, "preparedRevision") > revision && current(*a));
        const auto changed = window.grabWindow();
        QVERIFY(pixel(changed, a->mapToScene(a->mapToPosition({5, 5}))).green() > 115);
        QCOMPARE(stat(*b, "vertexUploads"), bUploads);
    }
};
QTEST_MAIN(ChartRenderTests)
#include "tst_chartrender.moc"
