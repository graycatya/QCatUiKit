// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatchartseries.h>
#include <QRectF>
#include <algorithm>
#include <cmath>
#include <utility>

namespace QCatUiKit::Detail
{
struct ChartCubic
{
    QPointF a, c1, c2, b;
};
inline QPointF interpolate(QPointF a, QPointF b, double t)
{
    return a * (1 - t) + b * t;
}
inline bool finite(QPointF p)
{
    return std::isfinite(p.x()) && std::isfinite(p.y());
}
inline ChartCubic straightCubic(QPointF a, QPointF b)
{
    return {a, interpolate(a, b, 1. / 3), interpolate(a, b, 2. / 3), b};
}
// Shape-preserving Hermite slopes; repeated or decreasing X breaks the smooth run.
inline double monotoneSlope(QPointF before, QPointF point, QPointF after, double fallback)
{
    const double left = point.x() - before.x(), right = after.x() - point.x();
    if (!(left > 0 && right > 0))
        return fallback;
    const double a = (point.y() - before.y()) / left, b = (after.y() - point.y()) / right;
    if (a == 0 || b == 0 || (a < 0) != (b < 0))
        return 0;
    const double scale = std::max(left, right);
    const double w1 = 2 * (right / scale) + left / scale;
    const double w2 = right / scale + 2 * (left / scale);
    return (w1 + w2) / (w1 / a + w2 / b);
}
inline ChartCubic chartCubic(QPointF before, QPointF a, QPointF b, QPointF after,
                             ChartSeries::CurveMode mode, double tension)
{
    auto result = straightCubic(a, b);
    if (mode == ChartSeries::CatmullRomSpline)
    {
        const double weight = (1 - tension) / 6;
        result.c1 = a + (b - before) * weight;
        result.c2 = b - (after - a) * weight;
    }
    else if (mode == ChartSeries::MonotoneSpline && b.x() > a.x())
    {
        const double dx = b.x() - a.x(), slope = (b.y() - a.y()) / dx;
        const double m1 = monotoneSlope(before, a, b, slope);
        const double m2 = monotoneSlope(a, b, after, slope);
        result.c1 = a + QPointF(dx / 3, m1 * dx / 3);
        result.c2 = b - QPointF(dx / 3, m2 * dx / 3);
    }
    return finite(result.c1) && finite(result.c2) ? result : straightCubic(a, b);
}
// Natural parametric cubic spline, C2 continuous with zero second derivatives at the ends.
// Solve in local coordinates to avoid amplifying large absolute timestamps in the tridiagonal RHS.
inline QVector<ChartCubic> naturalCubics(const QVector<QPointF> &points)
{
    const int n = int(points.size()) - 1;
    if (n < 1)
        return {};
    if (n == 1)
        return {straightCubic(points[0], points[1])};
    const QPointF origin = points.first();
    QVector<QPointF> local;
    for (const auto &p : points)
        local.append(p - origin);
    QVector<QPointF> first(n);
    QVector<double> upper(n);
    first[0] = (local[0] + local[1] * 2) / 2;
    upper[0] = .5;
    for (int i = 1; i < n; ++i)
    {
        const double diagonal = (i == n - 1 ? 3.5 : 4) - upper[i - 1];
        const QPointF rhs =
            i == n - 1 ? (local[i] * 8 + local[i + 1]) / 2 : local[i] * 4 + local[i + 1] * 2;
        first[i] = (rhs - first[i - 1]) / diagonal;
        upper[i] = 1 / diagonal;
    }
    for (int i = n - 2; i >= 0; --i)
        first[i] -= first[i + 1] * upper[i];
    QVector<ChartCubic> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        const auto second =
            i < n - 1 ? local[i + 1] * 2 - first[i + 1] : (local[n] + first[n - 1]) / 2;
        const ChartCubic c{points[i], first[i] + origin, second + origin, points[i + 1]};
        result.append(finite(c.c1) && finite(c.c2) ? c : straightCubic(c.a, c.b));
    }
    return result;
}
inline QPointF cubicAt(const ChartCubic &c, double t)
{
    const auto a = interpolate(c.a, c.c1, t), b = interpolate(c.c1, c.c2, t),
               d = interpolate(c.c2, c.b, t);
    return interpolate(interpolate(a, b, t), interpolate(b, d, t), t);
}
inline double segmentDistanceSquared(QPointF p, QPointF a, QPointF b)
{
    const auto delta = b - a;
    const double length2 = QPointF::dotProduct(delta, delta);
    const double t =
        length2 > 0 ? std::clamp(QPointF::dotProduct(p - a, delta) / length2, 0., 1.) : 0;
    const auto d = p - interpolate(a, b, t);
    return QPointF::dotProduct(d, d);
}
inline void flattenCubic(const ChartCubic &c, double error2, QVector<QPointF> &points,
                         int depth = 0)
{
    if (depth >= 10 || (segmentDistanceSquared(c.c1, c.a, c.b) <= error2 &&
                        segmentDistanceSquared(c.c2, c.a, c.b) <= error2))
    {
        points.append(c.b);
        return;
    }
    const auto a = interpolate(c.a, c.c1, .5), b = interpolate(c.c1, c.c2, .5),
               d = interpolate(c.c2, c.b, .5);
    const auto e = interpolate(a, b, .5), f = interpolate(b, d, .5), mid = interpolate(e, f, .5);
    flattenCubic({c.a, a, e, mid}, error2, points, depth + 1);
    flattenCubic({mid, f, d, c.b}, error2, points, depth + 1);
}
// Paired boundaries share parameter samples, preserving equal X at every band edge.
// Flattening them independently would connect unrelated points and create false wedges.
struct ChartBandPoint
{
    QPointF upper, lower;
    double parameter = 0;
};
inline void flattenBandCubics(const ChartCubic &upper, const ChartCubic &lower, double error2,
                              QVector<ChartBandPoint> &points, double from = 0, double to = 1,
                              int depth = 0)
{
    const auto flat = [error2](const ChartCubic &c)
    {
        return segmentDistanceSquared(c.c1, c.a, c.b) <= error2 &&
               segmentDistanceSquared(c.c2, c.a, c.b) <= error2;
    };
    if (depth >= 10 || (flat(upper) && flat(lower)))
    {
        points.append({upper.b, lower.b, to});
        return;
    }
    const auto split = [](const ChartCubic &c)
    {
        const auto a = interpolate(c.a, c.c1, .5), b = interpolate(c.c1, c.c2, .5),
                   d = interpolate(c.c2, c.b, .5);
        const auto e = interpolate(a, b, .5), f = interpolate(b, d, .5),
                   mid = interpolate(e, f, .5);
        return std::pair<ChartCubic, ChartCubic>{{c.a, a, e, mid}, {mid, f, d, c.b}};
    };
    const auto u = split(upper), l = split(lower);
    const double mid = (from + to) / 2;
    flattenBandCubics(u.first, l.first, error2, points, from, mid, depth + 1);
    flattenBandCubics(u.second, l.second, error2, points, mid, to, depth + 1);
}
// Include derivative roots, not merely the control polygon: automatic axes fit the curve itself.
inline QVector<QPointF> cubicExtrema(const ChartCubic &c, QVector<double> *parameters = nullptr)
{
    QVector<QPointF> points{c.a, c.b};
    if (parameters)
        *parameters = {0, 1};
    for (int axis = 0; axis < 2; ++axis)
    {
        const auto value = [axis](QPointF p) { return axis ? p.y() : p.x(); };
        double d1 = value(c.c1) - value(c.a), d2 = value(c.c2) - value(c.a),
               d3 = value(c.b) - value(c.a);
        const double deltaScale = std::max({std::abs(d1), std::abs(d2), std::abs(d3)});
        if (!std::isfinite(deltaScale) || deltaScale == 0)
            continue;
        d1 /= deltaScale;
        d2 /= deltaScale;
        d3 /= deltaScale;
        double a = 3 * (3 * d1 - 3 * d2 + d3), b = 2 * (-6 * d1 + 3 * d2), d = 3 * d1;
        const double scale = std::max({std::abs(a), std::abs(b), std::abs(d)});
        if (!std::isfinite(scale) || scale == 0)
            continue;
        a /= scale;
        b /= scale;
        d /= scale;
        const auto append = [&](double t)
        {
            if (t > 0 && t < 1)
            {
                points.append(cubicAt(c, t));
                if (parameters)
                    parameters->append(t);
            }
        };
        if (std::abs(a) < 1e-14)
        {
            if (std::abs(b) > 1e-14)
                append(-d / b);
        }
        else if (const double discriminant = b * b - 4 * a * d; discriminant >= 0)
        {
            const double q = -.5 * (b + std::copysign(std::sqrt(discriminant), b));
            append(q / a);
            if (q != 0)
                append(d / q);
        }
    }
    return points;
}
inline bool horizontalBar(ChartSeries::Type type)
{
    return type == ChartSeries::HorizontalBar || type == ChartSeries::HorizontalStackedBar ||
           type == ChartSeries::HorizontalPercentBar;
}
inline bool percentBar(ChartSeries::Type type)
{
    return type == ChartSeries::PercentBar || type == ChartSeries::HorizontalPercentBar;
}
inline bool groupedBar(ChartSeries::Type type)
{
    return type == ChartSeries::Bar || type == ChartSeries::HorizontalBar;
}
inline bool barType(ChartSeries::Type type)
{
    return groupedBar(type) || percentBar(type) || type == ChartSeries::StackedBar ||
           type == ChartSeries::HorizontalStackedBar || type == ChartSeries::Histogram || type == ChartSeries::Waterfall;
}
struct SignedTotal
{
    double scale[2]{}, sum[2]{};
    void add(double value)
    {
        const int sign = value < 0;
        const double absolute = std::abs(value);
        if (absolute == 0)
            return;
        if (absolute > scale[sign])
        {
            sum[sign] *= scale[sign] / absolute;
            scale[sign] = absolute;
        }
        sum[sign] += absolute / scale[sign];
    }
    double percentage(double value) const
    {
        const int sign = value < 0;
        return sum[sign] > 0 ? (value / scale[sign]) / sum[sign] * 100 : 0;
    }
};
} // namespace QCatUiKit::Detail
