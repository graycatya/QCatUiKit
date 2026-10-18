// SPDX-License-Identifier: Apache-2.0
#include "qcatchartcontour.h"
#include "qcatchartstatistics_p.h"
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <array>
#include <cmath>
namespace QCatUiKit
{
namespace
{
struct Node
{
    QPointF point;
    double value;
};
Node crossing(const Node &a, const Node &b, double v)
{
    const double scale = std::max({1., std::abs(a.value), std::abs(b.value), std::abs(v)});
    const double t =
        std::clamp((v / scale - a.value / scale) / (b.value / scale - a.value / scale), 0., 1.);
    return {a.point * (1 - t) + b.point * t, v};
}
QVector<Node> clip(const QVector<Node> &polygon, double level, bool above, bool inclusive = true)
{
    QVector<Node> out;
    if (polygon.isEmpty())
        return out;
    Node a = polygon.last();
    const auto inside = [&](double value) {
        return above ? value >= level : inclusive ? value <= level : value < level;
    };
    bool inA = inside(a.value);
    for (const auto &b : polygon)
    {
        const bool inB = inside(b.value);
        if (inA != inB)
            out.append(crossing(a, b, level));
        if (inB)
            out.append(b);
        a = b;
        inA = inB;
    }
    return out;
}
} // namespace
class ChartContour::Impl
{
  public:
    QPointer<ChartGrid> source;
    QList<QMetaObject::Connection> connections;
    QVariantList levels{-1., 0., 1.};
    bool filled = false, labels = true;
    QString error;
    ChartSeries *series = nullptr;
    QTimer timer;
};
ChartContour::ChartContour(QObject *p) : QObject(p), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Contour);
    d->timer.setParent(this);
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, &ChartContour::refresh);
}
ChartContour::~ChartContour() = default;
ChartGrid *ChartContour::source() const
{
    return d->source;
}
void ChartContour::setSource(ChartGrid *v)
{
    if (QThread::currentThread() != thread() || d->source == v || (v && v->thread() != thread()))
        return;
    for (const auto &c : d->connections)
        disconnect(c);
    d->connections.clear();
    d->source = v;
    if (v)
    {
        d->connections.append(
            connect(v, &ChartGrid::dataChanged, this, [this] { d->timer.start(0); }));
        d->connections.append(connect(v->series(), &ChartSeries::styleChanged, this,
                                      [this]
                                      {
                                          if (d->source &&
                                              (d->series->xAxis() != d->source->series()->xAxis() ||
                                               d->series->yAxis() != d->source->series()->yAxis()))
                                              d->timer.start(0);
                                      }));
        d->connections.append(connect(v, &QObject::destroyed, this,
                                      [this]
                                      {
                                          d->source = nullptr;
                                          refresh();
                                          emit sourceChanged();
                                      }));
    }
    d->timer.start(0);
    emit sourceChanged();
}
QVariantList ChartContour::levels() const
{
    return d->levels;
}
void ChartContour::setLevels(const QVariantList &v)
{
    if (QThread::currentThread() != thread())
        return;
    double previous = -INFINITY;
    bool valid = !v.isEmpty() && v.size() <= 16;
    for (const auto &item : v)
    {
        bool ok = false;
        double n = item.toDouble(&ok);
        if (!ok || !std::isfinite(n) || n <= previous)
            valid = false;
        previous = n;
    }
    if (!valid)
    {
        d->error = tr("Contour levels must contain 1–16 finite increasing values.");
        emit errorChanged();
        return;
    }
    d->levels = v;
    d->timer.start(0);
    emit appearanceChanged();
}
bool ChartContour::filled() const
{
    return d->filled;
}
void ChartContour::setFilled(bool v)
{
    if (QThread::currentThread() != thread() || v == d->filled)
        return;
    d->filled = v;
    d->timer.start(0);
    emit appearanceChanged();
}
bool ChartContour::labelsVisible() const
{
    return d->labels;
}
void ChartContour::setLabelsVisible(bool v)
{
    if (QThread::currentThread() != thread() || v == d->labels)
        return;
    d->labels = v;
    d->timer.start(0);
    emit appearanceChanged();
}
ChartSeries *ChartContour::series() const
{
    return d->series;
}
QString ChartContour::error() const
{
    return d->error;
}
bool ChartContour::refresh()
{
    if (QThread::currentThread() != thread())
        return false;
    d->timer.stop();
    const auto fail = [this](const QString &error)
    {
        d->series->clear();
        ChartStatisticsAccess::setVisual(d->series, {});
        if (d->error != error)
        {
            d->error = error;
            emit errorChanged();
        }
        return false;
    };
    if (!d->source)
        return fail({});
    const int rows = d->source->rows(), columns = d->source->columns();
    if (qint64(rows) * columns > 65536)
        return fail(
            tr("Contours support at most 65,536 grid nodes and 250,000 generated primitives."));
    const auto values = d->source->values();
    const auto origin = d->source->origin();
    const auto spacing = d->source->spacing();
    QVariantList fills, lines;
    auto *scale = d->source->colorScale();
    for (int r = 0; r + 1 < rows; ++r)
        for (int c = 0; c + 1 < columns; ++c)
        {
            const int ids[]{r * columns + c, r * columns + c + 1, (r + 1) * columns + c + 1,
                            (r + 1) * columns + c};
            bool missing = false;
            for (int i : ids)
                missing |= !values[i].isValid() || values[i].isNull();
            if (missing)
                continue;
            const double x = origin.x() + c * spacing.width(),
                         y = origin.y() + r * spacing.height();
            const std::array<Node, 4> corners{
                {{{x, y}, values[ids[0]].toDouble()},
                 {{x + spacing.width(), y}, values[ids[1]].toDouble()},
                 {{x + spacing.width(), y + spacing.height()}, values[ids[2]].toDouble()},
                 {{x, y + spacing.height()}, values[ids[3]].toDouble()}}};
            const Node center{{x + spacing.width() / 2, y + spacing.height() / 2},
                              corners[0].value / 4 + corners[1].value / 4 + corners[2].value / 4 +
                                  corners[3].value / 4};
            const auto append = [&](QVariantList &list, const QVector<Node> &nodes, double lower,
                                    double upper, bool fill)
            {
                QVariantList vertices;
                QPointF centroid;
                for (const auto &node : nodes)
                {
                    vertices.append(node.point);
                    centroid += node.point / double(nodes.size());
                }
                QVariantMap meta{
                    {"cellRow", r},
                    {"cellColumn", c},
                    {"sourceIndex", ids[0]},
                    {"vertices", vertices},
                    {"filled", fill},
                    {"lowerLevel", lower},
                    {"upperLevel", upper},
                    {"level", lower},
                    {"mappedColor", scale->colorAt(fill ? lower / 2 + upper / 2 : lower)}};
                if (!fill && d->labels)
                {
                    meta.insert("contourLabel", QString::number(lower, 'g', 4));
                }
                list.append(QVariantMap{{"x", centroid.x()},
                                        {"y", centroid.y()},
                                        {"value", lower},
                                        {"statistics", meta}});
            };
            for (int side = 0; side < 4; ++side)
            {
                const QVector<Node> tri{corners[side], corners[(side + 1) % 4], center};
                if (d->filled)
                    for (int k = 1; k < d->levels.size(); ++k)
                    {
                        const double lower = d->levels[k - 1].toDouble(),
                                     upper = d->levels[k].toDouble();
                        const auto polygon =
                            clip(clip(tri, lower, true), upper, false, k + 1 == d->levels.size());
                        for (int j = 1; j + 1 < polygon.size(); ++j)
                        {
                            const auto a = polygon[j].point - polygon[0].point,
                                       b = polygon[j + 1].point - polygon[0].point;
                            if (std::abs(a.x() * b.y() - a.y() * b.x()) > 0)
                                append(fills, {polygon[0], polygon[j], polygon[j + 1]}, lower,
                                       upper, true);
                        }
                    }
                for (int k = 0; k < d->levels.size(); ++k)
                {
                    const double level = d->levels[k].toDouble();
                    QVector<Node> intersections;
                    for (int edge = 0; edge < 3; ++edge)
                    {
                        const auto a = tri[edge], b = tri[(edge + 1) % 3];
                        if ((a.value > level) != (b.value > level))
                            intersections.append(crossing(a, b, level));
                    }
                    if (intersections.size() == 2 &&
                        intersections[0].point != intersections[1].point)
                        append(lines, intersections, level, level, false);
                }
                if (fills.size() + lines.size() > 250000)
                    return fail(tr("Contours support at most 65,536 grid nodes and 250,000 "
                                   "generated primitives."));
            }
        }
    fills.append(lines);
    auto visual = ChartStatisticsAccess::visual(d->source->series());
    visual.remove("atlas");
    visual.remove("image");
    visual.insert("contourOrigin", origin);
    visual.insert("contourEnd", origin + QPointF(std::max(0, columns - 1) * spacing.width(),
                                                 std::max(0, rows - 1) * spacing.height()));
    ChartStatisticsAccess::setVisual(d->series, visual);
    ChartStatisticsAccess::publish(d->series, fills, false);
    d->series->setXAxis(d->source->series()->xAxis());
    d->series->setYAxis(d->source->series()->yAxis());
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    return true;
}
} // namespace QCatUiKit
