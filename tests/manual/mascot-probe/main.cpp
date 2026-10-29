// SPDX-License-Identifier: Apache-2.0
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSaveFile>
#include <QScreen>
#include <QSysInfo>
#include <cstdio>
#if defined(Q_OS_LINUX)
#include "linuxprobe.h"
#include <QApplication>
#include <QTimer>
#endif
#ifdef Q_OS_MACOS
#include "macosprobe.h"
#include <QTimer>
#endif
#ifdef Q_OS_WIN
#include "windowsprobe.h"
#include <QApplication>
#include <QTimer>
using ProbeApplication = QApplication;
#elif defined(Q_OS_LINUX)
using ProbeApplication = QApplication;
#else
using ProbeApplication = QGuiApplication;
#endif

static QString session(const QString &platform, const QString &desktopSession)
{
    if (platform.startsWith("wayland"))
        return "NativeWayland";
    if (platform == "xcb")
        return desktopSession == "wayland" ? "XWayland"
               : desktopSession == "x11"   ? "X11"
                                           : "X11OrXWaylandUnknown";
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
    ProbeApplication app(argc, argv);
#ifdef Q_OS_LINUX
    const int linuxTarget = app.arguments().indexOf("--linux-target");
    if (linuxTarget >= 0)
        return linuxTarget + 1 < app.arguments().size()
                   ? LinuxProbe::runTarget(app.arguments()[linuxTarget + 1])
                   : 2;
#endif
#ifdef Q_OS_WIN
    const auto targetArgument = app.arguments().indexOf("--windows-target");
    if (targetArgument >= 0 && targetArgument + 1 < app.arguments().size())
        return WindowsProbe::runTarget(app.arguments()[targetArgument + 1]);
#endif
#ifdef Q_OS_MACOS
    if (app.arguments().contains("--macos-target"))
        return QGuiApplication::platformName() == "cocoa" ? MacosProbe::runTarget() : 2;
#endif
    if (app.arguments().contains("--self-test"))
    {
        const bool ok = session("wayland", "x11") == "NativeWayland" &&
                        session("wayland-egl", "wayland") == "NativeWayland" &&
                        session("xcb", "wayland") == "XWayland" && session("xcb", "x11") == "X11" &&
                        session("xcb", "") == "X11OrXWaylandUnknown" &&
                        session("offscreen", "wayland") == "Headless" &&
                        session("minimal", "x11") == "Headless" &&
                        session("cocoa", "") == "macOS" && session("windows", "") == "Windows";
        return ok ? 0 : 1;
    }
    const QString platform = QGuiApplication::platformName();
    const QString kind = session(platform, qEnvironmentVariable("XDG_SESSION_TYPE"));
    QJsonObject capabilities;
    const QStringList names = {"transparency",
                               "inputRegions",
                               "wholeWindowPassthrough",
                               "drag",
                               "position",
                               "smoothMovement",
                               "alwaysOnTop",
                               "focus",
                               "multiScreenDpi",
                               "bubblePlacement",
                               "tray"};
    for (const QString &name : names)
    {
        capabilities.insert(
            name,
            QJsonObject{{"status", "Unknown"},
                        {"reason", kind == "Headless"
                                       ? "No desktop input or compositor evidence in headless mode"
                                       : "Requires interactive platform verification"}});
    }
    QJsonArray screens;
    for (QScreen *screen : app.screens())
    {
        const QRect r = screen->availableGeometry();
        screens.append(
            QJsonObject{{"name", screen->name()},
                        {"dpr", screen->devicePixelRatio()},
                        {"availableGeometry", QJsonArray{r.x(), r.y(), r.width(), r.height()}}});
    }
    QJsonObject report{{"schemaVersion", 1},
                       {"system", QSysInfo::prettyProductName()},
                       {"qtVersion", qVersion()},
                       {"qpaPlatform", platform},
                       {"session", kind},
                       {"desktop", qEnvironmentVariable("XDG_CURRENT_DESKTOP")},
                       {"declaredSession", qEnvironmentVariable("XDG_SESSION_TYPE")},
                       {"graphicsApi", "NotInitialized"},
                       {"requestedQuickBackend", qEnvironmentVariable("QT_QUICK_BACKEND")},
                       {"acceptance", "Pending"},
                       {"companionVersion", QJsonValue::Null},
                       {"companionConnection", "NotProbed"},
                       {"capabilities", capabilities},
                       {"screens", screens}};
    if (app.arguments().contains("--json") && !app.arguments().contains("--windows-check") &&
        !app.arguments().contains("--windows-prototype") &&
        !app.arguments().contains("--macos-check") &&
        !app.arguments().contains("--macos-prototype") &&
        !app.arguments().contains("--linux-check") &&
        !app.arguments().contains("--linux-prototype"))
    {
        const QByteArray json = QJsonDocument(report).toJson();
        std::fwrite(json.constData(), 1, size_t(json.size()), stdout);
        return 0;
    }
    QQmlApplicationEngine engine;
#ifdef Q_OS_LINUX
    const int linuxCheck = app.arguments().indexOf("--linux-check");
    if (linuxCheck >= 0 || app.arguments().contains("--linux-prototype"))
    {
        app.setQuitOnLastWindowClosed(false);
        if ((platform != "xcb" && !platform.startsWith("wayland")) ||
            (linuxCheck >= 0 && linuxCheck + 1 >= app.arguments().size()))
        {
            report.insert("prototypeAcceptance", "Pending");
            report.insert("prototypeReason",
                          "Linux xcb or Wayland desktop and report path required");
            if (linuxCheck >= 0 && linuxCheck + 1 < app.arguments().size())
            {
                QSaveFile file(app.arguments()[linuxCheck + 1]);
                if (!file.open(QIODevice::WriteOnly) ||
                    file.write(QJsonDocument(report).toJson()) < 0 || !file.commit())
                    return 1;
            }
            return 2;
        }
        LinuxProbe probe(&engine, report);
        engine.setInitialProperties({{"probe", QVariant::fromValue(&probe)}});
        engine.load(QUrl("qrc:/LinuxProbe.qml"));
        if (engine.rootObjects().isEmpty())
            return 1;
        if (linuxCheck >= 0)
            qobject_cast<QQuickWindow *>(engine.rootObjects().first())->showMinimized();
        const QString output = linuxCheck >= 0 ? app.arguments()[linuxCheck + 1] : QString();
        QObject::connect(&probe, &LinuxProbe::finished, &app, [&app](int code) { app.exit(code); });
        QTimer::singleShot(0, &probe, [&probe, output] { probe.start(output); });
        const int result = app.exec();
        for (auto *root : engine.rootObjects())
            delete root;
        return result;
    }
#endif
    const int macosCheck = app.arguments().indexOf("--macos-check");
    if (macosCheck >= 0 || app.arguments().contains("--macos-prototype"))
    {
        if (platform != "cocoa" || (macosCheck >= 0 && macosCheck + 1 >= app.arguments().size()))
        {
            report.insert("prototypeAcceptance", "Pending");
            report.insert("prototypeReason", "macOS Cocoa interactive desktop is required");
            if (macosCheck >= 0 && macosCheck + 1 < app.arguments().size())
            {
                QSaveFile file(app.arguments()[macosCheck + 1]);
                if (!file.open(QIODevice::WriteOnly))
                    return 1;
                file.write(QJsonDocument(report).toJson());
                if (!file.commit())
                    return 1;
            }
            return 2;
        }
#ifdef Q_OS_MACOS
        MacosProbe probe(report);
        engine.setInitialProperties({{"probe", QVariant::fromValue(&probe)}});
        engine.load(QUrl("qrc:/MacosProbe.qml"));
        if (engine.rootObjects().isEmpty())
            return 1;
        if (macosCheck >= 0)
            engine.rootObjects().first()->setProperty("visible", false);
        const QString output = macosCheck >= 0 ? app.arguments()[macosCheck + 1] : QString();
        QObject::connect(&probe, &MacosProbe::finished, &app, [&app](int code) { app.exit(code); });
        QTimer::singleShot(0, &probe, [&probe, output] { probe.start(output); });
        const int result = app.exec();
        for (auto *root : engine.rootObjects())
            delete root;
        return result;
#else
        return 2;
#endif
    }
#ifdef Q_OS_WIN
    const bool windowsPrototype = app.arguments().contains("--windows-prototype");
    const int windowsCheck = app.arguments().indexOf("--windows-check");
    if (windowsPrototype || windowsCheck >= 0)
    {
        if (platform != "windows" ||
            (windowsCheck >= 0 && windowsCheck + 1 >= app.arguments().size()))
        {
            std::fputs("Windows prototype requires the windows QPA desktop and --windows-check "
                       "<report.json>\n",
                       stderr);
            report.insert("prototypeAcceptance", "Pending");
            report.insert("prototypeReason", "Windows QPA interactive desktop is required");
            if (windowsCheck >= 0 && windowsCheck + 1 < app.arguments().size())
            {
                QSaveFile file(app.arguments()[windowsCheck + 1]);
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(QJsonDocument(report).toJson());
                    file.commit();
                }
            }
            return 2;
        }
        WindowsProbe probe(&engine, report);
        engine.setInitialProperties({{"probe", QVariant::fromValue(&probe)}});
        engine.load(QUrl("qrc:/WindowsProbe.qml"));
        if (engine.rootObjects().isEmpty())
            return 1;
        if (windowsCheck >= 0)
        {
            const QString path = app.arguments()[windowsCheck + 1];
            QObject::connect(&probe, &WindowsProbe::finished, &app,
                             [&app](bool passed) { app.exit(passed ? 0 : 1); });
            QTimer::singleShot(0, &probe, [&probe, path] { probe.runChecks(path); });
        }
        else
            QTimer::singleShot(0, &probe, &WindowsProbe::start);
        const int result = app.exec();
        // The engine must release QML references before its context object is destroyed.
        for (auto *root : engine.rootObjects())
            delete root;
        return result;
    }
#endif
#ifndef Q_OS_WIN
    if (app.arguments().contains("--windows-prototype") ||
        app.arguments().contains("--windows-check"))
    {
        std::fputs("Windows prototype is only built on Windows\n", stderr);
        return 2;
    }
#endif
    engine.rootContext()->setContextProperty("probeReport",
                                             QString::fromUtf8(QJsonDocument(report).toJson()));
    engine.load(QUrl("qrc:/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (!window)
        return 1;
    QObject::connect(
        window, &QQuickWindow::sceneGraphInitialized, &app,
        [window, report]() mutable
        {
            report.insert("graphicsApi", int(window->rendererInterface()->graphicsApi()));
            const QString updated = QString::fromUtf8(QJsonDocument(report).toJson());
            window->setProperty("renderedReport", updated);
        },
        Qt::QueuedConnection);
    return app.exec();
}
