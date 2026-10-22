// SPDX-License-Identifier: Apache-2.0
#include "qcatdocknativedrag_p.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QMimeData>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest>

namespace QCatUiKit
{
class DockNativeDragTest : public QObject
{
    Q_OBJECT
  private slots:
    void platformIsolation();

    void nativeThumbnail();

    void localDropEvents();
};

void DockNativeDragTest::platformIsolation()
{
    DockNativeDrag backend;
    QCOMPARE(backend.enabled(), QGuiApplication::platformName().startsWith("wayland"));
    qInfo() << "QPA:" << QGuiApplication::platformName()
            << "toplevel-drag:" << backend.toplevelDragSupported();
    QVERIFY(!backend.active());
    if (!backend.enabled())
    {
        QQuickWindow window;
        backend.watchWindow(&window, "main");
        QVERIFY(backend.m_windows.isEmpty());
        QVERIFY(!backend.moveWindow(&window));
        backend.start(&window, &window, QPoint());
        QVERIFY(!backend.active());
        QVERIFY(!backend.toplevelDragSupported());
    }
}

void DockNativeDragTest::nativeThumbnail()
{
    DockNativeDrag backend;
    if (!backend.enabled())
        QSKIP("Native thumbnail capture is enabled only by the actual Wayland QPA plugin");
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
            import QtQuick
            Window {
                width: 320; height: 240; visible: true; color: "#808080"
                Rectangle { objectName: "panel"; x: 50; y: 50; width: 120; height: 80; color: "red" }
            })",
                      QUrl());
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object, qPrintable(component.errorString()));
    auto *window = qobject_cast<QQuickWindow *>(object.data());
    QVERIFY(QTest::qWaitForWindowExposed(window));
    auto *panel = window->findChild<QQuickItem *>("panel");
    QVERIFY(panel);
    QTRY_VERIFY(!window->grabWindow().isNull());
    QVERIFY(backend.prepare(panel, "Panel", Qt::white, Qt::black, Qt::blue, 14));
    const auto icon = backend.m_pixmap.toImage();
    const auto dpr = icon.devicePixelRatio();
    QCOMPARE(qRound(icon.width() / dpr), 280);
    // The icon must contain panel pixels, rather than the window background
    // or just a title fallback, before QDrag is started.
    QCOMPARE(icon.pixelColor(qRound(140 * dpr), qRound(90 * dpr)), QColor(Qt::red));
}

void DockNativeDragTest::localDropEvents()
{
    DockNativeDrag backend;
    if (!backend.enabled())
        QSKIP("Native event routing is enabled only by the actual Wayland QPA plugin");
    QQuickWindow window;
    backend.watchWindow(&window, "floating-a");
    backend.watchWindow(&window, "floating-a");
    backend.m_active = true;
    QSignalSpy moved(&backend, &DockNativeDrag::moved);
    QSignalSpy dropped(&backend, &DockNativeDrag::dropped);
    QSignalSpy left(&backend, &DockNativeDrag::left);
    bool accept = true;
    connect(&backend, &DockNativeDrag::moved, &backend,
            [&](const QString &, const QPointF &) { backend.m_acceptsDrop = accept; });
    QMimeData data;
    data.setData("application/x-qcatuikit-dock-panel", backend.m_token);
    QDragEnterEvent enter(QPoint(25, 37), Qt::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &enter);
    QVERIFY(enter.isAccepted());
    QCOMPARE(moved.size(), 1);
    QCOMPARE(moved.last().at(0).toString(), QString("floating-a"));
    QCOMPARE(moved.last().at(1).toPointF(), QPointF(25, 37));
    QDropEvent drop(QPointF(40, 60), Qt::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &drop);
    QVERIFY(drop.isAccepted());
    QCOMPARE(dropped.size(), 1);
    accept = false;
    QDropEvent rejected(QPointF(50, 70), Qt::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &rejected);
    QVERIFY(!rejected.isAccepted());
    QCOMPARE(dropped.size(), 1);
    QMimeData foreign;
    foreign.setData("application/x-qcatuikit-dock-panel", "another-workspace");
    QDragEnterEvent foreignEnter(QPoint(10, 10), Qt::MoveAction, &foreign, Qt::LeftButton,
                                 Qt::NoModifier);
    QVERIFY(!backend.eventFilter(&window, &foreignEnter));
    QCOMPARE(moved.size(), 3);
    QDragLeaveEvent leave;
    QCoreApplication::sendEvent(&window, &leave);
    QCOMPARE(left.size(), 1);
    backend.m_active = false;
}
} // namespace QCatUiKit
QTEST_MAIN(QCatUiKit::DockNativeDragTest)
#include "tst_docknative.moc"
