// SPDX-License-Identifier: Apache-2.0
#include "qcatchart3d_p.h"
#include <QElapsedTimer>
#include <QImageReader>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace QCatUiKit
{
using namespace Detail;
namespace
{
constexpr double pi = 3.14159265358979323846;
bool finite(QVector3D p)
{
    return std::isfinite(p.x()) && std::isfinite(p.y()) && std::isfinite(p.z());
}
QVector3D position(const ChartVertex3D &v)
{
    return {v.x, v.y, v.z};
}
double fraction(double x, double lo, double hi)
{
    const double span = hi - lo;
    return std::isfinite(span) ? (x - lo) / span : (x * .5 - lo * .5) / (hi * .5 - lo * .5);
}
void vertex(ChartGeometry3D &g, QVector3D p, QVector3D n, QColor color = Qt::white, QPointF uv = {},
            int source = -1)
{
    g.vertices.append({p.x(), p.y(), p.z(), n.x(), n.y(), n.z(), float(color.redF()),
                       float(color.greenF()), float(color.blueF()), float(uv.x()), float(uv.y())});
    g.sourceIndices.append(source);
}
void triangle(ChartGeometry3D &g, QVector3D a, QVector3D b, QVector3D c)
{
    const auto n = QVector3D::crossProduct(b - a, c - a).normalized();
    if (n.isNull())
        return;
    vertex(g, a, n);
    vertex(g, b, n);
    vertex(g, c, n);
}
void cube(ChartGeometry3D &g)
{
    const QVector3D p[] = {{-.5, -.5, -.5}, {.5, -.5, -.5}, {.5, .5, -.5}, {-.5, .5, -.5},
                           {-.5, -.5, .5},  {.5, -.5, .5},  {.5, .5, .5},  {-.5, .5, .5}};
    const int faces[][4] = {{0, 3, 2, 1}, {4, 5, 6, 7}, {0, 4, 7, 3},
                            {1, 2, 6, 5}, {0, 1, 5, 4}, {3, 7, 6, 2}};
    for (const auto &f : faces)
    {
        triangle(g, p[f[0]], p[f[1]], p[f[2]]);
        triangle(g, p[f[0]], p[f[2]], p[f[3]]);
    }
}
void roundedCube(ChartGeometry3D &g, int segments)
{
    // Project a subdivided cube onto the Minkowski sum of a .84 cube and a
    // radius .08 sphere. Flat faces, cylindrical edges and spherical corners
    // share analytic normals; the outer bounds remain exactly [-.5, .5].
    QVector<float> steps;
    for (int i = 0; i <= segments; ++i)
        steps.append(-.5f + .08f * i / segments);
    for (int i = 0; i <= segments; ++i)
        steps.append(.42f + .08f * i / segments);
    auto emitVertex = [&](QVector3D p)
    {
        QVector3D core;
        for (int k = 0; k < 3; ++k)
            core[k] = std::clamp(p[k], -.42f, .42f);
        const auto normal = (p - core).normalized();
        vertex(g, core + normal * .08f, normal);
    };
    for (int axis = 0; axis < 3; ++axis)
        for (float sign : {-1.f, 1.f})
        {
            QVector3D n, u, v;
            n[axis] = sign;
            u[(axis + 1) % 3] = 1;
            v[(axis + 2) % 3] = sign;
            for (int row = 0; row + 1 < steps.size(); ++row)
                for (int col = 0; col + 1 < steps.size(); ++col)
                {
                    const auto a = n * .5f + u * steps[col] + v * steps[row];
                    const auto b = n * .5f + u * steps[col + 1] + v * steps[row];
                    const auto c = n * .5f + u * steps[col + 1] + v * steps[row + 1];
                    const auto d = n * .5f + u * steps[col] + v * steps[row + 1];
                    for (auto p : {a, b, c, a, c, d})
                        emitVertex(p);
                }
        }
}
void sphere(ChartGeometry3D &g, int rings, int sectors)
{
    auto point = [=](int r, int c)
    {
        if (r == 0 || r == rings)
            return QVector3D(0, r == 0 ? .5f : -.5f, 0);
        const double a = pi * r / rings, b = 2 * pi * (c % sectors) / sectors;
        return QVector3D(float(std::sin(a) * std::cos(b) * .5), float(std::cos(a) * .5),
                         float(std::sin(a) * std::sin(b) * .5));
    };
    for (int r = 0; r < rings; ++r)
        for (int c = 0; c < sectors; ++c)
        {
            const auto a = point(r, c), b = point(r + 1, c), d = point(r, c + 1),
                       e = point(r + 1, c + 1);
            if (r != 0)
                for (auto p : {a, d, b})
                    vertex(g, p, p.normalized());
            if (r != rings - 1)
                for (auto p : {b, d, e})
                    vertex(g, p, p.normalized());
        }
}
void cylinder(ChartGeometry3D &g, int sectors)
{
    for (int i = 0; i < sectors; ++i)
    {
        const float a = float(i * 2 * pi / sectors), b = float((i + 1) * 2 * pi / sectors);
        QVector3D p(std::cos(a) * .5f, -.5f, std::sin(a) * .5f),
            q(std::cos(b) * .5f, -.5f, std::sin(b) * .5f), up(0, 1, 0);
        triangle(g, {0, -.5, 0}, p, q);
        triangle(g, {0, .5, 0}, q + up, p + up);
        for (auto v : {p, p + up, q, q, p + up, q + up})
            vertex(g, v, QVector3D(v.x(), 0, v.z()).normalized());
    }
}
void tube(ChartGeometry3D &g, QVector3D a, QVector3D b, float radius)
{
    const auto axis = (b - a).normalized();
    if ((b - a).lengthSquared() < 1e-14f)
        return;
    auto u = QVector3D::crossProduct(axis, std::abs(axis.y()) < .9 ? QVector3D(0, 1, 0)
                                                                   : QVector3D(1, 0, 0))
                 .normalized() *
             radius;
    auto v = QVector3D::crossProduct(axis, u);
    for (int i = 0; i < 6; ++i)
    {
        const float x = float(i * 2 * pi / 6), y = float((i + 1) * 2 * pi / 6);
        auto n = u * std::cos(x) + v * std::sin(x), m = u * std::cos(y) + v * std::sin(y);
        const auto nn = n.normalized(), mn = m.normalized();
        vertex(g, a + n, nn);
        vertex(g, a + m, mn);
        vertex(g, b + n, nn);
        vertex(g, a + m, mn);
        vertex(g, b + m, mn);
        vertex(g, b + n, nn);
    }
}
QColor ramp(float f)
{
    return QColor::fromHsvF((1 - std::clamp(f, 0.f, 1.f)) * .65, .8, .95);
}
bool rayBox(QVector3D o, QVector3D v, QVector3D lo, QVector3D hi, float maximum)
{
    // Broad-phase rounding must not reject a second object at the same depth.
    // The triangle test below still decides the exact nearest hit and draw-order tie.
    float near = 0, far = maximum + 8 * std::numeric_limits<float>::epsilon();
    for (int k = 0; k < 3; ++k)
    {
        if (std::abs(v[k]) < 1e-12f)
        {
            if (o[k] < lo[k] || o[k] > hi[k])
                return false;
        }
        else
        {
            float a = (lo[k] - o[k]) / v[k], b = (hi[k] - o[k]) / v[k];
            if (a > b)
                std::swap(a, b);
            near = std::max(near, a);
            far = std::min(far, b);
            if (far < near)
                return false;
        }
    }
    return true;
}
float rayTriangle(QVector3D o, QVector3D v, QVector3D a, QVector3D b, QVector3D c,
                  QVector3D *weights)
{
    const auto e = b - a, f = c - a, p = QVector3D::crossProduct(v, f);
    const float det = QVector3D::dotProduct(e, p);
    if (std::abs(det) < 1e-12f)
        return -1;
    const auto t = o - a;
    const float u = QVector3D::dotProduct(t, p) / det;
    if (u < 0 || u > 1)
        return -1;
    const auto q = QVector3D::crossProduct(t, e);
    const float w = QVector3D::dotProduct(v, q) / det;
    if (w < 0 || u + w > 1)
        return -1;
    if (weights)
        *weights = {1 - u - w, u, w};
    return QVector3D::dotProduct(f, q) / det;
}
} // namespace

class ChartScene3D::Impl
{
  public:
    bool enabled = false, gridVisible = true, supported = false, dirty = true, viewDirty = true;
    QTimer *statusTimer = nullptr;
    bool software = true;
    QVector<ChartSeries3D *> series;
    ChartAxis *axes[3];
    double yaw = 45, pitch = 25, distance = 5, nearPlane = .05, farPlane = 100;
    QVector3D target, lightDirection{.5f, 1.f, 1.f};
    Projection projection = Perspective;
    double ambient = .25, diffuse = .7, specular = .5, shininess = 32;
    LightingModel lightingModel = BlinnPhong;
    double ambientGroundRatio = .6, exposure = 1;
    SelectionMode selectionMode = PointSelection;
    SliceMode sliceMode = NoSlice;
    int sliceIndex = 0, selectedIndex = -1;
    QPointer<ChartSeries3D> selectedSeries;
    QString error;
    QSizeF size;
    double lo[3] = {-1, -1, -1}, hi[3] = {1, 1, 1};
    std::shared_ptr<ChartGeometry3D> geometry;
    std::shared_ptr<const ChartFrame3D> frame;
    std::shared_ptr<ChartRenderStatistics> statistics = std::make_shared<ChartRenderStatistics>();
    quint64 revision = 0, builds = 0, publishedBuild = 0;
    qint64 prepareNs = 0;
    mutable int pickObjects = 0, pickTriangles = 0;
    struct Node
    {
        QVector3D lo, hi;
        int begin = 0, end = 0, left = -1, right = -1;
    };
    QVector<Node> nodes;
    QVector<int> order;
    void coordinates(const ChartSeries3D *s, const ChartSeries3D::Point &p, double *v) const
    {
        v[0] = p.x;
        v[1] = p.y;
        v[2] = p.z;
        if (s->polar() &&
            (s->type() == ChartSeries3D::Scatter || s->type() == ChartSeries3D::Surface))
        {
            const double angle =
                std::remainder(p.x, s->degrees() ? 360 : 2 * pi) * (s->degrees() ? pi / 180 : 1);
            v[0] = p.z * std::cos(angle);
            v[2] = p.z * std::sin(angle);
        }
    }
    bool valid(const ChartSeries3D *s, const ChartSeries3D::Point &p) const
    {
        if (!p.valid ||
            (s->polar() &&
             (s->type() == ChartSeries3D::Scatter || s->type() == ChartSeries3D::Surface) &&
             p.z < 0))
            return false;
        double v[3];
        coordinates(s, p, v);
        for (int k = 0; k < 3; ++k)
            if (!std::isfinite(v[k]) || (axes[k]->scale() == ChartAxis::Logarithmic && v[k] <= 0))
                return false;
        return true;
    }
    QVector3D map(double x, double y, double z) const
    {
        double v[] = {x, y, z};
        QVector3D p;
        for (int k = 0; k < 3; ++k)
        {
            if (axes[k]->scale() == ChartAxis::Logarithmic)
            {
                if (v[k] <= 0)
                    return {NAN, NAN, NAN};
                v[k] = std::log10(v[k]);
            }
            double f = fraction(v[k], lo[k], hi[k]);
            if (axes[k]->reversed())
                f = 1 - f;
            p[k] = float(std::clamp(f * 2 - 1, -10000., 10000.));
        }
        return p;
    }
    QVector3D map(const ChartSeries3D *s, const ChartSeries3D::Point &p) const
    {
        double v[3];
        coordinates(s, p, v);
        return map(v[0], v[1], v[2]);
    }
    bool included(ChartSeries3D *s, int i) const
    {
        if (!s->columns() || sliceMode == NoSlice)
            return true;
        return (sliceMode == RowSlice ? i / s->columns() : i % s->columns()) == sliceIndex;
    }
    bool selected(ChartSeries3D *s, int i) const
    {
        if (selectedSeries != s || selectedIndex < 0)
            return false;
        if (!s->columns() || selectionMode == PointSelection || selectionMode == NoSelection)
            return i == selectedIndex;
        return selectionMode == RowSelection ? i / s->columns() == selectedIndex / s->columns()
                                             : i % s->columns() == selectedIndex % s->columns();
    }
    void instance(int draw, QMatrix4x4 model, QColor color, int si = -1, int index = -1)
    {
        auto &g = *geometry;
        if (g.draws[draw].count <= 0 ||
            (g.draws[draw].firstIndex >= 0 && !g.draws[draw].indexCount))
            return;
        bool invertible = false;
        auto inverse = model.inverted(&invertible);
        if (!invertible)
            return;
        ChartInstance3D instance{};
        std::memcpy(instance.matrix, model.constData(), 64);
        instance.color[0] = float(color.redF());
        instance.color[1] = float(color.greenF());
        instance.color[2] = float(color.blueF());
        instance.color[3] = 1;
        g.instances.append(instance);
        ++g.draws[draw].instances;
        if (si < 0)
            return;
        QVector3D min(1e20f, 1e20f, 1e20f), max(-1e20f, -1e20f, -1e20f);
        const auto &d = g.draws[draw];
        // Shared primitives use cached mesh bounds; transformed corners suffice for the BVH.
        QVector3D a(1e20f, 1e20f, 1e20f), b(-1e20f, -1e20f, -1e20f);
        if (meshBounds.contains(draw))
        {
            a = meshBounds[draw].first;
            b = meshBounds[draw].second;
        }
        else
        {
            for (int i = d.first; i < d.first + d.count; ++i)
            {
                const auto p = position(g.vertices[i]);
                for (int k = 0; k < 3; ++k)
                {
                    a[k] = std::min(a[k], p[k]);
                    b[k] = std::max(b[k], p[k]);
                }
            }
            meshBounds.insert(draw, {a, b});
        }
        for (int i = 0; i < 8; ++i)
        {
            const auto p = model.map(
                QVector3D(i & 1 ? b.x() : a.x(), i & 2 ? b.y() : a.y(), i & 4 ? b.z() : a.z()));
            for (int k = 0; k < 3; ++k)
            {
                min[k] = std::min(min[k], p[k]);
                max[k] = std::max(max[k], p[k]);
            }
        }
        g.objects.append({model, inverse, min, max, draw, si, index});
    }
    QHash<int, QPair<QVector3D, QVector3D>> meshBounds;
    int beginDraw(int texture = -1)
    {
        auto &g = *geometry;
        int i = int(g.draws.size());
        g.draws.append({int(g.vertices.size()), 0, int(g.instances.size()), 0, texture});
        return i;
    }
    void endMesh(int draw)
    {
        geometry->draws[draw].count = int(geometry->vertices.size()) - geometry->draws[draw].first;
    }
    int buildNode(int begin, int end)
    {
        Node node;
        node.begin = begin;
        node.end = end;
        node.lo = QVector3D(1e20f, 1e20f, 1e20f);
        node.hi = -node.lo;
        for (int j = begin; j < end; ++j)
        {
            const auto &o = geometry->objects[order[j]];
            for (int k = 0; k < 3; ++k)
            {
                node.lo[k] = std::min(node.lo[k], o.minimum[k]);
                node.hi[k] = std::max(node.hi[k], o.maximum[k]);
            }
        }
        int index = int(nodes.size());
        nodes.append(node);
        if (end - begin > 8)
        {
            auto span = node.hi - node.lo;
            int axis = span.y() > span.x() ? 1 : 0;
            if (span.z() > span[axis])
                axis = 2;
            int middle = (begin + end) / 2;
            std::nth_element(order.begin() + begin, order.begin() + middle, order.begin() + end,
                             [&](int a, int b)
                             {
                                 const auto &x = geometry->objects[a], &y = geometry->objects[b];
                                 return x.minimum[axis] + x.maximum[axis] <
                                        y.minimum[axis] + y.maximum[axis];
                             });
            int left = buildNode(begin, middle), right = buildNode(middle, end);
            nodes[index].left = left;
            nodes[index].right = right;
        }
        return index;
    }
    void prepare()
    {
        if (!dirty)
            return;
        QElapsedTimer timer;
        timer.start();
        dirty = false;
        meshBounds.clear();
        geometry = std::make_shared<ChartGeometry3D>();
        geometry->revision = ++builds;
        bool found[3] = {false, false, false};
        for (auto *s : series)
            if (s->visible())
                for (const auto &p : s->samples())
                    if (valid(s, p))
                    {
                        double v[3];
                        coordinates(s, p, v);
                        for (int k = 0; k < 3; ++k)
                        {
                            if (axes[k]->scale() == ChartAxis::Logarithmic)
                                v[k] = std::log10(v[k]);
                            if (!found[k])
                            {
                                lo[k] = hi[k] = v[k];
                                found[k] = true;
                            }
                            else
                            {
                                lo[k] = std::min(lo[k], v[k]);
                                hi[k] = std::max(hi[k], v[k]);
                            }
                        }
                        if (s->type() == ChartSeries3D::Bars &&
                            axes[1]->scale() != ChartAxis::Logarithmic)
                        {
                            lo[1] = std::min(lo[1], s->baseline());
                            hi[1] = std::max(hi[1], s->baseline());
                        }
                    }
        for (int k = 0; k < 3; ++k)
        {
            if (!found[k])
            {
                lo[k] = -1;
                hi[k] = 1;
            }
            if (!axes[k]->automatic())
            {
                lo[k] = axes[k]->minimum();
                hi[k] = axes[k]->maximum();
                if (axes[k]->scale() == ChartAxis::Logarithmic)
                {
                    lo[k] = std::log10(lo[k]);
                    hi[k] = std::log10(hi[k]);
                }
            }
            if (!(hi[k] > lo[k]) || !std::isfinite(lo[k]) || !std::isfinite(hi[k]))
            {
                double c = std::isfinite(lo[k]) ? lo[k] : 0;
                double d = std::max(1., std::abs(c) * .01);
                lo[k] = c - d;
                hi[k] = c + d;
                if (!std::isfinite(lo[k]))
                    lo[k] = c;
                if (!std::isfinite(hi[k]))
                    hi[k] = c;
            }
        }
        int barCount = 0;
        for (auto *s : series)
            if (s->visible() && s->type() == ChartSeries3D::Bars && s->count() && s->rows() &&
                s->columns())
                ++barCount;
        int barOrdinal = 0;
        for (int si = 0; si < series.size(); ++si)
        {
            auto *s = series[si];
            if (!s->visible() || !s->count())
                continue;
            if (s->type() == ChartSeries3D::Bars || s->type() == ChartSeries3D::Surface)
            {
                if (!s->rows() || !s->columns())
                {
                    geometry->error =
                        QStringLiteral("Bars and surfaces require setGrid(rows, columns, points).");
                    continue;
                }
            }
            const auto &points = s->samples();
            if (s->type() == ChartSeries3D::Scatter || s->type() == ChartSeries3D::Bars ||
                (s->type() == ChartSeries3D::Curve && s->nodesVisible()))
            {
                int draw = beginDraw();
                const int detail = int(s->meshDetail());
                const int rings[] = {8, 12, 20}, sectors[] = {12, 24, 40};
                if (s->shape() == ChartSeries3D::CustomMesh)
                {
                    const auto mesh = s->customMesh();
                    for (int j = 0; j + 2 < mesh.size(); j += 3)
                        triangle(*geometry, mesh[j].value<QVector3D>(),
                                 mesh[j + 1].value<QVector3D>(), mesh[j + 2].value<QVector3D>());
                }
                else if (s->shape() == ChartSeries3D::RoundedCube)
                    roundedCube(*geometry, 1 << detail);
                else if (s->shape() == ChartSeries3D::Cube ||
                         (s->type() == ChartSeries3D::Bars &&
                          s->shape() != ChartSeries3D::Cylinder))
                    cube(*geometry);
                else if (s->shape() == ChartSeries3D::Cylinder)
                    cylinder(*geometry, sectors[detail]);
                else
                    sphere(*geometry, rings[detail], sectors[detail]);
                endMesh(draw);
                if (!geometry->draws[draw].count)
                {
                    geometry->error =
                        QStringLiteral("Custom mesh contains no non-degenerate triangles.");
                    continue;
                }
                if (s->type() == ChartSeries3D::Bars && s->shape() == ChartSeries3D::CustomMesh)
                {
                    QVector3D lo(100, 100, 100), hi(-100, -100, -100);
                    for (int i = geometry->draws[draw].first; i < geometry->vertices.size(); ++i)
                    {
                        const auto p = position(geometry->vertices[i]);
                        for (int k = 0; k < 3; ++k)
                        {
                            lo[k] = std::min(lo[k], p[k]);
                            hi[k] = std::max(hi[k], p[k]);
                        }
                    }
                    const auto span = hi - lo, center = (hi + lo) * .5f;
                    if (span.x() < 1e-8f || span.y() < 1e-8f || span.z() < 1e-8f)
                    {
                        geometry->error = QStringLiteral(
                            "A custom bar mesh requires nonzero extent on all three axes.");
                        continue;
                    }
                    for (int i = geometry->draws[draw].first; i < geometry->vertices.size(); ++i)
                    {
                        auto &v = geometry->vertices[i];
                        const auto p = (position(v) - center) / span;
                        auto n = (QVector3D(v.nx, v.ny, v.nz) * span).normalized();
                        v.x = p.x();
                        v.y = p.y();
                        v.z = p.z();
                        v.nx = n.x();
                        v.ny = n.y();
                        v.nz = n.z();
                    }
                }
                for (int i = 0; i < points.size(); ++i)
                    if (valid(s, points[i]) && included(s, i))
                    {
                        auto p = map(s, points[i]);
                        QMatrix4x4 model;
                        if (s->type() == ChartSeries3D::Bars)
                        {
                            if (axes[1]->scale() == ChartAxis::Logarithmic && s->baseline() <= 0)
                            {
                                geometry->error =
                                    QStringLiteral("Logarithmic bars require a positive baseline.");
                                continue;
                            }
                            float base = map(points[i].x, s->baseline(), points[i].z).y();
                            float height = std::abs(p.y() - base);
                            if (height < 1e-7f)
                                continue;
                            float width = float(2 * s->barWidth() / std::max(1, s->columns()) /
                                                std::max(1, barCount));
                            p.setX(p.x() + (barOrdinal - (barCount - 1) * .5f) * width);
                            model.translate(p.x(), (p.y() + base) * .5f, p.z());
                            model.scale(width, height,
                                        float(2 * s->barWidth() / std::max(1, s->rows())));
                        }
                        else
                        {
                            model.translate(p);
                            model.rotate(QQuaternion::fromEulerAngles(points[i].rotation));
                            model.scale(float(s->pointSize() * points[i].size));
                        }
                        QColor color = selected(s, i)              ? QColor("#ffbf00")
                                       : points[i].color.isValid() ? points[i].color
                                       : s->colorByHeight()        ? ramp((p.y() + 1) * .5f)
                                                                   : s->color();
                        instance(draw, model, color, si, i);
                    }
                if (s->type() == ChartSeries3D::Bars)
                    ++barOrdinal;
            }
            if (s->type() == ChartSeries3D::Curve)
            {
                if (qint64(points.size()) * s->subdivisions() * 36 > 6000000)
                {
                    geometry->error = QStringLiteral("Curve tessellation exceeds the 6000000 "
                                                     "vertex budget; reduce subdivisions or data.");
                    continue;
                }
                int draw = beginDraw();
                int n = int(points.size());
                for (int i = 0; i < (s->closed() ? n : n - 1); ++i)
                {
                    int next = (i + 1) % n;
                    if (!valid(s, points[i]) || !valid(s, points[next]))
                        continue;
                    auto at = [&](int j)
                    {
                        if (s->closed())
                            j = (j + n) % n;
                        else
                            j = std::clamp(j, 0, n - 1);
                        return valid(s, points[j]) ? map(s, points[j]) : map(s, points[i]);
                    };
                    auto a = at(i), b = at(i + 1),
                         m = (at(i + 1) - at(i - 1)) * float((1 - s->tension()) * .5),
                         v = (at(i + 2) - at(i)) * float((1 - s->tension()) * .5), last = a;
                    for (int step = 1; step <= s->subdivisions(); ++step)
                    {
                        float t = float(step) / s->subdivisions(), t2 = t * t, t3 = t2 * t;
                        auto p = a * (2 * t3 - 3 * t2 + 1) + m * (t3 - 2 * t2 + t) +
                                 b * (-2 * t3 + 3 * t2) + v * (t3 - t2);
                        int first = int(geometry->vertices.size());
                        tube(*geometry, last, p, float(s->pointSize() * .15));
                        for (int k = first; k < geometry->sourceIndices.size(); ++k)
                        {
                            int source = t < .5 ? i : next;
                            geometry->sourceIndices[k] = source;
                            const auto color = selected(s, source) ? QColor("#ffbf00") : s->color();
                            auto &v = geometry->vertices[k];
                            v.r = float(color.redF());
                            v.g = float(color.greenF());
                            v.b = float(color.blueF());
                        }
                        last = p;
                    }
                }
                endMesh(draw);
                instance(draw, QMatrix4x4(), Qt::white, si, -1);
            }
            if (s->type() == ChartSeries3D::Surface)
                surface(s, si);
        }
        if (gridVisible)
        {
            int draw = beginDraw();
            for (int i = 0; i <= 10; ++i)
            {
                float f = -1 + i * .2f;
                tube(*geometry, {-1, -1, f}, {1, -1, f}, .0025f);
                tube(*geometry, {f, -1, -1}, {f, -1, 1}, .0025f);
            }
            for (int axis = 0; axis < 3; ++axis)
                for (int side = 0; side < 4; ++side)
                {
                    QVector3D a(-1, -1, -1), b;
                    a[(axis + 1) % 3] = (side & 1) ? 1.f : -1.f;
                    a[(axis + 2) % 3] = (side & 2) ? 1.f : -1.f;
                    b = a;
                    b[axis] = 1;
                    tube(*geometry, a, b, .0025f);
                }
            for (int axis = 0; axis < 3; ++axis)
                if (axes[axis]->isVisible())
                {
                    QVector3D end(-1, -1, -1);
                    end[axis] = 1;
                    tube(*geometry, {-1, -1, -1}, end, .006f);
                }
            endMesh(draw);
            instance(draw, QMatrix4x4(), QColor("#888888"));
        }
        nodes.clear();
        order.resize(geometry->objects.size());
        for (int i = 0; i < order.size(); ++i)
            order[i] = i;
        if (!order.isEmpty())
            buildNode(0, int(order.size()));
        prepareNs = timer.nsecsElapsed();
    }
    void surface(ChartSeries3D *s, int si)
    {
        const auto &points = s->samples();
        int rows = s->rows(), cols = s->columns();
        if (sliceMode == NoSlice &&
            qint64(rows - 1) * (cols - 1) * (s->surfaceMode() == ChartSeries3D::Filled ? 6 : 150) >
                6000000)
        {
            geometry->error = QStringLiteral("Surface tessellation exceeds the 6000000 vertex "
                                             "budget; reduce the grid or use a slice.");
            return;
        }
        QVector<QVector3D> p;
        p.reserve(points.size());
        for (const auto &v : points)
            p.append(map(s, v));
        if (sliceMode != NoSlice)
        {
            int draw = beginDraw();
            const int count = sliceMode == RowSlice ? cols : rows;
            for (int j = 0; j < count - 1; ++j)
            {
                int a = sliceMode == RowSlice ? sliceIndex * cols + j : j * cols + sliceIndex,
                    b = sliceMode == RowSlice ? a + 1 : a + cols;
                if (a < 0 || b >= points.size() ||
                    (sliceMode == ColumnSlice && sliceIndex >= cols) || !valid(s, points[a]) ||
                    !valid(s, points[b]))
                    continue;
                int first = int(geometry->vertices.size());
                tube(*geometry, p[a], p[b], .008f);
                for (int k = first; k < geometry->sourceIndices.size(); ++k)
                {
                    auto &v = geometry->vertices[k];
                    const int source =
                        (position(v) - p[a]).lengthSquared() <= (position(v) - p[b]).lengthSquared()
                            ? a
                            : b;
                    geometry->sourceIndices[k] = source;
                    const auto color = selected(s, source)              ? QColor("#ffbf00")
                                       : points[source].color.isValid() ? points[source].color
                                       : s->colorByHeight() ? ramp((p[source].y() + 1) * .5f)
                                                            : s->color();
                    v.r = float(color.redF());
                    v.g = float(color.greenF());
                    v.b = float(color.blueF());
                }
            }
            endMesh(draw);
            instance(draw, QMatrix4x4(), Qt::white, si, -1);
            return;
        }
        QVector<int> indices;
        for (int r = 0; r < rows - 1; ++r)
            for (int c = 0; c < (s->polar() && s->closed() ? cols : cols - 1); ++c)
            {
                int a = r * cols + c, b = r * cols + (c + 1) % cols, d = a + cols, e = b + cols;
                if (!valid(s, points[a]) || !valid(s, points[b]) || !valid(s, points[d]) ||
                    !valid(s, points[e]))
                    continue;
                indices << a << d << b << b << d << e;
            }
        QVector<QVector3D> normals(points.size());
        for (int i = 0; i < indices.size(); i += 3)
        {
            int a = indices[i], b = indices[i + 1], c = indices[i + 2];
            auto n = QVector3D::crossProduct(p[b] - p[a], p[c] - p[a]);
            normals[a] += n;
            normals[b] += n;
            normals[c] += n;
        }
        for (auto &n : normals)
        {
            if (n.isNull())
                n = {0, 1, 0};
            else
                n.normalize();
            if (n.y() < 0)
                n = -n;
        }
        int texture = -1;
        if (!s->textureSource().isEmpty())
        {
            const auto u = s->textureSource();
            const auto path = u.isLocalFile()        ? u.toLocalFile()
                              : u.scheme() == "qrc"  ? ":" + u.path()
                              : u.scheme().isEmpty() ? u.toString()
                                                     : QString();
            QImageReader reader(path);
            const auto size = path.isEmpty() ? QSize() : reader.size();
            if (!path.isEmpty() && size.width() > 0 && size.height() > 0 && size.width() <= 4096 &&
                size.height() <= 4096)
            {
                auto image = reader.read();
                if (!image.isNull())
                {
                    texture = int(geometry->textures.size());
                    geometry->textures.append(image);
                }
            }
            if (texture < 0)
                geometry->error = QStringLiteral(
                    "Surface texture requires a readable local/qrc image up to 4096x4096.");
        }
        if (s->surfaceMode() != ChartSeries3D::Wireframe && !indices.isEmpty())
        {
            int draw = beginDraw(texture);
            geometry->draws[draw].firstIndex = int(geometry->indices.size());
            geometry->draws[draw].indexCount = int(indices.size());
            for (int index : indices)
                geometry->indices.append(quint32(index));
            for (int i = 0; i < points.size(); ++i)
            {
                QColor color = selected(s, i)              ? QColor("#ffbf00")
                               : points[i].color.isValid() ? points[i].color
                               : s->colorByHeight()        ? ramp((p[i].y() + 1) * .5f)
                                                           : s->color();
                vertex(*geometry, p[i], normals[i], color,
                       {double(i % cols) / std::max(1, cols - 1),
                        double(i / cols) / std::max(1, rows - 1)},
                       i);
            }
            endMesh(draw);
            instance(draw, QMatrix4x4(), Qt::white, si, -1);
        }
        if (s->surfaceMode() != ChartSeries3D::Filled)
        {
            int draw = beginDraw();
            // Draw only edges belonging to valid cells. Raised tubes avoid coplanar depth flicker.
            QSet<quint64> edges;
            for (int i = 0; i < indices.size(); i += 6)
            {
                int a = indices[i], d = indices[i + 1], b = indices[i + 2], e = indices[i + 5];
                for (auto edge :
                     {qMakePair(a, b), qMakePair(a, d), qMakePair(b, e), qMakePair(d, e)})
                {
                    int x = std::min(edge.first, edge.second),
                        y = std::max(edge.first, edge.second);
                    quint64 key = (quint64(x) << 32) | quint32(y);
                    if (edges.contains(key))
                        continue;
                    edges.insert(key);
                    int first = int(geometry->vertices.size());
                    tube(*geometry, p[x] + normals[x] * .003f, p[y] + normals[y] * .003f, .0025f);
                    for (int k = first; k < geometry->sourceIndices.size(); ++k)
                    {
                        geometry->sourceIndices[k] =
                            (position(geometry->vertices[k]) - p[x]).lengthSquared() <=
                                    (position(geometry->vertices[k]) - p[y]).lengthSquared()
                                ? x
                                : y;
                        const auto color = selected(s, x) || selected(s, y) ? QColor("#ffbf00")
                                                                            : QColor("#333333");
                        auto &v = geometry->vertices[k];
                        v.r = float(color.redF());
                        v.g = float(color.greenF());
                        v.b = float(color.blueF());
                    }
                }
            }
            endMesh(draw);
            instance(draw, QMatrix4x4(), Qt::white, si, -1);
        }
    }
    void camera()
    {
        auto next = std::make_shared<ChartFrame3D>();
        next->geometry = geometry;
        next->revision = revision;
        const double a = yaw * pi / 180, b = pitch * pi / 180;
        next->eye = target + QVector3D(float(std::sin(a) * std::cos(b) * distance),
                                       float(std::sin(b) * distance),
                                       float(std::cos(a) * std::cos(b) * distance));
        QMatrix4x4 view, projectionMatrix;
        view.lookAt(next->eye, target, {0, 1, 0});
        const float aspect = float(std::max(1., size.width()) / std::max(1., size.height()));
        if (projection == Perspective)
            projectionMatrix.perspective(40, aspect, float(nearPlane), float(farPlane));
        else
        {
            float span = float(distance * .36);
            projectionMatrix.ortho(-span * aspect, span * aspect, -span, span, float(nearPlane),
                                   float(farPlane));
        }
        QMatrix4x4 inset;
        inset.scale(.82f, .82f, 1.f);
        next->viewProjection = inset * projectionMatrix * view;
        next->light = lightDirection;
        next->ambient = float(ambient);
        next->diffuse = float(diffuse);
        next->specular = float(specular);
        next->shininess = float(shininess);
        next->lightingModel = lightingModel;
        next->ambientGroundRatio = float(ambientGroundRatio);
        next->exposure = float(exposure);
        frame = next;
    }
    QPointF project(QVector3D p) const
    {
        if (!frame || !finite(p))
            return {NAN, NAN};
        const auto clip = frame->viewProjection * QVector4D(p, 1);
        if (clip.w() <= 0 || clip.z() < -clip.w() || clip.z() > clip.w())
            return {NAN, NAN};
        return {(clip.x() / clip.w() + 1) * size.width() * .5,
                (1 - clip.y() / clip.w()) * size.height() * .5};
    }
};

ChartScene3D::ChartScene3D(QObject *parent) : QObject(parent), d(new Impl)
{
    for (auto &axis : d->axes)
    {
        axis = new ChartAxis(this);
        connect(axis, &ChartAxis::changed, this,
                [this]
                {
                    d->dirty = true;
                    emit changed();
                });
    }
    d->statusTimer = new QTimer(this);
    d->statusTimer->setInterval(250);
    connect(d->statusTimer, &QTimer::timeout, this,
            [this]
            {
                const QString renderError = d->statistics->values().value("error").toString();
                const QString error =
                    d->software ? QStringLiteral("3D charts require a Qt Quick RHI graphics "
                                                 "backend; software rendering is unsupported.")
                    : d->geometry && !d->geometry->error.isEmpty() ? d->geometry->error
                                                                   : renderError;
                const bool supported = !d->software && renderError.isEmpty();
                if (error != d->error || supported != d->supported)
                {
                    d->error = error;
                    d->supported = supported;
                    emit statusChanged();
                }
            });
    connect(this, &ChartScene3D::changed, this, [this] { d->dirty = true; });
    connect(this, &ChartScene3D::cameraChanged, this, [this] { d->viewDirty = true; });
    connect(this, &ChartScene3D::materialChanged, this, [this] { d->viewDirty = true; });
}
ChartScene3D::~ChartScene3D()
{
    for (auto *s : d->series)
        disconnect(s, nullptr, this, nullptr);
}
QQmlListProperty<ChartSeries3D> ChartScene3D::series()
{
    return {this,
            nullptr,
            [](QQmlListProperty<ChartSeries3D> *p, ChartSeries3D *s)
            { static_cast<ChartScene3D *>(p->object)->addSeries(s); },
            [](QQmlListProperty<ChartSeries3D> *p) -> qsizetype
            { return static_cast<ChartScene3D *>(p->object)->seriesCount(); },
            [](QQmlListProperty<ChartSeries3D> *p, qsizetype i)
            { return static_cast<ChartScene3D *>(p->object)->seriesAt(int(i)); },
            [](QQmlListProperty<ChartSeries3D> *p)
            { static_cast<ChartScene3D *>(p->object)->clearSeries(); }};
}
void ChartScene3D::addSeries(ChartSeries3D *s)
{
    if (QThread::currentThread() != thread() || !s || s->thread() != thread() ||
        d->series.contains(s))
        return;
    d->series.append(s);
    connect(s, &ChartSeries3D::changed, this,
            [this, s]
            {
                if (d->selectedSeries == s)
                    emit selectionChanged();
                emit changed();
            });
    connect(s, &ChartSeries3D::dataChanged, this,
            [this, s]
            {
                if (d->selectedSeries == s)
                    clearSelection();
                emit changed();
            });
    connect(s, &QObject::destroyed, this,
            [this, s]
            {
                d->series.removeAll(s);
                if (!d->selectedSeries)
                {
                    d->selectedIndex = -1;
                    emit selectionChanged();
                }
                emit changed();
            });
    emit changed();
}
void ChartScene3D::removeSeries(ChartSeries3D *s)
{
    if (QThread::currentThread() != thread() || !d->series.removeOne(s))
        return;
    if (d->selectedSeries == s)
        clearSelection();
    disconnect(s, nullptr, this, nullptr);
    if (d->selectedSeries)
        emit selectionChanged();
    emit changed();
}
void ChartScene3D::clearSeries()
{
    if (QThread::currentThread() != thread())
        return;
    const auto list = d->series;
    for (auto *s : list)
        removeSeries(s);
}
int ChartScene3D::seriesCount() const
{
    return int(d->series.size());
}
ChartSeries3D *ChartScene3D::seriesAt(int i) const
{
    return i >= 0 && i < d->series.size() ? d->series[i] : nullptr;
}
ChartAxis *ChartScene3D::xAxis() const
{
    return d->axes[0];
}
ChartAxis *ChartScene3D::yAxis() const
{
    return d->axes[1];
}
ChartAxis *ChartScene3D::zAxis() const
{
    return d->axes[2];
}
QVariantMap ChartScene3D::selectedPoint() const
{
    if (!d->selectedSeries || d->selectedIndex < 0)
        return {};
    auto result = d->selectedSeries->at(d->selectedIndex);
    if (result.isEmpty())
        return {};
    result.insert("seriesIndex", d->series.indexOf(d->selectedSeries));
    result.insert("seriesName", d->selectedSeries->name());
    result.insert("index", d->selectedIndex);
    if (d->selectedSeries->columns())
    {
        result.insert("row", d->selectedIndex / d->selectedSeries->columns());
        result.insert("column", d->selectedIndex % d->selectedSeries->columns());
    }
    return result;
}
bool ChartScene3D::selectPoint(int si, int index)
{
    if (QThread::currentThread() != thread())
        return false;
    auto *s = seriesAt(si);
    if (!s || index < 0 || index >= s->count() || !s->samples()[index].valid)
        return false;
    if (d->selectedSeries == s && d->selectedIndex == index)
        return true;
    d->selectedSeries = s;
    d->selectedIndex = index;
    d->dirty = true;
    emit selectionChanged();
    return true;
}
void ChartScene3D::clearSelection()
{
    if (QThread::currentThread() != thread() || d->selectedIndex < 0)
        return;
    d->selectedIndex = -1;
    d->selectedSeries = nullptr;
    d->dirty = true;
    emit selectionChanged();
}
void ChartScene3D::resetView()
{
    setTarget({});
    setDistance(5);
    setNearPlane(.05);
    setFarPlane(100);
    setProjection(Perspective);
    setPreset(Isometric);
}
void ChartScene3D::setPreset(Preset preset)
{
    switch (preset)
    {
    case Isometric:
        setYaw(45);
        setPitch(25);
        break;
    case Front:
        setYaw(0);
        setPitch(0);
        break;
    case Top:
        setYaw(0);
        setPitch(89.9);
        break;
    case Right:
        setYaw(90);
        setPitch(0);
        break;
    }
}
void ChartScene3D::rotateBy(QPointF v)
{
    if (std::isfinite(v.x()) && std::isfinite(v.y()))
    {
        setYaw(d->yaw + v.x());
        setPitch(d->pitch + v.y());
    }
}
void ChartScene3D::panBy(QPointF v)
{
    if (!std::isfinite(v.x()) || !std::isfinite(v.y()))
        return;
    const auto rotation = QQuaternion::fromEulerAngles(float(-d->pitch), float(d->yaw), 0);
    auto delta = rotation.rotatedVector(QVector3D(float(-v.x()), float(v.y()), 0)) *
                 float(d->distance * .72 / (.82 * std::max(1., d->size.height())));
    setTarget(d->target + delta);
}
void ChartScene3D::zoomBy(double factor)
{
    if (std::isfinite(factor) && factor > 0)
        setDistance(std::clamp(d->distance / factor, .2, 100.));
}
bool ChartScene3D::supported() const
{
    return d->supported;
}
QString ChartScene3D::error() const
{
    return d->error;
}
QPointF ChartScene3D::mapToPosition(double x, double y, double z) const
{
    d->prepare();
    d->camera();
    return d->project(d->map(x, y, z));
}
QVariantMap ChartScene3D::renderingStatistics() const
{
    auto m = d->statistics->values();
    m.insert("pending", d->dirty || d->viewDirty || d->publishedBuild != d->builds);
    m.insert("geometryBuilds", d->builds);
    m.insert("preparedRevision", d->revision);
    m.insert("preparationNanoseconds", d->prepareNs);
    m.insert("pickObjects", d->pickObjects);
    m.insert("pickTriangles", d->pickTriangles);
    m.insert("backend", d->supported ? "rhi3d" : "unsupported");
    return m;
}
QVariantMap ChartScene3D::dataAt(QPointF at) const
{
    if (!d->supported || !std::isfinite(at.x()) || !std::isfinite(at.y()) ||
        !QRectF(QPointF(), d->size).contains(at))
        return {};
    d->prepare();
    d->camera();
    if (!d->frame || d->nodes.isEmpty())
        return {};
    bool invertible = false;
    const auto inv = d->frame->viewProjection.inverted(&invertible);
    if (!invertible)
        return {};
    const float x = float(at.x() / d->size.width() * 2 - 1),
                y = float(1 - at.y() / d->size.height() * 2);
    auto origin = inv.map(QVector3D(x, y, -1)), end = inv.map(QVector3D(x, y, 1)),
         ray = end - origin;
    float nearest = 1;
    int found = -1, source = -1;
    d->pickObjects = d->pickTriangles = 0;
    QVector<int> stack{0};
    while (!stack.isEmpty())
    {
        const auto &node = d->nodes[stack.takeLast()];
        if (!rayBox(origin, ray, node.lo, node.hi, nearest))
            continue;
        if (node.left >= 0)
        {
            stack << node.left << node.right;
            continue;
        }
        for (int j = node.begin; j < node.end; ++j)
        {
            int id = d->order[j];
            const auto &object = d->geometry->objects[id];
            if (!rayBox(origin, ray, object.minimum, object.maximum, nearest))
                continue;
            ++d->pickObjects;
            auto o = object.inverse.map(origin), v = object.inverse.mapVector(ray);
            const auto &draw = d->geometry->draws[object.draw];
            const int count = draw.firstIndex >= 0 ? draw.indexCount : draw.count;
            for (int i = 0; i + 2 < count; i += 3)
            {
                int ids[3];
                for (int k = 0; k < 3; ++k)
                    ids[k] = draw.first + (draw.firstIndex >= 0
                                               ? int(d->geometry->indices[draw.firstIndex + i + k])
                                               : i + k);
                ++d->pickTriangles;
                QVector3D weights;
                float t = rayTriangle(o, v, position(d->geometry->vertices[ids[0]]),
                                      position(d->geometry->vertices[ids[1]]),
                                      position(d->geometry->vertices[ids[2]]), &weights);
                if (t >= 0 && (t < nearest || (t == nearest && id > found)))
                {
                    nearest = t;
                    found = id;
                    int k = weights.y() > weights.x() ? 1 : 0;
                    if (weights.z() > weights[k])
                        k = 2;
                    source = object.index >= 0 ? object.index : d->geometry->sourceIndices[ids[k]];
                }
            }
        }
    }
    if (found < 0 || source < 0)
        return {};
    const auto &object = d->geometry->objects[found];
    auto *s = seriesAt(object.series);
    if (!s)
        return {};
    auto result = s->at(source);
    result.insert("index", source);
    result.insert("seriesIndex", object.series);
    result.insert("seriesName", s->name());
    result.insert("selected", d->selected(s, source));
    if (s->columns())
    {
        result.insert("row", source / s->columns());
        result.insert("column", source % s->columns());
    }
    return result;
}

std::shared_ptr<const ChartFrame3D> ChartScene3DAccess::frame(ChartScene3D *s, QSizeF size,
                                                              bool software)
{
    s->d->size = size;
    s->d->software = software;
    s->d->prepare();
    ++s->d->revision;
    s->d->camera();
    s->d->viewDirty = false;
    s->d->publishedBuild = s->d->builds;
    const auto renderError = s->d->statistics->values().value("error").toString();
    QString error = software ? QStringLiteral("3D charts require a Qt Quick RHI graphics backend; "
                                              "software rendering is unsupported.")
                    : !s->d->geometry->error.isEmpty() ? s->d->geometry->error
                                                       : renderError;
    bool supported = !software && renderError.isEmpty();
    if (s->d->supported != supported || s->d->error != error)
    {
        s->d->supported = supported;
        s->d->error = error;
        emit s->statusChanged();
    }
    return s->d->frame;
}
std::shared_ptr<ChartRenderStatistics> ChartScene3DAccess::statistics(ChartScene3D *s)
{
    return s->d->statistics;
}
void ChartScene3DAccess::click(ChartScene3D *s, QPointF position)
{
    auto result = s->dataAt(position);
    if (s->selectionMode() != ChartScene3D::NoSelection)
    {
        if (result.isEmpty())
            s->clearSelection();
        else
            s->selectPoint(result.value("seriesIndex").toInt(), result.value("index").toInt());
    }
    if (!result.isEmpty())
        result.insert("selected", s->d->selected(s->seriesAt(result.value("seriesIndex").toInt()),
                                                 result.value("index").toInt()));
    emit s->pointClicked(result);
}
QVector<ChartLabel3D> ChartScene3DAccess::labels(ChartScene3D *s)
{
    QVector<ChartLabel3D> result;
    if (!s->d->supported)
        return result;
    if (s->d->gridVisible)
        for (int k = 0; k < 3; ++k)
            if (s->d->axes[k]->isVisible())
            {
                auto *axis = s->d->axes[k];
                const float sx = s->d->frame->eye.x() >= 0 ? 1.f : -1.f,
                            sz = s->d->frame->eye.z() >= 0 ? 1.f : -1.f;
                QVector3D anchor(-1.12f * sx, -1.12f, 1.12f * sz);
                QVector3D p = anchor;
                p[k] = 1.15f;
                const QPointF offset = k == 0   ? QPointF(8, 10)
                                       : k == 1 ? QPointF(0, -10)
                                                : QPointF(-18, 0);
                result.append({s->d->project(p) + offset,
                               axis->title().isEmpty() ? QString(QChar('X' + k)) : axis->title(),
                               1});
                for (int i = 0; i < 5; ++i)
                {
                    p = anchor;
                    p[k] = -1 + i * .5f;
                    double f = i * .25;
                    if (axis->reversed())
                        f = 1 - f;
                    double v = s->d->lo[k] * (1 - f) + s->d->hi[k] * f;
                    if (axis->scale() == ChartAxis::Logarithmic)
                        v = std::pow(10, v);
                    const auto categories = axis->categories();
                    QString label = axis->scale() == ChartAxis::Category && v >= -.5 &&
                                            v < double(categories.size()) - .5
                                        ? categories[int(qRound64(v))]
                                        : QString::number(v, 'g', 4);
                    result.append({s->d->project(p), label});
                }
            }
    if (s->d->selectedSeries && s->d->selectedIndex >= 0)
    {
        auto *series = s->d->selectedSeries.data();
        const auto &p = series->samples()[s->d->selectedIndex];
        auto screen = s->d->project(s->d->map(series, p));
        const auto hit = s->dataAt(screen);
        if (hit.value("seriesIndex", -1).toInt() == s->d->series.indexOf(series) &&
            hit.value("index", -1).toInt() == s->d->selectedIndex)
            result.append({screen + QPointF(0, -18),
                           p.label.isEmpty()
                               ? QStringLiteral("%1, %2, %3").arg(p.x).arg(p.y).arg(p.z)
                               : p.label,
                           2});
    }
    return result;
}
bool ChartScene3D::enabled() const
{
    return d->enabled;
}
void ChartScene3D::setEnabled(bool value)
{
    if (QThread::currentThread() != thread())
        return;
    if (d->enabled == value)
        return;
    d->enabled = value;
    if (value)
        d->statusTimer->start();
    else
        d->statusTimer->stop();
    emit changed();
}
double ChartScene3D::yaw() const
{
    return d->yaw;
}
void ChartScene3D::setYaw(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value))
        return;
    value = std::remainder(value, 360.);
    if (d->yaw == value)
        return;
    d->yaw = value;
    emit cameraChanged();
}
double ChartScene3D::pitch() const
{
    return d->pitch;
}
void ChartScene3D::setPitch(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value))
        return;
    value = std::clamp(value, -89.9, 89.9);
    if (d->pitch == value)
        return;
    d->pitch = value;
    emit cameraChanged();
}
double ChartScene3D::distance() const
{
    return d->distance;
}
void ChartScene3D::setDistance(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < .2 || value > 100)
        return;
    if (d->distance == value)
        return;
    d->distance = value;
    emit cameraChanged();
}
QVector3D ChartScene3D::target() const
{
    return d->target;
}
void ChartScene3D::setTarget(QVector3D value)
{
    if (QThread::currentThread() != thread() || !finite(value) || value.length() > 100)
        return;
    if (d->target == value)
        return;
    d->target = value;
    emit cameraChanged();
}
ChartScene3D::Projection ChartScene3D::projection() const
{
    return d->projection;
}
void ChartScene3D::setProjection(Projection value)
{
    if (QThread::currentThread() != thread() || value < Perspective || value > Orthographic)
        return;
    if (d->projection == value)
        return;
    d->projection = value;
    emit cameraChanged();
}
double ChartScene3D::nearPlane() const
{
    return d->nearPlane;
}
void ChartScene3D::setNearPlane(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < .001 ||
        value >= d->farPlane)
        return;
    if (d->nearPlane == value)
        return;
    d->nearPlane = value;
    emit cameraChanged();
}
double ChartScene3D::farPlane() const
{
    return d->farPlane;
}
void ChartScene3D::setFarPlane(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value <= d->nearPlane ||
        value > 10000)
        return;
    if (d->farPlane == value)
        return;
    d->farPlane = value;
    emit cameraChanged();
}
ChartScene3D::LightingModel ChartScene3D::lightingModel() const
{
    return d->lightingModel;
}
void ChartScene3D::setLightingModel(LightingModel value)
{
    if (QThread::currentThread() != thread() || value < Phong || value > BlinnPhong ||
        d->lightingModel == value)
        return;
    d->lightingModel = value;
    emit materialChanged();
}
double ChartScene3D::ambientGroundRatio() const
{
    return d->ambientGroundRatio;
}
void ChartScene3D::setAmbientGroundRatio(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 1 ||
        d->ambientGroundRatio == value)
        return;
    d->ambientGroundRatio = value;
    emit materialChanged();
}
double ChartScene3D::exposure() const
{
    return d->exposure;
}
void ChartScene3D::setExposure(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 8 ||
        d->exposure == value)
        return;
    d->exposure = value;
    emit materialChanged();
}
double ChartScene3D::ambient() const
{
    return d->ambient;
}
void ChartScene3D::setAmbient(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 2)
        return;
    if (d->ambient == value)
        return;
    d->ambient = value;
    emit materialChanged();
}
double ChartScene3D::diffuse() const
{
    return d->diffuse;
}
void ChartScene3D::setDiffuse(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 2)
        return;
    if (d->diffuse == value)
        return;
    d->diffuse = value;
    emit materialChanged();
}
double ChartScene3D::specular() const
{
    return d->specular;
}
void ChartScene3D::setSpecular(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 2)
        return;
    if (d->specular == value)
        return;
    d->specular = value;
    emit materialChanged();
}
double ChartScene3D::shininess() const
{
    return d->shininess;
}
void ChartScene3D::setShininess(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 1 || value > 256)
        return;
    if (d->shininess == value)
        return;
    d->shininess = value;
    emit materialChanged();
}
QVector3D ChartScene3D::lightDirection() const
{
    return d->lightDirection;
}
void ChartScene3D::setLightDirection(QVector3D value)
{
    if (QThread::currentThread() != thread() || !finite(value) || value.lengthSquared() < 1e-8)
        return;
    value /= std::max({std::abs(value.x()), std::abs(value.y()), std::abs(value.z())});
    value.normalize();
    if (d->lightDirection == value)
        return;
    d->lightDirection = value;
    emit materialChanged();
}
bool ChartScene3D::gridVisible() const
{
    return d->gridVisible;
}
void ChartScene3D::setGridVisible(bool value)
{
    if (QThread::currentThread() != thread())
        return;
    if (d->gridVisible == value)
        return;
    d->gridVisible = value;
    emit changed();
}
ChartScene3D::SelectionMode ChartScene3D::selectionMode() const
{
    return d->selectionMode;
}
void ChartScene3D::setSelectionMode(SelectionMode value)
{
    if (QThread::currentThread() != thread() || value < NoSelection || value > ColumnSelection)
        return;
    if (d->selectionMode == value)
        return;
    d->selectionMode = value;
    emit changed();
}
ChartScene3D::SliceMode ChartScene3D::sliceMode() const
{
    return d->sliceMode;
}
void ChartScene3D::setSliceMode(SliceMode value)
{
    if (QThread::currentThread() != thread() || value < NoSlice || value > ColumnSlice)
        return;
    if (d->sliceMode == value)
        return;
    d->sliceMode = value;
    emit changed();
}
int ChartScene3D::sliceIndex() const
{
    return d->sliceIndex;
}
void ChartScene3D::setSliceIndex(int value)
{
    if (QThread::currentThread() != thread() || value < 0 || value > 1000000)
        return;
    if (d->sliceIndex == value)
        return;
    d->sliceIndex = value;
    emit changed();
}

} // namespace QCatUiKit
