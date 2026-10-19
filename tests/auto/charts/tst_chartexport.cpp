// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatchartitem.h>
#include <QFile>
#include <QPainter>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
#include <functional>

using namespace QCatUiKit;

namespace
{
class Overlay : public QQuickPaintedItem
{
  public:
    using QQuickPaintedItem::QQuickPaintedItem;
    void paint(QPainter *painter) override { painter->fillRect(boundingRect(), Qt::cyan); }
};
int coloredPixels(const QImage &image, const std::function<bool(QColor)> &matches)
{
    int result = 0;
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            result += matches(image.pixelColor(x, y));
    return result;
}
}

class ChartExportTests : public QObject
{
    Q_OBJECT
  private slots:
    void completeChartAndOverlay()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QQuickWindow window;
        window.setColor(Qt::magenta);
        window.resize(700, 500);
        ChartItem chart(window.contentItem());
        chart.setPosition({30, 40});
        chart.setSize({640, 420});
        chart.setBackgroundColor(Qt::black);
        chart.setTextColor(Qt::white);
        chart.setTitle("Export title");
        chart.xAxis()->setTitle("X axis");
        chart.yAxis()->setTitle("Y axis");
        ChartSeries series;
        series.setName("Export legend");
        series.setColor(Qt::red);
        series.setLineWidth(5);
        series.replacePoints({{0, 1}, {1, 3}, {2, 2}});
        chart.addSeries(&series);
        Overlay overlay(&chart);
        overlay.setPosition({300, 180});
        overlay.setSize({30, 20});
        overlay.setZ(100);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QSignalSpy finished(&chart, &ChartItem::imageExportFinished);
        QSignalSpy state(&chart, &ChartItem::exportingImageChanged);
        const auto url = QUrl::fromLocalFile(directory.filePath("complete 图表.png"));
        // Export immediately after a data change: the next frame must contain it.
        series.replacePoints({{0, 1}, {1, 4}, {2, 2}});
        QVERIFY(chart.exportImage(url));
        QVERIFY(chart.exportingImage());
        QTRY_COMPARE(finished.size(), 1);
        QCOMPARE(finished[0][0].toUrl(), url);
        QVERIFY2(finished[0][1].toBool(), qPrintable(finished[0][2].toString()));
        QVERIFY(!chart.exportingImage());
        QCOMPARE(state.size(), 2);
        const QImage image(url.toLocalFile());
        const auto dpr = window.effectiveDevicePixelRatio();
        QCOMPARE(image.size(), QSize(qCeil(640 * dpr), qCeil(420 * dpr)));
        QCOMPARE(image.pixelColor(qRound(310 * dpr), qRound(190 * dpr)), QColor(Qt::cyan));
        QVERIFY(coloredPixels(image, [](QColor c) { return c.red() > 200 && c.green() < 40; }) > 100);
        const auto white = [](QColor c) { return c.red() > 150 && c.green() > 150 && c.blue() > 150; };
        QVERIFY(coloredPixels(image.copy(0, 0, image.width(), qRound(40 * dpr)), white) > 20);
        QVERIFY(coloredPixels(image.copy(0, qRound(380 * dpr), image.width(), qRound(40 * dpr)), white) > 20);
        // The exported subtree includes decorations and children, without the window background.
        const auto screen = window.grabWindow();
        const auto reference = screen.copy(qRound(30 * dpr), qRound(40 * dpr), image.width(), image.height());
        int different = 0;
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
                different += image.pixelColor(x, y) != reference.pixelColor(x, y);
        QVERIFY2(different < image.width() * image.height() / 100, qPrintable(QString::number(different)));
    }

    void dimensionsTransparencyAndBusy()
    {
        QTemporaryDir directory;
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::transparent);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QSignalSpy finished(&chart, &ChartItem::imageExportFinished);
        const auto url = QUrl::fromLocalFile(directory.filePath("scaled.png"));
        const auto busy = QUrl::fromLocalFile(directory.filePath("busy.png"));
        QVERIFY(chart.exportImage(url, {321, 217}));
        QVERIFY(!chart.exportImage(busy));
        QCOMPARE(finished.size(), 1);
        QVERIFY(!finished[0][1].toBool());
        QVERIFY(chart.exportingImage());
        QTRY_COMPARE(finished.size(), 2);
        QVERIFY2(finished[1][1].toBool(), qPrintable(finished[1][2].toString()));
        const QImage image(url.toLocalFile());
        QCOMPARE(image.size(), QSize(321, 217));
        QCOMPARE(image.pixelColor(0, 0).alpha(), 0);
        QVERIFY(!QFile::exists(busy.toLocalFile()));
        // Replacing an existing image is supported after the first request finishes.
        QVERIFY(chart.exportImage(url, {200, 150}));
        QTRY_COMPARE(finished.size(), 3);
        QVERIFY(finished[2][1].toBool());
        QCOMPARE(QImage(url.toLocalFile()).size(), QSize(200, 150));
    }

    void rejectionAndWriteFailure()
    {
        QTemporaryDir directory;
        const auto url = QUrl::fromLocalFile(directory.filePath("preserved.png"));
        QFile original(url.toLocalFile());
        QVERIFY(original.open(QIODevice::WriteOnly));
        original.write("original");
        original.close();
        QQuickWindow window;
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        window.resize(640, 480);
        QSignalSpy finished(&chart, &ChartItem::imageExportFinished);
        QVERIFY(!chart.exportImage(url)); // Hidden window.
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(!chart.exportImage(QUrl("https://example.org/image.png")));
        QVERIFY(!chart.exportImage(QUrl::fromLocalFile(directory.filePath("wrong.jpg"))));
        QVERIFY(!chart.exportImage(url, {0, 200}));
        QVERIFY(!chart.exportImage(url, {9000, 9000}));
        chart.setVisible(false);
        QVERIFY(!chart.exportImage(url));
        chart.setVisible(true);
        QVERIFY(original.open(QIODevice::ReadOnly));
        QCOMPARE(original.readAll(), QByteArray("original"));
        original.close();
        for (const auto &result : finished)
        {
            QVERIFY(!result[1].toBool());
            QVERIFY(!result[2].toString().isEmpty());
        }
        finished.clear();
        QVERIFY(chart.exportImage(QUrl::fromLocalFile(directory.filePath("missing/image.png"))));
        QTRY_COMPARE(finished.size(), 1);
        QVERIFY(!finished[0][1].toBool());
        QVERIFY(!finished[0][2].toString().isEmpty());
        QVERIFY(!chart.exportingImage());
    }

    void destroyedChartCancels()
    {
        QTemporaryDir directory;
        const auto url = QUrl::fromLocalFile(directory.filePath("cancelled.png"));
        QQuickWindow window;
        window.resize(640, 480);
        auto *chart = new ChartItem(window.contentItem());
        chart->setSize({640, 480});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(chart->exportImage(url));
        delete chart;
        QTest::qWait(100);
        QVERIFY(!QFile::exists(url.toLocalFile()));
    }

    void threeDimensionalChart()
    {
        QTemporaryDir directory;
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setTextColor(Qt::white);
        chart.setTitle("3D export");
        auto *scene = chart.scene3D();
        scene->setEnabled(true);
        scene->setPreset(ChartScene3D::Front);
        scene->setAmbient(1);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        ChartSeries3D series;
        series.setShape(ChartSeries3D::Cube);
        series.setColor(Qt::red);
        series.setPointSize(1);
        series.setSamples({{0, 0, 0}});
        scene->addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QSignalSpy finished(&chart, &ChartItem::imageExportFinished);
        const auto url = QUrl::fromLocalFile(directory.filePath("3d.png"));
        if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software)
        {
            QVERIFY(!chart.exportImage(url));
            QCOMPARE(finished.size(), 1);
            QVERIFY(!finished[0][1].toBool());
            QVERIFY(!QFile::exists(url.toLocalFile()));
            return;
        }
        QVERIFY(chart.exportImage(url));
        QTRY_COMPARE(finished.size(), 1);
        QVERIFY2(finished[0][1].toBool(), qPrintable(finished[0][2].toString()));
        const QImage image(url.toLocalFile());
        QVERIFY(coloredPixels(image, [](QColor c) { return c.red() > 100 && c.green() < 40; }) > 100);
        QVERIFY(coloredPixels(image.copy(0, 0, image.width(), 50), [](QColor c) { return c.red() > 150 && c.green() > 150; }) > 20);
    }
};
QTEST_MAIN(ChartExportTests)
#include "tst_chartexport.moc"
