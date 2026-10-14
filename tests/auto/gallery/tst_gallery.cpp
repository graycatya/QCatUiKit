// SPDX-License-Identifier: Apache-2.0
#include <QtQuickTest/quicktest.h>
#include <QtQml/qqmlextensionplugin.h>
#include <QFont>
#include <QGuiApplication>
Q_IMPORT_QML_PLUGIN(QCatUiKit_GalleryPlugin)
class GalleryTestSetup : public QObject
{
    Q_OBJECT
public slots:
    void applicationAvailable()
    {
        QFont font = QGuiApplication::font();
        const QString family = font.defaultFamily();
        if (!family.isEmpty())
            font.setFamilies({family});
        QGuiApplication::setFont(font);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(qcatgallery, GalleryTestSetup)
#include "tst_gallery.moc"
