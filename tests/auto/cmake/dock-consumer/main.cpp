// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/DockController.h>
#include <QCatUiKit/DockController>
#include <QCatUiKit/qcatdockcontroller.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlExpression>
#include <memory>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QCatUiKit::DockController controller;
    if (!controller.registerPanel("a") || !controller.registerPanel("b") ||
        !controller.dock("b", "a", QCatUiKit::DockController::Bottom) ||
        !controller.restoreLayout(controller.saveLayout()))
        return 1;
    if (!controller.floatPanel("b") || controller.floatingWindows().size() != 1 ||
        !controller.dockBack(controller.floatingWindowForPanel("b")))
        return 4;
    if (!controller.dockToWorkspace("b", QCatUiKit::DockController::Right, 0.65) ||
        controller.layout().value("orientation").toString() != "horizontal" ||
        controller.layout().value("ratio").toDouble() != 0.65 ||
        !controller.restoreLayout(controller.saveLayout()))
        return 5;
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(QCAT_INSTALLED_QML_PATH));
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QCatUiKit
CatDockWorkspace {
    width: 800; height: 600
    CatDockPanel { dockId: "editor"; CatTextField { text: "installed" } }
    CatDockPanel { dockId: "log"; CatLabel { text: "ready" } }
}
)",
                      QUrl());
    std::unique_ptr<QObject> root(component.create());
    if (!root)
    {
        qCritical() << component.errors();
        return 2;
    }
    auto *state = root->findChild<QCatUiKit::DockController *>();
    if (!state || state->panelIds().size() != 2)
        return 3;
    QQmlExpression edge(qmlContext(root.get()), root.get(),
                        QStringLiteral("controller.dockToWorkspace('log', 2, 0.65)"));
    if (!edge.evaluate().toBool() || edge.hasError())
        return 6;
    qInfo() << "Installed Dock consumer passed";
    return 0;
}
