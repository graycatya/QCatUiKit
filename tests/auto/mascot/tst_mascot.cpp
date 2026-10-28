// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotpackage_p.h"
#include <QCatUiKit/MascotPetController>
#include <QCatUiKit/VectorItem>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
using namespace QCatUiKit;
using Controller = MascotPetController;
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
        QSignalSpy ended(&c, &Controller::actionEnded);
        if (!qEnvironmentVariableIsSet("QCAT_MASCOT_GPU"))
        {
            QTRY_COMPARE(c.status(), Controller::Error);
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
        QTRY_COMPARE_WITH_TIMEOUT(c.status(), Controller::Ready, 10000);
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
            QSKIP("Native QRhi window required");
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
