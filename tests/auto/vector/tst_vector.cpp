// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/VectorItem.h>
#include <QCatUiKit/VectorItem>
#include <QCatUiKit/qcatvectoritem.h>
#include <QFile>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest>
#include <limits>
using namespace QCatUiKit;
class VectorTest : public QObject
{
    Q_OBJECT
  private slots:
    void propertiesAndValidation()
    {
        VectorItem item;
        QCOMPARE(item.status(), VectorItem::Null);
        QSignalSpy changed(&item, &VectorItem::progressChanged);
        item.setProgress(0.5);
        item.setProgress(0.5);
        item.setProgress(std::numeric_limits<double>::quiet_NaN());
        QCOMPARE(changed.size(), 1);
        item.setProgress(4);
        QCOMPARE(item.progress(), 1.0);
        item.setPlaybackRate(-1);
        QCOMPARE(item.playbackRate(), 1.0);
        item.setLoops(0);
        QCOMPARE(item.loops(), -1);
        item.setFormat(VectorItem::Canvas);
        const QVariantMap rect{
            {"type", "rectangle"}, {"width", 60}, {"height", 40}, {"fill", "red"}};
        QVERIFY(item.appendShape(rect));
        QCOMPARE(item.shapes().size(), 1);
        QVERIFY(!item.appendShape({{"type", "ellipse"}, {"width", -1}, {"height", 4}}));
        QCOMPARE(item.shapes().size(), 1);
        const auto originalBox = item.viewBox();
        item.setViewBox({0, 0, -2, 10});
        QCOMPARE(item.viewBox(), originalBox);
        item.setShapes(
            {QVariantMap{{"type", "path"}, {"commands", QVariantList{QVariantList{"L", 1, 2}}}}});
        QCOMPARE(item.status(), VectorItem::Error);
        QVERIFY(item.errorString().contains("start with M"));
        item.clearShapes();
        QCOMPARE(item.shapes().size(), 0);
    }
    void qmlWrappers()
    {
        QQmlEngine engine;
        QSignalSpy warnings(&engine, &QQmlEngine::warnings);
        for (const auto &type : {"CatSvgImage", "CatLottieAnimation", "CatVectorCanvas"})
        {
            QQmlComponent component(&engine);
            component.setData(QByteArray("import QCatUiKit\n") + type + " {}", QUrl());
            std::unique_ptr<QObject> object(component.create());
            QVERIFY2(object, qPrintable(component.errorString()));
            QVERIFY(qobject_cast<VectorItem *>(object.get()));
            QCOMPARE(warnings.size(), 0);
        }
    }
    void softwareFailureIsObservable()
    {
        if (qEnvironmentVariableIsSet("QCAT_VECTOR_GPU"))
            return;
        QQuickWindow window;
        auto *item = new VectorItem(window.contentItem());
        item->setSize({80, 80});
        item->setSourceData("<svg xmlns='http://www.w3.org/2000/svg' width='80' height='80'><rect "
                            "width='80' height='80'/></svg>");
        window.resize(100, 100);
        window.show();
        QTRY_COMPARE(item->status(), VectorItem::Error);
        QVERIFY(!item->errorString().isEmpty());
        item->setSourceData({});
        QCOMPARE(item->status(), VectorItem::Null);
    }
    void qrhiQmlCanvas()
    {
        if (!qEnvironmentVariableIsSet("QCAT_VECTOR_GPU"))
            QSKIP("Native QRhi window required");
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(
import QCatUiKit
CatVectorCanvas {
    width: 240; height: 160
    viewBox: Qt.rect(0, 0, 100, 100)
    shapes: [{type: "path", commands: [["M", 10, 10], ["L", 90, 10], ["L", 50, 90], ["Z"]],
              fill: {type: "linear", x1: 0, y1: 0, x2: 100, y2: 100,
                     stops: [{offset: 0, color: "#3b82f6"}, {offset: 1, color: "#a855f7"}]}}]
}
)",
                          QUrl());
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *item = qobject_cast<VectorItem *>(object.get());
        QVERIFY(item);
        QQuickWindow window;
        window.resize(240, 160);
        item->setParentItem(window.contentItem());
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE_WITH_TIMEOUT(item->status(), VectorItem::Ready, 10000);
        auto image = window.grabWindow();
        QVERIFY(!image.isNull());
        QVERIFY(image.pixelColor(image.width() / 2, image.height() / 2).blue() > 100);
        image.save("vector-qml-canvas.png");
        item->setParentItem(nullptr);
    }
    void qrhiPixelsAndPlayback()
    {
        if (!qEnvironmentVariableIsSet("QCAT_VECTOR_GPU"))
            QSKIP("Enable QCATUIKIT_TEST_VECTOR_GPU for a real QRhi window");
        QQuickWindow window;
        window.setColor(Qt::black);
        window.resize(240, 160);
        auto *item = new VectorItem(window.contentItem());
        QVERIFY(item->backendAvailable());
        item->setSize({240, 160});
        item->setSourceData("<svg xmlns='http://www.w3.org/2000/svg' width='100' "
                            "height='100'><rect width='100' height='100' fill='red'/></svg>");
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE_WITH_TIMEOUT(item->status(), VectorItem::Ready, 10000);
        QVERIFY(item->renderedFrames() > 0);
        auto image = window.grabWindow();
        QVERIFY(!image.isNull());
        QVERIFY(image.pixelColor(image.width() / 2, image.height() / 2).red() > 200);
        QVERIFY(image.pixelColor(5, image.height() / 2).red() < 40); // aspect-fit padding
        item->setFillMode(VectorItem::Stretch);
        QTest::qWait(100);
        image = window.grabWindow();
        QVERIFY(image.pixelColor(5, image.height() / 2).red() > 200);
        window.resize(360, 220);
        item->setSize({360, 220});
        QTest::qWait(100);
        QVERIFY(!window.grabWindow().isNull());
        item->setSourceData("broken");
        QTRY_COMPARE(item->status(), VectorItem::Error);
        item->setSourceData({});
        item->setFormat(VectorItem::Lottie);
        item->setSource(
            QUrl::fromLocalFile(QStringLiteral(QCAT_VECTOR_DATA_DIR "/vector-demo.json")));
        QTRY_COMPARE_WITH_TIMEOUT(item->status(), VectorItem::Ready, 10000);
        QCOMPARE(item->frameCount(), 60);
        QVERIFY(item->duration() > 0.9);
        item->play();
        QTRY_VERIFY(item->progress() > 0.1);
        item->pause();
        const auto progress = item->progress();
        QTest::qWait(100);
        QCOMPARE(item->progress(), progress);
        item->setProgress(0.5);
        QCOMPARE(item->currentFrame(), 29.5);
        item->setLoops(1);
        QSignalSpy finished(item, &VectorItem::finished);
        item->play();
        QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 4000);
        QVERIFY(!item->playing());
        QCOMPARE(item->progress(), 1.0);
        item->stop();
        QCOMPARE(item->progress(), 0.0);
        item->setFormat(VectorItem::Canvas);
        item->setViewBox({0, 0, 100, 100});
        item->setShapes({QVariantMap{
            {"type", "rectangle"},
            {"width", 100},
            {"height", 100},
            {"fill",
             QVariantMap{{"type", "linear"},
                         {"x1", 0},
                         {"y1", 0},
                         {"x2", 100},
                         {"y2", 0},
                         {"stops", QVariantList{QVariantMap{{"offset", 0}, {"color", "blue"}},
                                                QVariantMap{{"offset", 1}, {"color", "red"}}}}}}}});
        QTRY_COMPARE(item->status(), VectorItem::Ready);
        image = window.grabWindow();
        const auto left = image.pixelColor(image.width() / 4, image.height() / 2);
        const auto right = image.pixelColor(3 * image.width() / 4, image.height() / 2);
        QVERIFY(left.blue() > left.red());
        QVERIFY(right.red() > right.blue());
        item->setShapes({QVariantMap{
            {"type", "path"},
            {"commands", QVariantList{QVariantList{"M", 10, 10}, QVariantList{"L", 90, 10},
                                      QVariantList{"L", 50, 90}, QVariantList{"Z"}}},
            {"fill", "lime"}}});
        QTRY_COMPARE(item->status(), VectorItem::Ready);
        image = window.grabWindow();
        QVERIFY(image.pixelColor(image.width() / 2, image.height() / 2).green() > 200);
        item->setShapes({QVariantMap{
            {"type", "ellipse"},
            {"width", 100},
            {"height", 100},
            {"fill", QVariantMap{{"type", "radial"},
                                 {"cx", 50},
                                 {"cy", 50},
                                 {"radius", 50},
                                 {"stops",
                                  QVariantList{QVariantMap{{"offset", 0}, {"color", "white"}},
                                               QVariantMap{{"offset", 1}, {"color", "blue"}}}}}}}});
        QTRY_COMPARE(item->status(), VectorItem::Ready);
        image = window.grabWindow();
        QVERIFY(image.pixelColor(image.width() / 2, image.height() / 2).red() > 200);
        auto *other = new QQuickWindow;
        other->resize(200, 160);
        other->show();
        item->setParentItem(other->contentItem());
        item->setSize({200, 160});
        item->reload();
        QTRY_COMPARE(item->status(), VectorItem::Ready);
        QVERIFY(!other->grabWindow().isNull());
        delete other; // Item, renderer and canvas must retire safely before the QRhi.
    }
};
QTEST_MAIN(VectorTest)
#include "tst_vector.moc"
