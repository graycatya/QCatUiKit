// SPDX-License-Identifier: Apache-2.0
#include "gallerylanguage.h"
#include "galleryleddemo.h"
#include <QCatUiKit/qcatledproject.h>
#include <cmath>
#include <QCatUiKit/qcatledpreview.h>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <atomic>
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QtQml/qqmlextensionplugin.h>

Q_IMPORT_QML_PLUGIN(QCatUiKit_GalleryPlugin)

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QCatUiKit Gallery"));
    QIcon applicationIcon;
    for (const int size : {16, 24, 32, 48, 64, 128, 256, 512})
        applicationIcon.addFile(QStringLiteral(":/qt/qml/QCatUiKit/Gallery/assets/brand/png/qcat-uikit-%1.png").arg(size), QSize(size, size));
    app.setWindowIcon(applicationIcon);
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#ifdef Q_OS_WIN
    font.setFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei UI")});
#endif
    app.setFont(font);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"smoke-test", "Load the gallery and exit after rendering."});
    parser.addOption({"screenshot", "Save a rendered gallery image and exit.", "path"});
    parser.addOption({"chart-demo", "Open a chart demonstration (0-29) in its matching chart page.", "index"});
    parser.addOption({"led-count", "LED count (0-100000); use --page led-preview.", "count", "4096"});
    parser.addOption({"led-layout", "LED layout: matrix or sphere.", "layout", "matrix"});
    parser.addOption({"led-interval", "Producer interval in milliseconds (4-1000).", "ms", "16"});
    parser.addOption({"led-benchmark", "Measure LED passes and window presentation after a 2s warmup, print JSON and exit.", "seconds"});
    parser.addOption({"led-report", "Also save the LED benchmark JSON to this file.", "path"});
    parser.addOption({"dark", "Use the dark theme."});
    parser.addOption({"density", "Use desktop, touch or mobile density.", "density", "desktop"});
    parser.addOption({"width", "Initial window width.", "pixels", "1200"});
    parser.addOption({"language", "Use en or zh_CN; defaults to the system language.", "language"});
    parser.addOption({"page", "Open home, all, settings, or a component id (e.g. button).", "page", "home"});
    parser.process(app);
    const QString density = parser.value("density");
    if (density != "desktop" && density != "touch" && density != "mobile")
        parser.showHelp(2);
    const QString languageName = parser.value("language");
    if (parser.isSet("language") && languageName != "en" && languageName != "zh_CN")
        parser.showHelp(2);
    bool validWidth = false;
    const int width = parser.value("width").toInt(&validWidth);
    if (!validWidth || width < 360)
        parser.showHelp(2);

    // Direct frameSwapped callbacks can still run during engine/window teardown.
    // Keep this counter alive until after the engine has destroyed its windows.
    std::atomic<quint64> presented{0};
    QQmlApplicationEngine engine;
    const QDir executableDirectory(QCoreApplication::applicationDirPath());
    engine.addImportPath(executableDirectory.filePath(QStringLiteral(QCAT_GALLERY_QML_RELATIVE_PATH)));
    engine.addImportPath(executableDirectory.filePath(QStringLiteral("qml")));
    auto *language = engine.singletonInstance<GalleryLanguage *>("QCatUiKit.Gallery", "GalleryLanguage");
    if (!language)
        return 1;
    if (parser.isSet("language"))
        language->setLanguage(languageName);
    engine.setInitialProperties({{"darkTheme", parser.isSet("dark")},
                                 {"initialDensity", density == "touch" ? 1 : density == "mobile" ? 2 : 0},
                                 {"initialPage", parser.value("page")},
                                 {"width", width}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("QCatUiKit.Gallery", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;
    if (engine.rootObjects().first()->property("invalidInitialPage").toBool())
        parser.showHelp(2);
    if (parser.isSet("chart-demo"))
    {
        bool valid = false;
        const int index = parser.value("chart-demo").toInt(&valid);
        bool opened = false;
        if (!valid || index < 0 || index > 29
            || !QMetaObject::invokeMethod(engine.rootObjects().first(), "showChartDemo",
                                         Q_RETURN_ARG(bool, opened), Q_ARG(int, index)) || !opened)
            parser.showHelp(2);
    }
    const bool ledOptions = parser.isSet("led-count") || parser.isSet("led-layout")
                            || parser.isSet("led-interval") || parser.isSet("led-benchmark") || parser.isSet("led-report");
    auto *ledDemo = engine.rootObjects().first()->findChild<QCatLedProject *>(QStringLiteral("ledProjectData"));
    auto *ledWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    QElapsedTimer benchmarkClock;
    QList<quint64> initialFrames, initialColors;
    if (ledOptions)
    {
        bool countValid = false, intervalValid = false, secondsValid = true;
        const int count = parser.value("led-count").toInt(&countValid);
        const int interval = parser.value("led-interval").toInt(&intervalValid);
        const int seconds = parser.isSet("led-benchmark")
            ? parser.value("led-benchmark").toInt(&secondsValid) : 0;
        const QString layout = parser.value("led-layout");
        if (!ledDemo || !countValid || count < 0 || count > 100000 || !intervalValid
            || interval < 4 || interval > 1000 || !secondsValid
            || (parser.isSet("led-benchmark") && (seconds < 1 || seconds > 3600))
            || (parser.isSet("led-report") && !parser.isSet("led-benchmark"))
            || (layout != "matrix" && layout != "sphere"))
            parser.showHelp(2);
        auto config=ledDemo->layout()->configuration();
        if(layout=="sphere"){config["type"]="SphereSurface";config["count"]=count;}
        else {int x=qMax(1,int(std::sqrt(count)));while(count && count%x)--x;config["type"]="Matrix2D";config["countX"]=count?x:0;config["countY"]=count?count/x:1;}
        ledDemo->layout()->applyNow(config);
        ledDemo->deckA()->setProductionInterval(interval);
        ledDemo->deckB()->setProductionInterval(interval);
        if (parser.isSet("led-benchmark"))
        {
            if (!ledWindow || parser.isSet("smoke-test") || parser.isSet("screenshot"))
                parser.showHelp(2);
            QObject::connect(ledWindow, &QQuickWindow::frameSwapped, &app,
                             [&] { presented.fetch_add(1); }, Qt::DirectConnection);
            QTimer::singleShot(2000, &app, [&, seconds] {
                for (const auto *name : {"ledDeckA", "ledOutput", "ledDeckB"})
                {
                    const auto *item = ledWindow->findChild<QCatLedPreview *>(QString::fromLatin1(name));
                    if (!item) { QCoreApplication::exit(3); return; }
                    initialFrames.append(item->renderedFrames());
                    initialColors.append(item->source()->submittedFrames());
                }
                presented.store(0);
                benchmarkClock.start();
                QTimer::singleShot(seconds * 1000, &app, [&] {
                    const double elapsed = benchmarkClock.elapsed() / 1000.0;
                    QJsonArray previews;
                    int index = 0;
                    bool valid = true;
                    for (const auto *name : {"ledDeckA", "ledOutput", "ledDeckB"})
                    {
                        const auto *item = ledWindow->findChild<QCatLedPreview *>(QString::fromLatin1(name));
                        const quint64 passes = item->renderedFrames() - initialFrames[index];
                        const quint64 colors = item->source()->submittedFrames() - initialColors[index++];
                        valid = valid && passes > 0 && item->renderError().isEmpty();
                        previews.append(QJsonObject{{"name", QString::fromLatin1(name)},
                            {"points", item->source()->pointCount()}, {"shape",int(item->pointShape())}, {"sizeMode",int(item->sizeMode())}, {"renderPassFps", passes / elapsed},
                            {"colorUpdateHz", colors / elapsed}, {"renderingInfo", item->renderingInfo()},
                            {"error", item->renderError()}});
                    }
                    const QJsonObject report{{"qt", QString::fromLatin1(qVersion())},
                        {"seconds", elapsed}, {"layout", parser.value("led-layout")},
                        {"producerIntervalMs", ledDemo->deckA()->productionInterval()},
                        {"windowPresentedFps", presented.load() / elapsed}, {"previews", previews},
                        {"valid", valid}};
                    const QByteArray json = QJsonDocument(report).toJson(QJsonDocument::Indented);
                    qInfo().noquote() << json;
                    if (parser.isSet("led-report"))
                    {
                        QFile file(parser.value("led-report"));
                        if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size())
                        { QCoreApplication::exit(4); return; }
                    }
                    QCoreApplication::exit(valid ? 0 : 3);
                });
            });
        }
    }
    if (parser.isSet("smoke-test") || parser.isSet("screenshot"))
    {
        QTimer::singleShot(700, &app, [&] {
            if (parser.isSet("screenshot"))
            {
                auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
                if (!window || !window->grabWindow().save(parser.value("screenshot")))
                {
                    QCoreApplication::exit(2);
                    return;
                }
            }
            app.quit();
        });
    }
    return app.exec();
}
