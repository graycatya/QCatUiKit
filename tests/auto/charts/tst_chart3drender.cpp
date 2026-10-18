// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/ChartItem>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QtTest>
using namespace QCatUiKit;
namespace
{
quint64 stat(ChartItem &c, const char *key)
{
    return c.renderingStatistics().value(key).toULongLong();
}
bool current(ChartItem &c)
{
    return !c.renderingStatistics().value("pending").toBool() && stat(c, "frames") > 0 &&
           stat(c, "renderedRevision") == stat(c, "preparedRevision");
}
QColor pixel(QQuickWindow &w, QPointF p)
{
    auto image = w.grabWindow();
    return image.pixelColor(qRound(p.x() * image.devicePixelRatio()),
                            qRound(p.y() * image.devicePixelRatio()));
}
} // namespace
class Chart3DRenderTests : public QObject
{
    Q_OBJECT
  private slots:
    void linearColorAndMaterialUploads()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        auto *scene = chart.scene3D();
        scene->setEnabled(true);
        scene->setGridVisible(false);
        scene->setPreset(ChartScene3D::Front);
        scene->setAmbient(.25);
        scene->setAmbientGroundRatio(1);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        ChartSeries3D series;
        series.setShape(ChartSeries3D::Cube);
        series.setColor(QColor(128, 128, 128));
        series.setPointSize(1);
        series.setSamples({{0, 0, 0}});
        scene->addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        // A quarter of sRGB 128 in linear light encodes to about 66, not 32.
        QVERIFY(std::abs(pixel(window, {320, 240}).red() - 66) <= 2);
        const auto builds = stat(chart, "geometryBuilds"), uploads = stat(chart, "vertexUploads"),
                   instances = stat(chart, "instanceUploads"), bytes = stat(chart, "uploadedBytes"),
                   textures = stat(chart, "textureUploads"),
                   allocations = stat(chart, "bufferAllocations");
        scene->setLightingModel(ChartScene3D::Phong);
        QTRY_VERIFY(current(chart));
        QVERIFY(std::abs(pixel(window, {320, 240}).red() - 32) <= 1);
        scene->setLightingModel(ChartScene3D::BlinnPhong);
        scene->setExposure(2);
        QTRY_VERIFY(current(chart));
        QVERIFY(std::abs(pixel(window, {320, 240}).red() - 92) <= 2);
        for (int i = 0; i < 12; ++i)
        {
            scene->setLightingModel(i % 2 ? ChartScene3D::Phong : ChartScene3D::BlinnPhong);
            scene->setAmbientGroundRatio(i / 12.);
            scene->rotateBy({2, 1});
            QTRY_VERIFY(current(chart));
        }
        QCOMPARE(stat(chart, "geometryBuilds"), builds);
        QCOMPARE(stat(chart, "vertexUploads"), uploads);
        QCOMPARE(stat(chart, "instanceUploads"), instances);
        QCOMPARE(stat(chart, "uploadedBytes"), bytes);
        QCOMPARE(stat(chart, "textureUploads"), textures);
        QCOMPARE(stat(chart, "bufferAllocations"), allocations);

