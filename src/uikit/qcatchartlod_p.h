// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartcurves_p.h"

namespace QCatUiKit::Detail
{
struct ChartLodPoint
{
    QPointF point;
    double order = 0;
};
struct ChartLodNode
{
    double minX = 0, maxX = 0;
    ChartLodPoint low, high;
    int first = 0, last = 0, left = -1, right = -1;
};
// Index contiguous runs in transformed axis coordinates. Nodes retain actual extrema,
// while X bounds enclose the control polygon (including offscreen overshoot).
struct ChartCurveLod
{
    QVector<ChartLodNode> nodes;
    QVector<int> roots, isolated;
    template <typename Curve> int append(int first, int last, Curve curve)
    {
        const int index = int(nodes.size());
        nodes.append(ChartLodNode{});
        ChartLodNode node;
        node.first = first;
        node.last = last;
        if (last == first + 1)
        {
            const ChartCubic c = curve(first);
            node.minX = std::min({c.a.x(), c.b.x(), c.c1.x(), c.c2.x()});
            node.maxX = std::max({c.a.x(), c.b.x(), c.c1.x(), c.c2.x()});
            QVector<double> parameters;
            const auto points = cubicExtrema(c, &parameters);
            node.low = node.high = {points[0], double(first)};
            for (int i = 1; i < points.size(); ++i)
            {
                const ChartLodPoint point{points[i], first + parameters[i]};
                if (point.point.y() < node.low.point.y())
                    node.low = point;
                if (point.point.y() > node.high.point.y())
                    node.high = point;
            }
        }
        else
        {
            const int mid = first + (last - first) / 2;
            node.left = append(first, mid, curve);
            node.right = append(mid, last, curve);
            const auto &a = nodes[node.left], &b = nodes[node.right];
            node.minX = std::min(a.minX, b.minX);
            node.maxX = std::max(a.maxX, b.maxX);
            node.low = a.low.point.y() <= b.low.point.y() ? a.low : b.low;
            node.high = a.high.point.y() >= b.high.point.y() ? a.high : b.high;
        }
        nodes[index] = node;
        return index;
    }
};
} // namespace QCatUiKit::Detail
