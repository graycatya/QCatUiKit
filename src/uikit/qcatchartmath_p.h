// SPDX-License-Identifier: MIT
// Private implementation helpers. Not part of the public API.
#pragma once
#include <QCatUiKit/qcatchartaxis.h>
#include <QCatUiKit/qcatchartseries.h>
#include <QDateTime>
#include <QLocale>
#include <algorithm>
#include <cmath>
#include <limits>
namespace QCatUiKit::Detail
{
struct Range
{
    double min = 0, max = 1;
};
inline double transform(double x, ChartAxis::Scale scale)
{
    return scale == ChartAxis::Logarithmic ? std::log10(x) : x;
}
inline double inverse(double x, ChartAxis::Scale scale)
{
    return scale == ChartAxis::Logarithmic ? std::pow(10., x) : x;
}
inline double fraction(double x, Range r, ChartAxis::Scale scale)
{
    return (transform(x, scale) - transform(r.min, scale)) /
           (transform(r.max, scale) - transform(r.min, scale));
}
inline double valueAt(double fraction, Range r, ChartAxis::Scale scale)
{
    const double a = transform(r.min, scale), b = transform(r.max, scale);
    return inverse(a + fraction * (b - a), scale);
}
inline Range expanded(double a, double b, bool log)
{
    if (!std::isfinite(a) || !std::isfinite(b) || a > b || !std::isfinite(b - a))
        return log ? Range{1, 10} : Range{0, 1};
    if (a == b)
    {
        if (log)
            return {a / 2, a * 2};
        const double pad = std::max(1., std::abs(a) * 0.05);
        return {a - pad, b + pad};
    }
    return {a, b};
}
inline QVector<double> ticks(Range range, ChartAxis::Scale scale, int count)
{
    QVector<double> result;
    if (scale == ChartAxis::Logarithmic)
    {
        const double start = std::ceil(std::log10(range.min)),
                     end = std::floor(std::log10(range.max));
        const double step = std::max(1., std::ceil((end - start) / std::max(1, count - 1)));
        for (double e = start; e <= end && result.size() < 20; e += step)
            result.append(std::pow(10., e));
        if (result.size() >= 2)
            return result;
        result.clear();
        for (int i = 0; i < count; ++i)
            result.append(valueAt(double(i) / (count - 1), range, scale));
        return result;
    }
    const double raw = (range.max - range.min) / std::max(1, count - 1);
    const double power = std::pow(10., std::floor(std::log10(raw)));
    const double factor = raw / power;
    double step = (factor <= 1 ? 1 : factor <= 2 ? 2 : factor <= 5 ? 5 : 10) * power;
    if (scale == ChartAxis::Category)
        step = std::max(1., std::ceil(step));
    if (!std::isfinite(step) || step <= 0)
        return result;
    const double first = std::ceil(range.min / step) * step;
    for (int i = 0; i < 32; ++i)
    {
        const double v = first + i * step;
        if (v > range.max + step * 1e-8)
            break;
        result.append(std::abs(v) < step * 1e-10 ? 0 : v);
    }
    return result;
}
inline QString label(double v, ChartAxis *axis, Range range)
{
    if (axis->scale() == ChartAxis::Category)
    {
        const auto categories = axis->categories();
        const double index = std::round(v);
        if (index >= 0 && index < categories.size())
            return categories[int(index)];
    }
    if (axis->scale() == ChartAxis::DateTime)
    {
        // Date/time coordinates are UTC Unix milliseconds.
        if (v < -8.64e15 || v > 8.64e15)
            return {};
        return QDateTime::fromMSecsSinceEpoch(qint64(v), Qt::UTC)
            .toString(range.max - range.min < 60000      ? "HH:mm:ss.zzz"
                      : range.max - range.min < 86400000 ? "HH:mm:ss"
                                                         : "MM-dd HH:mm");
    }
    // Keep large tick values readable and distinct when zooming into a small
    // interval at a large offset. General-format precision rounds these alike.
    const auto values = ticks(range, axis->scale(), axis->tickCount());
    double step = std::numeric_limits<double>::infinity();
    for (int i = 1; i < values.size(); ++i)
        if (values[i] > values[i - 1])
            step = std::min(step, values[i] - values[i - 1]);
    if (!std::isfinite(step))
        step = (range.max - range.min) / std::max(1, axis->tickCount() - 1);
    const QLocale locale;
    if (!(step > 0) || !std::isfinite(step))
        return locale.toString(v, 'g', 15);
    const int decimals = std::max(0, int(std::ceil(-std::log10(step) - 1e-9)));
    // Extremely small ranges still need scientific notation to avoid rounding
    // nonzero ticks to zero or producing hundreds of leading fractional zeros.
    return decimals <= 12 ? locale.toString(v, 'f', decimals) : locale.toString(v, 'g', 15);
}
// Keep endpoints and extrema in source order; never modify the raw data.
inline QVector<int> visibleIndices(const ChartSeries *s, Range x, int budget)
{
    int first = 0, last = s->count();
    if (s->sortedX())
    {
        first = std::max(0, s->lowerBound(x.min) - 1);
        last = std::min(last, s->lowerBound(x.max) + 1);
    }
    QVector<int> out;
    if (budget <= 0 || last - first <= budget)
    {
        for (int i = first; i < last; ++i)
            out.append(i);
        return out;
    }
    const int buckets = std::max(1, (budget - 2) / 2);
    out.append(first);
    for (int b = 0; b < buckets; ++b)
    {
        const int from = first + 1 + int(qint64(last - first - 2) * b / buckets);
        const int to = first + 1 + int(qint64(last - first - 2) * (b + 1) / buckets);
        if (from >= to)
            continue;
        int low = from, high = from;
        for (int i = from + 1; i < to; ++i)
        {
            if (s->sampleAt(i).y < s->sampleAt(low).y)
                low = i;
            if (s->sampleAt(i).y > s->sampleAt(high).y)
                high = i;
        }
        out.append(std::min(low, high));
        if (low != high)
            out.append(std::max(low, high));
    }
    if (last - 1 != first)
        out.append(last - 1);
    return out;
}
} // namespace QCatUiKit::Detail
