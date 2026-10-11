// SPDX-License-Identifier: Apache-2.0
#include "gallerylanguage.h"
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
    parser.addOption({"chart-demo", "Choose a chart demonstration (0-14); use with a chart page.", "index"});
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
        QObject *picker = engine.rootObjects().first()->findChild<QObject *>(QStringLiteral("chartDemoPicker"));
        if (!valid || index < 0 || index > 14 || !picker)
            parser.showHelp(2);
        QMetaObject::invokeMethod(picker, "activated", Q_ARG(int, index));
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