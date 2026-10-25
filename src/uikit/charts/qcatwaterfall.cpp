// SPDX-License-Identifier: Apache-2.0
#include "qcatwaterfall.h"
#include "qcatchartstatistics_p.h"
#include <QThread>
#include <cmath>
namespace QCatUiKit
{
class ChartWaterfall::Impl
{
  public:
    QVariantList steps;
    double initial = 0, final = 0;
    bool reset = false, connectors = true;
    QColor colors[4];
    ChartSeries *series = nullptr;
    QStringList categories;
    QString error;
};
ChartWaterfall::ChartWaterfall(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Waterfall);
}
ChartWaterfall::~ChartWaterfall() = default;
QVariantList ChartWaterfall::steps() const
{
    return d->steps;
}
void ChartWaterfall::setSteps(const QVariantList &steps)
{
    setData(steps);
}
bool ChartWaterfall::setData(const QVariantList &steps)
{
    return update(steps, d->initial, d->reset);
}
double ChartWaterfall::initialValue() const
{
    return d->initial;
}
void ChartWaterfall::setInitialValue(double value)
{
    update(d->steps, value, d->reset);
}
bool ChartWaterfall::resetOnSubtotal() const
{
    return d->reset;
}
void ChartWaterfall::setResetOnSubtotal(bool value)
{
    update(d->steps, d->initial, value);
}
bool ChartWaterfall::connectorsVisible() const
{
    return d->connectors;
}
void ChartWaterfall::setConnectorsVisible(bool value)
{
    if (QThread::currentThread() != thread() || d->connectors == value)
        return;
    d->connectors = value;
    update(d->steps, d->initial, d->reset);
    emit appearanceChanged();
}
QColor ChartWaterfall::increaseColor() const
{
    return d->colors[0];
}
QColor ChartWaterfall::decreaseColor() const
{
    return d->colors[1];
}
QColor ChartWaterfall::subtotalColor() const
{
    return d->colors[2];
}
QColor ChartWaterfall::totalColor() const
{
    return d->colors[3];
}
void ChartWaterfall::setIncreaseColor(const QColor &value)
{
    setColor(0, value);
}
void ChartWaterfall::setDecreaseColor(const QColor &value)
{
    setColor(1, value);
}
void ChartWaterfall::setSubtotalColor(const QColor &value)
{
    setColor(2, value);
}
void ChartWaterfall::setTotalColor(const QColor &value)
{
    setColor(3, value);
}
void ChartWaterfall::setColor(int index, const QColor &value)
{
    if (QThread::currentThread() != thread() || d->colors[index] == value)
        return;
    d->colors[index] = value;
    update(d->steps, d->initial, d->reset);
    emit appearanceChanged();
}
ChartSeries *ChartWaterfall::series() const
{
    return d->series;
}
QStringList ChartWaterfall::categories() const
{
    return d->categories;
}
double ChartWaterfall::finalValue() const
{
    return d->final;
}
QString ChartWaterfall::error() const
{
    return d->error;
}
bool ChartWaterfall::update(const QVariantList &steps, double initial, bool reset)
{
    if (QThread::currentThread() != thread())
        return false;
    const auto fail = [this](const QString &error)
    {
        if (d->error != error)
        {
            d->error = error;
            emit errorChanged();
        }
        return false;
    };
    if (!std::isfinite(initial))
        return fail(tr("The initial waterfall value must be finite."));
    if (steps.size() > 100000)
        return fail(tr("A waterfall supports at most 100,000 steps."));
    QVariantList points;
    points.reserve(steps.size());
    QStringList categories;
    double running = initial;
    bool previousReset = false;
    for (int i = 0; i < steps.size(); ++i)
    {
        const auto step = steps[i].toMap();
        bool ok = false;
        const double rawKind = step.value("kind", Change).toDouble(&ok);
        if (!ok || rawKind < Change || rawKind > Total || std::floor(rawKind) != rawKind)
            return fail(tr("Unknown waterfall step kind."));
        const int kind = int(rawKind);
        double delta = 0;
        if (kind == Change)
        {
            delta = step.value("value").toDouble(&ok);
            if (!ok || !std::isfinite(delta))
                return fail(tr("Waterfall changes must be finite numbers."));
        }
        else if (step.contains("value"))
            return fail(tr("Subtotal and total steps are calculated; omit their value."));
        const double before = running;
        if (kind == Change)
            running += delta;
        if (!std::isfinite(running))
            return fail(tr("Waterfall totals exceed the finite numeric range."));
        const bool resetAfter = kind == Subtotal && reset;
        const double bottom = kind == Change ? before : 0;
        const QString label = step.value("label", QString::number(i + 1)).toString();
        const int color = kind == Change ? (delta < 0 ? 1 : 0) : kind == Subtotal ? 2 : 3;
        const QVariantMap statistics{{"sourceIndex", i},
                                     {"waterfallKind", kind},
                                     {"delta", delta},
                                     {"cumulativeBefore", before},
                                     {"cumulativeAfter", running},
                                     {"resetAfter", resetAfter},
                                     {"connectorBefore", d->connectors && i > 0 && !previousReset},
                                     {"barColor", d->colors[color]}};
        points.append(QVariantMap{{"x", i},
                                  {"lower", bottom},
                                  {"upper", running},
                                  {"value", kind == Change ? delta : running},
                                  {"label", label},
                                  {"statistics", statistics}});
        categories.append(label);
        previousReset = resetAfter;
        if (resetAfter)
            running = initial;
    }
    const bool changedSteps = d->steps != steps, changedInitial = d->initial != initial,
               changedReset = d->reset != reset;
    d->steps = steps;
    d->initial = initial;
    d->reset = reset;
    d->final = running;
    d->categories = categories;
    ChartStatisticsAccess::publish(d->series, points, true);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    if (changedSteps)
        emit stepsChanged();
    if (changedInitial)
        emit initialValueChanged();
    if (changedReset)
        emit resetOnSubtotalChanged();
    emit statisticsChanged();
    return true;
}
} // namespace QCatUiKit
