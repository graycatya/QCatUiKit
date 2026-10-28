// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/MascotPetController>
#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <cstdio>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QCatUiKit::MascotPetController controller;
    QQmlApplicationEngine engine;
#ifdef QCAT_MASCOT_IMPORT_PATH
    engine.addImportPath(QStringLiteral(QCAT_MASCOT_IMPORT_PATH));
#endif
    const QStringList args = app.arguments();
    auto option = [&](const QString &name)
    {
        const int i = args.indexOf(name);
        return i >= 0 && i + 1 < args.size() ? args[i + 1] : QString();
    };
    const QString importPath = option("--import-path");
    if (!importPath.isEmpty())
        engine.addImportPath(importPath);
    const QString input = option("--package");
    controller.setPackageSource(
        input.isEmpty()
            ? QUrl(args.contains("--lottie") ? "qrc:/mascot-player/catgray-lottie/mascot.json"
                                             : "qrc:/mascot-player/catgray/manifest.json")
            : QUrl::fromUserInput(input, QDir::currentPath(), QUrl::AssumeLocalFile));
    const QString action = option("--action");
    QObject::connect(&controller, &QCatUiKit::MascotPetController::statusChanged, &app,
                     [&controller]
                     {
                         if (controller.status() == QCatUiKit::MascotPetController::Error)
                             std::fprintf(stderr, "%s\n", qPrintable(controller.errorString()));
                     });
    if (!option("--scale").isEmpty())
    {
        bool ok = false;
        const qreal scale = option("--scale").toDouble(&ok);
        if (!ok || !qIsFinite(scale) || scale <= 0)
            return 1;
        controller.setPetScale(scale);
    }
    engine.setInitialProperties({{"controller", QVariant::fromValue(&controller)},
                                 {"previewDark", args.contains("--dark")}});
    engine.load(QUrl("qrc:/mascot-player/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;
    auto *preview = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (preview && !option("--width").isEmpty())
        preview->setWidth(qMax(420, option("--width").toInt()));
    if (preview && !option("--height").isEmpty())
        preview->setHeight(qMax(520, option("--height").toInt()));
    const QString screenshot = option("--screenshot");
    QTimer captureTimer;
    QElapsedTimer deadline;
    deadline.start();
    bool actionSubmitted = action.isEmpty();
    int stableTicks = 0;
    if (!action.isEmpty() || !screenshot.isEmpty())
    {
        captureTimer.setInterval(100);
        QObject::connect(&captureTimer, &QTimer::timeout, &app,
                         [&]
                         {
                             if (controller.status() == QCatUiKit::MascotPetController::Error ||
                                 deadline.elapsed() > 30000)
                             {
                                 captureTimer.stop();
                                 if (!screenshot.isEmpty())
                                     app.exit(1);
                                 return;
                             }
                             if (controller.status() != QCatUiKit::MascotPetController::Ready)
                             {
                                 stableTicks = 0;
                                 return;
                             }
                             if (!actionSubmitted)
                             {
                                 actionSubmitted = true;
                                 controller.requestAction(action);
                                 return;
                             }
                             if (!action.isEmpty() && controller.currentAction() != action)
                                 return;
                             if (++stableTicks < 4)
                                 return;
                             captureTimer.stop();
                             if (!screenshot.isEmpty())
                                 app.exit(preview && preview->grabWindow().save(screenshot) ? 0 : 1);
                         });
        captureTimer.start();
    }
    return app.exec();
}
