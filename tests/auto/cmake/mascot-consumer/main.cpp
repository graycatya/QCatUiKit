// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/MascotPetController.h>
#include <QCatUiKit/MascotPetController>
#include <QCatUiKit/qcatmascotpetcontroller.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <memory>
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(QCAT_INSTALLED_QML_PATH));
    QQmlComponent component(&engine);
    component.setData(R"(import QtQuick
import QCatUiKit
CatMascotPet { visible: false }
)",
                      QUrl());
    std::unique_ptr<QObject> root(component.create());
    if (!root)
    {
        qCritical() << component.errors();
        return 1;
    }
    auto *window = qobject_cast<QQuickWindow *>(root.get());
    auto *controller =
        qvariant_cast<QCatUiKit::MascotPetController *>(root->property("controller"));
    if (!window || !controller || controller->status() != QCatUiKit::MascotPetController::Null)
        return 2;
    window->show();
    window->close();
    if (window->isVisible())
        return 3;
    qInfo() << "Installed Mascot consumer passed";
    return 0;
}
