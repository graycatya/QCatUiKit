// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QRectF>
#include <QVector>
#include <algorithm>
#include <cmath>

namespace QCatUiKit::Detail
{
// Bounded screen-space BVH. Large bands occupy one entry instead of every grid cell.
class ChartPickIndex
{
  public:
    struct Entry
    {
        QRectF rect;
        int id;
    };
    struct Node
    {
        QRectF rect;
        int first, count, left = -1, right = -1;
    };
    QVector<Entry> entries;
    QVector<Node> nodes;
    QRectF bounds;
    void reset(QRectF plot)
    {
        entries.clear();
        nodes.clear();
        bounds = plot;
    }
    void add(QRectF rect, int id)
    {
        rect = rect.normalized().intersected(bounds);
        if (!rect.isEmpty() && std::isfinite(rect.left()) && std::isfinite(rect.top()) &&
            std::isfinite(rect.right()) && std::isfinite(rect.bottom()))
            entries.append({rect, id});
    }
    int build(int first, int count)
    {
        QRectF rect = entries[first].rect;
        for (int i = first + 1; i < first + count; ++i)
            rect = rect.united(entries[i].rect);
        const int index = int(nodes.size());
        nodes.append({rect, first, count});
        if (count > 8)
        {
            const bool horizontal = rect.width() >= rect.height();
            const int middle = first + count / 2;
            std::nth_element(entries.begin() + first, entries.begin() + middle,
                             entries.begin() + first + count,
                             [horizontal](const Entry &a, const Entry &b)
                             {
                                 return horizontal ? a.rect.center().x() < b.rect.center().x()
                                                   : a.rect.center().y() < b.rect.center().y();
                             });
            const int left = build(first, middle - first),
                      right = build(middle, first + count - middle);
            nodes[index].left = left;
            nodes[index].right = right;
        }
        return index;
    }
    void finish()
    {
        if (!entries.isEmpty())
            build(0, int(entries.size()));
    }
    QVector<int> query(QPointF point, quint64 &visited) const
    {
        QVector<int> result;
        const auto visit = [&](auto &&self, int index) -> void
        {
            ++visited;
            const auto &node = nodes[index];
            if (!node.rect.contains(point))
                return;
            if (node.left >= 0)
            {
                self(self, node.left);
                self(self, node.right);
            }
            else
                for (int i = node.first; i < node.first + node.count; ++i)
                    if (entries[i].rect.contains(point))
                        result.append(entries[i].id);
        };
        if (!nodes.isEmpty())
            visit(visit, 0);
        std::sort(result.begin(), result.end(), std::greater<int>());
        return result;
    }
};
} // namespace QCatUiKit::Detail
