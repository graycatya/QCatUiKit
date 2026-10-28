// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QMap>
#include <QPointF>
#include <QSize>
#include <QUrl>
#include <QVariantList>
#include <functional>

namespace QCatUiKit::MascotPrivate
{
struct Sprite
{
    QUrl source;
    QSize atlasSize;
    QSize frameSize;
    int startFrame = 0;
    int frameCount = 0;
    double fps = 0;
};
struct Action
{
    int loops = -1;
    QMap<QString, Sprite> directions;
    QVariantList regions;
};
struct Package
{
    QString id;
    QString renderer;
    QSize logicalSize = {0, 0};
    QPointF footAnchor;
    QPointF bubbleAnchor;
    QString defaultAction;
    QString defaultFacing;
    bool allowMirror = false;
    QUrl fallbackImage;
    QMap<QString, Action> actions;
};
bool loadPackage(const QUrl &source, Package *package, QString *error,
                 const std::function<bool()> &cancelled = {});
} // namespace QCatUiKit::MascotPrivate
