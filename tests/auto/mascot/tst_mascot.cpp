// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotpackage_p.h"
#include <QCatUiKit/MascotPetController>
#include <QCatUiKit/VectorItem>
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QLoggingCategory>
#include <QMouseEvent>
#include <QMutex>
#include <QProcess>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QScreen>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>
#include <algorithm>
#include <functional>
#include <limits>
using namespace QCatUiKit;
using Controller = MascotPetController;
namespace
{
// Optional X11 input injector: no build dependency on XTest development headers.
class DesktopPointer
{
    QLibrary x11{"X11", 6}, xtst{"Xtst", 6};
    using Open = void *(*)(const char *);
    using Close = int (*)(void *);
    using Flush = int (*)(void *);
    using Motion = int (*)(void *, int, int, int, unsigned long);
    using Button = int (*)(void *, unsigned int, int, unsigned long);
    Open open = reinterpret_cast<Open>(x11.resolve("XOpenDisplay"));
    Close close = reinterpret_cast<Close>(x11.resolve("XCloseDisplay"));
    Flush flush = reinterpret_cast<Flush>(x11.resolve("XFlush"));
    Motion motion = reinterpret_cast<Motion>(xtst.resolve("XTestFakeMotionEvent"));
    Button button = reinterpret_cast<Button>(xtst.resolve("XTestFakeButtonEvent"));
    void *display = nullptr;
    QPoint original = QCursor::pos();

  public:
    DesktopPointer()
    {
        if (open && close && flush && motion && button)
            display = open(nullptr);
    }
    ~DesktopPointer()
    {
        if (display)
        {
            button(display, 1, false, 0);
            motion(display, 0, original.x(), original.y(), 0);
            flush(display);
            close(display);
        }
    }
    bool available() const
    {
        return display;
    }
    void move(QPoint p)
    {
        motion(display, 0, p.x(), p.y(), 0);
        flush(display);
        QTest::qWait(80);
    }
    QPoint windowPoint(QWindow *window, QPoint local)
    {
        using Root = unsigned long (*)(void *);
        using Translate =
            int (*)(void *, unsigned long, unsigned long, int, int, int *, int *, unsigned long *);
        auto root = reinterpret_cast<Root>(x11.resolve("XDefaultRootWindow"));
        auto translate = reinterpret_cast<Translate>(x11.resolve("XTranslateCoordinates"));
        int x = 0, y = 0;
        unsigned long child = 0;
        if (root && translate &&
            translate(display, window->winId(), root(display), local.x(), local.y(), &x, &y,
                      &child))
            return {x, y};
        return window->mapToGlobal(local);
    }
    QSize windowSize(QWindow *window)
    {
        using Geometry = int (*)(void *, unsigned long, unsigned long *, int *, int *,
                                 unsigned int *, unsigned int *, unsigned int *, unsigned int *);
        auto geometry = reinterpret_cast<Geometry>(x11.resolve("XGetGeometry"));
        unsigned long root = 0;
        int x = 0, y = 0;
        unsigned int w = 0, h = 0, b = 0, d = 0;
        if (geometry)
            geometry(display, window->winId(), &root, &x, &y, &w, &h, &b, &d);
        return {int(w), int(h)};
    }
    unsigned long childWindow()
    {
        using Root = unsigned long (*)(void *);
        using Query = int (*)(void *, unsigned long, unsigned long *, unsigned long *, int *, int *,
                              int *, int *, unsigned int *);
        auto root = reinterpret_cast<Root>(x11.resolve("XDefaultRootWindow"));
        auto query = reinterpret_cast<Query>(x11.resolve("XQueryPointer"));
        unsigned long r = 0, c = 0;
        int rx = 0, ry = 0, x = 0, y = 0;
        unsigned int mask = 0;
        if (root && query)
            query(display, root(display), &r, &c, &rx, &ry, &x, &y, &mask);
        return c;
    }
    void press()
    {
        button(display, 1, true, 0);
        flush(display);
        QTest::qWait(60);
    }
    void release()
    {
        button(display, 1, false, 0);
        flush(display);
        QTest::qWait(80);
    }
    void click(QPoint p)
    {
        move(p);
        press();
        release();
    }
};
class InputTarget : public QQuickWindow
{
  public:
    int presses = 0;

  protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        ++presses;
        event->accept();
    }
};
// Scoped capture also observes render/worker threads; restore QtTest's handler on every exit.
class LoadingLog
{
    inline static QMutex mutex;
    inline static LoadingLog *active = nullptr;
    inline static QtMessageHandler previous = nullptr;
    QStringList messages;
    static void handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        {
            QMutexLocker lock(&mutex);
            if (active && QByteArray(context.category).startsWith("qcatuikit."))
                active->messages.append(message);
        }
        if (previous)
            previous(type, context, message);
    }

  public:
    LoadingLog()
    {
        active = this;
        previous = qInstallMessageHandler(handler);
        QLoggingCategory::setFilterRules("qcatuikit.vector.loading.debug=true\n"
                                         "qcatuikit.mascot.loading.debug=true");
    }
    ~LoadingLog()
    {
        QLoggingCategory::setFilterRules(qEnvironmentVariable("QT_LOGGING_RULES"));
        {
            QMutexLocker lock(&mutex);
            active = nullptr;
        }
        qInstallMessageHandler(previous);
    }
    int count(const QString &text)
    {
        QMutexLocker lock(&mutex);
        return int(std::count_if(messages.begin(), messages.end(), [&text](const QString &message)
                                 { return message.contains(text); }));
    }
};
} // namespace
class TestMascot : public QObject
{
    Q_OBJECT
    QJsonObject fixture()
    {
        QFile f(":/mascot-test/data/mascot.json");
        if (!f.open(QIODevice::ReadOnly))
            qFatal("Cannot open embedded mascot fixture");
        return QJsonDocument::fromJson(f.readAll()).object();
    }
    void write(const QString &root, const QJsonObject &obj)
    {
        QFile f(root + "/mascot.json");
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QJsonDocument(obj).toJson());
        f.close();
        QFile::copy(":/mascot-test/data/atlas.png", root + "/atlas.png");
        QFile::setPermissions(root + "/atlas.png",
                              QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }
    QUrl fixtureUrl()
    {
        return QUrl("qrc:/mascot-test/data/mascot.json");
    }
  private slots:
    void publicWindowLifecycle()
    {
        const int initialWindows = QGuiApplication::allWindows().size();
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet { packageSource: "qrc:/mascot-test/data/mascot.json" }
)",
                          QUrl());
        QTRY_VERIFY_WITH_TIMEOUT(component.status() != QQmlComponent::Loading, 5000);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        QVERIFY2(pet, qPrintable(component.errorString()));
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        QVERIFY(window);
        auto *controller = qvariant_cast<Controller *>(pet->property("controller"));
        QVERIFY(controller);
        QTRY_COMPARE(controller->status(), Controller::Ready);
        QVERIFY(!window->isVisible());
        window->show();
        QTRY_VERIFY(window->isVisible());
        window->close();
        QTRY_VERIFY(!window->isVisible());
        QCOMPARE(controller->status(), Controller::Ready);
        const auto caps = pet->property("capabilities").toMap();
        QCOMPARE(caps.value("position").toMap().value("status").toString(), QString("Unavailable"));
        QSignalSpy moved(pet.get(), SIGNAL(moveEnded(QString, QString)));
        QString request;
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "moveTo", Q_RETURN_ARG(QString, request),
                                          Q_ARG(QString, ""), Q_ARG(QPointF, QPointF(20, 20)),
                                          Q_ARG(int, 50)));
        QVERIFY(!request.isEmpty());
        QTRY_COMPARE(moved.count(), 1);
        QCOMPARE(moved.at(0).at(0).toString(), request);
        QCOMPARE(moved.at(0).at(1).toString(), QString("Failed"));
        QVERIFY(pet->metaObject()->indexOfProperty("trayEnabled") < 0);
        QVERIFY(!pet->property("inputTransparent").toBool());
        QVERIFY(!pet->property("inputRegionsEnabled").toBool());
        QVERIFY(pet->property("alwaysOnTop").toBool());
        QVERIFY(!window->transientParent());
        QVERIFY(pet->setProperty("inputTransparent", true));
        QVERIFY(window->flags().testFlag(Qt::WindowTransparentForInput));
        QVERIFY(pet->setProperty("inputTransparent", false));
        QVERIFY(!window->flags().testFlag(Qt::WindowTransparentForInput));
        window->show();
        QVERIFY(window->isVisible());
        pet.reset();
        QTRY_COMPARE(QGuiApplication::allWindows().size(), initialWindows);
    }
    void publicBubbleLifecycle()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    property string testBubble: ""
    function openMessage() { testBubble = showBubble("<b>plain</b>", {durationMs: 80, actions: [{id: "ok", text: "OK"}]}) }
    function replaceMessage() { testBubble = showBubble("replacement") }
    function oldUpdate() { return updateBubble("obsolete", "bad") }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        QVERIFY2(pet, qPrintable(component.errorString()));
        QSignalSpy closed(pet.get(), SIGNAL(bubbleClosed(QString, QString)));
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "openMessage"));
        const auto first = pet->property("bubbleId").toString();
        QVERIFY(!first.isEmpty());
        QCOMPARE(pet->property("bubbleText").toString(), QString("<b>plain</b>"));
        QTest::qWait(120); // Hidden lifetime must freeze the timeout.
        QCOMPARE(pet->property("bubbleId").toString(), first);
        qobject_cast<QQuickWindow *>(pet.get())->show();
        QTRY_COMPARE_WITH_TIMEOUT(closed.count(), 1, 2000);
        QCOMPARE(closed.at(0).at(1).toString(), QString("TimedOut"));
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "openMessage"));
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "replaceMessage"));
        QCOMPARE(closed.count(), 2);
        QCOMPARE(closed.at(1).at(1).toString(), QString("Replaced"));
        QTest::qWait(120);
        QCOMPARE(pet->property("bubbleText").toString(), QString("replacement"));
        QVariant updated;
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "oldUpdate", Q_RETURN_ARG(QVariant, updated)));
        QVERIFY(!updated.toBool());
        QCOMPARE(pet->property("bubbleText").toString(), QString("replacement"));
    }
    void publicBubbleKeyboardLifecycle()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    function openMessage() { showBubble("Keyboard message", {actions: [{id: "reply", text: "Reply"}]}) }
    function closeMessage() { closeBubble(bubbleId) }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        QVERIFY(pet);
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        actor->setPackageSource(fixtureUrl());
        QTRY_COMPARE(actor->status(), Controller::Ready);
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        window->setPosition(QPoint(300, 300));
        QVERIFY(window->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
        QSignalSpy action(pet.get(), SIGNAL(bubbleActionTriggered(QString, QString)));
        const auto activate = [&]
        {
            if (!QMetaObject::invokeMethod(pet.get(), "openMessage"))
                return false;
            // Cocoa's threaded renderer polishes the new bubble asynchronously;
            // its initial implicit button geometry is not a valid input target.
            QTest::qWait(100);
            std::function<QQuickItem *(QQuickItem *)> findReply =
                [&](QQuickItem *item) -> QQuickItem *
            {
                if (item->objectName() == "mascotBubbleAction_reply")
                    return item;
                for (auto *child : item->childItems())
                    if (auto *found = findReply(child))
                        return found;
                return nullptr;
            };
            auto *reply = findReply(window->contentItem());
            if (!reply)
                return false;
            QTest::mouseClick(
                window, Qt::LeftButton, Qt::NoModifier,
                reply->mapToScene(QPointF(reply->width() / 2, reply->height() / 2)).toPoint());
            return true;
        };
        QVERIFY(activate());
        QTRY_COMPARE(action.count(), 1);
        QVERIFY(!window->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
        QTest::keyClick(window, Qt::Key_Space);
        QTRY_COMPARE(action.count(), 2);
        QCOMPARE(action.at(1).at(1).toString(), QString("reply"));
        for (int i = 0; i < 8 && window->activeFocusItem() &&
                        window->activeFocusItem()->objectName() != "mascotBubbleClose";
             ++i)
            QTest::keyClick(window, Qt::Key_Tab, Qt::ShiftModifier);
        QVERIFY(window->activeFocusItem());
        QCOMPARE(window->activeFocusItem()->objectName(), QString("mascotBubbleClose"));
        QTest::keyClick(window, Qt::Key_Space);
        QTRY_VERIFY(pet->property("bubbleId").toString().isEmpty());
        QTRY_VERIFY(window->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
        QVERIFY(activate());
        pet->setProperty("inputTransparent", true);
        QTRY_VERIFY(window->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
        pet->setProperty("inputTransparent", false);
        QVERIFY(activate());
        window->hide();
        QTRY_VERIFY(window->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        QVERIFY(window->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
    }
    void legacySpriteMouseDrag()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QCatUiKit\nCatMascotPet { petScale: 0.5 }",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        QVERIFY(window);
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QVERIFY(actor);
        actor->setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_SAMPLE));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        const QPoint local(48, 52);
        QCOMPARE(actor->hitTest(local, true).value("id").toString(), QString("body"));
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        window->setPosition(300, 300);
        QTest::qWait(100);
        const QPoint before = window->position();
        const QPointF global = window->mapToGlobal(local);
        QSignalSpy started(actor, &Controller::dragStarted);
        QSignalSpy ended(actor, &Controller::dragFinished);
        auto send = [&](QEvent::Type type, QPointF position, QPointF screen, Qt::MouseButton button,
                        Qt::MouseButtons buttons)
        {
            QMouseEvent event(type, position, screen, button, buttons, Qt::NoModifier);
            QCoreApplication::sendEvent(window, &event);
        };
        send(QEvent::MouseButtonPress, local, global, Qt::LeftButton, Qt::LeftButton);
        send(QEvent::MouseMove, local + QPoint(40, 30), global + QPoint(40, 30), Qt::NoButton,
             Qt::LeftButton);
        QCOMPARE(started.count(), 1);
        if (QGuiApplication::platformName() == "xcb")
            QCOMPARE(window->position(), before + QPoint(40, 30));
        send(QEvent::MouseButtonRelease, local, global + QPoint(40, 30), Qt::LeftButton,
             Qt::NoButton);
        QCOMPARE(ended.count(), 1);
        QVERIFY(!actor->dragging());
    }

    void desktopInputRouting()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_DESKTOP") ||
            QGuiApplication::platformName() != "xcb")
            QSKIP("Requires an explicitly selected xcb desktop and XTest");
        DesktopPointer pointer;
        QVERIFY(pointer.available());
        InputTarget behind;
        behind.setGeometry(100, 100, 900, 700);
        behind.show();
        QVERIFY(QTest::qWaitForWindowExposed(&behind));
        if (behind.devicePixelRatio() != 1)
            QSKIP("This native input test requires desktop DPR 1");
        pointer.click(pointer.windowPoint(&behind, QPoint(30, 30)));
        QTRY_COMPARE(behind.presses, 1);
        behind.presses = 0;
        QTemporaryDir dir;
        auto definition = fixture();
        definition["regions"] = QJsonArray{QJsonObject{{"id", "body"},
                                                       {"shape", "rect"},
                                                       {"rect", QJsonArray{1, 1, 2, 3}},
                                                       {"clickable", true},
                                                       {"draggable", true}}};
        auto actions = definition["actions"].toObject();
        auto jump = actions["jump"].toObject();
        jump["regions"] = QJsonArray{QJsonObject{{"id", "jump"},
                                                 {"shape", "ellipse"},
                                                 {"rect", QJsonArray{4, 4, 4, 4}},
                                                 {"clickable", true},
                                                 {"draggable", true}}};
        actions["jump"] = jump;
        definition["actions"] = actions;
        write(dir.path(), definition);
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    petScale: 20
    function message() { return showBubble("Native input message", {actions: [{id: "ok", text: "OK"}]}) }
    function clearMessage() { closeBubble(bubbleId) }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        QVERIFY(window);
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        actor->setPackageSource(QUrl::fromLocalFile(dir.path()));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        actor->pauseActions();
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        window->setPosition(400, 400);
        QTest::qWait(150);
        QSignalSpy activated(actor, &Controller::regionActivated);
        auto click = [&](QPoint local) { pointer.click(pointer.windowPoint(window, local)); };
        click({5, 5}); // Default full-window input, including transparent pixels.
        QCOMPARE(behind.presses, 0);
        QVERIFY(pet->setProperty("inputRegionsEnabled", true));
        QTest::qWait(100);
        click({5, 5});
        QCOMPARE(behind.presses, 1);
        click({40, 40});
        QCOMPARE(behind.presses, 1);
        QCOMPARE(activated.count(), 1);
        actor->setFacing("left");
        QTest::qWait(100);
        click({40, 40});
        QCOMPARE(behind.presses, 2);
        click({120, 40});
        QCOMPARE(activated.count(), 2);
        actor->setFacing("right");
        actor->setPetScale(10);
        QTest::qWait(150);
        click({20, 20});
        QCOMPARE(activated.count(), 3);
        click({40, 40});
        QCOMPARE(behind.presses, 3);
        actor->requestAction("jump", Controller::Replace, {{"loops", -1}});
        QTRY_COMPARE(actor->currentAction(), QString("jump"));
        QTest::qWait(100);
        click({20, 20});
        QCOMPARE(behind.presses, 4);
        click({60, 60});
        QCOMPARE(activated.count(), 4);
        QVERIFY(pet->setProperty("inputTransparent", true));
        QTest::qWait(150);
        click({60, 60});
        QCOMPARE(behind.presses, 5);
        QVERIFY(pet->setProperty("inputTransparent", false));
        QTest::qWait(150);
        click({60, 60});
        QCOMPARE(activated.count(), 5);
        QMetaObject::invokeMethod(pet.get(), "message");
        QTest::qWait(250);
        std::function<QQuickItem *(QQuickItem *)> findButton = [&](QQuickItem *item) -> QQuickItem *
        {
            if (item->objectName() == "mascotBubbleAction_ok")
                return item;
            for (auto *child : item->childItems())
                if (auto *found = findButton(child))
                    return found;
            return nullptr;
        };
        auto *button = findButton(window->contentItem());
        QVERIFY(button);
        QSignalSpy bubbleAction(pet.get(), SIGNAL(bubbleActionTriggered(QString, QString)));
        pointer.click(pointer.windowPoint(
            window,
            button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint()));
        QCOMPARE(bubbleAction.count(), 1);
        QCOMPARE(behind.presses, 5);
        QVERIFY(pet->setProperty("inputTransparent", true));
        QTest::qWait(150);
        pointer.click(pointer.windowPoint(
            window,
            button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint()));
        QCOMPARE(behind.presses, 6);
        QCOMPARE(bubbleAction.count(), 1);
        QVERIFY(pet->setProperty("inputTransparent", false));
        QMetaObject::invokeMethod(pet.get(), "clearMessage");
        QTest::qWait(150);
        click({5, 5});
        QCOMPARE(behind.presses, 7);
        qInfo() << "Native XTest routing passed: default, regions, mirror, scale, action, "
                   "passthrough, bubble";
    }

    void desktopMovementInterruptions()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_DESKTOP") ||
            QGuiApplication::platformName() != "xcb")
            QSKIP("Requires an explicitly selected xcb desktop");
        DesktopPointer pointer;
        QVERIFY(pointer.available());
        // XWayland accepts synthetic pointer warps while the pointer belongs
        // to an X11 surface. Keep an app window behind the non-activating pet.
        InputTarget host;
        host.setGeometry(100, 100, 900, 700);
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));
        pointer.click(pointer.windowPoint(&host, QPoint(30, 30)));
        QTRY_COMPARE(host.presses, 1);
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QCatUiKit\nCatMascotPet { petScale: 0.5 }",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        QVERIFY(window);
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        actor->setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_SAMPLE));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        if (window->devicePixelRatio() != 1)
            QSKIP("This native pointer test requires desktop DPR 1");
        window->setPosition(350, 350);
        QTest::qWait(100);
        actor->pauseActions();
        actor->requestAction("working", Controller::Replace, {{"loops", -1}});
        QTRY_COMPARE(actor->currentAction(), QString("working"));
        QSignalSpy ended(pet.get(), SIGNAL(moveEnded(QString, QString)));
        auto move = [&](QPointF point, int duration = 300)
        {
            QString id;
            const bool invoked = QMetaObject::invokeMethod(
                pet.get(), "moveTo", Q_RETURN_ARG(QString, id), Q_ARG(QString, ""),
                Q_ARG(QPointF, point), Q_ARG(int, duration));
            return invoked ? id : QString();
        };
        auto position = [&](QPointF point)
        {
            bool accepted = false;
            QMetaObject::invokeMethod(pet.get(), "setPosition", Q_RETURN_ARG(bool, accepted),
                                      Q_ARG(QString, ""), Q_ARG(QPointF, point));
            return accepted;
        };
        auto terminal = [&](const QString &id, const QString &reason)
        {
            int count = 0;
            for (const auto &entry : ended)
                if (entry[0].toString() == id)
                {
                    ++count;
                    if (entry[1].toString() != reason)
                        return false;
                }
            return count == 1;
        };
        const auto first = move({450, 400});
        QTest::qWait(50);
        const auto second = move({400, 350}, 80);
        QVERIFY(terminal(first, "Cancelled"));
        QTRY_VERIFY(terminal(second, "Completed"));
        QCOMPARE(actor->currentAction(), QString("working"));
        const auto relocated = move({450, 450});
        QTest::qWait(40);
        QVERIFY(position({350, 350}));
        QVERIFY(terminal(relocated, "Cancelled"));
        const auto cancelled = move({450, 450});
        QMetaObject::invokeMethod(pet.get(), "cancelMove");
        QMetaObject::invokeMethod(pet.get(), "cancelMove");
        QVERIFY(terminal(cancelled, "Cancelled"));
        const auto hidden = move({450, 450});
        window->hide();
        QVERIFY(terminal(hidden, "Cancelled"));
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        const auto screenChange = move({450, 450});
        // Verify the screen-change notification path; actual multi-screen removal is separate.
        QVERIFY(
            QMetaObject::invokeMethod(window, "screenChanged", Q_ARG(QScreen *, window->screen())));
        QVERIFY(terminal(screenChange, "Cancelled"));
        const auto switched = move({450, 450});
        actor->setPackageSource({});
        QVERIFY(terminal(switched, "Cancelled"));
        actor->setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_SAMPLE));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        QVERIFY(position({350, 350}));
        QTest::qWait(200);
        window->raise();
        qInfo() << "Drag fixture" << window->isVisible() << window->isExposed() << window->size()
                << "native" << pointer.windowSize(window) << pointer.windowPoint(window, {0, 0});
        const auto dragged = move({550, 450}, 10000);
        QTRY_VERIFY((
            [&]
            {
                pointer.move(pointer.windowPoint(window, QPoint(48, 52)));
                const bool owned = pointer.childWindow() == window->winId();
                if (!owned)
                    qInfo() << "Pointer mismatch" << pointer.childWindow() << window->winId()
                            << QCursor::pos() << pointer.windowPoint(window, {48, 52})
                            << pointer.windowSize(window);
                return owned;
            })());
        qInfo() << "Before press" << window->position() << window->size() << QCursor::pos()
                << "owner" << pointer.childWindow() << "pet" << window->winId();
        pointer.press();
        pointer.move(QCursor::pos() + QPoint(30, 20));
        qInfo() << "After motion" << window->position() << QCursor::pos() << "owner"
                << pointer.childWindow();
        QTRY_VERIFY(actor->dragging());
        QVERIFY(terminal(dragged, "Cancelled"));
        const auto atThreshold = window->position();
        pointer.move(QCursor::pos() + QPoint(30, 20));
        QVERIFY((window->position() - atThreshold).manhattanLength() >= 10);
        const auto busy = move({450, 450});
        QTRY_VERIFY(terminal(busy, "Failed"));
        QVERIFY(!position({450, 450}));
        pointer.release();
        QTRY_VERIFY(!actor->dragging());
        const auto afterDrag = window->position();
        QTest::qWait(350);
        QCOMPARE(window->position(), afterDrag);
        QVERIFY(terminal(dragged, "Cancelled"));
        QVERIFY(terminal(busy, "Failed"));
        // Hiding mid-drag releases state, even when no native release arrives until later.
        QTRY_VERIFY((
            [&]
            {
                pointer.move(pointer.windowPoint(window, QPoint(48, 52)));
                const bool owned = pointer.childWindow() == window->winId();
                if (!owned)
                    qInfo() << "Pointer mismatch" << pointer.childWindow() << window->winId()
                            << QCursor::pos() << pointer.windowPoint(window, {48, 52})
                            << pointer.windowSize(window);
                return owned;
            })());
        qInfo() << "Before press" << window->position() << window->size() << QCursor::pos()
                << "owner" << pointer.childWindow() << "pet" << window->winId();
        pointer.press();
        pointer.move(QCursor::pos() + QPoint(30, 20));
        qInfo() << "After motion" << window->position() << QCursor::pos() << "owner"
                << pointer.childWindow();
        QTRY_VERIFY(actor->dragging());
        window->hide();
        QVERIFY(!actor->dragging());
        pointer.release();
        QTest::qWait(350);
        for (const auto &id :
             {first, relocated, cancelled, hidden, screenChange, switched, dragged})
            QVERIFY(terminal(id, "Cancelled"));
        qInfo() << "Native movement interruptions passed; screen-change notification simulated";
    }

    void desktopBubbleEdges()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_DESKTOP") ||
            QGuiApplication::platformName() != "xcb")
            QSKIP("Requires an explicitly selected xcb desktop");
        QTest::failOnWarning(QRegularExpression(".*Binding loop.*"));
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    petScale: 0.5
    theme: ThemePalette { id: palette }
    function dark(value) { palette.colorScheme = value ? ThemePalette.Dark : ThemePalette.Light }
    function message() { showBubble("Long wrapped message ".repeat(180), {actions: [{id: "ok", text: "Continue"}]}) }
    function clearMessage() { closeBubble(bubbleId) }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        QVERIFY(window);
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        actor->setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_SAMPLE));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        const auto available = window->screen()->availableGeometry();
        const QPoint corners[] = {
            available.topLeft() + QPoint(2, 2),
            QPoint(available.right() - window->width() - 2, available.top() + 2),
            QPoint(available.left() + 2, available.bottom() - window->height() - 2),
            available.bottomRight() - QPoint(window->width() + 2, window->height() + 2)};
        for (bool dark : {false, true})
        {
            QMetaObject::invokeMethod(pet.get(), "dark", Q_ARG(QVariant, QVariant(dark)));
            for (auto corner : corners)
            {
                window->setPosition(corner);
                QTest::qWait(100);
                QVariant before;
                QVERIFY(QMetaObject::invokeMethod(pet.get(), "exportState",
                                                  Q_RETURN_ARG(QVariant, before)));
                QVERIFY(QMetaObject::invokeMethod(pet.get(), "message"));
                QTest::qWait(250);
                QVariant after;
                QVERIFY(QMetaObject::invokeMethod(pet.get(), "exportState",
                                                  Q_RETURN_ARG(QVariant, after)));
                QCOMPARE(after.toMap().value("position").toPoint(),
                         before.toMap().value("position").toPoint());
                QVERIFY2(available.contains(window->geometry()),
                         qPrintable(QString("Bubble outside screen: %1,%2 %3x%4")
                                        .arg(window->x())
                                        .arg(window->y())
                                        .arg(window->width())
                                        .arg(window->height())));
                const auto image = window->grabWindow();
                QVERIFY(!image.isNull());
                const auto capture = qEnvironmentVariable("QCAT_MASCOT_BUBBLE_CAPTURE");
                if (!capture.isEmpty())
                    QVERIFY(image.save(
                        capture +
                        QString("-%1-%2-%3.png").arg(dark).arg(corner.x()).arg(corner.y())));
                QVERIFY(QMetaObject::invokeMethod(pet.get(), "clearMessage"));
                QTest::qWait(150);
                QVariant closed;
                QVERIFY(QMetaObject::invokeMethod(pet.get(), "exportState",
                                                  Q_RETURN_ARG(QVariant, closed)));
                QCOMPARE(closed.toMap().value("position").toPoint(),
                         before.toMap().value("position").toPoint());
            }
        }
    }

    void independentWindowState()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    packageSource: "qrc:/mascot-test/data/mascot.json"
    function snapshot() { return exportState() }
    function restore(snapshot: var): var { return restoreState(snapshot) }
    function message() { return showBubble("Independent message") }
})", QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> first(component.create()), second(component.create());
        QVERIFY(first && second);
        auto *a = qvariant_cast<Controller *>(first->property("controller"));
        auto *b = qvariant_cast<Controller *>(second->property("controller"));
        QVERIFY(a && b && a != b);
        QTRY_COMPARE(a->status(), Controller::Ready);
        QTRY_COMPARE(b->status(), Controller::Ready);
        a->requestAction("jump", Controller::Replace, {{"loops", -1}});
        a->requestAction("jump", Controller::Enqueue);
        b->requestAction("jump", Controller::Replace, {{"loops", -1}});
        b->requestAction("jump", Controller::Enqueue);
        QTRY_COMPARE(a->queueLength(), 1);
        QTRY_COMPARE(b->queueLength(), 1);
        QVariant bubble;
        QVERIFY(QMetaObject::invokeMethod(second.get(), "message", Q_RETURN_ARG(QVariant, bubble)));
        QVERIFY(!bubble.toString().isEmpty());
        const auto snapshot = [](QObject *pet)
        {
            QVariant value;
            QMetaObject::invokeMethod(pet, "snapshot", Q_RETURN_ARG(QVariant, value));
            return value.toMap();
        };
        const auto restore = [](QObject *pet, const QVariantMap &state)
        {
            QVariant result;
            QMetaObject::invokeMethod(pet, "restore", Q_RETURN_ARG(QVariant, result),
                                      Q_ARG(QVariant, QVariant(state)));
            return result.toMap();
        };
        const auto original = snapshot(second.get());
        QCOMPARE(original.size(), 8);
        QVERIFY(!original.contains("packageSource"));
        QVERIFY(!original.contains("bubbleText"));
        const QList<QPair<QString, QVariant>> invalid{
            {"version", 2}, {"version", "1"}, {"petScale", -1.},
            {"petScale", std::numeric_limits<double>::quiet_NaN()},
            {"position", QPointF(std::numeric_limits<double>::infinity(), 0)},
            {"facing", "up"}, {"screenId", 7}, {"alwaysOnTop", 1},
            {"inputTransparent", "false"}, {"inputRegionsEnabled", 0}};
        for (const auto &field : invalid)
        {
            auto state = original;
            state["petScale"] = 2.;
            state["alwaysOnTop"] = false;
            state[field.first] = field.second;
            QVERIFY2(!restore(second.get(), state).value("ok").toBool(), qPrintable(field.first));
            QCOMPARE(snapshot(second.get()), original);
            QCOMPARE(b->queueLength(), 1);
            QCOMPARE(second->property("bubbleId").toString(), bubble.toString());
        }
        for (const auto &key : original.keys())
        {
            if (key == "inputRegionsEnabled")
                continue; // Optional in older v1 snapshots.
            auto state = original;
            state.remove(key);
            QVERIFY2(!restore(second.get(), state).value("ok").toBool(), qPrintable(key));
            QCOMPARE(snapshot(second.get()), original);
        }
        // A missing screen is recoverable on position-capable QPA backends.
        const auto position = second->property("capabilities").toMap().value("position").toMap();
        if (position.value("status") != "Unavailable")
        {
            auto state = original;
            state["screenId"] = "removed-screen";
            state["position"] = QPointF(24, 32);
            state["petScale"] = 2.;
            state["inputTransparent"] = true;
            const auto result = restore(second.get(), state);
            QVERIFY(result.value("ok").toBool());
            QVERIFY(result.value("corrected").toBool());
            QCOMPARE(b->petScale(), 2.);
            QCOMPARE(second->property("inputTransparent").toBool(), true);
            QVERIFY(snapshot(second.get()).value("screenId") != "removed-screen");
            state = snapshot(second.get());
            state.remove("inputRegionsEnabled");
            QVERIFY(restore(second.get(), state).value("ok").toBool());
            QCOMPARE(second->property("inputRegionsEnabled").toBool(), false);
        }
        // A syntactically valid but unsupported facing must also reject atomically.
        a->setPackageSource(QUrl::fromLocalFile(QStringLiteral(QCAT_MASCOT_SAMPLE)));
        QTRY_COMPARE(a->status(), Controller::Ready);
        const auto legacy = snapshot(first.get());
        auto unsupported = legacy;
        unsupported["facing"] = "left";
        unsupported["petScale"] = 3.;
        unsupported["alwaysOnTop"] = false;
        QVERIFY(!restore(first.get(), unsupported).value("ok").toBool());
        QCOMPARE(snapshot(first.get()), legacy);
        a->setPackageSource(QUrl());
        QCOMPARE(b->status(), Controller::Ready);
        QCOMPARE(b->currentAction(), QString("jump"));
        QCOMPARE(b->queueLength(), 1);
        first.reset();
        QCoreApplication::processEvents();
        QCOMPARE(b->status(), Controller::Ready);
        QCOMPARE(b->queueLength(), 1);
        QCOMPARE(second->property("bubbleId").toString(), bubble.toString());
    }

    void desktopWindowIntegration()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_DESKTOP"))
            QSKIP("Requires an explicitly selected real desktop session");
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    packageSource: "qrc:/mascot-test/data/mascot.json"
    petScale: 4
    function message() { showBubble("Host message <b>plain text</b>", {actions: [{id: "ok", text: "OK"}]}) }
    function invalidState() { const old = exportState(); old.petScale = -1; return restoreState(old).ok }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pet(component.create());
        QVERIFY2(pet, qPrintable(component.errorString()));
        auto *window = qobject_cast<QQuickWindow *>(pet.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QVERIFY(window && actor);
        QTRY_COMPARE(actor->status(), Controller::Ready);
        window->show();
        QVERIFY(QTest::qWaitForWindowExposed(window));
        QVERIFY(!window->transientParent());
        {
            QQuickWindow host;
            host.resize(400, 300);
            host.show();
            QVERIFY(QTest::qWaitForWindowExposed(&host));
            window->setTransientParent(&host);
            QVERIFY(!window->transientParent());
            host.showMinimized();
            QTest::qWait(150);
            QVERIFY(window->isVisible());
            QVERIFY(!(window->windowStates() & Qt::WindowMinimized));
        }
        if (QGuiApplication::platformName() == "xcb" &&
            !QStandardPaths::findExecutable("xprop").isEmpty())
        {
            const auto state = [&]
            {
                QProcess query;
                query.start(QStandardPaths::findExecutable("xprop"),
                            {"-id", QString::number(window->winId()), "_NET_WM_STATE"});
                if (!query.waitForFinished(2000))
                    return QByteArray();
                return query.readAllStandardOutput();
            };
            QTRY_VERIFY_WITH_TIMEOUT(state().contains("_NET_WM_STATE_SKIP_TASKBAR"), 5000);
            QTRY_VERIFY_WITH_TIMEOUT(state().contains("_NET_WM_STATE_ABOVE"), 5000);
            qInfo().noquote() << state();
        }
        const auto cap = pet->property("capabilities").toMap().value("position").toMap();
        const bool limited = cap.value("status").toString() == "Unavailable";
        bool positioned = false;
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "setPosition", Q_RETURN_ARG(bool, positioned),
                                          Q_ARG(QString, ""), Q_ARG(QPointF, QPointF(300, 300))));
        QCOMPARE(positioned, !limited);
        QSignalSpy moved(pet.get(), SIGNAL(moveEnded(QString, QString)));
        QString id;
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "moveTo", Q_RETURN_ARG(QString, id),
                                          Q_ARG(QString, ""), Q_ARG(QPointF, QPointF(340, 320)),
                                          Q_ARG(int, 80)));
        QTRY_COMPARE(moved.count(), 1);
        QCOMPARE(moved.at(0).at(1).toString(), limited ? QString("Failed") : QString("Completed"));
        if (!limited)
        {
            actor->requestAction("jump", Controller::Replace, {{"loops", -1}});
            QTRY_COMPARE(actor->currentAction(), QString("jump"));
            QTest::qWait(150);
            const QPoint stablePosition = window->position();
            const QSize stableSize = window->size();
            QSignalSpy frames(actor, &Controller::spriteFrameChanged);
            QTest::qWait(450);
            QVERIFY(frames.count() >= 3);
            QCOMPARE(window->position(), stablePosition);
            QCOMPARE(window->size(), stableSize);
            actor->resetToDefault();
            QTRY_COMPARE(actor->currentAction(), QString("idle"));
        }
        const qreal scale = actor->petScale();
        QVariant invalid;
        QVERIFY(
            QMetaObject::invokeMethod(pet.get(), "invalidState", Q_RETURN_ARG(QVariant, invalid)));
        QVERIFY(!invalid.toBool());
        QCOMPARE(actor->petScale(), scale);
        QVariant beforeBubble;
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "exportState",
                                          Q_RETURN_ARG(QVariant, beforeBubble)));
        QVERIFY(QMetaObject::invokeMethod(pet.get(), "message"));
        QVERIFY(!pet->property("bubbleId").toString().isEmpty());
        QTRY_VERIFY_WITH_TIMEOUT(window->width() >= 120, 2000);
        QTest::qWait(150);
        qInfo() << "Bubble window size" << window->size() << "content"
                << pet->property("contentWidth") << pet->property("contentHeight") << "id"
                << pet->property("bubbleId") << "minimum" << window->minimumSize() << "maximum"
                << window->maximumSize();
        QVERIFY(window->width() >= 120);
        if (!limited)
        {
            QVariant afterBubble;
            QVERIFY(QMetaObject::invokeMethod(pet.get(), "exportState",
                                              Q_RETURN_ARG(QVariant, afterBubble)));
            qInfo() << "Character origin before/after bubble"
                    << beforeBubble.toMap().value("position").toPoint()
                    << afterBubble.toMap().value("position").toPoint();
            QCOMPARE(afterBubble.toMap().value("position").toPoint(),
                     beforeBubble.toMap().value("position").toPoint());
            QVariant restored;
            QVERIFY(QMetaObject::invokeMethod(pet.get(), "restoreState",
                                              Q_RETURN_ARG(QVariant, restored),
                                              Q_ARG(QVariant, beforeBubble)));
            QVERIFY2(restored.toMap().value("ok").toBool(),
                     qPrintable(restored.toMap().value("reason").toString()));
            const auto count = moved.count();
            QString cancelled;
            QVERIFY(QMetaObject::invokeMethod(pet.get(), "moveTo", Q_RETURN_ARG(QString, cancelled),
                                              Q_ARG(QString, ""), Q_ARG(QPointF, QPointF(500, 400)),
                                              Q_ARG(int, 500)));
            QVERIFY(QMetaObject::invokeMethod(pet.get(), "cancelMove"));
            QCOMPARE(moved.count(), count + 1);
            QCOMPARE(moved.last().at(0).toString(), cancelled);
            QCOMPARE(moved.last().at(1).toString(), QString("Cancelled"));
            QTest::qWait(100);
            QCOMPARE(moved.count(), count + 1);
        }
        if (!qEnvironmentVariable("QCAT_MASCOT_CAPTURE").isEmpty())
            QVERIFY(window->grabWindow().save(qEnvironmentVariable("QCAT_MASCOT_CAPTURE")));
        window->hide();
        QVERIFY(!window->isVisible());
        qInfo() << "Desktop QPA" << QGuiApplication::platformName() << "position limited"
                << limited;
    }
    void cacheLimitConfiguration()
    {
        Controller first, second;
        QCOMPARE(first.actionCacheLimitMiB(), 150);
        QSignalSpy changed(&first, &Controller::actionCacheLimitMiBChanged);
        QSignalSpy rejected(&first, &Controller::configurationRejected);
        first.setActionCacheLimitMiB(300);
        QCOMPARE(first.actionCacheLimitMiB(), 300);
        QCOMPARE(second.actionCacheLimitMiB(), 150);
        QCOMPARE(changed.size(), 1);
        first.setActionCacheLimitMiB(300);
        QCOMPARE(changed.size(), 1);
        first.setActionCacheLimitMiB(-1);
        QCOMPARE(first.actionCacheLimitMiB(), 300);
        QCOMPARE(rejected.size(), 1);
        QCOMPARE(rejected[0][0].toString(), "InvalidActionCacheLimit");
        first.setActionCacheLimitMiB(0);
        QCOMPARE(first.actionCacheLimitMiB(), 0);
        first.setPackageSource(fixtureUrl());
        QTRY_COMPARE(first.status(), Controller::Ready);
        QCOMPARE(first.actionCacheLimitMiB(), 0);
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QCatUiKit\nMascotPetController { actionCacheLimitMiB: 75 }",
                          QUrl());
        QTRY_VERIFY(!component.isLoading());
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        QCOMPARE(object->property("actionCacheLimitMiB").toInt(), 75);
    }
    void packages()
    {
        MascotPrivate::Package p;
        QString error;
        QVERIFY2(MascotPrivate::loadPackage(fixtureUrl(), &p, &error), qPrintable(error));
        QCOMPARE(p.actions.size(), 2);
        QVERIFY2(MascotPrivate::loadPackage(QUrl("qrc:/mascot-test/data/"), &p, &error),
                 qPrintable(error));
        QVERIFY2(MascotPrivate::loadPackage(QUrl("qrc:/mascot.json"), &p, &error),
                 qPrintable(error));
        QCOMPARE(p.actions.value("jump").directions.value("right").startFrame, 1);
        QTemporaryDir dir;
        write(dir.path(), fixture());
        QVERIFY2(MascotPrivate::loadPackage(QUrl::fromLocalFile(dir.path()), &p, &error),
                 qPrintable(error));
        QVERIFY(MascotPrivate::loadPackage(QUrl::fromLocalFile(dir.filePath("mascot.json")), &p,
                                           &error));
        QVERIFY2(MascotPrivate::loadPackage(QUrl::fromLocalFile(QCAT_MASCOT_SAMPLE), &p, &error),
                 qPrintable(error));
        QCOMPARE(p.actions.size(), 10);
        QCOMPARE(p.logicalSize, QSize(192, 208));
        QCOMPARE(p.footAnchor, QPointF(96, 191.36));
        QCOMPARE(p.actions.value("jump").loops, 1);
        QCOMPARE(p.actions.value("working").directions.value("right").startFrame, 56);
    }
    void invalid_data()
    {
        QTest::addColumn<QString>("kind");
        for (const QString &s :
             {"version", "size", "default", "escape", "symlink", "remote", "range", "fps", "loop",
              "region", "missing", "decode", "oversized"})
            QTest::newRow(qPrintable(s)) << s;
    }
    void invalid()
    {
        QFETCH(QString, kind);
        QTemporaryDir dir, outside;
        auto obj = fixture();
        auto actions = obj["actions"].toObject();
        auto idle = actions["idle"].toObject();
        auto right = idle["right"].toObject();
        if (kind == "version")
            obj["schemaVersion"] = 2;
        if (kind == "size")
            obj["logicalSize"] = QJsonArray{0, 8};
        if (kind == "default")
            obj["defaultAction"] = "unknown";
        if (kind == "escape")
            right["source"] = "../atlas.png";
        if (kind == "remote")
            right["source"] = "https://example.org/atlas.png";
        if (kind == "missing")
            right["source"] = "missing.png";
        if (kind == "range")
            right["frameCount"] = 100;
        if (kind == "fps")
            right["fps"] = 0;
        if (kind == "loop")
            idle["loops"] = 0;
        if (kind == "region")
            obj["regions"] = QJsonArray{QJsonObject{
                {"id", "body"}, {"shape", "ellipse"}, {"rect", QJsonArray{0, 0, -1, 5}}}};
        if (kind == "symlink")
        {
            QVERIFY(QFile::copy(":/mascot-test/data/atlas.png", outside.filePath("outside.png")));
            QVERIFY(QFile::link(outside.filePath("outside.png"), dir.filePath("link.png")));
            right["source"] = "link.png";
        }
        idle["right"] = right;
        actions["idle"] = idle;
        obj["actions"] = actions;
        write(dir.path(), obj);
        if (kind == "decode")
        {
            QFile f(dir.filePath("atlas.png"));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("bad png");
        }
        if (kind == "oversized")
        {
            QFile f(dir.filePath("mascot.json"));
            QVERIFY(f.open(QIODevice::Append));
            f.write(QByteArray(1024 * 1024, ' '));
        }
        MascotPrivate::Package p;
        QString error;
        QVERIFY(!MascotPrivate::loadPackage(QUrl::fromLocalFile(dir.path()), &p, &error));
        QVERIFY(!error.isEmpty());
    }
    void lottieExplicitError()
    {
        QTemporaryDir dir;
        auto obj = fixture();
        obj["renderer"] = "lottie";
        obj["actions"] = QJsonObject{
            {"idle", QJsonObject{{"loops", -1}, {"right", QJsonObject{{"source", "idle.json"}}}}}};
        write(dir.path(), obj);
        QFile animation(dir.filePath("idle.json"));
        QVERIFY(animation.open(QIODevice::WriteOnly));
        animation.write("{}");
        animation.close();
        Controller c;
        QSignalSpy ended(&c, &Controller::actionEnded);
        c.setPackageSource(QUrl::fromLocalFile(dir.path()));
        QQuickWindow window;
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        item->setParentItem(window.contentItem());
        item->setSize({80, 80});
        window.resize(80, 80);
        window.show();
        QTRY_COMPARE(c.status(), Controller::Error);
        QCOMPARE(c.errorCode(), "RenderFailed");
        QVERIFY(c.spriteFrame().isEmpty());
        QCOMPARE(ended.size(), 0);
        QVERIFY(animation.open(QIODevice::WriteOnly));
        animation.write(
            QJsonDocument(QJsonObject{{"assets", QJsonArray{QJsonObject{
                                                     {"p", "https://example.org/image.png"}}}}})
                .toJson());
        animation.close();
        MascotPrivate::Package package;
        QString error;
        QVERIFY(!MascotPrivate::loadPackage(QUrl::fromLocalFile(dir.path()), &package, &error));
        QVERIFY(!error.isEmpty());
    }
    void lottieRendering()
    {
        Controller c;
        c.setPackageSource(QUrl::fromLocalFile(QStringLiteral(QCAT_MASCOT_LOTTIE_SAMPLE)));
        QQuickWindow window;
        window.setColor(Qt::white);
        window.resize(192, 208);
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        item->setParentItem(window.contentItem());
        item->setSize({192, 208});
        window.show();
        auto *vector = item->findChild<VectorItem *>();
        QVERIFY(vector);
        for (auto *child : item->findChildren<QObject *>())
            QVERIFY(!QByteArray(child->metaObject()->className()).contains("CatBusyIndicator"));
        QSignalSpy ended(&c, &Controller::actionEnded);
        const bool canRender =
            vector->backendAvailable() &&
            window.rendererInterface()->graphicsApi() != QSGRendererInterface::Software;
        if (qEnvironmentVariableIsSet("QCAT_MASCOT_GPU"))
            QVERIFY2(canRender, "QCAT_MASCOT_GPU requires the ThorVG backend and native QRhi");
        if (!canRender)
        {
            // The complete real package is validated before renderer failure is reported.
            QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Error, 30000);
            QCOMPARE(c.errorCode(), "RenderFailed");
            QVERIFY(!c.errorString().isEmpty());
            QCOMPARE(ended.size(), 0);
            auto result = item->grabToImage();
            QVERIFY(result);
            QSignalSpy ready(result.data(), &QQuickItemGrabResult::ready);
            if (result->image().isNull())
                QVERIFY(ready.wait(2000));
            QVERIFY(result->image().pixelColor(96, 100).alpha() > 0);
            return;
        }
        QVERIFY(window.rendererInterface()->graphicsApi() != QSGRendererInterface::Software);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QVERIFY(!vector->autoPlay());
        QTRY_VERIFY(vector->progress() > 0.05);
        const auto rendered = window.grabWindow();
        QVERIFY(!rendered.isNull());
        QVERIFY(rendered.pixelColor(96, 70).red() < 180);
        c.pauseActions();
        const qreal progress = vector->progress();
        QTest::qWait(150);
        QCOMPARE(vector->progress(), progress);
        c.resumeActions();
        window.hide();
        QTRY_VERIFY(!c.playbackEnabled());
        const qreal hidden = vector->progress();
        QTest::qWait(150);
        QCOMPARE(vector->progress(), hidden);
        window.show();
        QTRY_VERIFY(c.playbackEnabled());
        const QString id = c.requestAction("working", Controller::Replace, {{"loops", 1}});
        QTRY_COMPARE(c.currentAction(), "working");
        QTRY_COMPARE_WITH_TIMEOUT(ended.size(), 1, 5000);
        QCOMPARE(ended[0][0].toString(), id);
        QCOMPARE(ended[0][1].toString(), "Completed");
        QTRY_COMPARE(c.currentAction(), "idle");
        QTest::qWait(100);
        QCOMPARE(ended.size(), 1);
        c.setDragAction("jump");
        QVERIFY(c.beginDrag());
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.requestAction("idle");
        QSignalSpy rejected(&c, &Controller::requestRejected);
        QTRY_COMPARE(rejected.size(), 1);
        QCOMPARE(rejected[0][1].toString(), "InteractionBusy");
        QTest::qWait(2200);
        QCOMPARE(c.currentAction(), "jump");
        QCOMPARE(ended.size(), 1);
        c.endDrag();
        QTRY_COMPARE(c.currentAction(), "idle");
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.requestAction("working");
        c.requestAction("working", Controller::Enqueue);
        QTRY_COMPARE(c.queueLength(), 1);
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QCOMPARE(c.currentAction(), "idle");
        QTest::qWait(100);
        QCOMPARE(c.status(), Controller::Ready);
        QCOMPARE(ended.size(), 3);
    }
    void actionCacheAndLatency()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_GPU"))
            QSKIP("Set QCAT_MASCOT_GPU=1 to enable optional native QRhi action cache checks");
        LoadingLog log;
        Controller c;
        c.setActionCacheLimitMiB(40);
        QQuickWindow window;
        window.setColor(Qt::white);
        window.resize(192, 208);
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        item->setParentItem(window.contentItem());
        item->setSize({192, 208});
        window.show();
        auto *vector = item->findChild<VectorItem *>();
        QVERIFY(vector);
        const auto package = QUrl::fromLocalFile(QStringLiteral(QCAT_MASCOT_LOTTIE_SAMPLE));
        QElapsedTimer timer;
        timer.start();
        c.setPackageSource(package);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        const auto coldMs = timer.elapsed();
        QTRY_VERIFY_WITH_TIMEOUT(log.count("mascot prewarm finished") >= 2, 30000);
        const auto switchTo = [&](const QString &name) -> qint64
        {
            timer.restart();
            c.requestAction(name, Controller::Replace, {{"loops", -1}});
            do
                QTest::qWait(1);
            while ((c.currentAction() != name || c.status() == Controller::Loading) &&
                   timer.elapsed() < 30000);
            return timer.elapsed();
        };
        const auto prewarmMs = switchTo("working");
        QCOMPARE(c.status(), Controller::Ready);
        QVERIFY(log.count("mascot cache hit prewarm") >= 1);
        const int decodes = log.count("Lottie CPU decode ms");
        QList<qint64> samples;
        for (int i = 0; i < 20; ++i)
        {
            if (i == 10)
            {
                window.resize(384, 416);
                item->setSize({384, 416});
            }
            samples.append(switchTo(i % 2 ? "working" : "idle"));
            QCOMPARE(c.status(), Controller::Ready);
            QVERIFY(vector->progress() < 0.1);
            const auto frame = window.grabWindow();
            QVERIFY(!frame.isNull());
            int occupied = 0;
            for (int y = 0; y < frame.height(); ++y)
                for (int x = 0; x < frame.width(); ++x)
                    occupied += frame.pixelColor(x, y) != QColor(Qt::white);
            QVERIFY(occupied > 1000);
            if (i == 19 && qEnvironmentVariableIsSet("QCAT_MASCOT_SCREENSHOT"))
                QVERIFY(frame.save(qEnvironmentVariable("QCAT_MASCOT_SCREENSHOT")));
        }
        QCOMPARE(log.count("Lottie CPU decode ms"), decodes);
        QVERIFY(log.count("mascot cache hit resident") >= 20);
        std::sort(samples.begin(), samples.end());
        qInfo() << "action cache latency ms: cold package" << coldMs << "prewarm" << prewarmMs
                << "resident n" << samples.size() << "P50" << samples[9] << "P95" << samples[18];
        switchTo("thinking"); // the total source-byte budget forces an eviction
        QCOMPARE(c.status(), Controller::Ready);
        QVERIFY(log.count("mascot cache evict") >= 1);
        const auto beforeReload = log.count("Lottie CPU decode ms");
        vector->reload();
        QCOMPARE(vector->status(), VectorItem::Loading);
        QTRY_COMPARE_WITH_TIMEOUT(vector->status(), VectorItem::Ready, 30000);
        QVERIFY(log.count("Lottie CPU decode ms") > beforeReload);
        const auto priorDecodes = log.count("Lottie CPU decode ms");
        c.setPackageSource({});
        QCOMPARE(c.status(), Controller::Null);
        c.setPackageSource(package);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QVERIFY(log.count("Lottie CPU decode ms") > priorDecodes);
        // Destroy the visual while speculative preparation may still be running.
        object.reset();
        QTest::qWait(100);
    }
    void configurableActionCache()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_GPU"))
            QSKIP("Set QCAT_MASCOT_GPU=1 to enable optional native QRhi cache budget checks");
        LoadingLog log;
        Controller c;
        QCOMPARE(c.actionCacheLimitMiB(), 150);
        QQuickWindow window;
        window.setColor(Qt::white);
        window.resize(192, 208);
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        item->setParentItem(window.contentItem());
        item->setSize({192, 208});
        window.show();
        c.setPackageSource(QUrl::fromLocalFile(QStringLiteral(QCAT_MASCOT_LOTTIE_SAMPLE)));
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        c.pauseActions(); // background preparation must continue with a frozen playback clock
        const auto actions = c.availableActions();
        QCOMPARE(actions.size(), 10);
        QTRY_COMPARE_WITH_TIMEOUT(log.count("mascot prewarm finished"), 9, 60000);
        const auto decodes = log.count("Lottie CPU decode ms");
        const auto switchTo = [&](const QString &name)
        { c.requestAction(name, Controller::Replace, {{"loops", -1}}); };
        for (int i = 1; i <= 20; ++i)
        {
            // A reproducible permutation crosses all ten actions twice.
            const auto name = actions[(i * 7) % actions.size()];
            switchTo(name);
            QTRY_COMPARE_WITH_TIMEOUT(c.currentAction(), name, 30000);
            QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        }
        QCOMPARE(log.count("Lottie CPU decode ms"), decodes);
        QCOMPARE(log.count("mascot cache evict"), 0);
        QVERIFY(log.count("mascot cache hit resident") >= 10);
        qInfo() << "150 MiB cache: all ten actions reused, no repeated CPU decoding";
        const auto hits = log.count("mascot cache hit");
        c.setActionCacheLimitMiB(1);
        QTRY_VERIFY(log.count("mascot cache evict") > 0);
        switchTo(actions[1]);
        QTRY_COMPARE(c.currentAction(), actions[1]);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QVERIFY(log.count("Lottie CPU decode ms") > decodes);
        QCOMPARE(log.count("mascot cache hit"), hits);
        c.setActionCacheLimitMiB(0);
        const auto disabledDecodes = log.count("Lottie CPU decode ms");
        switchTo(actions[2]);
        QTRY_COMPARE(c.currentAction(), actions[2]);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QVERIFY(log.count("Lottie CPU decode ms") > disabledDecodes);
        QCOMPARE(log.count("mascot cache hit"), hits);
        const auto warmedBeforeIncrease = log.count("mascot prewarm finished");
        c.setActionCacheLimitMiB(150);
        QTRY_COMPARE_WITH_TIMEOUT(log.count("mascot prewarm finished"), warmedBeforeIncrease + 9,
                                  60000);
        switchTo(actions[0]);
        QTRY_COMPARE(c.currentAction(), actions[0]);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QVERIFY(log.count("mascot cache hit") > hits);
    }
    void catgrayLottieRendering()
    {
        const QString package = qEnvironmentVariable("QCAT_CATGRAY_LOTTIE_PACKAGE");
        if (package.isEmpty() || !qEnvironmentVariableIsSet("QCAT_MASCOT_GPU"))
            QSKIP("Set QCAT_CATGRAY_LOTTIE_PACKAGE and QCAT_MASCOT_GPU for real IP rendering");
        Controller c;
        c.setPackageSource(QUrl::fromLocalFile(package));
        QQuickWindow window;
        window.setColor(Qt::white);
        window.resize(384, 416);
        c.setPetScale(2);
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        item->setParentItem(window.contentItem());
        item->setSize({384, 416});
        window.show();
        auto *vector = item->findChild<VectorItem *>();
        QVERIFY(vector);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QTRY_VERIFY_WITH_TIMEOUT(vector->progress() > 0.05, 10000);
        c.pauseActions();
        const auto first = window.grabWindow();
        QVERIFY(!first.isNull());
        int colored = 0;
        for (int y = 0; y < first.height(); ++y)
            for (int x = 0; x < first.width(); ++x)
                colored += first.pixelColor(x, y) != QColor(Qt::white);
        QVERIFY(colored > 1000);
        const qreal paused = vector->progress();
        QTest::qWait(200);
        QCOMPARE(vector->progress(), paused);
        const QString alternative = QFileInfo(package).isDir() ? package + "/mascot.json"
                                                               : QFileInfo(package).absolutePath();
        c.setPackageSource(QUrl::fromLocalFile(alternative));
        QTest::qWait(10);
        QVERIFY(item->isVisible());
        const auto retained = window.grabWindow();
        QVERIFY(!retained.isNull());
        int retainedPixels = 0;
        for (int y = 0; y < retained.height(); ++y)
            for (int x = 0; x < retained.width(); ++x)
                retainedPixels += retained.pixelColor(x, y) != QColor(Qt::white);
        QVERIFY2(retainedPixels > 1000, "Package validation must retain the old visual");
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        c.resumeActions();
        QTRY_VERIFY_WITH_TIMEOUT(vector->progress() != paused, 5000);
        const QStringList expected = {"idle",    "thinking", "working", "waiting", "speaking",
                                      "success", "error",    "walk",    "run",     "jump"};
        QCOMPARE(c.availableActions().size(), expected.size());
        for (const auto &action : expected)
        {
            QVERIFY(c.availableActions().contains(action));
            c.requestAction(action, Controller::Replace, {{"loops", -1}});
            QTRY_COMPARE_WITH_TIMEOUT(c.currentAction(), action, 30000);
            QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
            QTRY_VERIFY_WITH_TIMEOUT(vector->progress() > 0.05, 10000);
            c.pauseActions();
            const auto frame = window.grabWindow();
            QVERIFY(!frame.isNull());
            int occupied = 0;
            for (int y = 0; y < frame.height(); ++y)
                for (int x = 0; x < frame.width(); ++x)
                    occupied += frame.pixelColor(x, y) != QColor(Qt::white);
            QVERIFY(occupied > 1000);
            c.resumeActions();
        }
        c.resetToDefault();
        QTRY_COMPARE_WITH_TIMEOUT(c.currentAction(), "idle", 30000);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QSignalSpy ended(&c, &Controller::actionEnded);
        const QString id = c.requestAction("working", Controller::Replace, {{"loops", 1}});
        QTRY_COMPARE_WITH_TIMEOUT(c.currentAction(), "working", 30000);
        QTRY_COMPARE_WITH_TIMEOUT(ended.size(), 1, 30000);
        QCOMPARE(ended[0][0].toString(), id);
        QCOMPARE(ended[0][1].toString(), "Completed");
        QTRY_COMPARE_WITH_TIMEOUT(c.currentAction(), "idle", 30000);
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        // Reloading an alias of the same package must read modified action data,
        // rather than mistakenly reusing the retained CPU scene for that URL.
        QTemporaryDir modified;
        QFile manifest(QStringLiteral(QCAT_MASCOT_LOTTIE_SAMPLE) + "/mascot.json");
        QVERIFY(manifest.open(QIODevice::ReadOnly));
        auto definition = QJsonDocument::fromJson(manifest.readAll()).object();
        definition.remove("fallbackImage");
        definition["actions"] = QJsonObject{{"idle", definition["actions"].toObject()["idle"]}};
        QFile localManifest(modified.filePath("mascot.json"));
        QVERIFY(localManifest.open(QIODevice::WriteOnly));
        localManifest.write(QJsonDocument(definition).toJson());
        localManifest.close();
        QFile original(QStringLiteral(QCAT_MASCOT_LOTTIE_SAMPLE) + "/idle.json");
        QVERIFY(original.open(QIODevice::ReadOnly));
        QByteArray data = original.readAll();
        QFile action(modified.filePath("idle.json"));
        QVERIFY(action.open(QIODevice::WriteOnly));
        action.write(data);
        action.close();
        c.setPackageSource(QUrl::fromLocalFile(modified.path()));
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QCOMPARE(vector->frameCount(), 96);
        const auto offset = data.indexOf("\"op\":96");
        QVERIFY(offset >= 0);
        data.replace(offset, 7, "\"op\":48");
        QVERIFY(action.open(QIODevice::WriteOnly | QIODevice::Truncate));
        action.write(data);
        action.close();
        c.setPackageSource(QUrl::fromLocalFile(modified.filePath("mascot.json")));
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 30000);
        QCOMPARE(vector->frameCount(), 48);
    }
    void largePackageKeepsEventLoopResponsive()
    {
        Controller c;
        int heartbeats = 0;
        qint64 maxGap = 0;
        QElapsedTimer gap;
        gap.start();
        QTimer heartbeat;
        heartbeat.setInterval(2);
        connect(&heartbeat, &QTimer::timeout, this,
                [&]
                {
                    maxGap = qMax(maxGap, gap.restart());
                    if (c.availableActions().isEmpty() && c.status() == Controller::Loading)
                        ++heartbeats;
                });
        heartbeat.start();
        c.setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_LOTTIE_SAMPLE));
        QCOMPARE(c.status(), Controller::Loading);
        QTRY_COMPARE_WITH_TIMEOUT(c.availableActions().size(), 10, 30000);
        heartbeat.stop();
        QVERIFY2(heartbeats >= 3, "GUI events must run during the large package validation");
        qInfo() << "package validation GUI heartbeats" << heartbeats << "max gap ms" << maxGap;
        // Cancel a real large read/parse, then ensure it cannot overwrite a new sprite package.
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_LOTTIE_SAMPLE));
        QTest::qWait(10);
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QTest::qWait(250);
        QCOMPARE(c.status(), Controller::Ready);
        QCOMPARE(c.availableActions().size(), 2);
        auto *temporary = new Controller;
        temporary->setPackageSource(QUrl::fromLocalFile(QCAT_MASCOT_LOTTIE_SAMPLE));
        QTest::qWait(10);
        delete temporary;
        QTest::qWait(100);
    }
    void asynchronousLottieHandoff()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_GPU"))
            QSKIP("Set QCAT_MASCOT_GPU=1 to enable optional native QRhi handoff checks");
        QQuickWindow window;
        window.setColor(Qt::black);
        window.resize(192, 208);
        auto *item = new VectorItem(window.contentItem());
        item->setSize({192, 208});
        item->setFormat(VectorItem::Canvas);
        item->setViewBox({0, 0, 192, 208});
        item->setShapes(
            {QVariantMap{{"type", "rectangle"}, {"width", 192}, {"height", 208}, {"fill", "red"}}});
        window.show();
        QTRY_COMPARE_WITH_TIMEOUT(item->status(), VectorItem::Ready, 10000);
        QCOMPARE(window.grabWindow().pixelColor(0, 0), QColor(Qt::red));
        const auto source = [](const QString &name)
        {
            return QUrl::fromLocalFile(QStringLiteral(QCAT_MASCOT_LOTTIE_SAMPLE) + '/' + name +
                                       ".json");
        };
        int heartbeats = 0, retainedFrames = 0;
        qint64 maxGap = 0;
        QElapsedTimer gap;
        gap.start();
        QTimer heartbeat;
        heartbeat.setInterval(2);
        connect(&heartbeat, &QTimer::timeout, this,
                [&]
                {
                    maxGap = qMax(maxGap, gap.restart());
                    if (item->status() == VectorItem::Loading)
                        ++heartbeats;
                });
        heartbeat.start();
        item->setFormat(VectorItem::Lottie);
        item->setSource(source("idle"));
        QElapsedTimer deadline;
        deadline.start();
        while (item->status() == VectorItem::Loading && deadline.elapsed() < 30000)
        {
            QTest::qWait(10);
            const auto image = window.grabWindow();
            QVERIFY(!image.isNull());
            int occupied = 0;
            for (int y = 0; y < image.height(); ++y)
                for (int x = 0; x < image.width(); ++x)
                    occupied += image.pixelColor(x, y) != QColor(Qt::black);
            QVERIFY2(occupied > 1000, "Loading a new scene must not blank the old frame");
            ++retainedFrames;
        }
        QCOMPARE(item->status(), VectorItem::Ready);
        heartbeat.stop();
        QVERIFY(heartbeats >= 3);
        QVERIFY(retainedFrames > 0);
        QCOMPARE(item->frameCount(), 96);
        QCOMPARE(window.grabWindow().pixelColor(0, 0), QColor(Qt::black));
        qInfo() << "Lottie load GUI heartbeats" << heartbeats << "max gap ms" << maxGap
                << "nonblank loading samples" << retainedFrames;
        item->setSource(source("working"));
        QTest::qWait(10);
        item->setSource(source("jump"));
        item->setSource(source("idle"));
        QTRY_COMPARE_WITH_TIMEOUT(item->status(), VectorItem::Ready, 30000);
        QCOMPARE(item->frameCount(), 96);
        item->play();
        QTRY_VERIFY(item->progress() > .05);
        item->pause();
        auto *temporary = new VectorItem(window.contentItem());
        temporary->setSize({192, 208});
        temporary->setFormat(VectorItem::Lottie);
        temporary->setSource(source("working"));
        QTest::qWait(30);
        delete temporary;
        QTest::qWait(300);
        QCOMPARE(item->status(), VectorItem::Ready);
        item->setSource({});
        QCOMPARE(item->status(), VectorItem::Null);
        QTest::qWait(100);
        QCOMPARE(item->status(), VectorItem::Null);
        QCOMPARE(window.grabWindow().pixelColor(96, 104), QColor(Qt::black));
    }
    void spriteRuntimeError()
    {
        QTemporaryDir dir;
        auto package = fixture();
        package["footAnchor"] = QJsonArray{2, 7};
        write(dir.path(), package);
        Controller c;
        c.setPackageSource(QUrl::fromLocalFile(dir.path()));
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.pauseActions();
        c.setFacing("left");
        QCOMPARE(c.footAnchor(), QPointF(6, 7));
        QSignalSpy ended(&c, &Controller::actionEnded);
        c.requestAction("jump");
        c.requestAction("idle", Controller::Enqueue);
        QTRY_COMPARE(c.queueLength(), 1);
        QSignalSpy anchors(&c, &Controller::anchorsChanged);
        QFile image(dir.filePath("atlas.png"));
        QVERIFY(image.open(QIODevice::WriteOnly));
        image.write("broken after validation");
        image.close();
        QQuickWindow window;
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        QTRY_COMPARE(c.status(), Controller::Error);
        QCOMPARE(c.errorCode(), "RenderFailed");
        QCOMPARE(c.footAnchor(), QPointF(2, 7));
        QCOMPARE(anchors.size(), 1);
        QCOMPARE(ended.size(), 2);
        QCOMPARE(ended[0][1].toString(), "Failed");
        QCOMPARE(ended[1][1].toString(), "Cancelled");
    }
    void loadingGeneration()
    {
        Controller c;
        c.setPackageSource(QUrl::fromLocalFile("/missing/mascot.json"));
        QCOMPARE(c.status(), Controller::Loading);
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QCOMPARE(c.currentAction(), "idle");
        c.setPackageSource({});
        QCOMPARE(c.status(), Controller::Null);
        QCOMPARE(c.spriteFrame(), QVariantMap{});
        c.setPackageSource(fixtureUrl());
        c.setPackageSource({});
        QTest::qWait(30);
        QCOMPARE(c.status(), Controller::Null);
        auto *temporary = new Controller;
        temporary->setPackageSource(fixtureUrl());
        delete temporary;
        QTest::qWait(10);
        c.setPackageSource(QUrl::fromLocalFile("/missing/mascot.json"));
        QTRY_COMPARE(c.status(), Controller::Error);
        QVERIFY(c.availableActions().isEmpty());
        QVERIFY(c.currentAction().isEmpty());
    }
    void playbackAndQueue()
    {
        Controller c;
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QSignalSpy ended(&c, &Controller::actionEnded), started(&c, &Controller::actionStarted),
            rejected(&c, &Controller::requestRejected);
        // Default is not busy; Enqueue starts immediately.
        const auto a = c.requestAction("jump", Controller::Enqueue);
        const auto b = c.requestAction("jump", Controller::Enqueue);
        const auto ignored = c.requestAction("idle", Controller::IgnoreIfBusy);
        QTRY_COMPARE(c.queueLength(), 1);
        QTRY_VERIFY(ended.size() >= 3);
        QCOMPARE(ended[0][0].toString(), ignored);
        QCOMPARE(ended[0][1].toString(), "Ignored");
        QCOMPARE(ended[1][0].toString(), a);
        QCOMPARE(ended[1][1].toString(), "Completed");
        QCOMPARE(ended[2][0].toString(), b);
        QCOMPARE(ended[2][1].toString(), "Completed");
        QCOMPARE(started.size(), 2);
        QCOMPARE(c.currentAction(), "idle");
        c.requestAction("absent");
        QTRY_COMPARE(rejected.size(), 1);
        QCOMPARE(c.currentAction(), "idle");
        const auto unlimitedId = c.requestAction("idle");
        QTRY_COMPARE(started.size(), 3);
        const auto queued = c.requestAction("jump", Controller::Enqueue);
        QTRY_COMPARE(c.queueLength(), 1);
        c.requestAction("jump");
        QTRY_VERIFY(ended.size() >= 6);
        QCOMPARE(ended[3][0].toString(), unlimitedId);
        QCOMPARE(ended[3][1].toString(), "Interrupted");
        QCOMPARE(ended[4][0].toString(), queued);
        QCOMPARE(ended[4][1].toString(), "Cancelled");
    }
    void pauseAndHide()
    {
        Controller c;
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QSignalSpy ended(&c, &Controller::actionEnded), started(&c, &Controller::actionStarted);
        c.requestAction("jump", Controller::Replace, {{"loops", 3}});
        QTRY_COMPARE(started.size(), 1);
        c.pauseActions();
        const auto frozen = c.spriteFrame();
        QTest::qWait(300);
        QCOMPARE(c.spriteFrame(), frozen);
        QCOMPARE(ended.size(), 0);
        c.resumeActions();
        c.setPlaybackEnabled(false);
        const auto hidden = c.spriteFrame();
        QTest::qWait(300);
        QCOMPARE(c.spriteFrame(), hidden);
        QCOMPARE(ended.size(), 0);
        c.setPlaybackEnabled(true);
        QTRY_COMPARE(ended.size(), 1);
        QCOMPARE(ended[0][1].toString(), "Completed");
        c.setFacing("left");
        QVERIFY(c.spriteFrame()["mirrored"].toBool());
        c.setPetScale(2);
        QCOMPARE(c.petScale(), 2.0);
        c.setPetScale(qQNaN());
        QCOMPARE(c.petScale(), 2.0);
        c.setPetScale(0);
        QCOMPARE(c.petScale(), 2.0);
    }
    void queueLimitsAndInvalidOptions()
    {
        Controller c;
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QSignalSpy rejected(&c, &Controller::requestRejected), ended(&c, &Controller::actionEnded);
        c.requestAction("idle");
        for (int i = 0; i < 65; ++i)
            c.requestAction("jump", Controller::Enqueue);
        QTRY_COMPARE(c.queueLength(), 64);
        QTRY_COMPARE(rejected.size(), 1);
        QCOMPARE(rejected[0][1].toString(), "QueueFull");
        c.requestAction("jump", Controller::Replace, {{"loops", 0}});
        QTRY_COMPARE(rejected.size(), 2);
        QCOMPARE(c.queueLength(), 64);
        c.resetToDefault();
        QTRY_COMPARE(ended.size(), 65);
        QCOMPARE(c.queueLength(), 0);
        QTest::qWait(40);
        QCOMPARE(ended.size(), 65);
    }
    void packageSwitchAndIsolation()
    {
        Controller a, b;
        a.setPackageSource(fixtureUrl());
        b.setPackageSource(fixtureUrl());
        QTRY_COMPARE(a.status(), Controller::Ready);
        QTRY_COMPARE(b.status(), Controller::Ready);
        QSignalSpy aEnded(&a, &Controller::actionEnded), bEnded(&b, &Controller::actionEnded);
        a.requestAction("jump");
        a.requestAction("jump", Controller::Enqueue);
        b.requestAction("idle");
        QTRY_COMPARE(a.queueLength(), 1);
        a.setPackageSource({});
        QTRY_COMPARE(aEnded.size(), 2);
        QCOMPARE(aEnded[0][1].toString(), "Cancelled");
        QCOMPARE(aEnded[1][1].toString(), "Cancelled");
        QCOMPARE(b.currentAction(), "idle");
        QCOMPARE(bEnded.size(), 0);
        a.setPackageSource(fixtureUrl());
        QTRY_COMPARE(a.status(), Controller::Ready);
        const auto pending = a.requestAction("jump");
        a.setPackageSource({});
        QTRY_COMPARE(aEnded.size(), 3);
        QCOMPARE(aEnded.last()[0].toString(), pending);
        QCOMPARE(a.status(), Controller::Null);
    }
    void dragFeedback()
    {
        Controller c;
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QSignalSpy ended(&c, &Controller::actionEnded), rejected(&c, &Controller::requestRejected);
        QSignalSpy startedDrag(&c, &Controller::dragStarted),
            finishedDrag(&c, &Controller::dragFinished);
        c.pauseActions();
        c.requestAction("jump");
        c.requestAction("idle", Controller::Enqueue);
        QTRY_COMPARE(c.queueLength(), 1);
        c.setDragAction("jump");
        QVERIFY(c.beginDrag());
        QCOMPARE(c.currentAction(), "jump");
        QCOMPARE(c.queueLength(), 0);
        QCOMPARE(ended.size(), 2);
        QCOMPARE(ended[0][1].toString(), "Interrupted");
        QCOMPARE(ended[1][1].toString(), "Cancelled");
        c.requestAction("idle");
        c.endDrag(); // Busy is captured when the request is submitted.
        QTRY_COMPARE(rejected.size(), 1);
        QCOMPARE(rejected[0][1].toString(), "InteractionBusy");
        QCOMPARE(c.currentAction(), "idle");
        QVERIFY(c.paused());
        QCOMPARE(startedDrag.size(), 1);
        QCOMPARE(finishedDrag.size(), 1);
        c.setDragAction({});
        c.requestAction("jump", Controller::Replace, {{"loops", -1}});
        QTRY_COMPARE(c.currentAction(), "jump");
        QVERIFY(c.beginDrag());
        QCOMPARE(c.currentAction(), "jump");
        c.endDrag();
        QCOMPARE(c.currentAction(), "jump");
        QCOMPARE(ended.size(), 2);
        c.setDragEnabled(false);
        QVERIFY(!c.beginDrag());
        c.setDragEnabled(true);
        c.setDragAction("jump");
        QVERIFY(c.beginDrag());
        c.setPackageSource({});
        QVERIFY(!c.dragging());
        QCOMPARE(c.status(), Controller::Null);
        c.resumeActions();
        QTest::qWait(250);
        QCOMPARE(ended.size(), 3);
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        QVERIFY(c.beginDrag());
        c.resumeActions();
        QTest::qWait(250);
        QCOMPARE(c.currentAction(),
                 "jump"); // Feedback loops, even though the declared action is finite.
        c.setDragEnabled(false);
        QVERIFY(!c.dragging());
        QCOMPARE(c.currentAction(), "idle");
    }
    void regionsAndAnchors()
    {
        QTemporaryDir dir;
        auto obj = fixture();
        obj["footAnchor"] = QJsonArray{2, 7};
        obj["bubbleAnchor"] = QJsonArray{3, 1};
        obj["regions"] = QJsonArray{QJsonObject{{"id", "back"},
                                                {"shape", "ellipse"},
                                                {"rect", QJsonArray{0, 0, 8, 8}},
                                                {"clickable", true},
                                                {"draggable", true}},
                                    QJsonObject{{"id", "front"},
                                                {"shape", "rect"},
                                                {"rect", QJsonArray{1, 1, 2, 3}},
                                                {"clickable", true}}};
        auto actions = obj["actions"].toObject();
        auto jump = actions["jump"].toObject();
        jump["regions"] = QJsonArray{QJsonObject{
            {"id", "triangle"},
            {"shape", "polygon"},
            {"points", QJsonArray{QJsonArray{0, 0}, QJsonArray{4, 0}, QJsonArray{0, 4}}},
            {"clickable", true}}};
        actions["jump"] = jump;
        obj["actions"] = actions;
        write(dir.path(), obj);
        Controller c;
        c.setPackageSource(QUrl::fromLocalFile(dir.path()));
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.pauseActions();
        c.setPetScale(2);
        QCOMPARE(c.hitTest({4, 4})["id"].toString(), "front");
        QCOMPARE(c.hitTest({4, 4}, true)["id"].toString(), "back");
        QVERIFY(c.hitTest({0, 0}).isEmpty());
        QVERIFY(c.hitTest({-1, 4}).isEmpty());
        QVERIFY(c.hitTest({qQNaN(), 0}).isEmpty());
        QSignalSpy activated(&c, &Controller::regionActivated);
        QVERIFY(c.activateRegion({4, 4}));
        QCOMPARE(activated[0][0].toString(), "front");
        QCOMPARE(activated[0][1].toPointF(), QPointF(2, 2));
        c.setFacing("left");
        QCOMPARE(c.footAnchor(), QPointF(6, 7));
        QCOMPARE(c.bubbleAnchor(), QPointF(5, 1));
        QCOMPARE(c.hitTest({12, 4})["id"].toString(), "front");
        QCOMPARE(c.interactionRegions()[1].toMap()["rect"].toList(),
                 QVariantList({10.0, 2.0, 4.0, 6.0}));
        c.requestAction("jump");
        QTRY_COMPARE(c.currentAction(), "jump");
        QCOMPARE(c.interactionRegions().size(), 1);
        QCOMPARE(c.hitTest({14, 2})["id"].toString(), "triangle");
        QVERIFY(c.hitTest({2, 14}).isEmpty());
        auto unmirrored = obj;
        unmirrored["allowMirror"] = false;
        write(dir.path(), unmirrored);
        c.setPackageSource({});
        c.setPackageSource(QUrl::fromLocalFile(dir.path()));
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.setFacing("left");
        QCOMPARE(c.facing(), "right");
        QQuickWindow window;
        QQmlEngine engine;
        QQmlComponent visual(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!visual.isLoading());
        std::unique_ptr<QObject> object(
            visual.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(visual.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        item->setParentItem(window.contentItem());
        item->setSize({16, 16});
        window.resize(16, 16);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QSignalSpy clicked(&c, &Controller::regionActivated);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(4, 4));
        QTRY_COMPARE(clicked.size(), 1);
        QCOMPARE(clicked[0][0].toString(), "front");
    }
    void spriteSamplingStability()
    {
        QTemporaryDir dir;
        auto data = fixture();
        data["logicalSize"] = QJsonArray{192, 208};
        data["footAnchor"] = QJsonArray{96, 200};
        data["bubbleAnchor"] = QJsonArray{96, 0};
        auto idle = data["actions"].toObject()["idle"].toObject();
        auto direction = idle["right"].toObject();
        direction["frameSize"] = QJsonArray{192, 208};
        direction["frameCount"] = 4;
        idle["right"] = direction;
        data["actions"] = QJsonObject{{"idle", idle}};
        QImage atlas(768, 208, QImage::Format_ARGB32_Premultiplied);
        const QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::yellow};
        for (int y = 0; y < atlas.height(); ++y)
            for (int x = 0; x < atlas.width(); ++x)
                atlas.setPixelColor(x, y, colors[x / 192]);
        QVERIFY(atlas.save(dir.path() + "/atlas.png"));
        write(dir.path(), data);
        Controller actor;
        actor.pauseActions();
        actor.setPackageSource(QUrl::fromLocalFile(dir.path()));
        QTRY_COMPARE(actor.status(), Controller::Ready);
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!component.isLoading());
        std::unique_ptr<QObject> object(component.createWithInitialProperties(
            {{"controller", QVariant::fromValue(&actor)}, {"smooth", true}}));
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        QVERIFY(item);
        QQuickWindow window;
        window.resize(400, 400);
        window.setColor(Qt::transparent);
        item->setParentItem(window.contentItem());
        window.show();
        for (int frame = 0; frame < 4; ++frame)
        {
            if (frame)
            {
                actor.resumeActions();
                QTRY_COMPARE(actor.spriteFrame().value("index").toInt(), frame);
                actor.pauseActions();
            }
            for (qreal scale : {0.35, 0.5, 1.0, 1.25})
            {
                QSignalSpy presented(&window, &QQuickWindow::frameSwapped);
                item->setSize({192 * scale, 208 * scale});
                window.requestUpdate();
                QTRY_VERIFY(presented.count() > 0);
                auto grab = item->grabToImage();
                QVERIFY(grab);
                QSignalSpy ready(grab.data(), &QQuickItemGrabResult::ready);
                if (grab->image().isNull())
                    QVERIFY(ready.wait(2000));
                const auto image = grab->image();
                QVERIFY(!image.isNull());
                for (int y = 1; y < image.height() - 1; ++y)
                    for (int x = 1; x < image.width() - 1; ++x)
                    {
                        if (image.pixelColor(x, y) != colors[frame])
                        {
                            image.save("/tmp/qcat-sprite-contamination.png");
                            QFAIL(qPrintable(
                                QString(
                                    "frame=%1 scale=%2 at=(%3,%4) actual=%5 expected=%6 size=%7x%8")
                                    .arg(frame)
                                    .arg(scale)
                                    .arg(x)
                                    .arg(y)
                                    .arg(image.pixelColor(x, y).name(QColor::HexArgb))
                                    .arg(colors[frame].name(QColor::HexArgb))
                                    .arg(image.width())
                                    .arg(image.height())));
                        }
                    }
            }
        }
    }
    void framesRendered()
    {
        Controller c;
        c.setPackageSource(fixtureUrl());
        QTRY_COMPARE(c.status(), Controller::Ready);
        c.pauseActions();
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml"));
        QTRY_VERIFY(!component.isLoading());
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(
            component.createWithInitialProperties({{"controller", QVariant::fromValue(&c)}}));
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *item = qobject_cast<QQuickItem *>(object.get());
        QVERIFY(item);
        QQuickWindow window;
        window.setColor(Qt::transparent);
        window.resize(64, 64);
        item->setParentItem(window.contentItem());
        item->setSize({32, 32});
        window.show();
        auto grab = [&]() -> QImage
        {
            auto result = item->grabToImage();
            if (!result)
                return {};
            QSignalSpy ready(result.data(), &QQuickItemGrabResult::ready);
            if (result->image().isNull())
                ready.wait(2000);
            return result->image();
        };
        QImage image = grab();
        QVERIFY(!image.isNull());
        QCOMPARE(image.pixelColor(4, 16), QColor(Qt::red));
        QCOMPARE(image.pixelColor(27, 16), QColor(Qt::blue));
        c.setFacing("left");
        image = grab();
        QCOMPARE(image.pixelColor(4, 16), QColor(Qt::blue));
        QCOMPARE(image.pixelColor(27, 16), QColor(Qt::red));
        c.requestAction("jump");
        QTRY_COMPARE(c.currentAction(), "jump");
        image = grab();
        QCOMPARE(image.pixelColor(16, 16), QColor(Qt::green));
        c.resumeActions();
        QTRY_COMPARE(c.spriteFrame()["index"].toInt(), 1);
        c.pauseActions();
        image = grab();
        QCOMPARE(image.pixelColor(16, 16), QColor(Qt::yellow));
        c.resumeActions();
        item->setVisible(false);
        QTRY_VERIFY(!c.playbackEnabled());
        const auto hiddenFrame = c.spriteFrame();
        QTest::qWait(300);
        QCOMPARE(c.spriteFrame(), hiddenFrame);
        item->setVisible(true);
        QTRY_VERIFY(c.playbackEnabled());
        auto *theme = item->property("theme").value<QObject *>();
        QVERIFY(theme);
        QVERIFY(theme->setProperty("reducedMotion", true));
        QTRY_VERIFY(!c.playbackEnabled());
        const auto reducedFrame = c.spriteFrame();
        QTest::qWait(300);
        QCOMPARE(c.spriteFrame(), reducedFrame);
        QVERIFY(theme->setProperty("reducedMotion", false));
        QTRY_VERIFY(c.playbackEnabled());
        window.hide();
        QTRY_VERIFY(!c.playbackEnabled());
    }
};
QTEST_MAIN(TestMascot)
#include "tst_mascot.moc"
