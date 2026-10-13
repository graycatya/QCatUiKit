// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/LedDeck.h>
#include <QCatUiKit/LedEffectRegistry.h>
#include <QCatUiKit/LedLayoutModel.h>
#include <QCatUiKit/LedMixer>
#include <QCatUiKit/LedOutput.h>
#include <QCatUiKit/LedPreview.h>
#include <QCatUiKit/LedPreview>
#include <QCatUiKit/LedProject>
#include <QCatUiKit/LedSource.h>
#include <QCatUiKit/LedSource>
#include <QCatUiKit/LedStorage>
#include <QCatUiKit/LightingFrame>
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
    if (!object)
        return 3;
    QCatLedProject project;
    if (project.layout()->pointCount() != 2048)
        return 4;
    project.repository()->populateExamples();
    if (project.registry()->effects().size() < 4 || project.repository()->storages().size() < 2)
        return 5;
    QQmlComponent deck(&engine);
    deck.setData("import QCatUiKit\nLedDeck { runtime: LedDeckRuntime { layout: LedLayoutModel {} "
                 "registry: LedEffectRegistry {} repository: LedStorageRepository {} } }",
                 QUrl());
    QScopedPointer<QObject> deckObject(deck.create());
    if (!deckObject)
        qWarning().noquote() << deck.errorString();
    return deckObject ? 0 : 6;
}
