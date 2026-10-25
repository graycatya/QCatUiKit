// SPDX-License-Identifier: Apache-2.0
#ifndef QCAT_PLUGIN_ONLY
#include <QCatUiKit/ThemePalette>
#include <QCatUiKit/ThemePalette.h>
#include <QCatUiKit/qcatthemepalette.h>
#endif
#include <QDebug>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <memory>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
#ifndef QCAT_PLUGIN_ONLY
    QCatUiKit::ThemePalette palette;
    palette.setDensity(QCatUiKit::ThemePalette::Touch);
    if (palette.tokens().value("controlHeight").toInt() != 48)
        return 1;
#endif
    // No addImportPath(): use the installed Qt kit's normal import paths.
    QQmlEngine engine;
    QQmlComponent component(&engine);
#ifndef QCAT_PLUGIN_ONLY
    component.loadFromModule("InstallCheck", "Main");
#else
    component.setData(R"(
import QtQuick
import QCatUiKit
CatButton {
    text: "Installed Qt module"
    property string installedVersion: QCatUiKit.version
    property QtObject installedModel: TableModel {}
}
)", QUrl());
#endif
    std::unique_ptr<QObject> object(component.create());
    if (!object) {
        qCritical() << component.errors();
        return 2;
    }
    if (object->property("installedVersion").toString().isEmpty())
        return 3;
    qInfo() << "Installed Qt module OK" << object->property("installedVersion");
    return 0;
}
