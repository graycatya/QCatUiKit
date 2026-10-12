// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatledpreview.h>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtTest>
#include <cmath>
#ifdef QCAT_TEST_LED_DEMO
#include "galleryleddemo.h"
#endif

class LedRenderTests : public QObject
{
    Q_OBJECT
  private slots:
    void dragInsideScrollingPage_data()
    {
        QTest::addColumn<QString>("scroller");
        QTest::addColumn<QString>("flickable");
        QTest::newRow("flickable") << QStringLiteral("Flickable { id: page; anchors.fill: parent; "
                                                     "contentWidth: width; contentHeight: 1200;")
                                   << QStringLiteral("page");
        QTest::newRow("gallery-scroll-view")
            << QStringLiteral("CatScrollView { id: page; anchors.fill: parent; "
                              "contentWidth: availableWidth; contentHeight: 1200;")
            << QStringLiteral("page.contentItem as Flickable");
    }
    void dragInsideScrollingPage()
    {
        QFETCH(QString, scroller);
        QFETCH(QString, flickable);
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent factory(&engine);
        factory.setData(QString(R"(
            import QtQuick
            import QCatUiKit
            Window {
                width: 360; height: 300; visible: true
                property Flickable pageFlickable: %2
                property real scrollY: pageFlickable.contentY
                %1
                    Item {
                        width: 360; height: 1200
                        LedPreview {
                            objectName: "preview"; x: 20; y: 20; width: 240; height: 220
                            showStatistics: false; pointSize: 30
                        }
                    }
                }
            }
        )")
                            .arg(scroller, flickable)
                            .toUtf8(),
                        QUrl());
        QScopedPointer<QObject> root(factory.create());
        QVERIFY2(root, qPrintable(factory.errorString()));
        auto *window = qobject_cast<QQuickWindow *>(root.data());
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto *preview = root->findChild<QCatLedPreview *>("preview");
        QVERIFY(preview);
        QCatLedSource source;
        source.setLayout({{-0.5f, 0, 0.5f}, {0.5f, 0, -0.5f}});
        QVERIFY(source.submitFrame(QByteArray::fromHex("00ff00ffff0000ff"), 1));
        preview->setSource(&source);
        QTRY_COMPARE(preview->renderedFrameId(), quint64(1));
        const auto initial = window->grabWindow();
        // Multiple moves exceed the flick threshold and must remain with the preview.
        for (const auto button : {Qt::LeftButton, Qt::MiddleButton})
        {
            QTest::mousePress(window, button, Qt::NoModifier, {100, 180});
            for (int step = 1; step <= 6; ++step)
                QTest::mouseMove(window, {100 + step * 8, 180 - step * 15}, 20);
            QTest::mouseRelease(window, button, Qt::NoModifier, {148, 90});
            QTest::qWait(80);
            QCOMPARE(window->property("scrollY").toReal(), 0.0);
            QVERIFY(window->grabWindow() != initial);
            preview->resetView();
            QTest::qWait(80);
            QCOMPARE(window->grabWindow(), initial);
        }
        QTest::wheelEvent(window, {140, 130}, {0, 120});
        QTest::qWait(80);
        QCOMPARE(window->property("scrollY").toReal(), 0.0);
        QVERIFY(window->grabWindow() != initial);
        // Dragging outside the preview must still scroll the enclosing page.
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, {300, 240});
        for (int step = 1; step <= 6; ++step)
            QTest::mouseMove(window, {300, 240 - step * 15}, 20);
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, {300, 150});
        QTRY_VERIFY(window->property("scrollY").toReal() > 50);
    }
    void framesCameraAndLifecycle()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QCAT_QML_IMPORT_PATH));
        QQmlComponent factory(&engine);
        factory.setData(R"(
            import QtQuick
            import QCatUiKit
            Window {
                width: 600; height: 240; visible: true; color: "black"
                LedPreview { objectName: "a"; width: 200; height: 240; showStatistics: false; pointSize: 30 }
                LedPreview { objectName: "output"; x: 200; width: 200; height: 240; showStatistics: false; pointSize: 30 }
                LedPreview { objectName: "b"; x: 400; width: 200; height: 240; showStatistics: false; pointSize: 30 }
            }
        )",
                        QUrl());
        QScopedPointer<QObject> root(factory.create());
        QVERIFY2(root, qPrintable(factory.errorString()));
        auto *window = qobject_cast<QQuickWindow *>(root.data());
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto *a = root->findChild<QCatLedPreview *>("a");
        auto *output = root->findChild<QCatLedPreview *>("output");
        auto *b = root->findChild<QCatLedPreview *>("b");
        QVERIFY(a && output && b);
        QCatLedSource source;
        source.setLayout({{0, 0, 0.5f}, {0, 0, -0.5f}});
        // Near green, far red: verify depth writes even when far point is submitted last.
        QVERIFY(source.submitFrame(QByteArray::fromHex("00ff00ffff0000ff"), 1));
        for (auto *item : {a, output, b})
            item->setSource(&source);
        QTRY_VERIFY(a->renderedFrameId() == 1 && output->renderedFrameId() == 1 &&
                    b->renderedFrameId() == 1);
        QCOMPARE(a->renderError(), QString());
        auto image = window->grabWindow();
        QVERIFY(!image.isNull());
        const qreal dpr = image.width() / qreal(window->width());
        auto pixel = [&](const QImage &im, int x, int y)
        { return im.pixelColor(qRound(x * dpr), qRound(y * dpr)); };
        for (int x : {100, 300, 500})
        {
            const QColor c = pixel(image, x, 120);
            QVERIFY2(c.green() > 200 && c.red() < 20, qPrintable(c.name()));
        }
        // Constant 30 logical pixel diameter, including at a scaled device pixel ratio.
        QVERIFY(pixel(image, 114, 120).green() > 200);
        QVERIFY(pixel(image, 116, 120).green() < 100);
        QVERIFY(pixel(image, 112, 132).green() < 100); // discarded quad corner
        output->setPreviewBrightness(0.25);
        QTest::qWait(80);
        auto dim = window->grabWindow();
        QVERIFY(pixel(dim, 300, 120).green() < 100);
        QVERIFY(pixel(dim, 100, 120).green() > 200);
        QVERIFY(source.submitFrame(QByteArray::fromHex("0000ffffff0000ff"), 2));
        QTRY_COMPARE(a->renderedFrameId(), quint64(2));
        image = window->grabWindow();
        QVERIFY(pixel(image, 100, 120).blue() > 200);
        // Change camera and check the actual rendered image, then reset restores it.
        a->orbit(100, 50);
        a->pan(15, 10);
        a->zoom(2);
        QTest::qWait(80);
        QVERIFY(window->grabWindow() != image);
        a->resetView();
        QTest::qWait(80);
        QCOMPARE(window->grabWindow(), image);
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, {80, 80});
        QTest::mouseMove(window, {130, 105}, 20);
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, {130, 105});
        QTest::qWait(80);
        QVERIFY(window->grabWindow() != image);
        QTest::mouseDClick(window, Qt::LeftButton, Qt::NoModifier, {80, 80});
        QTest::qWait(80);
        QCOMPARE(window->grabWindow(), image);
        QTest::mousePress(window, Qt::MiddleButton, Qt::NoModifier, {80, 80});
        QTest::mouseMove(window, {110, 90}, 20);
        QTest::mouseRelease(window, Qt::MiddleButton, Qt::NoModifier, {110, 90});
        QTest::qWait(80);
        QVERIFY(window->grabWindow() != image);
        a->resetView();
        a->orbit(100, 0);
        QTest::qWait(80);
        const auto beforeWheel = window->grabWindow();
        QTest::wheelEvent(window, {100, 120}, {0, 120});
        QTest::qWait(80);
        QVERIFY(window->grabWindow() != beforeWheel);
        a->resetView();
        source.setLayout({{-1, -0.5f, 0}, {1, 0.5f, 0}});
        QByteArray ownedFrame = QByteArray::fromHex("00ff00ff0000ffff");
        QVERIFY(source.submitFrame(ownedFrame, 1));
        ownedFrame.fill('\0'); // producer can immediately reuse memory
        a->setViewMode(QCatLedPreview::Plane2D);
        QTest::qWait(80);
        const auto flat = window->grabWindow();
        a->orbit(100, 50);
        QTest::qWait(50);
        QCOMPARE(window->grabWindow(), flat);
        a->pan(12, 8);
        a->zoom(-2);
        QTest::qWait(80);
        QVERIFY(window->grabWindow() != flat);
        a->resetView();
        QTest::qWait(80);
        QCOMPARE(window->grabWindow(), flat);
        a->setViewMode(QCatLedPreview::Spatial3D);
        source.setLayout({});
        QTest::qWait(80);
        QVERIFY(pixel(window->grabWindow(), 100, 120).blue() < 100);
        a->setBackgroundColor(QColor(255, 0, 0, 128));
        QTest::qWait(80);
        const QColor transparentClear = pixel(window->grabWindow(), 100, 120);
        QVERIFY(transparentClear.red() >= 126 && transparentClear.red() <= 130);
        a->setBackgroundColor(QColor("#090d15"));
        source.setLayout({{5, 5, 5}});
        QVERIFY(source.submitFrame(QByteArray::fromHex("ffffffff"), 1));
        QTRY_COMPARE(a->renderedFrameId(), quint64(1));
        source.setLayout({{5, 5, 5}, {5, 5, 5}});
        QVERIFY(source.submitFrame(QByteArray::fromHex("ffffffffffffffff"), 1));
        QTest::qWait(80);
        QVERIFY(pixel(window->grabWindow(), 100, 120).red() > 200);
        source.setLayout({{1e12f, 1e12f, 1e12f}, {1e12f, 1e12f, 1e12f}});
        QVERIFY(source.submitFrame(QByteArray::fromHex("ffffffffffffffff"), 1));
        QTest::qWait(80);
        QVERIFY(pixel(window->grabWindow(), 100, 120).red() > 200);
        a->setFixedColorBufferWidth(400);
        a->setFixedColorBufferHeight(120);
        QTest::qWait(80);
        const auto fixed = window->grabWindow();
        QVERIFY(pixel(fixed, 113, 120).red() > 200);
        QVERIFY(pixel(fixed, 100, 132).red() > 200);
        QVERIFY(pixel(fixed, 118, 120).red() < 100);
        QVERIFY(pixel(fixed, 100, 138).red() < 100);
        int widthX = 0, widthY = 0;
        for (int offset = -25; offset <= 25; ++offset)
        {
            widthX += pixel(fixed, 100 + offset, 120).red() > 128;
            widthY += pixel(fixed, 100, 120 + offset).red() > 128;
        }
        // Allow resampling at the low backing resolution, but reject stretched ellipses.
        QVERIFY2(
            widthX >= 28 && widthX <= 32 && widthY >= 28 && widthY <= 32 &&
                std::abs(widthX - widthY) <= 2,
            qPrintable(QString("Fixed texture LED diameter: %1 x %2").arg(widthX).arg(widthY)));
        a->setFixedColorBufferWidth(0);
        a->setFixedColorBufferHeight(0);
        window->resize(650, 280);
        a->setWidth(220);
        a->setHeight(260);
        a->setSampleCount(4);
        QTest::qWait(100);
        QVERIFY(a->renderError().isEmpty());
        QVERIFY(!window->grabWindow().isNull());
        // Destroy/recreate a control while the same data source stays alive.
        delete b;
        auto *replacement = new QCatLedPreview(window->contentItem());
        replacement->setPosition({400, 0});
        replacement->setSize({200, 240});
        replacement->setSource(&source);
        QTRY_COMPARE(replacement->renderedFrameId(), quint64(1));
        // Reparent into a different window/QRhi; old GPU resources must be discarded.
        QQuickWindow second;
        second.resize(240, 260);
        replacement->setParentItem(second.contentItem());
        replacement->setPosition({0, 0});
        second.show();
        QVERIFY(QTest::qWaitForWindowExposed(&second));
        QTest::qWait(100);
        QVERIFY(!second.grabWindow().isNull());
        QVERIFY(replacement->renderError().isEmpty());
        second.setPersistentSceneGraph(false);
        second.setPersistentGraphics(false);
        replacement->pan(20, 10);
        replacement->zoom(1);
        QTest::qWait(80);
        const auto beforeRebuild = second.grabWindow();
        second.hide();
        second.releaseResources();
        QTest::qWait(100);
        second.show();
        QVERIFY(QTest::qWaitForWindowExposed(&second));
        QTest::qWait(100);
        QVERIFY(!second.grabWindow().isNull());
        QCOMPARE(second.grabWindow(), beforeRebuild);
        replacement->setSource(nullptr);
        QTest::qWait(50);
        delete replacement;
    }
