// SPDX-License-Identifier: Apache-2.0
#include "../testfonts.h"
#include <QFont>
#include <QQuickItem>
#include <QQuickWindow>
#ifdef Q_OS_MACOS
#import <AppKit/AppKit.h>
#endif
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryDir>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQuickTest/quicktest.h>
Q_IMPORT_QML_PLUGIN(QCatUiKit_GalleryPlugin)
class GalleryTestSetup : public QObject
{
    Q_OBJECT
    QTemporaryDir exportDirectory;
  public:
    Q_INVOKABLE bool nativeCocoa() const
    {
        return QGuiApplication::platformName() == "cocoa";
    }
    Q_INVOKABLE void activateHost(QQuickItem *item) const
    {
#ifdef Q_OS_MACOS
        [NSApp activateIgnoringOtherApps:YES];
#endif
        if (item && item->window())
            item->window()->requestActivate();
    }
    Q_INVOKABLE qreal nativePositionTolerance(QQuickItem *item) const
    {
#ifdef Q_OS_MACOS
        if (item && item->window())
        {
            const qreal ratio = item->window()->devicePixelRatio();
            if (qAbs(ratio - qRound(ratio)) > 0.000001)
            {
                NSView *view = reinterpret_cast<NSView *>(item->window()->winId());
                return 1.0 / view.window.backingScaleFactor;
            }
        }
#else
        Q_UNUSED(item);
#endif
        return 0.000001;
    }
    Q_INVOKABLE QPointF nativePosition(QQuickItem *item) const
    {
#ifdef Q_OS_MACOS
        if (item && item->window())
        {
            auto *window = item->window();
            NSView *view = reinterpret_cast<NSView *>(window->winId());
            const QPointF local = item->mapToScene(QPointF());
            const NSRect bounds = view.bounds;
            const NSPoint point = NSMakePoint(bounds.origin.x + local.x() * bounds.size.width / window->width(),
                                              bounds.origin.y + local.y() * bounds.size.height / window->height());
            const NSPoint screen = [view.window convertPointToScreen:[view convertPoint:point toView:nil]];
            return QPointF(screen.x, screen.y);
        }
#else
        Q_UNUSED(item);
#endif
        return {};
    }
  public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        engine->rootContext()->setContextProperty(QStringLiteral("nativeMascotTest"), this);
        engine->rootContext()->setContextProperty(
            QStringLiteral("chartExportTestDirectory"),
            QUrl::fromLocalFile(exportDirectory.path() + QLatin1Char('/')));
    }
    void applicationAvailable()
    {
        initializeTestFonts();
    }
};

QUICK_TEST_MAIN_WITH_SETUP(qcatgallery, GalleryTestSetup)
#include "tst_gallery.moc"
