// SPDX-License-Identifier: Apache-2.0
#include "qcatcharthistogram.h"
#include "qcatchartstatistics_p.h"
#include <QThread>
#include <algorithm>
#include <cmath>
#include <limits>

namespace QCatUiKit
{
class ChartHistogram::Impl
{
  public:
    QVariantList values;
    QList<double> edges;
    int bins = 10, included = 0, underflow = 0, overflow = 0;
    Normalization normalization = Count;
    ChartSeries *series = nullptr;
    QString error;
};
ChartHistogram::ChartHistogram(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Histogram);
}
ChartHistogram::~ChartHistogram() = default;
QVariantList ChartHistogram::values() const
{
    return d->values;
}
int ChartHistogram::binCount() const
{
    return d->bins;
}
QList<double> ChartHistogram::binEdges() const
{
    return d->edges;
}
ChartHistogram::Normalization ChartHistogram::normalization() const
{
    return d->normalization;
}
ChartSeries *ChartHistogram::series() const
{
    return d->series;
}
int ChartHistogram::sampleCount() const
{
    return int(d->values.size());
}
int ChartHistogram::includedCount() const
{
    return d->included;
}
int ChartHistogram::underflowCount() const
{
    return d->underflow;
}
int ChartHistogram::overflowCount() const
{
    return d->overflow;
}
QString ChartHistogram::error() const
{
    return d->error;
}
void ChartHistogram::setValues(const QVariantList &values)
{
    setData(values);
}
bool ChartHistogram::setData(const QVariantList &values)
{
    return update(values, d->bins, d->edges, d->normalization);
}
void ChartHistogram::setBinCount(int value)
{
    update(d->values, value, d->edges, d->normalization);
}
void ChartHistogram::setBinEdges(const QList<double> &edges)
{
    update(d->values, d->bins, edges, d->normalization);
}
void ChartHistogram::setNormalization(Normalization value)
{
    update(d->values, d->bins, d->edges, value);
}

bool ChartHistogram::update(const QVariantList &values, int bins,
                            const QList<double> &requestedEdges, Normalization normalization)
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
    if (bins < 1 || bins > 4096 ||
        (!requestedEdges.isEmpty() && (requestedEdges.size() < 2 || requestedEdges.size() > 4097)))
        return fail(tr("A histogram requires 1 to 4096 bins."));
    if (normalization < Count || normalization > Density)
        return fail(tr("Unknown histogram normalization."));
    if (values.size() > 1000000)
        return fail(tr("A histogram supports at most 1,000,000 observations."));
    QList<double> samples;
    samples.reserve(values.size());
    double minimum = INFINITY, maximum = -INFINITY;
    for (const auto &value : values)
    {
        bool ok = false;
        const double number = value.toDouble(&ok);
        if (!ok || !std::isfinite(number))
            return fail(tr("Histogram observations must be finite numbers."));
        samples.append(number);
        minimum = std::min(minimum, number);
        maximum = std::max(maximum, number);
    }
    QList<double> edges = requestedEdges;
    if (edges.isEmpty() && !samples.isEmpty())
    {
        if (minimum == maximum)
        {
            // A constant dataset still needs a representable interval containing its value.
            const double delta = std::max(1., std::abs(minimum) * .01);
            minimum = std::isfinite(minimum - delta) ? minimum - delta : minimum;
            maximum = std::isfinite(maximum + delta) ? maximum + delta : maximum;
        }
        edges.reserve(bins + 1);
        for (int i = 0; i <= bins; ++i)
        {
            const double fraction = double(i) / bins;
            edges.append(i == 0      ? minimum
                         : i == bins ? maximum
                                     : minimum * (1 - fraction) + maximum * fraction);
        }
    }
    for (qsizetype i = 0; i < edges.size(); ++i)
        if (!std::isfinite(edges[i]) ||
            (i && (!(edges[i] > edges[i - 1]) || !std::isfinite(edges[i] - edges[i - 1]))))
            return fail(
                tr("Bin edges must be finite, strictly increasing, and have finite widths."));
    QList<int> counts(std::max(qsizetype(0), edges.size() - 1), 0);
    int included = 0, underflow = 0, overflow = 0;
    for (double value : samples)
    {
        if (value < edges.first())
        {
            ++underflow;
            continue;
        }
        if (value > edges.last())
        {
            ++overflow;
            continue;
        }
        const qsizetype index = std::min(
            qsizetype(std::upper_bound(edges.cbegin(), edges.cend(), value) - edges.cbegin() - 1),
            counts.size() - 1);
        ++counts[index];
        ++included;
    }
    QVariantList points;
    points.reserve(counts.size());
    for (qsizetype i = 0; i < counts.size(); ++i)
    {
        const double lower = edges[i], upper = edges[i + 1];
        const double frequency = included ? double(counts[i]) / included : 0;
        const double density = frequency / (upper - lower);
        if (!std::isfinite(density))
            return fail(tr("Bin density exceeds the finite numeric range."));
        const double height = normalization == Count       ? counts[i]
                              : normalization == Frequency ? frequency
                                                           : density;
        const QString label = QStringLiteral("[%1, %2%3")
                                  .arg(lower, 0, 'g', 8)
                                  .arg(upper, 0, 'g', 8)
                                  .arg(i + 1 == counts.size() ? "]" : ")");
        QVariantMap statistics{{"binIndex", i},
                               {"binLower", lower},
                               {"binUpper", upper},
                               {"binCount", counts[i]},
                               {"frequency", frequency},
                               {"density", density},
                               {"upperInclusive", i + 1 == counts.size()}};
        points.append(QVariantMap{{"x", lower / 2 + upper / 2},
                                  {"y", height},
                                  {"value", counts[i]},
                                  {"label", label},
                                  {"statistics", statistics}});
    }
    const bool dataChanged = d->values != values, binsChanged = d->bins != bins,
               edgesChanged = d->edges != requestedEdges,
               modeChanged = d->normalization != normalization;
    d->values = values;
    d->bins = bins;
    d->edges = requestedEdges;
    d->normalization = normalization;
    d->included = included;
    d->underflow = underflow;
    d->overflow = overflow;
    ChartStatisticsAccess::publish(d->series, points, false);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    if (dataChanged)
        emit valuesChanged();
    if (binsChanged)
        emit binCountChanged();
    if (edgesChanged)
        emit binEdgesChanged();
    if (modeChanged)
        emit normalizationChanged();
    emit statisticsChanged();
    return true;
}
} // namespace QCatUiKit
