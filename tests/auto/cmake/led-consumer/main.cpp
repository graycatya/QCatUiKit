// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/LedPreview.h>
#include <QCatUiKit/LedPreview>
#include <QCatUiKit/LedSource.h>
#include <QCatUiKit/LedSource>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QCatLedSource source;
    const auto layout = source.setLayout({{0, 0, 0}, {1, 0, 0}});
    if (!layout || !source.submitFrame(QByteArray::fromHex("ff0000ff00ff00ff"), 1, layout))
        return 1;
    QCatLedPreview item;
    item.setSource(&source);
    item.setViewMode(QCatLedPreview::Plane2D);
    item.setPointSize(8);
    item.setPreviewBrightness(0.5);
    item.resetView();
    if (item.source() != &source || item.pointSize() != 8 || source.pointCount() != 2)
        return 2;
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QCatUiKit\nLedPreview { source: LedSource {} }", QUrl());
    QScopedPointer<QObject> object(component.create());
    return object ? 0 : 3;
}
