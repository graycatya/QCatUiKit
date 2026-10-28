// SPDX-License-Identifier: Apache-2.0
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QScreen>
#include <QSysInfo>
#include <cstdio>

static QString session(const QString &platform, const QString &desktopSession)
{
    if (platform.startsWith("wayland"))
        return "NativeWayland";
    if (platform == "xcb")
        return desktopSession == "wayland" ? "XWayland" :
               desktopSession == "x11" ? "X11" : "X11OrXWaylandUnknown";
    if (platform == "offscreen" || platform == "minimal" || platform == "vnc")
        return "Headless";
    if (platform == "windows")
        return "Windows";
    if (platform == "cocoa")
        return "macOS";
    return "Unknown";
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (app.arguments().contains("--self-test")) {
        const bool ok = session("wayland", "x11") == "NativeWayland"
            && session("wayland-egl", "wayland") == "NativeWayland"
            && session("xcb", "wayland") == "XWayland"
            && session("xcb", "x11") == "X11"
            && session("xcb", "") == "X11OrXWaylandUnknown"
            && session("offscreen", "wayland") == "Headless"
            && session("minimal", "x11") == "Headless"
            && session("cocoa", "") == "macOS"
            && session("windows", "") == "Windows";
        return ok ? 0 : 1;
    }
    const QString platform = QGuiApplication::platformName();
    const QString kind = session(platform, qEnvironmentVariable("XDG_SESSION_TYPE"));
    QJsonObject capabilities;
    const QStringList names = {"transparency", "inputRegions", "wholeWindowPassthrough",
        "drag", "position", "smoothMovement", "alwaysOnTop", "focus", "multiScreenDpi",
        "bubblePlacement", "tray"};
    for (const QString &name : names) {
        capabilities.insert(name, QJsonObject{
            {"status", "Unknown"},
            {"reason", kind == "Headless" ? "No desktop input or compositor evidence in headless mode"
                                         : "Requires interactive platform verification"}});
    }
    QJsonArray screens;
    for (QScreen *screen : app.screens()) {
        const QRect r = screen->availableGeometry();
        screens.append(QJsonObject{{"name", screen->name()}, {"dpr", screen->devicePixelRatio()},
            {"availableGeometry", QJsonArray{r.x(), r.y(), r.width(), r.height()}}});
    }
    QJsonObject report{{"schemaVersion", 1}, {"system", QSysInfo::prettyProductName()},
        {"qtVersion", qVersion()}, {"qpaPlatform", platform}, {"session", kind},
        {"desktop", qEnvironmentVariable("XDG_CURRENT_DESKTOP")},
        {"declaredSession", qEnvironmentVariable("XDG_SESSION_TYPE")},
        {"graphicsApi", "NotInitialized"}, {"requestedQuickBackend", qEnvironmentVariable("QT_QUICK_BACKEND")},
        {"acceptance", "Pending"}, {"companionVersion", QJsonValue::Null},
        {"companionConnection", "NotProbed"}, {"capabilities", capabilities}, {"screens", screens}};
    if (app.arguments().contains("--json")) {
        const QByteArray json = QJsonDocument(report).toJson();
        std::fwrite(json.constData(), 1, size_t(json.size()), stdout);
        return 0;
    }
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("probeReport", QString::fromUtf8(QJsonDocument(report).toJson()));
    engine.load(QUrl("qrc:/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (!window)
        return 1;
    QObject::connect(window, &QQuickWindow::sceneGraphInitialized, &app, [window, report]() mutable {
        report.insert("graphicsApi", int(window->rendererInterface()->graphicsApi()));
        const QString updated = QString::fromUtf8(QJsonDocument(report).toJson());
        window->setProperty("renderedReport", updated);
    }, Qt::QueuedConnection);
    return app.exec();
}
