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
    parser.addOption({"smoke-test", "Load the gallery and exit after rendering."});
    parser.addOption({"screenshot", "Save a rendered gallery image and exit.", "path"});
    parser.addOption({"dark", "Use the dark theme."});
    parser.addOption({"density", "Use desktop, touch or mobile density.", "density", "desktop"});
    parser.addOption({"width", "Initial window width.", "pixels", "1100"});
    parser.process(app);
    const QString density = parser.value("density");
    if (density != "desktop" && density != "touch" && density != "mobile")
        parser.showHelp(2);
    bool validWidth = false;
    const int width = parser.value("width").toInt(&validWidth);
    if (!validWidth || width < 360)
        parser.showHelp(2);

    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"darkTheme", parser.isSet("dark")},
                                 {"initialDensity", density == "touch"    ? 1
                                                    : density == "mobile" ? 2
                                                                          : 0},
                                 {"width", width}});
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("QCatUiKit.Gallery", "Main");
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
                        QCoreApplication::exit(2);
                        return;
                    }
                }
                app.quit();
            });
    }
    return app.exec();
}
