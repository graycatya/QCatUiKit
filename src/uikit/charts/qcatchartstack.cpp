// SPDX-License-Identifier: Apache-2.0
#include "qcatchartstack.h"
#include "qcatchartcurves_p.h"
#include "qcatchartstatistics_p.h"
#include <QPointer>
#include <QScopedValueRollback>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace QCatUiKit
{
class ChartStack::Impl
{
  public:
    struct Entry
    {
        ChartSeries *source;
        ChartSeries *output;
        QList<QMetaObject::Connection> connections;
    };
    QList<Entry> entries;
    QList<QMetaObject::Connection> axisConnections;
    QTimer timer;
    bool percentage = false;
    bool updating = false;
    QString error;
};
ChartStack::ChartStack(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    d->timer.setParent(this);
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, &ChartStack::refresh);
}
ChartStack::~ChartStack() = default;
QQmlListProperty<ChartSeries> ChartStack::sources()
{
    return {this,
            this,
            [](QQmlListProperty<ChartSeries> *p, ChartSeries *s)
            { static_cast<ChartStack *>(p->data)->addSource(s); },
            [](QQmlListProperty<ChartSeries> *p) -> qsizetype
            { return static_cast<ChartStack *>(p->data)->count(); },
            [](QQmlListProperty<ChartSeries> *p, qsizetype i)
            { return static_cast<ChartStack *>(p->data)->d->entries.at(i).source; },
            [](QQmlListProperty<ChartSeries> *p)
            { static_cast<ChartStack *>(p->data)->clearSources(); }};
}
QQmlListProperty<ChartSeries> ChartStack::series()
{
    return {this, this,
            [](QQmlListProperty<ChartSeries> *p) -> qsizetype
            { return static_cast<ChartStack *>(p->data)->count(); },
            [](QQmlListProperty<ChartSeries> *p, qsizetype i)
            { return static_cast<ChartStack *>(p->data)->seriesAt(int(i)); }};
}
int ChartStack::count() const
{
    return int(d->entries.size());
}
ChartSeries *ChartStack::seriesAt(int i) const
{
    return i >= 0 && i < count() ? d->entries[i].output : nullptr;
}
bool ChartStack::percentage() const
{
    return d->percentage;
}
QString ChartStack::error() const
{
    return d->error;
}
void ChartStack::setPercentage(bool value)
{
    if (QThread::currentThread() != thread() || d->percentage == value)
        return;
    d->percentage = value;
    d->timer.start(0);
    emit percentageChanged();
}
void ChartStack::addSource(ChartSeries *source)
{
    if (!source || QThread::currentThread() != thread() || source->thread() != thread())
        return;
    for (const auto &entry : d->entries)
        if (entry.source == source || entry.output == source)
            return;
    auto *output = new ChartSeries(this);
    output->setType(ChartSeries::RangeArea);
    Impl::Entry entry{source, output, {}};
    const auto schedule = [this] { d->timer.start(0); };
    entry.connections = {
        connect(source, &ChartSeries::dataChanged, this, schedule),
        connect(source, &ChartSeries::styleChanged, this, schedule),
        connect(output, &ChartSeries::styleChanged, this,
                [this, source, output]
                {
                    // The chart legend toggles the derived series; make visibility a source
                    // decision so the remaining bands and percentage denominators are recomputed
                    // together.
                    if (!d->updating && source->isVisible() != output->isVisible())
                        source->setVisible(output->isVisible());
                }),
        connect(source, &QObject::destroyed, this, [this, source] { removeSource(source); })};
    d->entries.append(entry);
    d->timer.start(0);
    emit sourcesChanged();
    emit seriesChanged();
}
void ChartStack::removeSource(ChartSeries *source)
{
    if (QThread::currentThread() != thread())
        return;
    for (int i = 0; i < count(); ++i)
        if (d->entries[i].source == source)
        {
            const auto entry = d->entries.takeAt(i);
            for (const auto &c : entry.connections)
                disconnect(c);
            delete entry.output;
            d->timer.start(0);
            emit sourcesChanged();
            emit seriesChanged();
            return;
        }
}
void ChartStack::clearSources()
{
    if (QThread::currentThread() != thread())
        return;
    while (!d->entries.isEmpty())
        removeSource(d->entries.last().source);
}
bool ChartStack::refresh()
{
    if (QThread::currentThread() != thread())
        return false;
    if (d->updating)
        return false;
    QScopedValueRollback updating(d->updating, true);
    d->timer.stop();
    for (const auto &connection : d->axisConnections)
        disconnect(connection);
    d->axisConnections.clear();
    QSet<ChartAxis *> axes;
    for (const auto &entry : d->entries)
        if (auto *axis = entry.source->yAxis(); axis && !axes.contains(axis))
        {
            axes.insert(axis);
            d->axisConnections.append(connect(axis, &ChartAxis::changed, this,
                                              [this, axis, scale = axis->scale()]() mutable
                                              {
                                                  if (scale != axis->scale())
                                                  {
                                                      scale = axis->scale();
                                                      d->timer.start(0);
                                                  }
                                              }));
        }
    const auto fail = [this](const QString &message)
    {
        for (const auto &entry : d->entries)
            entry.output->clear();
        if (d->error != message)
        {
            d->error = message;
            emit errorChanged();
        }
        return false;
    };
    QVector<ChartSeries *> active;
    QVector<int> sourceIds;
    for (int i = 0; i < count(); ++i)
    {
        const auto &e = d->entries[i];
        auto *s = e.source;
        auto *o = e.output;
        o->setName(s->name());
        o->setColor(s->color());
        o->setVisible(s->isVisible());
        o->setXAxis(s->xAxis());
        o->setYAxis(s->yAxis());
        o->setFillOpacity(s->fillOpacity());
        o->setFillColor(s->fillColor());
        o->setFillStyle(s->fillStyle());
        o->setFillEndColor(s->fillEndColor());
        o->setFillGradientStops(s->fillGradientStops());
        o->setFillTexture(s->fillTexture());
        o->setFillTextureMode(s->fillTextureMode());
        o->setFillTextureSize(s->fillTextureSize());
        o->setBorderColor(s->borderColor());
        o->setLineWidth(s->lineWidth());
        o->setStrokeStyle(s->strokeStyle());
        o->setDashPattern(s->dashPattern());
        o->setDashOffset(s->dashOffset());
        o->setLineCap(s->lineCap());
        o->setLineJoin(s->lineJoin());
        if (s->isVisible())
        {
            active.append(s);
            sourceIds.append(i);
        }
    }
    if (active.size() > 64)
        return fail(tr("A stack supports at most 64 visible series."));
    const int n = active.isEmpty() ? 0 : active[0]->count();
    for (auto *s : active)
    {
        if (s->count() != n || s->xAxis() != active[0]->xAxis() || s->yAxis() != active[0]->yAxis())
            return fail(tr("Stack sources must share axes and the same X coordinates."));
        if (s->yAxis() && s->yAxis()->scale() == ChartAxis::Logarithmic)
            return fail(tr("Stacked areas require a linear value axis."));
        for (int i = 0; i < n; ++i)
            if (s->sampleAt(i).x != active[0]->sampleAt(i).x ||
                (i && s->sampleAt(i).x <= s->sampleAt(i - 1).x))
                return fail(
                    tr("Stack X coordinates must be strictly increasing and exactly aligned."));
    }
    QVector<QVariantList> result(count());
    qsizetype generated = 0;
    // Split all series at every zero crossing. Positive/negative accumulations
    // remain separate and share the same piecewise-linear X partition.
    for (int i = 0; i < n; ++i)
    {
        const int next = std::min(i + 1, n - 1);
        if (n > 1 && i == n - 1)
            break;
        bool gap = false;
        for (auto *s : active)
            gap |= next != i && s->breakBefore(next);
        if (gap)
            continue;
        QVector<double> cuts{0, 1};
        for (auto *s : active)
        {
            const double a = s->sampleAt(i).y, b = s->sampleAt(next).y;
            if ((a < 0 && b > 0) || (a > 0 && b < 0))
            {
                const double scale = std::max(std::abs(a), std::abs(b));
                cuts.append((a / scale) / (a / scale - b / scale));
            }
        }
        std::sort(cuts.begin(), cuts.end());
        cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
        for (int part = 1; part < cuts.size(); ++part)
        {
            const double t0 = cuts[part - 1], t1 = cuts[part];
            QVector<double> first(active.size()), last(active.size());
            const auto interpolate = [&](int k, double t)
            {
                const double a = active[k]->sampleAt(i).y, b = active[k]->sampleAt(next).y;
                if ((a < 0 && b > 0) || (a > 0 && b < 0))
                {
                    const double scale = std::max(std::abs(a), std::abs(b));
                    if (t == (a / scale) / (a / scale - b / scale))
                        return 0.;
                }
                return a * (1 - t) + b * t;
            };
            for (int k = 0; k < active.size(); ++k)
            {
                first[k] = interpolate(k, t0);
                last[k] = interpolate(k, t1);
            }
            Detail::SignedTotal totals[2];
            for (double value : first)
                totals[0].add(value);
            for (double value : last)
                totals[1].add(value);
            for (int endpoint = 0; endpoint < (i == next ? 1 : 2); ++endpoint)
            {
                const double t = endpoint ? t1 : t0;
                const auto &values = endpoint ? last : first;
                const auto &other = endpoint ? first : last;
                long double positive = 0, negative = 0;
                const double x =
                    active[0]->sampleAt(i).x * (1 - t) + active[0]->sampleAt(next).x * t;
                for (int k = 0; k < active.size(); ++k)
                {
                    const bool positiveSide = interpolate(k, (t0 + t1) / 2) >= 0;
                    const int sign = positiveSide ? 0 : 1;
                    // A generated crossing is not a zero-valued observation. Preserve the
                    // one-sided percentage limit when its entire sign group approaches zero;
                    // otherwise a constant 100% band would taper over the preceding interval.
                    const bool limit = t > 0 && t < 1 && totals[endpoint].sum[sign] == 0 &&
                                       totals[1 - endpoint].sum[sign] > 0;
                    const double percent = limit ? totals[1 - endpoint].percentage(other[k])
                                                 : totals[endpoint].percentage(values[k]);
                    auto &base = positiveSide ? positive : negative;
                    const double low = double(base);
                    base += d->percentage ? percent : values[k];
                    const double high = double(base);
                    if (!std::isfinite(low) || !std::isfinite(high))
                        return fail(tr("Stack totals exceed the finite numeric range."));
                    if (first[k] == 0 && last[k] == 0)
                        continue;
                    const int original = t <= .5 ? i : next;
                    const auto &raw = active[k]->sampleAt(original);
                    QVariantMap metadata{
                        {"sourceIndex", original},  {"sourceSeriesIndex", sourceIds[k]},
                        {"sourceX", raw.x},         {"sourceY", raw.y},
                        {"stackValue", values[k]},  {"percentage", percent},
                        {"percentageLimit", limit}, {"stackLower", low},
                        {"stackUpper", high}};
                    auto &output = result[sourceIds[k]];
                    if (!endpoint && !output.isEmpty())
                    {
                        const auto previous = output.last().toMap();
                        if (previous.value("x").toDouble() == x &&
                            previous.value("lower").toDouble() == low &&
                            previous.value("upper").toDouble() == high)
                            continue;
                    }
                    output.append(QVariantMap{{"x", x},
                                              {"lower", low},
                                              {"upper", high},
                                              {"label", raw.label},
                                              {"breakBefore", endpoint == 0},
                                              {"statistics", metadata}});
                    if (++generated > 1000000)
                        return fail(tr("Stack output exceeds 1,000,000 boundary points."));
                }
            }
        }
    }
    for (int i = 0; i < count(); ++i)
        ChartStatisticsAccess::publish(d->entries[i].output, result[i], true);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    return true;
}
} // namespace QCatUiKit
