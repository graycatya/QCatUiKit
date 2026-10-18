// SPDX-License-Identifier: Apache-2.0
#include "qcatchart3d_p.h"
#include <QCatUiKit/ChartItem>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
#include <cmath>
#include <limits>
using namespace QCatUiKit;
using namespace QCatUiKit::Detail;
class Chart3DTests : public QObject
{
    Q_OBJECT
  private slots:
    void lightingValidationAndGeometryReuse()
    {
        ChartScene3D scene;
        ChartSeries3D series;
        series.setSamples({{0, 0, 0}});
        scene.addSeries(&series);
        QCOMPARE(scene.lightingModel(), ChartScene3D::BlinnPhong);
        QSignalSpy changed(&scene, &ChartScene3D::materialChanged);
        auto original = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        scene.setLightingModel(ChartScene3D::Phong);
        scene.setAmbientGroundRatio(.2);
        scene.setExposure(2);
        QCOMPARE(changed.count(), 3);
        scene.setLightingModel(ChartScene3D::LightingModel(5));
        scene.setAmbientGroundRatio(NAN);
        scene.setAmbientGroundRatio(-.1);
        scene.setAmbientGroundRatio(1.1);
        scene.setExposure(INFINITY);
        scene.setExposure(-1);
        scene.setExposure(9);
        scene.setExposure(2);
        QCOMPARE(changed.count(), 3);
        auto next = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QCOMPARE(next->geometry, original->geometry);
        QCOMPARE(next->lightingModel, ChartScene3D::Phong);
        QCOMPARE(next->ambientGroundRatio, .2f);
        QCOMPARE(next->exposure, 2.f);
        scene.setLightingModel(ChartScene3D::BlinnPhong);
        QCOMPARE(ChartScene3DAccess::frame(&scene, {640, 480}, false)->geometry,
                 original->geometry);
    }
    void roundedGeometryAndDetail()
    {
        ChartScene3D scene;
        scene.setGridVisible(false);
        ChartSeries3D series;
        series.setSamples({{0, 0, 0}});
        scene.addSeries(&series);
        for (auto shape :
             {ChartSeries3D::Sphere, ChartSeries3D::Cylinder, ChartSeries3D::RoundedCube})
        {
            series.setShape(shape);
            qsizetype previous = 0;
            for (auto detail :
                 {ChartSeries3D::LowDetail, ChartSeries3D::MediumDetail, ChartSeries3D::HighDetail})
            {
                series.setMeshDetail(detail);
                auto frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
                const auto &vertices = frame->geometry->vertices;
                QVERIFY(vertices.size() > previous);
                previous = vertices.size();
                for (int i = 0; i < vertices.size(); i += 3)
                {
                    QVector3D p[3], n[3];
                    for (int j = 0; j < 3; ++j)
                    {
                        const auto &v = vertices[i + j];
                        p[j] = {v.x, v.y, v.z};
                        n[j] = {v.nx, v.ny, v.nz};
                        QVERIFY(std::abs(n[j].length() - 1) < 1e-5);
                        for (int k = 0; k < 3; ++k)
                            QVERIFY(std::abs(p[j][k]) <= .50001f);
                    }
                    const auto cross = QVector3D::crossProduct(p[1] - p[0], p[2] - p[0]);
                    QVERIFY(cross.lengthSquared() > 1e-12);
                    QVERIFY(QVector3D::dotProduct(cross, n[0] + n[1] + n[2]) > 0);
                }
            }
        }
        series.setMeshDetail(ChartSeries3D::MeshDetail(3));
        QCOMPARE(series.meshDetail(), ChartSeries3D::HighDetail);
        series.setType(ChartSeries3D::Bars);
        series.setSamples({{0, 1, 0}, {1, -1, 0}}, 1, 2);
        scene.yAxis()->setRange(-1, 1);
        auto frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QCOMPARE(frame->geometry->objects[0].minimum.y(), 0.f);
        QCOMPARE(frame->geometry->objects[0].maximum.y(), 1.f);
        QCOMPARE(frame->geometry->objects[1].minimum.y(), -1.f);
        QCOMPARE(frame->geometry->objects[1].maximum.y(), 0.f);
    }
    void smoothTubeNormalsFaceOutward()
    {
        ChartScene3D scene;
        scene.setGridVisible(false);
        ChartSeries3D series;
        series.setType(ChartSeries3D::Curve);
        series.setSamples({{0, 0, 0}, {1, 1, 0}, {2, 0, 1}});
        scene.addSeries(&series);
        auto frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        const auto &vertices = frame->geometry->vertices;
        QVERIFY(!vertices.isEmpty());
        for (int i = 0; i < vertices.size(); i += 3)
        {
            QVector3D p[3], normal;
            for (int j = 0; j < 3; ++j)
            {
                const auto &v = vertices[i + j];
                p[j] = {v.x, v.y, v.z};
                normal += QVector3D(v.nx, v.ny, v.nz);
            }
            const auto cross = QVector3D::crossProduct(p[1] - p[0], p[2] - p[0]);
            QVERIFY(QVector3D::dotProduct(cross, normal) > 0);
        }
    }
    void equalDepthAndRenderBudget()
    {
        ChartScene3D scene;
        scene.setGridVisible(false);
        scene.setPreset(ChartScene3D::Front);
        ChartSeries3D series;
        series.setShape(ChartSeries3D::Cube);
        series.setPointSize(.6);
        series.setSamples({{0, 0, 0}, {0, 0, 0}});
        scene.addSeries(&series);
        ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QCOMPARE(scene.dataAt({320, 240}).value("index", -1).toInt(), 1);
        series.setType(ChartSeries3D::Curve);
        series.setSubdivisions(64);
        QVector<ChartSeries3D::Point> large(10000);
        for (int i = 0; i < large.size(); ++i)
            large[i] = {double(i), std::sin(i * .1), 0};
        series.setSamples(large);
        auto frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QVERIFY(frame->geometry->vertices.isEmpty());
        QVERIFY(scene.error().contains("budget"));
        series.setType(ChartSeries3D::Scatter);
        series.setSamples({{std::numeric_limits<double>::max(), 0, 0}});
        frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QVERIFY(scene.error().isEmpty());
        QVERIFY(!frame->geometry->instances.isEmpty());
        for (float value : frame->geometry->instances[0].matrix)
            QVERIFY(std::isfinite(value));
    }
    void customBarBaselineAndSliceSelection()
    {
        ChartScene3D scene;
        scene.setGridVisible(false);
        scene.yAxis()->setRange(-1, 1);
        ChartSeries3D series;
        series.setType(ChartSeries3D::Bars);
        series.setShape(ChartSeries3D::CustomMesh);
        series.setCustomMesh({QVector3D(0, 2, 0), QVector3D(-1, 0, 1), QVector3D(1, 0, 1),
                              QVector3D(0, 2, 0), QVector3D(1, 0, 1), QVector3D(0, 0, -1),
                              QVector3D(0, 2, 0), QVector3D(0, 0, -1), QVector3D(-1, 0, 1)});
        series.setSamples({{0, 1, 0}, {1, -1, 0}}, 1, 2);
        scene.addSeries(&series);
        auto frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QCOMPARE(frame->geometry->objects.size(), 2);
        QCOMPARE(frame->geometry->objects[0].minimum.y(), 0.f);
        QCOMPARE(frame->geometry->objects[0].maximum.y(), 1.f);
        QCOMPARE(frame->geometry->objects[1].minimum.y(), -1.f);
        QCOMPARE(frame->geometry->objects[1].maximum.y(), 0.f);
        series.setType(ChartSeries3D::Surface);
        scene.setSliceMode(ChartScene3D::RowSlice);
        scene.setSelectionMode(ChartScene3D::RowSelection);
        QVERIFY(scene.selectPoint(0, 0));
        frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QVERIFY(!frame->geometry->vertices.isEmpty());
        for (const auto &v : frame->geometry->vertices)
        {
            QCOMPARE(v.r, 1.f);
            QCOMPARE(v.b, 0.f);
        }
        QCOMPARE(series.count(), 2);
    }
    void dataContract()
    {
        ChartSeries3D series;
        QSignalSpy data(&series, &ChartSeries3D::dataChanged);
        series.setPoints({QVariantMap{{"x", 1e12}, {"y", 2}, {"z", 3}}});
        QCOMPARE(series.count(), 1);
        QCOMPARE(data.count(), 1);
        QVERIFY(!series.append({{"x", 0}, {"y", 2}}));
        QCOMPARE(series.count(), 1);
        QVERIFY(!series.error().isEmpty());
        QVERIFY(!series.setSamples({{NAN, 2, 3}}));
        QCOMPARE(data.count(), 1);
        QVERIFY(!series.setGrid(2, 3, series.points()));
        QCOMPARE(series.rows(), 0);
        QVERIFY(series.setGrid(1, 1, series.points()));
        QCOMPARE(series.columns(), 1);
        QVERIFY(!series.append({{"x", 1}, {"y", 2}, {"z", 3}}));
        QVERIFY(!series.remove(0));
        QVERIFY(series.replace(0, {{"x", 4}, {"y", 5}, {"z", 6}, {"valid", false}}));
        QCOMPARE(series.at(0).value("valid").toBool(), false);
        series.clear();
        QCOMPARE(series.rows(), 0);
        QVERIFY(series.error().isEmpty());
        ChartSeries legacy;
        legacy.appendData({QVariantMap{{"x", 1}, {"y", 2}, {"value", 9}}});
        QCOMPARE(legacy.sampleAt(0).value, 9.);
    }
    void lifecycleAndValidation()
    {
        ChartItem chart;
        auto *scene = chart.scene3D();
        QVERIFY(!scene->enabled());
        auto *series = new ChartSeries3D;
        series->setSamples({{0, 0, 0}});
        scene->addSeries(series);
        scene->addSeries(series);
        QCOMPARE(scene->seriesCount(), 1);
        QVERIFY(scene->selectPoint(0, 0));
        delete series;
        QCOMPARE(scene->seriesCount(), 0);
        QVERIFY(scene->selectedPoint().isEmpty());
        scene->setPitch(1000);
        QCOMPARE(scene->pitch(), 89.9);
        scene->setDistance(NAN);
        QCOMPARE(scene->distance(), 5.);
        scene->setNearPlane(200);
        QCOMPARE(scene->nearPlane(), .05);
        scene->setFarPlane(-1);
        QCOMPARE(scene->farPlane(), 100.);
        scene->setLightDirection({});
        QVERIFY(!scene->lightDirection().isNull());
        scene->setAmbient(-1);
        QCOMPARE(scene->ambient(), .25);
        ChartScene3DAccess::frame(scene, {600, 400}, true);
        QVERIFY(!scene->supported());
        QVERIFY(scene->error().contains("software"));
    }
    void cameraAndDepthPicking()
    {
        ChartScene3D scene;
        ChartSeries3D points;
        points.setShape(ChartSeries3D::Cube);
        points.setPointSize(.6);
        points.setSamples({{1e12, 0, 1}, {1e12, 0, -1}});
        scene.addSeries(&points);
        scene.xAxis()->setRange(1e12 - 1, 1e12 + 1);
        scene.yAxis()->setRange(-1, 1);
        scene.zAxis()->setRange(-2, 2);
        scene.setPreset(ChartScene3D::Front);
        auto frame = ChartScene3DAccess::frame(&scene, {600, 400}, false);
        auto geometry = frame->geometry;
        const auto p = scene.mapToPosition(1e12, 0, 1);
        QCOMPARE(p, QPointF(300, 200));
        QCOMPARE(scene.dataAt(p).value("index", -1).toInt(), 0);
        scene.setYaw(180);
        ChartScene3DAccess::frame(&scene, {600, 400}, false);
        QCOMPARE(scene.dataAt({300, 200}).value("index", -1).toInt(), 1);
        scene.setProjection(ChartScene3D::Orthographic);
        auto next = ChartScene3DAccess::frame(&scene, {600, 400}, false);
        QCOMPARE(next->geometry, geometry);
        scene.setNearPlane(6);
        ChartScene3DAccess::frame(&scene, {600, 400}, false);
        QVERIFY(scene.dataAt({300, 200}).isEmpty());
        scene.resetView();
        QCOMPARE(scene.target(), QVector3D());
        QCOMPARE(scene.distance(), 5.);
    }
    void types_data()
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
    void types()
    {
        QFETCH(int, type);
        QFETCH(int, mode);
        QFETCH(bool, polar);
        ChartScene3D scene;
        scene.setGridVisible(false);
        ChartSeries3D series;
        series.setType(ChartSeries3D::Type(type));
        series.setPolar(polar);
        series.setClosed(true);
        series.setSurfaceMode(ChartSeries3D::SurfaceMode(mode));
        QVector<ChartSeries3D::Point> p;
        for (int r = 0; r < 5; ++r)
            for (int c = 0; c < 8; ++c)
            {
                ChartSeries3D::Point v;
                v.x = polar ? c * 45 : c;
                v.y = std::sin(r * .5) * std::cos(c * .5);
                v.z = r;
                v.valid = !(r == 2 && c == 3);
                p.append(v);
            }
        QVERIFY(series.setSamples(p, 5, 8));
        scene.addSeries(&series);
        auto frame = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QVERIFY(!frame->geometry->vertices.isEmpty());
        QVERIFY(!frame->geometry->instances.isEmpty());
        for (const auto &v : frame->geometry->vertices)
        {
            QVERIFY(std::isfinite(v.x));
            QVERIFY(std::isfinite(v.y));
            QVERIFY(std::isfinite(v.z));
            QVERIFY(std::isfinite(v.nx));
        }
        QVERIFY(scene.selectPoint(0, 0));
        scene.setSelectionMode(ChartScene3D::RowSelection);
        scene.setSliceMode(ChartScene3D::RowSlice);
        scene.setSliceIndex(1);
        auto sliced = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QCOMPARE(series.count(), 40);
        if (type == ChartSeries3D::Bars)
            QCOMPARE(sliced->geometry->objects.size(), 8);
        series.clear();
        auto empty = ChartScene3DAccess::frame(&scene, {640, 480}, false);
        QVERIFY(empty->geometry->vertices.isEmpty());
        QVERIFY(scene.selectedPoint().isEmpty());
    }
    void missingGridCellsAndPolarRecords()
    {
        ChartScene3D scene;
        scene.setGridVisible(false);
        scene.setPreset(ChartScene3D::Top);
        ChartSeries3D series;
        series.setType(ChartSeries3D::Surface);
        QVector<ChartSeries3D::Point> points;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                points.append({double(c), 0., double(r)});
        points[4].valid = false;
        series.setSamples(points, 3, 3);
        scene.addSeries(&series);
        auto f = ChartScene3DAccess::frame(&scene, {600, 400}, false);
        QVERIFY(f->geometry->vertices.isEmpty());
        QVERIFY(scene.dataAt({300, 200}).isEmpty());
        points[4].valid = true;
        series.setSamples(points, 3, 3);
        f = ChartScene3DAccess::frame(&scene, {600, 400}, false);
        QCOMPARE(f->geometry->vertices.size(), 9);
        QCOMPARE(f->geometry->indices.size(), 24);
        series.setType(ChartSeries3D::Scatter);
        series.setPolar(true);
        series.setPointSize(.3);
        series.setSamples({{90, 0, 2}});
        scene.xAxis()->setRange(-3, 3);
        scene.yAxis()->setRange(-3, 3);
        scene.zAxis()->setRange(-3, 3);
        scene.setPreset(ChartScene3D::Front);
        ChartScene3DAccess::frame(&scene, {600, 400}, false);
        auto hit = scene.dataAt(scene.mapToPosition(0, 0, 2));
        QCOMPARE(hit.value("x").toDouble(), 90.);
        QCOMPARE(hit.value("z").toDouble(), 2.);
    }
    void heightMapAndMesh()
    {
        QTemporaryDir dir;
        QImage image(3, 2, QImage::Format_ARGB32);
        image.fill(Qt::white);
        image.setPixelColor(0, 0, Qt::transparent);
        QVERIFY(image.save(dir.filePath("height.png")));
        ChartSeries3D series;
        QVERIFY(series.setHeightMap(QUrl::fromLocalFile(dir.filePath("height.png")), 5));
        QCOMPARE(series.rows(), 2);
        QCOMPARE(series.columns(), 3);
        QCOMPARE(series.samples()[1].y, 5.);
        QVERIFY(!series.samples()[0].valid);
        QVERIFY(!series.setHeightMap(QUrl("https://invalid/image.png")));
        QCOMPARE(series.count(), 6);
        series.setCustomMesh({QVector3D(0, 1, 0), QVector3D(-1, 0, 0), QVector3D(1, 0, 0)});
        QCOMPARE(series.customMesh().size(), 3);
        series.setCustomMesh({QVector3D(0, 0, 0)});
        QCOMPARE(series.customMesh().size(), 3);
    }
};
QTEST_MAIN(Chart3DTests)
#include "tst_chart3d.moc"
