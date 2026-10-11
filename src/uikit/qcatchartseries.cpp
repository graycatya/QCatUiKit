// SPDX-License-Identifier: MIT
#include <QCatUiKit/qcatchartseries.h>
#include <QThread>
#include <algorithm>
#include <cmath>
namespace QCatUiKit
{
class ChartSeries::Impl
{
  public:
    QString name;
    QColor color; // Invalid means use the owning chart's semantic palette.
    bool visible = true, sorted = true;
    Type type = Inherit;
    qreal width = 2;
    int capacity = 100000, start = 0;
    QVector<Sample> ring;
};
ChartSeries::ChartSeries(QObject *parent) : QObject(parent), d(new Impl) {}
ChartSeries::~ChartSeries() = default;
QString ChartSeries::name() const
{
    return d->name;
}
void ChartSeries::setName(const QString &v)
{
    if (d->name != v)
    {
        d->name = v;
        emit styleChanged();
    }
}
QColor ChartSeries::color() const
{
    return d->color;
}
void ChartSeries::setColor(const QColor &v)
{
    if (d->color != v)
    {
        d->color = v;
        emit styleChanged();
    }
}
bool ChartSeries::isVisible() const
{
    return d->visible;
}
void ChartSeries::setVisible(bool v)
{
    if (d->visible != v)
    {
        d->visible = v;
        emit styleChanged();
    }
}
ChartSeries::Type ChartSeries::type() const
{
    return d->type;
}
void ChartSeries::setType(Type v)
{
    if (v >= Inherit && v <= Heatmap && d->type != v)
    {
        d->type = v;
        emit styleChanged();
    }
}
qreal ChartSeries::lineWidth() const
{
    return d->width;
}
void ChartSeries::setLineWidth(qreal v)
{
    if (std::isfinite(v) && v > 0 && v <= 64 && d->width != v)
    {
        d->width = v;
        emit styleChanged();
    }
}
int ChartSeries::capacity() const
{
    return d->capacity;
}
void ChartSeries::setCapacity(int v)
{
    if (v <= 0 || v == d->capacity)
        return;
    QVector<Sample> keep;
    for (int i = std::max(0, count() - v); i < count(); ++i)
        keep.append(sampleAt(i));
    d->capacity = v;
    d->start = 0;
    d->ring = std::move(keep);
    emit capacityChanged();
    emit dataChanged();
}
int ChartSeries::count() const
{
    return int(d->ring.size());
}
const ChartSeries::Sample &ChartSeries::sampleAt(int i) const
{
    static const Sample empty;
    return i >= 0 && i < count() ? d->ring[(d->start + i) % count()] : empty;
}
bool ChartSeries::sortedX() const
{
    return d->sorted;
}
int ChartSeries::lowerBound(double x) const
{
    if (!d->sorted)
        return 0;
    int first = 0, last = count();
    while (first < last)
    {
        const int mid = first + (last - first) / 2;
        if (sampleAt(mid).x < x)
            first = mid + 1;
        else
            last = mid;
    }
    return first;
}
static bool validSamples(const QVector<ChartSeries::Sample> &values)
{
    for (const auto &p : values)
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.value))
            return false;
    return true;
}
bool ChartSeries::appendSamples(const QVector<Sample> &values)
{
    if (QThread::currentThread() != thread() || !validSamples(values))
        return false;
    if (values.isEmpty())
        return true;
    for (const auto &p : values)
    {
        if (count() && p.x < sampleAt(count() - 1).x)
            d->sorted = false;
        if (count() < d->capacity)
            d->ring.append(p);
        else
        {
            d->ring[d->start] = p;
            d->start = (d->start + 1) % d->capacity;
        }
    }
    emit dataChanged();
    return true;
}
bool ChartSeries::replaceSamples(const QVector<Sample> &values)
{
    if (QThread::currentThread() != thread() || !validSamples(values))
        return false;
    d->ring = values.mid(std::max(0, int(values.size()) - d->capacity));
    d->start = 0;
    d->sorted = true;
    for (int i = 1; i < count(); ++i)
        if (d->ring[i].x < d->ring[i - 1].x)
        {
            d->sorted = false;
            break;
        }
    emit dataChanged();
    return true;
}
bool ChartSeries::replacePoints(const QVector<QPointF> &points)
{
    QVector<Sample> values;
    values.reserve(points.size());
    for (const auto &p : points)
        values.append({p.x(), p.y(), 1, {}});
    return replaceSamples(values);
}
bool ChartSeries::append(double x, double y, double value)
{
    return appendSamples({{x, y, value, {}}});
}
static bool fromVariants(const QVariantList &values, QVector<ChartSeries::Sample> &out)
{
    out.reserve(values.size());
    for (const auto &v : values)
    {
        const auto m = v.toMap();
        bool xOk, yOk, valueOk = true;
        const double x = m.value("x").toDouble(&xOk), y = m.value("y").toDouble(&yOk);
        const double z = m.contains("value") ? m.value("value").toDouble(&valueOk) : 1;
        if (!xOk || !yOk || !valueOk)
            return false;
        out.append({x, y, z, m.value("label").toString()});
    }
    return true;
}
bool ChartSeries::appendData(const QVariantList &v)
{
    QVector<Sample> out;
    return fromVariants(v, out) && appendSamples(out);
}
bool ChartSeries::setData(const QVariantList &v)
{
    QVector<Sample> out;
    return fromVariants(v, out) && replaceSamples(out);
}
QVariantMap ChartSeries::at(int i) const
{
    if (i < 0 || i >= count())
        return {};
    const auto p = sampleAt(i);
    return {{"x", p.x}, {"y", p.y}, {"value", p.value}, {"label", p.label}};
}
void ChartSeries::clear()
{
    if (d->ring.isEmpty())
        return;
    d->ring.clear();
    d->start = 0;
    d->sorted = true;
    emit dataChanged();
}
} // namespace QCatUiKit
