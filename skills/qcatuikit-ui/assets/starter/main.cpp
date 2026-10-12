// SPDX-License-Identifier: Apache-2.0
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(
        {"qml-import-path", "QML module root for a development installation.", "path"});
    parser.addOption({"smoke-test", "Load the window and exit."});
    parser.addOption({"screenshot", "Save the visible window and exit.", "path"});
    parser.process(app);

    QQmlApplicationEngine engine;
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    if (parser.isSet("qml-import-path"))
        engine.addImportPath(parser.value("qml-import-path"));
    engine.loadFromModule("QCatUiKitStarter", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;
    if (parser.isSet("smoke-test") || parser.isSet("screenshot"))
    {
        QTimer::singleShot(
            700, &app,
            [&]
            {
                if (parser.isSet("screenshot"))
                {
                    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
                    if (!window || !window->grabWindow().save(parser.value("screenshot")))
                    {
                        app.exit(2);
                        return;
                    }
                }
                app.quit();
            });
    }
    return app.exec();
}
