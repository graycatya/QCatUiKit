// SPDX-License-Identifier: Apache-2.0
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include <QCatUiKit/MascotPetController>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QProcess>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSaveFile>
#include <QScopeGuard>
#include <QTimer>
#include <QScreen>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>
#include <QWheelEvent>
#include <memory>
#include <atomic>
using Controller = QCatUiKit::MascotPetController;

class MacosInputTest : public QObject
{
    Q_OBJECT
    QProcess behind;
    QJsonObject state;
    QByteArray pending;
    QString token;
    QJsonArray evidence;
    QPointer<QQuickWindow> pet;
    CGPoint previousCursor{};
    pid_t previousApplication = 0;
    bool leftDown = false;
    bool routingAttempted = false, routingComplete = false;
    bool structuralAttempted = false, structuralComplete = false;

    NSWindow *native() const
    {
        return reinterpret_cast<NSView *>(pet->winId()).window;
    }
    static NSArray<NSWindow *> *surfacesFor(NSWindow *owner)
    {
        NSMutableArray<NSWindow *> *result = [NSMutableArray array];
        NSString *title =
            [NSString stringWithFormat:@"QCatUiKit mascot input:%ld", (long)owner.windowNumber];
        for (NSWindow *window in NSApp.windows)
            if ([window.title isEqualToString:title])
                [result addObject:window];
        return result;
    }
    NSArray<NSWindow *> *surfaces() const
    {
        return surfacesFor(native());
    }
    QList<CGWindowID> surfaceIds() const
    {
        QList<CGWindowID> ids;
        for (NSWindow *surface in surfaces())
            ids.append(CGWindowID(surface.windowNumber));
        return ids;
    }
    static bool retired(const QList<CGWindowID> &ids)
    {
        for (auto id : ids)
        {
            CFArrayRef info = CGWindowListCopyWindowInfo(kCGWindowListOptionIncludingWindow, id);
            const bool remains = info && CFArrayGetCount(info);
            if (info)
                CFRelease(info);
            if (remains)
                return false;
        }
        return true;
    }
    NSPoint point(QPointF local) const
    {
        NSView *view = reinterpret_cast<NSView *>(pet->winId());
        const NSRect bounds = view.bounds;
        const NSPoint p =
            NSMakePoint(bounds.origin.x + local.x() * bounds.size.width / pet->width(),
                        bounds.origin.y + local.y() * bounds.size.height / pet->height());
        return [view.window convertPointToScreen:[view convertPoint:p toView:nil]];
    }
    bool owned(NSPoint p) const
    {
        const auto hit = [NSWindow windowNumberAtPoint:p belowWindowWithWindowNumber:0];
        if (hit == state.value("window").toInt())
            return true;
        for (NSWindow *child in surfaces())
            if (hit == child.windowNumber)
                return true;
        return false;
    }
    bool mouse(QPointF local, CGEventType type)
    {
        const auto front = NSWorkspace.sharedWorkspace.frontmostApplication.processIdentifier;
        if (front != QCoreApplication::applicationPid() && front != behind.processId())
            return false;
        const NSPoint p = point(local);
        if (!leftDown && !owned(p))
            return false;
        auto event = CGEventCreateMouseEvent(
            nullptr, type, CGPointMake(p.x, NSScreen.screens.firstObject.frame.size.height - p.y),
            kCGMouseButtonLeft);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
        if (type == kCGEventLeftMouseDown)
            leftDown = true;
        if (type == kCGEventLeftMouseUp)
            leftDown = false;
        QTest::qWait(80);
        return true;
    }
    bool click(QPointF local)
    {
        return mouse(local, kCGEventLeftMouseDown) && mouse(local, kCGEventLeftMouseUp);
    }
    bool key(CGKeyCode code, CGEventFlags flags = 0)
    {
        const auto front = NSWorkspace.sharedWorkspace.frontmostApplication.processIdentifier;
        if (front != QCoreApplication::applicationPid() && front != behind.processId())
            return false;
        for (bool down : {true, false})
        {
            auto event = CGEventCreateKeyboardEvent(nullptr, code, down);
            CGEventSetFlags(event, flags);
            CGEventPost(kCGHIDEventTap, event);
            CFRelease(event);
        }
        QTest::qWait(100);
        return true;
    }
    void capture(QString name)
    {
        evidence.append(QJsonObject{{"name", name},
                                    {"status", "Passed"},
                                    {"target", state},
                                    {"inputWindows", int(surfaces().count)},
                                    {"position", QJsonArray{pet->x(), pet->y()}}});
        qInfo().noquote()
            << QJsonDocument(evidence.last().toObject()).toJson(QJsonDocument::Compact);
    }
    void settle()
    {
        QTest::qWait(100);
        [native() orderFrontRegardless];
        QTest::qWait(50);
    }
    static QQuickItem *find(QQuickItem *item, const QString &name)
    {
        if (item->objectName() == name)
            return item;
        for (auto *child : item->childItems())
            if (auto *found = find(child, name))
                return found;
        return nullptr;
    }
    void makePackage(const QString &path)
    {
        QImage image(200, 160, QImage::Format_ARGB32_Premultiplied);
        image.fill(QColor("#e6a64b"));
        QVERIFY(image.save(path + "/atlas.png"));
        auto shape = [](QString kind, QJsonArray rect)
        {
            return QJsonObject{{"id", "body"},
                               {"shape", kind},
                               {"rect", rect},
                               {"clickable", true},
                               {"draggable", true}};
        };
        auto action = [](QJsonArray regions)
        {
            return QJsonObject{{"loops", -1},
                               {"regions", regions},
                               {"right", QJsonObject{{"source", "atlas.png"},
                                                     {"frameSize", QJsonArray{200, 160}},
                                                     {"frameCount", 1},
                                                     {"fps", 1}}}};
        };
        QJsonObject document{
            {"schemaVersion", 1},
            {"id", "macos-input"},
            {"renderer", "sprite"},
            {"logicalSize", QJsonArray{200, 160}},
            {"footAnchor", QJsonArray{100, 150}},
            {"bubbleAnchor", QJsonArray{100, 10}},
            {"defaultAction", "idle"},
            {"defaultFacing", "right"},
            {"allowMirror", true},
            {"actions",
             QJsonObject{
                 {"idle", action({shape("rect", {20, 30, 70, 80})})},
                 {"ellipse", action({shape("ellipse", {20, 30, 70, 80})})},
                 {"polygon",
                  action({QJsonObject{{"id", "body"},
                                      {"shape", "polygon"},
                                      {"points", QJsonArray{QJsonArray{20, 30}, QJsonArray{90, 30},
                                                            QJsonArray{20, 110}}},
                                      {"clickable", true},
                                      {"draggable", true}}})}}}};
        QFile manifest(path + "/mascot.json");
        QVERIFY(manifest.open(QIODevice::WriteOnly));
        manifest.write(QJsonDocument(document).toJson());
        manifest.close();
    }
  private slots:
    void initTestCase()
    {
        previousApplication = NSWorkspace.sharedWorkspace.frontmostApplication.processIdentifier;
        auto cursor = CGEventCreate(nullptr);
        previousCursor = CGEventGetLocation(cursor);
        CFRelease(cursor);
        connect(&behind, &QProcess::readyReadStandardOutput, this,
                [this]
                {
                    pending += behind.readAllStandardOutput();
                    while (pending.contains('\n'))
                    {
                        const auto i = pending.indexOf('\n');
                        const auto report = QJsonDocument::fromJson(pending.left(i)).object();
                        pending.remove(0, i + 1);
                        if (!report.isEmpty())
                            state = report;
                    }
                });
    }
    void nativeLifecycle()
    {
        structuralAttempted = true;
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QCatUiKit\nCatMascotPet { petScale: 10; "
            "packageSource: \"qrc:/mascot-macos-test/mascot.json\"; function message() { "
            "showBubble(\"Native message\", {actions:[{id:\"ok\",text:\"OK\"}]}) } }",
            QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QTRY_VERIFY(actor->status() != Controller::Loading && actor->status() != Controller::Null);
        QVERIFY2(actor->status() == Controller::Ready, qPrintable(actor->errorString()));
        pet->setPosition(240, 300);
        pet->show();
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        settle();
        qInfo() << "Cocoa input lifecycle DPR" << pet->devicePixelRatio() << "graphics API"
                << int(pet->rendererInterface()->graphicsApi());
        NSWindow *owner = [native() retain];
        QCOMPARE(surfacesFor(owner).count, NSUInteger(1));
        NSWindow *input = surfacesFor(owner).firstObject;
        qInfo() << "Visual ignores mouse" << bool(owner.ignoresMouseEvents) << "owner"
                << owner.windowNumber << "input" << input.windowNumber;
        QVERIFY(owner.ignoresMouseEvents);
        QVERIFY(!input.opaque);
        QVERIFY(!input.hasShadow);
        QCOMPARE(input.backgroundColor.alphaComponent, CGFloat(0));
        QVERIFY(!input.canBecomeKeyWindow);
        QCOMPARE([NSWindow windowNumberAtPoint:point({4, 4}) belowWindowWithWindowNumber:0],
                 input.windowNumber);
        pet->setProperty("inputTransparent", true);
        QTest::qWait(100);
        QVERIFY(!input.visible);
        pet->setProperty("inputTransparent", false);
        settle();
        QVERIFY(input.visible);
        QVERIFY(!owner.hidesOnDeactivate);
        if ([owner isKindOfClass:NSPanel.class])
            QVERIFY(!static_cast<NSPanel *>(owner).floatingPanel);
        // Reproduce a Gallery host activation followed by another application.
        // No system input injection or accessibility permission is needed.
        QQuickWindow host;
        host.setTitle("QCat mascot activation host");
        host.resize(160, 120);
        host.show();
        [NSApp activateIgnoringOtherApps:YES];
        host.requestActivate();
        QTRY_VERIFY(NSApp.active);
        const NSInteger activeLevel = owner.level;
        const auto bundledTarget = QCoreApplication::applicationDirPath() +
            "/../Helpers/mascot-macos-input-target.app/Contents/MacOS/mascot-macos-input-target";
        behind.start(QFileInfo::exists(bundledTarget) ? bundledTarget
                         : QString::fromUtf8(QCAT_MACOS_INPUT_TARGET),
                     {"QCat mascot activation target:" + QUuid::createUuid().toString()});
        QTRY_VERIFY(state.value("pid").toInt() == behind.processId());
        QTRY_VERIFY(state.value("keyWindow").toBool());
        QTRY_VERIFY(!NSApp.active);
        QTest::qWait(150);
        QVERIFY(owner.visible);
        QCOMPARE(owner.level, activeLevel);
        QVERIFY(input.visible);
        const NSInteger hit = [NSWindow windowNumberAtPoint:point({4, 4})
                                belowWindowWithWindowNumber:0];
        QCOMPARE(hit, input.windowNumber);
        behind.terminate();
        QVERIFY(behind.waitForFinished(2000));
        state = {};
        pending.clear();
        host.hide();
        pet->hide();
        QTest::qWait(100);
        QVERIFY(!input.visible);
        const auto ids = surfaceIds();
        object.reset();
        pet.clear();
        QTRY_VERIFY(retired(ids));
        [owner release];
        structuralComplete = true;
    }
    void nativeEventDelivery()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    petScale: 10
    packageSource: "qrc:/mascot-macos-test/mascot.json"
    function message() { showBubble("Native message", {actions:[{id:"ok",text:"OK"}]}) }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QTRY_VERIFY(actor->status() != Controller::Loading && actor->status() != Controller::Null);
        QVERIFY2(actor->status() == Controller::Ready, qPrintable(actor->errorString()));
        pet->setPosition(240, 300);
        pet->show();
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        QVERIFY(QMetaObject::invokeMethod(object.get(), "message"));
        settle();
        auto *button = find(pet->contentItem(), "mascotBubbleAction_ok");
        QVERIFY(button);
        const NSPoint screen =
            point(button->mapToScene(QPointF(button->width() / 2, button->height() / 2)));
        NSWindow *input = nil;
        for (NSWindow *child in surfaces())
            if (NSPointInRect(screen, child.frame))
            {
                input = [child retain];
                break;
            }
        QVERIFY(input);
        QSignalSpy activated(object.get(), SIGNAL(bubbleActionTriggered(QString, QString)));
        // This exercises AppKit->Qt delivery inside our process, not WindowServer
        // injection. Real cross-process acceptance is the opt-in routing test.
        for (auto type : {NSEventTypeLeftMouseDown, NSEventTypeLeftMouseUp})
        {
            auto *event = [NSEvent mouseEventWithType:type
                                             location:[input convertPointFromScreen:screen]
                                        modifierFlags:0
                                            timestamp:NSProcessInfo.processInfo.systemUptime
                                         windowNumber:input.windowNumber
                                              context:nil
                                          eventNumber:1
                                           clickCount:1
                                             pressure:1];
            [input sendEvent:event];
        }
        [input release];
        QTRY_COMPARE(activated.count(), 1);
        QTest::keyClick(pet, Qt::Key_Space);
        QTRY_COMPARE(activated.count(), 2);
        NSWindow *owner = [native() retain];
        const auto ids = surfaceIds();
        object.reset();
        pet.clear();
        QTRY_VERIFY(retired(ids));
        [owner release];
    }
    void nativeShapePlacement()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        makePackage(directory.path());
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QCatUiKit\nCatMascotPet { "
                          "constrainToAvailableGeometry: false }",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        actor->setPackageSource(QUrl::fromLocalFile(directory.filePath("mascot.json")));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        pet->setPosition(240, 300);
        pet->show();
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        settle();
        pet->setProperty("inputRegionsEnabled", true);
        actor->requestAction("ellipse");
        QTRY_COMPARE(actor->currentAction(), QString("ellipse"));
        settle();
        const NSPoint corner = point({21, 31});
        const NSInteger hit = [NSWindow windowNumberAtPoint:corner belowWindowWithWindowNumber:0];
        qInfo() << "Native corner" << QString::fromNSString(NSStringFromPoint(corner)) << "visual"
                << QString::fromNSString(NSStringFromRect(native().frame)) << "Qt size"
                << pet->size();
        for (NSWindow *child in surfaces())
            if (child.windowNumber == hit)
            {
                NSArray *info = (NSArray *)CGWindowListCopyWindowInfo(
                    kCGWindowListOptionIncludingWindow, CGWindowID(child.windowNumber));
                qInfo() << "Hit frame" << QString::fromNSString(NSStringFromRect(child.frame))
                        << "server"
                        << QString::fromNSString([[info.firstObject
                               objectForKey:(NSString *)kCGWindowBounds] description]);
                [info release];
                QVERIFY2(false, "Ellipse corner is intercepted by an input rectangle");
            }
        object.reset();
        pet.clear();
    }
    void nativeBubbleEdges()
    {
        [NSApp activateIgnoringOtherApps:YES];
        QTest::qWait(250);
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    petScale: 3
    inputRegionsEnabled: true
    packageSource: "qrc:/mascot-macos-test/mascot.json"
    theme: ThemePalette { id: palette }
    function dark(value) { palette.colorScheme = value ? ThemePalette.Dark : ThemePalette.Light }
    function message() { showBubble("Long text ".repeat(200), {actions:[{id:"ok",text:"Continue"}]}) }
    function updateMessage() { updateBubble(bubbleId, "Updated text ".repeat(250)) }
    function dismiss() { closeBubble(bubbleId) }
})", QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        pet->setPosition(0, 0);
        pet->show();
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        QTest::qWait(250);
        const QRect area = pet->screen()->availableGeometry();
        const QPoint corners[] = {area.topLeft() + QPoint(2, 2),
            QPoint(area.right() - 25, area.top() + 2),
            QPoint(area.left() + 2, area.bottom() - 25), area.bottomRight() - QPoint(25, 25)};
        auto inputAt = [&](QPointF local)
        {
            const NSInteger hit = [NSWindow windowNumberAtPoint:point(local)
                                     belowWindowWithWindowNumber:0];
            for (NSWindow *surface in surfaces())
                if (surface.visible && surface.windowNumber == hit)
                    return true;
            return false;
        };
        for (bool dark : {false, true})
        {
            QVERIFY(QMetaObject::invokeMethod(object.get(), "dark", Q_ARG(QVariant, QVariant(dark))));
            for (auto corner : corners)
            {
                pet->setPosition(corner);
                QTest::qWait(100);
                for (const auto *method : {"message", "updateMessage"})
                {
                    QVERIFY(QMetaObject::invokeMethod(object.get(), method));
                    QTest::qWait(200);
                    auto *bubble = find(pet->contentItem(), "mascotBubble");
                    auto *action = find(pet->contentItem(), "mascotBubbleAction_ok");
                    auto *close = find(pet->contentItem(), "mascotBubbleClose");
                    auto *scroll = find(pet->contentItem(), "mascotBubbleScroll");
                    QVERIFY(bubble && action && close && scroll);
                    auto *flickable = qvariant_cast<QObject *>(scroll->property("contentItem"));
                    QVERIFY(flickable);
                    const qreal oldY = flickable->property("contentY").toReal();
                    const auto scrollPoint = scroll->mapToScene(QPointF(scroll->width() / 2, scroll->height() / 2));
                    qInfo() << "Scroll viewport" << scroll->size() << flickable->property("height")
                            << "content" << flickable->property("contentHeight") << "position" << scrollPoint
                            << "bubble" << bubble->size() << "controls" << close->size() << action->size()
                            << "area" << area << "geometry" << pet->geometry()
                            << "above" << pet->property("bubbleAbove") << "spaces"
                            << pet->property("bubbleSpaceAbove") << pet->property("bubbleSpaceBelow");
                    QTest::mouseMove(pet, scrollPoint.toPoint());
                    QWheelEvent wheel(scrollPoint, pet->mapToGlobal(scrollPoint), QPoint(),
                                      QPoint(0, -120), Qt::NoButton, Qt::NoModifier,
                                      Qt::NoScrollPhase, false);
                    wheel.setTimestamp(quint64(NSProcessInfo.processInfo.systemUptime * 1000));
                    QCoreApplication::sendEvent(pet, &wheel);
                    QTRY_VERIFY(flickable->property("contentY").toReal() > oldY);
                    qInfo() << "Bubble native edge" << area << pet->screen()->availableGeometry()
                            << corner << pet->geometry();
                    QVERIFY(area.contains(pet->geometry()));
                    const QPoint offset = corner - pet->position();
                    const QPointF gap(offset.x() + 2, offset.y() > 0 ? offset.y() - 4 : 26);
                    QVERIFY(!inputAt(gap));
                    for (auto *control : {action, close})
                    {
                        const QPointF local = control->mapToScene(QPointF(control->width() / 2, control->height() / 2));
                        QVERIFY(control->height() > 0 && control->width() > 0);
                        QVERIFY(inputAt(local));
                        QVERIFY(QRectF(bubble->mapToScene(QPointF()), bubble->size()).contains(
                            QRectF(control->mapToScene(QPointF()), control->size())));
                    }
                }
                QVERIFY(QMetaObject::invokeMethod(object.get(), "dismiss"));
                QTest::qWait(150);
                QCOMPARE(pet->position(), corner);
            }
        }
        qInfo() << "Bubble input edges" << area << "DPR" << pet->devicePixelRatio()
                << "graphics API" << int(pet->rendererInterface()->graphicsApi());
        object.reset();
        pet.clear();
    }
    void nativeFullscreenRecovery()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QCatUiKit\nCatMascotPet { petScale: 10; "
                          "packageSource: \"qrc:/mascot-macos-test/mascot.json\" }", QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        pet->setPosition(360, 320);
        pet->setVisible(true);
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        settle();
        const QPoint before = pet->position();
        const NSInteger level = native().level;
        const auto bundledTarget = QCoreApplication::applicationDirPath() +
            "/../Helpers/mascot-macos-input-target.app/Contents/MacOS/mascot-macos-input-target";
        state = {};
        pending.clear();
        behind.start(QFileInfo::exists(bundledTarget) ? bundledTarget
                         : QString::fromUtf8(QCAT_MACOS_INPUT_TARGET),
                     {"QCat fullscreen acceptance:" + QUuid::createUuid().toString(), "--fullscreen"});
        QTRY_VERIFY_WITH_TIMEOUT(state.value("fullscreen").toBool(), 15000);
        QTRY_VERIFY_WITH_TIMEOUT(!native().onActiveSpace, 15000);
        auto hasInput = [&]
        {
            for (NSWindow *surface in surfaces())
                if (surface.visible)
                    return true;
            return false;
        };
        QTRY_VERIFY(!hasInput());
        QCOMPARE(pet->position(), before);
        qInfo() << "Independent fullscreen Space" << state << "pet activeSpace"
                << bool(native().onActiveSpace) << "input visible" << hasInput();
        behind.terminate();
        if (!behind.waitForFinished(3000))
        {
            behind.kill();
            QVERIFY(behind.waitForFinished(2000));
        }
        QTRY_VERIFY_WITH_TIMEOUT(native().onActiveSpace, 15000);
        QTRY_VERIFY(native().visible);
        QTRY_VERIFY(hasInput());
        QCOMPARE(native().level, level);
        QCOMPARE(pet->position(), before);
        qInfo() << "Desktop returned" << pet->position() << "level" << native().level
                << "input visible" << hasInput();
        const auto ids = surfaceIds();
        object.reset();
        pet.clear();
        QTRY_VERIFY(retired(ids));
        state = {};
        pending.clear();
    }
    void nativeSpacesReview()
    {
        const QString output = qEnvironmentVariable("QCAT_MASCOT_SPACES_REPORT");
        const QString done = qEnvironmentVariable("QCAT_MASCOT_SPACES_DONE");
        if (output.isEmpty() || done.isEmpty())
            QSKIP("Explicit interactive Spaces review required");
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    petScale: 10
    packageSource: "qrc:/mascot-macos-test/mascot.json"
    function message() { showBubble("Spaces and transparency review", {actions:[{id:"ok",text:"OK"}]}) }
})", QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QTRY_COMPARE(actor->status(), Controller::Ready);
        QQuickWindow host;
        host.setTitle("QCat macOS Spaces acceptance - switch desktop and return");
        host.setColor(QColor("#224466"));
        host.resize(500, 320);
        host.setPosition(600, 200);
        host.show();
        [NSApp activateIgnoringOtherApps:YES];
        host.requestActivate();
        pet->setPosition(820, 320);
        pet->setVisible(true);
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        settle();
        QVERIFY(QMetaObject::invokeMethod(object.get(), "message"));
        QTest::qWait(200);
        const QPoint before = pet->position();
        const NSInteger level = native().level;
        std::atomic_int changes{0};
        std::atomic_int *spaceCounter = &changes;
        auto *center = NSWorkspace.sharedWorkspace.notificationCenter;
        id observer = [center addObserverForName:NSWorkspaceActiveSpaceDidChangeNotification
                    object:nil queue:nil usingBlock:^(NSNotification *) {
                        ++(*spaceCounter);
                    }];
        const auto removeObserver = qScopeGuard([&] { [center removeObserver:observer]; });
        bool leftSpace = false, returned = false, wrote = true;
        QJsonArray samples;
        auto sample = [&]
        {
            bool inputVisible = false;
            for (NSWindow *surface in surfaces())
                inputVisible |= bool(surface.visible);
            const bool active = native().onActiveSpace;
            if (!active)
                leftSpace = true;
            if (leftSpace && active)
                returned = true;
            QJsonObject row{{"changes", changes.load()}, {"activeSpace", active},
                            {"visualVisible", bool(native().visible)}, {"inputVisible", inputVisible},
                            {"collectionBehavior", int(native().collectionBehavior)},
                            {"hostActiveSpace", bool(reinterpret_cast<NSView *>(host.winId()).window.onActiveSpace)},
                            {"level", int(native().level)}, {"x", pet->x()}, {"y", pet->y()}};
            if (samples.isEmpty() || samples.last().toObject() != row)
                samples.append(row);
            QSaveFile file(output);
            if (!file.open(QIODevice::WriteOnly)) { wrote = false; return; }
            file.write(QJsonDocument(QJsonObject{{"status", "Pending"},
                       {"spaceChanges", changes.load()}, {"leftSpace", leftSpace},
                       {"returned", returned}, {"samples", samples}}).toJson());
            wrote &= file.commit();
        };
        QTimer sampler;
        connect(&sampler, &QTimer::timeout, this, sample);
        sampler.start(100);
        sample();
        QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(done), 180000);
        sample();
        QVERIFY(wrote);
        if (!(changes.load() >= 2 && leftSpace && returned))
        {
            qInfo() << "Spaces automatic evidence incomplete" << samples;
            QSKIP("Automatic Space round trip not established; report stays Pending, manual evidence is separate");
        }
        QTRY_VERIFY(native().onActiveSpace && native().visible);
        QCOMPARE(native().level, level);
        QCOMPARE(pet->position(), before);
        qInfo() << "Interactive Spaces review" << samples;
        sampler.stop();
        QSaveFile final(output);
        QVERIFY(final.open(QIODevice::WriteOnly));
        final.write(QJsonDocument(QJsonObject{{"status", "Passed"}, {"spaceChanges", changes.load()},
                    {"leftSpace", leftSpace}, {"returned", returned}, {"samples", samples}}).toJson());
        QVERIFY(final.commit());
        object.reset();
        pet.clear();
    }
    void routing()
    {
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_DESKTOP"))
            QSKIP("Enable QCAT_MASCOT_DESKTOP for real input checks");
        if (!CGPreflightPostEventAccess())
            QSKIP("System input permission unavailable; acceptance Pending");
        routingAttempted = true;
        token = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto bundledTarget =
            QCoreApplication::applicationDirPath() +
            "/../Helpers/mascot-macos-input-target.app/Contents/MacOS/mascot-macos-input-target";
        behind.start(QFileInfo::exists(bundledTarget) ? bundledTarget
                                                      : QString::fromUtf8(QCAT_MACOS_INPUT_TARGET),
                     {"QCat mascot input target:" + token});
        QTRY_VERIFY_WITH_TIMEOUT(state.value("pid").toInt() == behind.processId(), 10000);
        QVERIFY(behind.processId() != QCoreApplication::applicationPid());
        QTRY_VERIFY(state.value("keyWindow").toBool());
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        makePackage(directory.path());
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet {
    constrainToAvailableGeometry: false
    function message() { showBubble("Native input", {actions:[{id:"ok",text:"OK"}]}) }
    function dismiss() { closeBubble(bubbleId) }
    function updateMessage() { updateBubble(bubbleId, "Updated without activation") }
})",
                          QUrl());
        QTRY_VERIFY(component.status() != QQmlComponent::Loading);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY(object);
        pet = qobject_cast<QQuickWindow *>(object.get());
        QVERIFY(pet);
        auto *actor = qvariant_cast<Controller *>(pet->property("controller"));
        QVERIFY(actor);
        actor->setPackageSource(QUrl::fromLocalFile(directory.filePath("mascot.json")));
        QTRY_VERIFY(actor->status() != Controller::Loading && actor->status() != Controller::Null);
        QVERIFY2(actor->status() == Controller::Ready, qPrintable(actor->errorString()));
        pet->setPosition(240, 300);
        pet->show();
        QVERIFY(QTest::qWaitForWindowExposed(pet));
        settle();
        const auto caps = pet->property("capabilities").toMap();
        QVERIFY(caps.value("inputRegions").toMap().value("status").toString() != "Unavailable");
        QSignalSpy region(object.get(), SIGNAL(regionActivated(QString, QPointF)));
        QSignalSpy bubble(object.get(), SIGNAL(bubbleActionTriggered(QString, QString)));
        // Default input covers the entire visual window, even outside the body.
        QVERIFY(click({150, 130}));
        QTest::qWait(60);
        QCOMPARE(state.value("clicks").toInt(), 0);
        capture("default-blocking");
        pet->setProperty("inputRegionsEnabled", true);
        settle();
        QVERIFY(click({150, 130}));
        QTRY_COMPARE(state.value("clicks").toInt(), 1);
        QVERIFY(click({50, 60}));
        QTRY_COMPARE(region.count(), 1);
        QCOMPARE(state.value("clicks").toInt(), 1);
        QTRY_VERIFY(state.value("keyWindow").toBool());
        QVERIFY(key(0));
        QTRY_COMPARE(state.value("keys").toInt(), 1);
        capture("rectangle-and-passive-focus");
        actor->requestAction("ellipse");
        QTRY_COMPARE(actor->currentAction(), QString("ellipse"));
        settle();
        qInfo() << "Ellipse regions" << actor->interactionRegions() << "children"
                << surfaces().count;
        const NSInteger cornerHit = [NSWindow windowNumberAtPoint:point({21, 31})
                                      belowWindowWithWindowNumber:0];
        qInfo() << "Corner point" << QString::fromNSString(NSStringFromPoint(point({21, 31})))
                << "visual frame" << QString::fromNSString(NSStringFromRect(native().frame))
                << "Qt size" << pet->size();
        for (NSWindow *child in surfaces())
            if (child.windowNumber == cornerHit)
            {
                qInfo() << "Corner hit input frame"
                        << QString::fromNSString(NSStringFromRect(child.frame));
                NSArray *info = (NSArray *)CGWindowListCopyWindowInfo(
                    kCGWindowListOptionIncludingWindow, CGWindowID(child.windowNumber));
                qInfo() << "Server window bounds"
                        << QString::fromNSString([[info.firstObject
                               objectForKey:(NSString *)kCGWindowBounds] description]);
                [info release];
            }
        QVERIFY(click({21, 31}));
        QTRY_COMPARE(state.value("clicks").toInt(), 2);
        QVERIFY(click({55, 70}));
        QTRY_COMPARE(region.count(), 2);
        capture("ellipse");
        actor->requestAction("polygon");
        QTRY_COMPARE(actor->currentAction(), QString("polygon"));
        settle();
        QVERIFY(click({85, 100}));
        QTRY_COMPARE(state.value("clicks").toInt(), 3);
        QVERIFY(click({35, 45}));
        QTRY_COMPARE(region.count(), 3);
        capture("polygon");
        actor->requestAction("idle");
        actor->setFacing("left");
        settle();
        QVERIFY(click({40, 60}));
        QTRY_COMPARE(state.value("clicks").toInt(), 4);
        QVERIFY(click({150, 60}));
        QTRY_COMPARE(region.count(), 4);
        capture("mirror");
        actor->setFacing("right");
        actor->setPetScale(1.5);
        settle();
        QVERIFY(click({75, 90}));
        QTRY_COMPARE(region.count(), 5);
        capture("scale");
        actor->setPetScale(1);
        settle();
        pet->setProperty("inputTransparent", true);
        settle();
        QVERIFY(click({50, 60}));
        QTRY_COMPARE(state.value("clicks").toInt(), 5);
        QCOMPARE(region.count(), 5);
        pet->setProperty("inputTransparent", false);
        settle();
        QVERIFY(click({50, 60}));
        QTRY_COMPARE(region.count(), 6);
        capture("whole-window-passthrough");
        QVERIFY(QMetaObject::invokeMethod(object.get(), "message"));
        settle();
        auto *button = find(pet->contentItem(), "mascotBubbleAction_ok");
        QVERIFY(button);
        const auto buttonPoint =
            button->mapToScene(QPointF(button->width() / 2, button->height() / 2));
        pet->setProperty("inputTransparent", true);
        settle();
        QVERIFY(click(buttonPoint));
        QTRY_COMPARE(state.value("clicks").toInt(), 6);
        QCOMPARE(bubble.count(), 0);
        pet->setProperty("inputTransparent", false);
        settle();
        QVERIFY(click(buttonPoint));
        QTRY_COMPARE(bubble.count(), 1);
        QVERIFY(key(49));
        QTRY_COMPARE(bubble.count(), 2);
        capture("bubble-and-real-keyboard");
        QVERIFY(QMetaObject::invokeMethod(object.get(), "dismiss"));
        settle();
        const QPoint start = pet->position();
        QVERIFY(mouse({50, 60}, kCGEventLeftMouseDown));
        // Keep the final event in the pressed surface's global trajectory while
        // the visual window follows it. No fake delivery to Qt is used by this test.
        const auto global = pet->mapToGlobal(QPointF(50, 60));
        for (int i = 1; i <= 6; ++i)
        {
            const QPointF local = pet->mapFromGlobal(global + QPointF(i * 5, i * 3));
            QVERIFY(mouse(local, kCGEventLeftMouseDragged));
        }
        QVERIFY(mouse(pet->mapFromGlobal(global + QPointF(30, 18)), kCGEventLeftMouseUp));
        QTRY_COMPARE(pet->position(), start + QPoint(30, 18));
        capture("continuous-drag");
        pet->setProperty("inputRegionsEnabled", false);
        settle();
        QVERIFY(click({150, 130}));
        QTest::qWait(60);
        QCOMPARE(state.value("clicks").toInt(), 6);
        capture("restore-full-window-input");

        // A separate app owns the keyboard throughout passive changes. Verify
        // delivery with real keys, rather than only inspecting Qt focus flags.
        auto activateTarget = [&]
        {
            [[NSRunningApplication runningApplicationWithProcessIdentifier:behind.processId()]
                activateWithOptions:0];
        };
        int expectedKeys = state.value("keys").toInt();
        auto passiveKey = [&]() -> bool
        {
            if (!state.value("keyWindow").toBool() || native().keyWindow ||
                !pet->flags().testFlag(Qt::WindowDoesNotAcceptFocus))
                return false;
            return key(0);
        };
        activateTarget();
        QTRY_VERIFY(state.value("keyWindow").toBool());
        pet->hide();
        pet->show();
        QTest::qWait(150);
        QVERIFY(passiveKey());
        ++expectedKeys;
        QTRY_COMPARE(state.value("keys").toInt(), expectedKeys);
        actor->requestAction("ellipse");
        QTRY_COMPARE(actor->currentAction(), QString("ellipse"));
        actor->requestAction("idle");
        QTRY_COMPARE(actor->currentAction(), QString("idle"));
        QVERIFY(passiveKey());
        ++expectedKeys;
        QTRY_COMPARE(state.value("keys").toInt(), expectedKeys);
        QVERIFY(QMetaObject::invokeMethod(object.get(), "message"));
        QVERIFY(QMetaObject::invokeMethod(object.get(), "updateMessage"));
        QTest::qWait(150);
        QVERIFY(passiveKey());
        ++expectedKeys;
        QTRY_COMPARE(state.value("keys").toInt(), expectedKeys);
        capture("passive-show-action-bubble-update");

        const NSInteger raisedLevel = native().level;
        QVERIFY(raisedLevel > NSNormalWindowLevel);
        pet->setProperty("alwaysOnTop", false);
        QTRY_COMPARE(native().level, NSInteger(NSNormalWindowLevel));
        QTRY_COMPARE([NSWindow windowNumberAtPoint:point({50, 60})
                      belowWindowWithWindowNumber:0], NSInteger(state.value("window").toInt()));
        QVERIFY(passiveKey());
        ++expectedKeys;
        QTRY_COMPARE(state.value("keys").toInt(), expectedKeys);
        pet->setProperty("alwaysOnTop", true);
        QTRY_COMPARE(native().level, raisedLevel);
        QTest::qWait(150);
        QVERIFY(passiveKey());
        ++expectedKeys;
        QTRY_COMPARE(state.value("keys").toInt(), expectedKeys);
        capture("always-on-top-toggle-without-activation");

        auto focusBubble = [&]() -> bool
        {
            auto *action = find(pet->contentItem(), "mascotBubbleAction_ok");
            return action && click(action->mapToScene(
                                 QPointF(action->width() / 2, action->height() / 2)));
        };
        QVERIFY(focusBubble());
        QTRY_VERIFY(native().keyWindow);
        QTRY_VERIFY(pet->activeFocusItem() &&
                    pet->activeFocusItem()->objectName() == "mascotBubbleAction_ok");
        QVERIFY(key(48)); // Tab: action -> close.
        QTRY_VERIFY(pet->activeFocusItem() &&
                    pet->activeFocusItem()->objectName() == "mascotBubbleClose");
        QVERIFY(key(48, kCGEventFlagMaskShift));
        QTRY_VERIFY(pet->activeFocusItem() &&
                    pet->activeFocusItem()->objectName() == "mascotBubbleAction_ok");
        QVERIFY(key(48));
        QVERIFY(key(49)); // Space closes through the actual focused control.
        QTRY_VERIFY(pet->property("bubbleId").toString().isEmpty());
        QTRY_VERIFY(pet->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
        capture("real-tab-backtab-and-keyboard-close");

        for (const QString &transition : {QString("hide"), QString("passthrough"),
                                           QString("deactivate")})
        {
            activateTarget();
            QTRY_VERIFY(state.value("keyWindow").toBool());
            QVERIFY(QMetaObject::invokeMethod(object.get(), "message"));
            QTest::qWait(150);
            QVERIFY(focusBubble());
            QTRY_VERIFY(native().keyWindow);
            if (transition == "hide")
                pet->hide();
            else if (transition == "passthrough")
                pet->setProperty("inputTransparent", true);
            else
                activateTarget();
            QTRY_VERIFY(pet->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
            activateTarget();
            QTRY_VERIFY(state.value("keyWindow").toBool());
            pet->setProperty("inputTransparent", false);
            pet->show();
            QTest::qWait(150);
            QVERIFY(passiveKey());
            ++expectedKeys;
            QTRY_COMPARE(state.value("keys").toInt(), expectedKeys);
            capture("focus-release-" + transition);
            QVERIFY(QMetaObject::invokeMethod(object.get(), "dismiss"));
        }
        const auto hiddenPoint = point({50, 60});
        pet->hide();
        QTest::qWait(100);
        QCOMPARE([NSWindow windowNumberAtPoint:hiddenPoint belowWindowWithWindowNumber:0],
                 NSInteger(state.value("window").toInt()));
        NSWindow *owner = [native() retain];
        const auto ids = surfaceIds();
        object.reset();
        pet.clear();
        QTRY_VERIFY(retired(ids));
        [owner release];
        routingComplete = true;
    }
    void cleanupTestCase()
    {
        if (leftDown)
        {
            auto event = CGEventCreateMouseEvent(nullptr, kCGEventLeftMouseUp, previousCursor,
                                                 kCGMouseButtonLeft);
            CGEventPost(kCGHIDEventTap, event);
            CFRelease(event);
        }
        if (routingAttempted && CGPreflightPostEventAccess())
            CGWarpMouseCursorPosition(previousCursor);
        if (behind.state() != QProcess::NotRunning)
        {
            behind.terminate();
            if (!behind.waitForFinished(2000))
            {
                behind.kill();
                behind.waitForFinished();
            }
        }
        [[NSRunningApplication runningApplicationWithProcessIdentifier:previousApplication]
            activateWithOptions:0];
        const auto output = qEnvironmentVariable("QCAT_MASCOT_REPORT");
        if (!output.isEmpty())
        {
            QSaveFile file(output);
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write(
                QJsonDocument(QJsonObject{{"acceptance", "Pending"},
                                          {"inputAcceptance", routingComplete    ? "Passed"
                                                              : routingAttempted ? "Failed"
                                                                                 : "Pending"},
                                          {"nativeLifecycle", structuralComplete    ? "Passed"
                                                              : structuralAttempted ? "Failed"
                                                                                    : "Pending"},
                                          {"checks", evidence},
                                          {"qtVersion", qVersion()},
                                          {"postEventAccess", bool(CGPreflightPostEventAccess())}})
                    .toJson());
            QVERIFY(file.commit());
        }
    }
};
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QStringList arguments = app.arguments();
    if (arguments.removeAll("--native-input"))
        qputenv("QCAT_MASCOT_DESKTOP", "1");
    const bool requestAccess = arguments.removeAll("--request-input-access") > 0;
    const int report = arguments.indexOf("--report");
    if (report >= 0 && report + 1 < arguments.size())
    {
        qputenv("QCAT_MASCOT_REPORT", arguments[report + 1].toUtf8());
        arguments.removeAt(report + 1);
        arguments.removeAt(report);
    }
    if (QGuiApplication::platformName() != "cocoa" || QGuiApplication::screens().isEmpty())
    {
        const auto output = qEnvironmentVariable("QCAT_MASCOT_REPORT");
        if (!output.isEmpty())
        {
            QSaveFile file(output);
            if (!file.open(QIODevice::WriteOnly))
                return 1;
            file.write(QJsonDocument(QJsonObject{{"acceptance", "Pending"},
                                                 {"inputAcceptance", "Pending"},
                                                 {"checks", QJsonArray{}},
                                                 {"postEventAccess", false},
                                                 {"reason", "Cocoa desktop required"}})
                           .toJson());
            if (!file.commit())
                return 1;
        }
        qInfo("macOS native input acceptance Pending: Cocoa desktop required");
        return 2;
    }
    if (requestAccess)
    {
        const QString output = qEnvironmentVariable("QCAT_MASCOT_REPORT");
        if (output.isEmpty())
            return 1;
        auto save = [&]()
        {
            QSaveFile file(output);
            if (!file.open(QIODevice::WriteOnly))
                return false;
            file.write(QJsonDocument(QJsonObject{{"acceptance", "Pending"},
                {"inputAcceptance", "Pending"}, {"purpose", "explicit permission diagnostic"},
                {"bundlePath", QString::fromNSString(NSBundle.mainBundle.bundlePath)},
                {"postEventAccess", bool(CGPreflightPostEventAccess())}}).toJson());
            return file.commit();
        };
        if (!save())
            return 1;
        QQuickWindow notice;
        notice.setTitle("QCat test input permission - enable Accessibility for this app");
        notice.resize(600, 160);
        notice.setColor(QColor("#224466"));
        notice.show();
        [NSApp activateIgnoringOtherApps:YES];
        QTimer poll;
        QObject::connect(&poll, &QTimer::timeout, &app, [&]
        {
            if (!save()) { app.exit(1); return; }
            if (CGPreflightPostEventAccess())
                app.quit();
        });
        poll.start(500);
        // Only this explicit, user-authorized mode may request permission.
        // Normal tests preflight and remain Pending without opening a prompt.
        QTimer::singleShot(0, &app, [] { CGRequestPostEventAccess(); });
        QTimer::singleShot(180000, &app, [&] { save(); app.exit(2); });
        return app.exec();
    }
    MacosInputTest test;
    return QTest::qExec(&test, arguments);
}
#include "macosinputtest.moc"
