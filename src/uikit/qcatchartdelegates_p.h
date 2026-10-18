// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSet>
#include <utility>

namespace QCatUiKit::Detail
{
// GUI-thread adapter. No QML object is published to the RHI renderer.
class ChartDelegates
{
  public:
    struct Request
    {
        QPointer<ChartSeries> series;
        QPointer<QQmlComponent> component;
        int kind, index;
        QRectF rect;
        qreal rotation;
        QVariantMap data;
    };
    struct Entry
    {
        QPointer<ChartSeries> series;
        QPointer<QQmlComponent> component;
        QPointer<QQuickItem> item;
    };
    int budget = 128;
    QVector<Request> requests;
    QHash<QString, Entry> entries;
    QPointer<QQuickItem> dataHost, labelHost;
    QString error;
    ~ChartDelegates()
    {
        clear();
    }
    void clear()
    {
        const auto old = std::exchange(entries, {});
        for (const auto &entry : old)
            delete entry.item;
    }
    int enqueue(ChartSeries *series, QQmlComponent *component, int kind, int index, QRectF rect,
                QVariantMap data, qreal rotation = 0)
    {
        if (!component || requests.size() >= budget || rect.isEmpty())
            return -1;
        const int id = int(requests.size());
        requests.append({series, component, kind, index, rect, rotation, std::move(data)});
        return id;
    }
    QSet<int> synchronize(QQuickItem *chart, QRectF plot)
    {
        error.clear();
        QSet<int> result;
        QSet<QString> used;
        QSet<QQmlComponent *> failed;
        if (!requests.isEmpty() && !dataHost)
        {
            dataHost = new QQuickItem(chart);
            labelHost = new QQuickItem(chart);
            dataHost->setZ(.5);
            labelHost->setZ(.75);
            dataHost->setClip(true);
            labelHost->setClip(true);
        }
        if (dataHost)
        {
            dataHost->setPosition(plot.topLeft());
            dataHost->setSize(plot.size());
            labelHost->setSize(chart->size());
        }
        // Complete geometry and payload snapshots before running application callbacks.
        const auto pending = requests;
        const auto keyFor = [](const Request &r)
        {
            return QString::number(quintptr(r.series.data()), 16) + QLatin1Char('/') +
                   QString::number(r.kind) + QLatin1Char('/') + QString::number(r.index);
        };
        QSet<QString> wanted;
        for (const auto &r : pending)
            if (r.series && r.component)
                wanted.insert(keyFor(r));
        for (const auto &key : entries.keys())
            if (!wanted.contains(key))
                delete entries.take(key).item;
        for (int i = 0; i < pending.size(); ++i)
        {
            const auto &r = pending[i];
            if (!r.series || !r.component || failed.contains(r.component))
                continue;
            auto component = r.component;
            if (!component->isReady())
            {
                error = component && component->isError() ? component->errorString()
                                                          : QStringLiteral("Delegate is loading");
                failed.insert(component);
                continue;
            }
            const QString key = keyFor(r);
            auto entry = entries.value(key);
            if (entry.component != component || !entry.item || entry.series != r.series)
            {
                entries.remove(key);
                delete entry.item;
                auto *context = component->creationContext();
                if (!context && component->engine())
                    context = component->engine()->rootContext();
                if (!context)
                {
                    error = QStringLiteral("Delegate has no QML context");
                    failed.insert(component);
                    continue;
                }
                QPointer<QObject> object = component->beginCreate(context);
                QPointer<QQuickItem> item = qobject_cast<QQuickItem *>(object.data());
                if (item)
                {
                    QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
                    auto *host = r.kind == 2 ? labelHost.data() : dataHost.data();
                    item->setParent(host);
                    item->setParentItem(host);
                    QVariantMap initial{{"x", r.rect.x() - (r.kind == 2 ? 0 : plot.x())},
                                        {"y", r.rect.y() - (r.kind == 2 ? 0 : plot.y())},
                                        {"width", r.rect.width()},
                                        {"height", r.rect.height()},
                                        {"rotation", r.rotation}};
                    if (item->metaObject()->indexOfProperty("chartData") >= 0)
                        initial.insert("chartData", r.data);
                    component->setInitialProperties(item, initial);
                }
                if (component)
                    component->completeCreate();
                if (!item || !component || component->isError())
                {
                    error = component && component->isError()
                                ? component->errorString()
                                : QStringLiteral("Delegate root must be a QQuickItem");
                    delete object;
                    failed.insert(component);
                    continue;
                }
                entry = {r.series, component, item};
                entries.insert(key, entry);
            }
            auto item = entry.item;
            if (!item || !r.series)
                continue;
            if (item->metaObject()->indexOfProperty("chartData") >= 0)
                item->setProperty("chartData", r.data);
            if (!item)
                continue;
            item->setPosition(r.rect.topLeft() - (r.kind == 2 ? QPointF() : plot.topLeft()));
            item->setSize(r.rect.size());
            item->setRotation(r.rotation);
            item->setZ(r.data.value("seriesIndex").toInt());
            used.insert(key);
            result.insert(i);
        }
        for (const auto &key : entries.keys())
            if (!used.contains(key))
                delete entries.take(key).item;
        return result;
    }
};
} // namespace QCatUiKit::Detail