#ifdef QCAT_TEST_LED_DEMO
    void generatedFramesAndMix()
    {
        GalleryLedDemo demo;
        demo.setRunning(false);
        demo.setPointCount(1);
        QQuickWindow window;
        window.resize(600, 200);
        QCatLedPreview a(window.contentItem()), output(window.contentItem()),
            b(window.contentItem());
        int x = 0;
        for (auto *item : {&a, &output, &b})
        {
            item->setPosition({qreal(x), 0});
            item->setSize({200, 200});
            item->setPointSize(30);
            x += 200;
        }
        a.setSource(demo.deckA());
        output.setSource(demo.output());
        b.setSource(demo.deckB());
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(demo.output()->pointCount(), 1);
        for (double mix : {0.0, 1.0, 0.5})
        {
            const auto oldId = demo.output()->frameId();
            demo.setMix(mix);
            QTRY_VERIFY(demo.output()->frameId() > oldId);
            const auto id = demo.output()->frameId();
            QTRY_VERIFY(a.renderedFrameId() == id && output.renderedFrameId() == id &&
                        b.renderedFrameId() == id);
            const auto image = window.grabWindow();
            const qreal dpr = image.width() / qreal(window.width());
            const QColor ca = image.pixelColor(qRound(100 * dpr), qRound(100 * dpr));
            const QColor co = image.pixelColor(qRound(300 * dpr), qRound(100 * dpr));
            const QColor cb = image.pixelColor(qRound(500 * dpr), qRound(100 * dpr));
            for (int c = 0; c < 3; ++c)
            {
                const int channelsA[] = {ca.red(), ca.green(), ca.blue()};
                const int channelsB[] = {cb.red(), cb.green(), cb.blue()};
                const int channelsO[] = {co.red(), co.green(), co.blue()};
                QVERIFY(std::abs(channelsO[c] -
                                 std::lround(channelsA[c] * (1 - mix) + channelsB[c] * mix)) <= 2);
            }
        }
        const auto oldId = demo.output()->frameId();
        demo.setRunning(true);
        QTRY_VERIFY(demo.output()->frameId() > oldId + 3);
        demo.setSpatial(true);
        demo.setPointCount(10000);
        QTRY_COMPARE(demo.output()->pointCount(), 10000);
        QTRY_VERIFY(output.renderedFrameId() > oldId + 3);
        const auto beforePause = demo.output()->frameId();
        demo.setRunning(false);
        QTRY_VERIFY(demo.output()->frameId() > beforePause);
        const auto pausedFrame = demo.output()->frameId();
        QTRY_COMPARE(b.renderedFrameId(), pausedFrame);
        const auto ripples = window.grabWindow();
        const qreal rippleDpr = ripples.width() / qreal(window.width());
        int darkest = 255, brightest = 0;
        for (int y = 60; y <= 140; y += 4)
            for (int x = 460; x <= 540; x += 4)
            {
                const int blue =
                    ripples.pixelColor(qRound(x * rippleDpr), qRound(y * rippleDpr)).blue();
                if (blue > 20)
                {
                    darkest = qMin(darkest, blue);
                    brightest = qMax(brightest, blue);
                }
            }
        QVERIFY2(brightest - darkest > 20, "Sphere ripple pattern must vary across LEDs");
        demo.setPointCount(0);
        QTRY_COMPARE(demo.output()->pointCount(), 0);
        demo.setPointCount(100000);
        QTRY_COMPARE(demo.output()->pointCount(), 100000);
        QVERIFY(demo.output()->lastError().isEmpty());
        demo.setRunning(false);
    }
#endif
};
QTEST_MAIN(LedRenderTests)
#include "tst_ledrender.moc"