        // Texture input uses the same sRGB contract as QColor and vertex colors.
        QTemporaryDir dir;
        QImage texture(8, 8, QImage::Format_RGB32);
        texture.fill(QColor(128, 128, 128));
        QVERIFY(texture.save(dir.filePath("gray.png")));
        series.setType(ChartSeries3D::Surface);
        series.setColor(Qt::white);
        series.setSamples({{-1, 0, -1}, {1, 0, -1}, {-1, 0, 1}, {1, 0, 1}}, 2, 2);
        series.setTextureSource(QUrl::fromLocalFile(dir.filePath("gray.png")));
        scene->setPreset(ChartScene3D::Top);
        scene->setLightingModel(ChartScene3D::BlinnPhong);
        scene->setAmbientGroundRatio(1);
        scene->setExposure(1);
        QTRY_VERIFY(current(chart));
        QVERIFY(std::abs(pixel(window, {320, 240}).red() - 66) <= 2);
    }
    void hemisphereAndHighlightShoulder()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        auto *scene = chart.scene3D();
        scene->setEnabled(true);
        scene->setGridVisible(false);
        scene->setPreset(ChartScene3D::Front);
        scene->setAmbient(.7);
        scene->setAmbientGroundRatio(0);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        scene->setLightDirection({0, 0, 1});
        ChartSeries3D series;
        series.setPointSize(1);
        series.setColor(QColor(128, 128, 128));
        series.setSamples({{0, 0, 0}});
        scene->addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        QVERIFY(pixel(window, {320, 210}).red() > pixel(window, {320, 270}).red() + 20);
        scene->setAmbientGroundRatio(1);
        QTRY_VERIFY(current(chart));
        QVERIFY(std::abs(pixel(window, {320, 210}).red() - pixel(window, {320, 270}).red()) <= 2);
        scene->setAmbient(0);
        scene->setSpecular(1);
        scene->setShininess(1);
        QTRY_VERIFY(current(chart));
        const auto lower = pixel(window, {320, 240}).red();
        scene->setSpecular(2);
        QTRY_VERIFY(current(chart));
        const auto upper = pixel(window, {320, 240}).red();
        QVERIFY(lower > 230 && upper > lower + 3 && upper < 255);
        QCOMPARE(pixel(window, {400, 240}), QColor(Qt::black)); // No halo outside geometry.
        scene->setLightDirection({0, 0, -1}); // Opposite half vectors must not produce NaN.
        QTRY_VERIFY(current(chart));
        QCOMPARE(pixel(window, {320, 240}), QColor(Qt::black));
        scene->setExposure(0);
        scene->setAmbient(2);
        QTRY_VERIFY(current(chart));
        QCOMPARE(pixel(window, {320, 240}), QColor(Qt::black));
    }
    void roundedCubeSilhouetteAndPicking()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        chart.setAntialiasing(false);
        auto *scene = chart.scene3D();
        scene->setEnabled(true);
        scene->setGridVisible(false);
        scene->setPreset(ChartScene3D::Front);
        scene->setProjection(ChartScene3D::Orthographic);
        scene->setLightingModel(ChartScene3D::Phong);
        scene->setAmbient(1);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        ChartSeries3D series;
        series.setPointSize(1);
        series.setShape(ChartSeries3D::Cube);
        series.setColor(Qt::red);
        series.setSamples({{0, 0, 0}});
        scene->addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        QCOMPARE(pixel(window, {373, 293}), QColor(Qt::red));
        QVERIFY(!scene->dataAt({373, 293}).isEmpty());
        series.setShape(ChartSeries3D::RoundedCube);
        QTRY_VERIFY(current(chart));
        QCOMPARE(pixel(window, {373, 293}), QColor(Qt::black));
        QVERIFY(scene->dataAt({373, 293}).isEmpty());
        QCOMPARE(pixel(window, {320, 240}), QColor(Qt::red));
        QCOMPARE(scene->dataAt({320, 240}).value("index", -1).toInt(), 0);
    }
    void sphereMaterialAndGestures()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        auto *scene = chart.scene3D();
        scene->setEnabled(true);
        scene->setGridVisible(false);
        scene->setPreset(ChartScene3D::Front);
        scene->setLightDirection({0, 0, 1});
        scene->setAmbient(0);
        scene->setDiffuse(1);
        scene->setSpecular(0);
        ChartSeries3D series;
        series.setPointSize(1);
        series.setColor(Qt::red);
        series.setSamples({{0, 0, 0}});
        scene->addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        const auto center = pixel(window, {320, 240}), edge = pixel(window, {375, 240});
        QVERIFY(center.red() > edge.red() + 25);
        QCOMPARE(pixel(window, {400, 240}), QColor(Qt::black));
        const auto builds = stat(chart, "geometryBuilds"), uploads = stat(chart, "vertexUploads");
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, {320, 240});
        QTest::mouseMove(&window, {360, 260}, 30);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, {360, 260});
        QTRY_VERIFY(scene->yaw() != 0);
        QTRY_VERIFY(current(chart));
        QCOMPARE(stat(chart, "geometryBuilds"), builds);
        QCOMPARE(stat(chart, "vertexUploads"), uploads);
        scene->setPreset(ChartScene3D::Front);
        QTRY_VERIFY(current(chart));
        std::unique_ptr<QPointingDevice> device(QTest::createTouchDevice());
        auto touch = QTest::touchEvent(&window, device.get(), false);
        touch.press(0, {320, 240}, &window).commit();
        touch.release(0, {320, 240}, &window).commit();
        QTRY_COMPARE(scene->selectedPoint().value("index", -1).toInt(), 0);
        chart.setAntialiasing(false);
        scene->setSpecular(.1);
        QTRY_VERIFY(current(chart));
        chart.setAntialiasing(true);
        scene->setSpecular(.2);
        QTRY_VERIFY(current(chart));
        scene->setSelectionMode(ChartScene3D::NoSelection);
        scene->clearSelection();
        touch.press(0, {300, 220}, &window).commit();
        touch.move(0, {340, 250}, &window).commit();
        touch.release(0, {340, 250}, &window).commit();
        QTRY_VERIFY(scene->yaw() != 0);
        QVERIFY(scene->selectedPoint().isEmpty());
    }
    void textureCustomMeshAndWindowMigration()
    {
        QTemporaryDir dir;
        QImage texture(8, 8, QImage::Format_RGB32);
        texture.fill(Qt::green);
        QVERIFY(texture.save(dir.filePath("surface.png")));
        QQuickWindow first, second;
        first.resize(640, 480);
        second.resize(640, 480);
        ChartItem chart(first.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        auto *scene = chart.scene3D();
        scene->setLightingModel(ChartScene3D::Phong);
        scene->setEnabled(true);
        scene->setGridVisible(false);
        scene->setPreset(ChartScene3D::Top);
        scene->setAmbient(1);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        ChartSeries3D series;
        series.setType(ChartSeries3D::Surface);
        series.setColor(Qt::white);
        series.setSamples({{-1, 0, -1}, {1, 0, -1}, {-1, 0, 1}, {1, 0, 1}}, 2, 2);
        series.setTextureSource(QUrl::fromLocalFile(dir.filePath("surface.png")));
        scene->addSeries(&series);
        first.show();
        QVERIFY(QTest::qWaitForWindowExposed(&first));
        QTRY_VERIFY(current(chart));
        QTRY_COMPARE(pixel(first, {320, 240}), QColor(Qt::green));
        QVERIFY(stat(chart, "textureUploads") > 0);
        second.show();
        QVERIFY(QTest::qWaitForWindowExposed(&second));
        chart.setParentItem(second.contentItem());
        QTRY_COMPARE(pixel(second, {320, 240}), QColor(Qt::green));
        series.setType(ChartSeries3D::Scatter);
        series.setShape(ChartSeries3D::CustomMesh);
        series.setColor(Qt::red);
        series.setPointSize(1);
        series.setCustomMesh({QVector3D(-.5, -.5, 0), QVector3D(.5, -.5, 0), QVector3D(0, .5, 0)});
        series.setSamples({{0, 0, 0}});
        scene->setPreset(ChartScene3D::Front);
        QTRY_VERIFY(current(chart));
        QTRY_COMPARE(pixel(second, {320, 240}), QColor(Qt::red));
        QCOMPARE(scene->dataAt({320, 240}).value("index", -1).toInt(), 0);
        second.resize(700, 480);
        chart.setSize({700, 480});
        QTRY_COMPARE(pixel(second, {350, 240}), QColor(Qt::red));
        chart.setParentItem(nullptr);
    }
    void depthLightingAndCameraUploads()
    {
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        auto *scene = chart.scene3D();
        scene->setLightingModel(ChartScene3D::Phong);
        scene->setEnabled(true);
        scene->setGridVisible(false);
        scene->setPreset(ChartScene3D::Front);
        scene->xAxis()->setRange(-2, 2);
        scene->yAxis()->setRange(-2, 2);
        scene->zAxis()->setRange(-2, 2);
        ChartSeries3D near, far;
        near.setColor(Qt::red);
        far.setColor(Qt::blue);
        near.setShape(ChartSeries3D::Cube);
        far.setShape(ChartSeries3D::Cube);
        near.setPointSize(.8);
        far.setPointSize(.8);
        near.setSamples({{0, 0, 1}});
        far.setSamples({{0, 0, -1}});
        scene->addSeries(&near);
        scene->addSeries(&far);
        scene->setAmbient(1);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        QVERIFY2(scene->error().isEmpty(), qPrintable(scene->error()));
        QCOMPARE(pixel(window, {320, 240}), QColor(Qt::red));
        QCOMPARE(chart.dataAt({320, 240}).value("seriesIndex", -1).toInt(), 0);
        const auto builds = stat(chart, "geometryBuilds"), uploads = stat(chart, "vertexUploads"),
                   bytes = stat(chart, "uploadedBytes");
        scene->setYaw(180);
        QTRY_VERIFY(current(chart));
        QTRY_COMPARE(pixel(window, {320, 240}), QColor(Qt::blue));
        QCOMPARE(chart.dataAt({320, 240}).value("seriesIndex", -1).toInt(), 1);
        QCOMPARE(stat(chart, "geometryBuilds"), builds);
        QCOMPARE(stat(chart, "vertexUploads"), uploads);
        QCOMPARE(stat(chart, "uploadedBytes"), bytes);
        scene->setYaw(0);
        scene->setAmbient(0);
        QTRY_VERIFY(current(chart));
        QTRY_COMPARE(pixel(window, {320, 240}), QColor(Qt::black));
        scene->setDiffuse(1);
        QTRY_VERIFY(current(chart));
        QTRY_VERIFY(pixel(window, {320, 240}).red() > 80);
        auto diffuse = pixel(window, {320, 240});
        scene->setLightDirection({0, 0, 1});
        scene->setSpecular(1);
        scene->setShininess(16);
        QTRY_VERIFY(current(chart));
        QTRY_VERIFY(pixel(window, {320, 240}).green() > diffuse.green() + 50);
        scene->selectPoint(0, 0);
        QTRY_VERIFY(current(chart));
        QVERIFY(!scene->selectedPoint().isEmpty());
        scene->setProjection(ChartScene3D::Orthographic);
        QTRY_VERIFY(current(chart));
        QVERIFY(!chart.dataAt({320, 240}).isEmpty());
        scene->clearSelection();
        scene->setAmbient(1);
        scene->setDiffuse(0);
        scene->setSpecular(0);
        far.setSamples({{0, 0, 1}});
        QTRY_VERIFY(current(chart));
        QTRY_COMPARE(pixel(window, {320, 240}), QColor(Qt::blue));
        QCOMPARE(scene->dataAt({320, 240}).value("seriesIndex", -1).toInt(), 1);
        scene->setEnabled(false);
        QTRY_VERIFY(chart.renderingStatistics().value("backend").toString() == "rhi");
        scene->setEnabled(true);
        QTRY_VERIFY(current(chart));
        QVERIFY(scene->error().isEmpty());
    }
    void charts_data()
    {
        QTest::addColumn<int>("type");
        QTest::addColumn<int>("mode");
        QTest::addColumn<bool>("polar");
        for (int type = 0; type < 4; ++type)
            for (int mode = 0; mode < (type == 3 ? 3 : 1); ++mode)
                for (bool polar : {false, true})
                    QTest::newRow(qPrintable(QString("%1-%2-%3").arg(type).arg(mode).arg(polar)))
                        << type << mode << polar;
    }
    void charts()
    {
        QFETCH(int, type);
        QFETCH(int, mode);
        QFETCH(bool, polar);
        QQuickWindow window;
        window.resize(640, 480);
        ChartItem chart(window.contentItem());
        chart.setSize({640, 480});
        chart.setBackgroundColor(Qt::black);
        auto *scene = chart.scene3D();
        scene->setEnabled(true);
        scene->setGridVisible(false);
        ChartSeries3D series;
        series.setType(ChartSeries3D::Type(type));
        series.setPolar(polar);
        series.setClosed(true);
        series.setSurfaceMode(ChartSeries3D::SurfaceMode(mode));
        series.setColorByHeight(true);
        series.setPointSize(.2);
        QVector<ChartSeries3D::Point> points;
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 12; ++c)
                points.append(
                    {polar ? c * 30. : double(c), std::sin(r * .6) * std::cos(c * .5), double(r)});
        series.setSamples(points, 9, 12);
        scene->addSeries(&series);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(chart));
        QVERIFY2(scene->error().isEmpty(), qPrintable(scene->error()));
        auto image = window.grabWindow();
        QVERIFY(!image.isNull());
        int colored = 0;
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
            {
                auto c = image.pixelColor(x, y);
                colored += c.red() + c.green() + c.blue() > 50;
            }
        QVERIFY2(colored > 300, qPrintable(QString::number(colored)));
        const auto builds = stat(chart, "geometryBuilds");
        for (int i = 0; i < 8; ++i)
        {
            scene->rotateBy({5, 1});
            QTRY_VERIFY(current(chart));
        }
        QCOMPARE(stat(chart, "geometryBuilds"), builds);
        series.setShape(ChartSeries3D::Cylinder);
        QTRY_VERIFY(current(chart));
        if (type == 1 || type == 3)
        {
            scene->setSliceMode(ChartScene3D::RowSlice);
            scene->setSliceIndex(3);
            QTRY_VERIFY(current(chart));
            QCOMPARE(series.count(), 108);
        }
        series.clear();
        QTRY_VERIFY(current(chart));
        QCOMPARE(stat(chart, "instanceCount"), quint64(0));
    }
    void transformedLayerWithMultipleCharts()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent component(&engine);
        component.setData(R"(
            import QtQuick
            import QCatUiKit
            Item {
                width: 640; height: 480
                opacity: 0.5; rotation: 3; clip: true; layer.enabled: true
                Repeater {
                    model: 2
                    CatChart {
                        required property int index
                        objectName: "chart3D" + index
                        x: 20 + index * 300; y: 40; width: 280; height: 360
                        backgroundColor: "transparent"
                        scene3D.enabled: true; scene3D.gridVisible: false
                        scene3D.ambient: 1; scene3D.diffuse: 0; scene3D.specular: 0
                        scene3D.series: [CatChartSeries3D {
                            pointSize: 0.6; color: "red"; points: [{x:0,y:0,z:0}]
                        }]
                    }
                }
            }
        )",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *root = qobject_cast<QQuickItem *>(object.get());
        QVERIFY(root);
        QQuickWindow window;
        window.resize(640, 480);
        window.setColor(Qt::black);
        root->setParentItem(window.contentItem());
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        int count = 0;
        for (auto *item : root->childItems())
            if (auto *chart = qobject_cast<ChartItem *>(item))
            {
                ++count;
                QTRY_VERIFY(current(*chart));
                const auto center = chart->mapToScene(chart->scene3D()->mapToPosition(0, 0, 0));
                const auto color = pixel(window, center);
                QVERIFY(color.red() > 110 && color.red() < 145);
                QVERIFY(color.green() < 10);
            }
        QCOMPARE(count, 2);
        root->setParentItem(nullptr);
    }
    void qmlIntegration()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent component(&engine);
        component.setData(R"(
            import QtQuick
            import QCatUiKit
            CatChart {
                width: 640; height: 480
                scene3D.enabled: true
                scene3D.projection: CatChartScene3D.Orthographic
                scene3D.series: [CatChartSeries3D {
                    shape: CatChartSeries3D.Cube
                    pointSize: 0.5
                    points: [{x:0,y:0,z:0},{x:1,y:1,z:1}]
                }]
            }
        )",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *chart = qobject_cast<ChartItem *>(object.get());
        QVERIFY(chart);
        QQuickWindow window;
        window.resize(640, 480);
        chart->setParentItem(window.contentItem());
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(current(*chart));
        QCOMPARE(chart->scene3D()->seriesCount(), 1);
        chart->setParentItem(nullptr);
    }
};
QTEST_MAIN(Chart3DRenderTests)
#include "tst_chart3drender.moc"
