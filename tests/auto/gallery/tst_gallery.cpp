// SPDX-License-Identifier: Apache-2.0
#include "../testfonts.h"
#include <QFont>
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
  public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
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
