// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QLineF>
#include <QVector>
#include <algorithm>
#include <cmath>

namespace QCatUiKit::Detail
{
struct ChartLeaderRoute
{
    QVector<QPointF> points;
    int index;
    double weight;
};
inline bool chartLeaderSegmentsCross(QPointF a, QPointF b, QPointF c, QPointF d)
{
    if (QLineF(a, b).intersects(QLineF(c, d)) == QLineF::BoundedIntersection)
        return true;
    const auto cross = [](QPointF a, QPointF b) { return a.x() * b.y() - a.y() * b.x(); };
    if (std::abs(cross(b - a, c - a)) > 1e-7 || std::abs(cross(b - a, d - a)) > 1e-7)
        return false;
    // Collinear overlapping routes also count as a conflict.
    const auto overlaps = [](double a, double b, double c, double d) {
        return std::max(std::min(a, b), std::min(c, d)) <= std::min(std::max(a, b), std::max(c, d));
    };
    return overlaps(a.x(), b.x(), c.x(), d.x()) && overlaps(a.y(), b.y(), c.y(), d.y());
}
inline QVector<ChartLeaderRoute> nonCrossingChartLeaders(QVector<ChartLeaderRoute> routes)
{
    std::stable_sort(routes.begin(), routes.end(),
                     [](const auto &a, const auto &b) { return a.weight > b.weight; });
    QVector<ChartLeaderRoute> accepted;
    for (const auto &route : routes)
    {
        bool conflict = false;
        for (int i = 1; i < route.points.size() && !conflict; ++i)
        {
            for (int j = i + 2; j < route.points.size(); ++j)
                conflict |= chartLeaderSegmentsCross(route.points[i - 1], route.points[i],
                                                     route.points[j - 1], route.points[j]);
            for (const auto &other : accepted)
                for (int j = 1; j < other.points.size(); ++j)
                    conflict |= chartLeaderSegmentsCross(route.points[i - 1], route.points[i],
                                                         other.points[j - 1], other.points[j]);
        }
        if (!conflict)
            accepted.append(route);
    }
    return accepted;
}
// A single label column. Preserve anchor order and favor larger slices if crowded.
struct ChartLabelSlot
{
    double y, weight;
    int index;
};
inline QVector<ChartLabelSlot> layoutChartLabelColumn(QVector<ChartLabelSlot> entries, double top,
                                                      double bottom, double height, double spacing)
{
    if (height <= 0 || bottom - top < height || spacing < 0)
        return {};
    const int capacity = int(std::min(double(entries.size()),
                                      std::floor((bottom - top + spacing) / (height + spacing))));
    std::stable_sort(entries.begin(), entries.end(),
                     [](const auto &a, const auto &b) { return a.weight > b.weight; });
    entries.resize(capacity);
    std::stable_sort(entries.begin(), entries.end(),
                     [](const auto &a, const auto &b)
                     { return a.y == b.y ? a.index < b.index : a.y < b.y; });
    double next = top + height / 2;
    for (auto &slot : entries)
    {
        slot.y = std::max(next, slot.y);
        next = slot.y + height + spacing;
    }
    next = bottom - height / 2;
    for (auto i = entries.rbegin(); i != entries.rend(); ++i)
    {
        i->y = std::min(next, i->y);
        next = i->y - height - spacing;
    }
    return entries;
}
} // namespace QCatUiKit::Detail
