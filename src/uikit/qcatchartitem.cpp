// SPDX-License-Identifier: MIT
#include "qcatchart3d_p.h"
#include "qcatchartcurves_p.h"
#include "qcatchartdelegates_p.h"
#include "qcatchartlabels_p.h"
#include "qcatchartlod_p.h"
#include "qcatchartmath_p.h"
#include "qcatchartpicking_p.h"
#include "qcatchartrhiitem_p.h"
#include "qcatchartstatistics_p.h"
#include <QCatUiKit/qcatchartitem.h>
#include <QElapsedTimer>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMap>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QSGClipNode>
#include <QSGGeometryNode>
#include <QSGRendererInterface>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QSGTransformNode>
#include <QSGVertexColorMaterial>
#include <QScopedValueRollback>
#include <QSet>
#include <QStyleHints>
#include <QTouchEvent>
#include <QVarLengthArray>
#include <QVariantAnimation>
#include <QWheelEvent>
#include <QtGui/private/qtriangulator_p.h>
#include <limits>

namespace QCatUiKit
{
using namespace Detail;
namespace
{
constexpr double pi = 3.14159265358979323846;
struct Batch
{
    QColor color;
    QVector<QPointF> vertices;
    bool clip = true;
    QVector<qreal> coverage;
    QColor endColor;
    QPointF gradientStart, gradientEnd;
    QGradientStops stops;
    QImage texture;
    QRectF textureRect;
    bool textureRepeats = false;
    QVector<ChartInstance> instances;
    QVector<QPair<int, int>> delegateInstances;
    QImage softwareTexture;
    quint64 textureId = 0;
    bool scalarTexture = false;
    QPointF textureScale{1,1};
};
// Interpolate premultiplied colors, including transparent endpoints, in plot coordinates.
QRgb batchPixel(const Batch &batch, int index)
{
    const qreal coverage = batch.coverage.value(index, 1);
    qreal t = 0;
    if (batch.endColor.isValid())
    {
        const auto delta = batch.gradientEnd - batch.gradientStart;
        t = std::clamp(QPointF::dotProduct(batch.vertices[index] - batch.gradientStart, delta) /
                           QPointF::dotProduct(delta, delta),
                       0., 1.);
    }
    QColor start = batch.color, end = batch.endColor.isValid() ? batch.endColor : batch.color;
    if (!batch.stops.isEmpty())
    {
        const auto it = std::upper_bound(batch.stops.cbegin(), batch.stops.cend(), t,
                                         [](double value, const QGradientStop &stop)
                                         { return value < stop.first; });
        if (it == batch.stops.cbegin())
            start = end = it->second;
        else if (it == batch.stops.cend())
            start = end = batch.stops.last().second;
        else
        {
            const auto &before = *(it - 1);
            start = before.second;
            end = it->second;
            t = (t - before.first) / (it->first - before.first);
        }
    }
    const qreal a = start.alphaF() * (1 - t) * coverage, b = end.alphaF() * t * coverage;
    return qRgba(qRound(start.red() * a + end.red() * b),
                 qRound(start.green() * a + end.green() * b),
                 qRound(start.blue() * a + end.blue() * b), qRound(255 * (a + b)));
}
struct Text
{
    QString text;
    QRectF rect;
    QColor color;
    int pixels = 12;
    qreal rotation = 0;
    int delegateId = -1;
};
struct Legend
{
    QRectF rect;
    QPointer<ChartSeries> series;
    int slice = -1;
};
struct Hit
{
    QRectF rect;
    QPointer<ChartSeries> series;
    int index = -1;
    int shape = -1;
    double percentage = std::numeric_limits<double>::quiet_NaN();
    QPolygonF polygon;
};
struct CurveHit
{
    QPointF a, b;
    QPointer<ChartSeries> series;
    int first = -1, second = -1;
    double approximation = 0;
};
struct BandHit
{
    QPointF upperA, upperB, lowerA, lowerB;
    QPointer<ChartSeries> series;
    int first, second;
    double from = 0, to = 1;
    double approximation = 0;
};
struct SliceHit
{
    QPointer<ChartSeries> series;
    int index, ring;
    QPointF center;
    double inner, outer, start, end, percentage;
    bool contains(QPointF point) const
    {
        const auto delta = point - center;
        const double radius = std::hypot(delta.x(), delta.y());
        if (radius < inner || radius > outer)
            return false;
        double angle = std::fmod(std::atan2(delta.y(), delta.x()) - start, 2 * pi);
        if (angle < 0)
            angle += 2 * pi;
        return angle < end - start;
    }
};
struct BarValue
{
    int index;
    double category, width, bottom, top;
    double percentage = std::numeric_limits<double>::quiet_NaN();
    QColor color;
    bool connectorBefore = false;
    double connectorLevel = std::numeric_limits<double>::quiet_NaN();
};
QColor contrastingText(const QColor &color)
{
    const auto linear = [](qreal value)
    { return value <= .04045 ? value / 12.92 : std::pow((value + .055) / 1.055, 2.4); };
    const qreal luminance = .2126 * linear(color.redF()) + .7152 * linear(color.greenF()) +
                            .0722 * linear(color.blueF());
    return luminance > .179 ? QColor(Qt::black) : QColor(Qt::white);
}
QColor heatColor(double f)
{
    return QColor::fromHsvF((1 - std::clamp(f, 0., 1.)) * 0.7, 0.8, 0.95);
}
void triangle(Batch &b, QPointF a, QPointF c, QPointF e, qreal ca = 1, qreal cc = 1, qreal ce = 1)
{
    b.vertices << a << c << e;
    b.coverage << ca << cc << ce;
}
// Clip before assigning gradient colors: offscreen vertices must not flatten the ramp.
void fillTriangle(Batch &batch, const QRectF &clip, QPointF a, QPointF b, QPointF c,
                  bool splitGradient = true)
{
    if (splitGradient && !batch.stops.isEmpty())
    {
        const bool vertical = batch.gradientStart.x() == batch.gradientEnd.x();
        const double start = vertical ? batch.gradientStart.y() : batch.gradientStart.x();
        const double length =
            vertical ? batch.gradientEnd.y() - start : batch.gradientEnd.x() - start;
        const double low =
            vertical ? std::min({a.y(), b.y(), c.y()}) : std::min({a.x(), b.x(), c.x()});
        const double high =
            vertical ? std::max({a.y(), b.y(), c.y()}) : std::max({a.x(), b.x(), c.x()});
        double previous = 0;
        for (int i = 0; i <= batch.stops.size(); ++i)
        {
            const double next = i == batch.stops.size() ? 1 : batch.stops[i].first;
            const double from = start + previous * length, to = start + next * length;
            if (next > previous && high >= from && low <= to)
            {
                QRectF slab = clip;
                if (vertical)
                {
                    slab.setTop(std::max(clip.top(), from));
                    slab.setBottom(std::min(clip.bottom(), to));
                }
                else
                {
                    slab.setLeft(std::max(clip.left(), from));
                    slab.setRight(std::min(clip.right(), to));
                }
                if (slab.width() > 0 && slab.height() > 0)
                    fillTriangle(batch, slab, a, b, c, false);
            }
            previous = next;
        }
        return;
    }
    const auto append = [&](QPointF p, QPointF q, QPointF r)
    {
        const double cross = (q.x() - p.x()) * (r.y() - p.y()) - (q.y() - p.y()) * (r.x() - p.x());
        if (cross > 0)
            triangle(batch, p, q, r);
        else if (cross < 0)
            triangle(batch, p, r, q);
    };
    if (clip.contains(a) && clip.contains(b) && clip.contains(c))
    {
        append(a, b, c);
        return;
    }
    QVarLengthArray<QPointF, 8> polygon{a, b, c};
    for (int edge = 0; edge < 4 && !polygon.isEmpty(); ++edge)
    {
        QVarLengthArray<QPointF, 8> output;
        const auto distance = [&](QPointF p)
        {
            if (edge == 0)
                return p.x() - clip.left();
            if (edge == 1)
                return clip.right() - p.x();
            if (edge == 2)
                return p.y() - clip.top();
            return clip.bottom() - p.y();
        };
        auto previous = polygon.last();
        double previousDistance = distance(previous);
        for (auto point : polygon)
        {
            const double d = distance(point);
            if ((d >= 0) != (previousDistance >= 0))
                output.append(
                    interpolate(previous, point, previousDistance / (previousDistance - d)));
            if (d >= 0)
                output.append(point);
            previous = point;
            previousDistance = d;
        }
        polygon = output;
    }
    for (int i = 2; i < polygon.size(); ++i)
        append(polygon[0], polygon[i - 1], polygon[i]);
}
void fillBand(Batch &fill, const QRectF &clip, QPointF a, QPointF b, QPointF c, QPointF d)
{
    const double first = a.y() - c.y(), second = b.y() - d.y();
    if ((first < 0 && second > 0) || (first > 0 && second < 0))
    {
        const auto crossing = interpolate(a, b, first / (first - second));
        fillTriangle(fill, clip, a, c, crossing);
        fillTriangle(fill, clip, b, d, crossing);
    }
    else
    {
        fillTriangle(fill, clip, a, c, b);
        fillTriangle(fill, clip, b, c, d);
    }
}
void rectangle(Batch &b, QRectF r)
{
    r = r.normalized();
    triangle(b, r.topLeft(), r.bottomLeft(), r.topRight());
    triangle(b, r.topRight(), r.bottomLeft(), r.bottomRight());
}
// Coverage fringes keep curves smooth even in a single-sample render target.
void featherEdge(Batch &b, QPointF a, QPointF c, QPointF offset)
{
    triangle(b, a, c, a + offset, 1, 1, 0);
    triangle(b, c, c + offset, a + offset, 1, 0, 0);
}
void segment(Batch &b, QPointF a, QPointF c, double width, qreal feather = 0, bool squareCap = true)
{
    const auto delta = c - a;
    const double len = std::hypot(delta.x(), delta.y());
    if (len < 1e-9)
        return;
    const QPointF tangent = delta / len, normal(-tangent.y(), tangent.x());
    const QPointF n = normal * std::max(0., (width - feather) / 2);
    const QPointF cap = squareCap ? tangent * width / 2 : QPointF();
    a -= cap;
    c += cap;
    triangle(b, a + n, a - n, c + n);
    triangle(b, c + n, a - n, c - n);
    if (feather > 0)
    {
        featherEdge(b, a + n, c + n, normal * feather);
        featherEdge(b, c - n, a - n, -normal * feather);
        featherEdge(b, a - n, a + n, -tangent * feather / 2);
        featherEdge(b, c + n, c - n, tangent * feather / 2);
    }
}
// Dash distances are logical screen pixels; clipped-away portions still advance phase.
struct Stroke
{
    ChartSeries *series;
    QRectF clip;
    qreal feather;
    QList<qreal> pattern;
    double phase = 0, period = 0, pathPhase = 0;
    QPainterPath path;
    Batch *pending = nullptr;
    bool styled() const
    {
        return series->lineCap() != ChartSeries::AutoCap ||
               series->lineJoin() != ChartSeries::AutoJoin;
    }
    void finish()
    {
        if (!pending || path.elementCount() < 2)
        {
            path = {};
            pending = nullptr;
            return;
        }
        QPainterPathStroker stroker;
        const double edgeFeather = std::min(double(feather), double(series->lineWidth()));
        const double width = std::max(.01, double(series->lineWidth()) - edgeFeather);
        stroker.setWidth(width);
        const auto cap = series->lineCap();
        stroker.setCapStyle(
            cap == ChartSeries::RoundCap ? Qt::RoundCap
            : cap == ChartSeries::FlatCap ||
                    (cap == ChartSeries::AutoCap && series->strokeStyle() == ChartSeries::DashLine)
                ? Qt::FlatCap
                : Qt::SquareCap);
        stroker.setJoinStyle(series->lineJoin() == ChartSeries::RoundJoin   ? Qt::RoundJoin
                             : series->lineJoin() == ChartSeries::MiterJoin ? Qt::MiterJoin
                                                                            : Qt::BevelJoin);
        stroker.setMiterLimit(series->miterLimit());
        if (series->strokeStyle() == ChartSeries::DashLine)
        {
            auto dashes = pattern;
            for (auto &length : dashes)
                length /= width;
            stroker.setDashPattern(dashes);
            stroker.setDashOffset(pathPhase / width);
        }
        const auto outline = stroker.createStroke(path);
        const auto mesh =
            qTriangulate(outline, QTransform(), feather > 0 ? std::max(1., 1. / feather) : 1.);
        const auto index = [&](int i)
        {
            return mesh.indices.type() == QVertexIndexVector::UnsignedInt
                       ? static_cast<const quint32 *>(mesh.indices.data())[i]
                       : quint32(static_cast<const quint16 *>(mesh.indices.data())[i]);
        };
        const auto point = [&](int i)
        {
            const int n = int(index(i)) * 2;
            return QPointF(mesh.vertices[n], mesh.vertices[n + 1]);
        };
        for (int i = 0; i + 2 < mesh.indices.size(); i += 3)
            triangle(*pending, point(i), point(i + 1), point(i + 2));
        if (feather > 0)
        {
            const double scale = std::max(1., 1. / feather);
            const auto contours = outline.toSubpathPolygons(QTransform::fromScale(scale, scale));
            double largest = 0;
            for (const auto &polygon : contours)
            {
                double area = 0;
                for (int i = 1; i < polygon.size(); ++i)
                    area +=
                        polygon[i - 1].x() * polygon[i].y() - polygon[i].x() * polygon[i - 1].y();
                if (std::abs(area) > std::abs(largest))
                    largest = area;
            }
            const double orientation = largest < 0 ? -1 : 1;
            for (const auto &polygon : contours)
                for (int i = 1; i < polygon.size(); ++i)
                {
                    const auto a = polygon[i - 1] / scale, b = polygon[i] / scale, delta = b - a;
                    const double length = std::hypot(delta.x(), delta.y());
                    if (length > 1e-9)
                        featherEdge(*pending, a, b,
                                    QPointF(delta.y(), -delta.x()) *
                                        (orientation * edgeFeather / length));
                }
        }
        path = {};
        pending = nullptr;
    }
    Stroke(ChartSeries *s, QRectF r, qreal aa)
        : series(s),
          clip(r.adjusted(-s->lineWidth() * s->miterLimit(), -s->lineWidth() * s->miterLimit(),
                          s->lineWidth() * s->miterLimit(), s->lineWidth() * s->miterLimit())),
          feather(aa), pattern(s->dashPattern())
    {
        for (auto n : pattern)
            period += n;
        reset();
    }
    void reset()
    {
        finish();
        phase = std::fmod(series->dashOffset(), period);
        if (phase < 0)
            phase += period;
    }
    void draw(Batch &batch, QPointF a, QPointF b)
    {
        if (!styled() && series->strokeStyle() == ChartSeries::SolidLine)
        {
            segment(batch, a, b, series->lineWidth(), feather);
            return;
        }
        const auto delta = b - a;
        const double length = std::hypot(delta.x(), delta.y());
        if (!std::isfinite(length) || length < 1e-9)
            return;
        const double initial = phase;
        phase = std::fmod(phase + std::fmod(length, period), period);
        double enter = 0, leave = 1;
        const auto edge = [&](double p, double q)
        {
            if (p == 0)
                return q >= 0;
            const double t = q / p;
            if (p < 0)
                enter = std::max(enter, t);
            else
                leave = std::min(leave, t);
            return enter <= leave;
        };
        if (!edge(-delta.x(), a.x() - clip.left()) || !edge(delta.x(), clip.right() - a.x()) ||
            !edge(-delta.y(), a.y() - clip.top()) || !edge(delta.y(), clip.bottom() - a.y()))
        {
            if (styled())
                finish();
            return;
        }
        if (styled())
        {
            const auto first = interpolate(a, b, enter), last = interpolate(a, b, leave);
            if (pending != &batch || path.isEmpty() ||
                QLineF(path.currentPosition(), first).length() > 1e-6)
            {
                finish();
                pending = &batch;
                path.moveTo(first);
                pathPhase = std::fmod(initial + std::fmod(enter * length, period), period);
            }
            path.lineTo(last);
            if (leave < 1)
                finish();
            return;
        }
        double cursor = enter * length, end = leave * length;
        double local = std::fmod(initial + std::fmod(cursor, period), period);
        int part = 0;
        while (part < pattern.size() - 1 && local >= pattern[part])
            local -= pattern[part++];
        while (cursor < end)
        {
            const double next = std::min(end, cursor + pattern[part] - local);
            if (next <= cursor)
                break;
            if (part % 2 == 0)
                segment(batch, interpolate(a, b, cursor / length), interpolate(a, b, next / length),
                        series->lineWidth(), feather, false);
            cursor = next;
            local = 0;
            part = (part + 1) % pattern.size();
        }
    }
};
void sector(Batch &b, QPointF center, double inner, double outer, double a, double z,
            qreal feather = 0)
{
    const int n =
        std::clamp(int(std::ceil((z - a) * outer / 3)), z - a >= 2 * pi - 1e-6 ? 12 : 1, 512);
    auto p = [&](double t, double r) { return center + QPointF(std::cos(t) * r, std::sin(t) * r); };
    const double solidOuter = std::max(0., outer - feather / 2);
    const double solidInner = inner > 0 ? inner + feather / 2 : 0;
    for (int i = 0; i < n; ++i)
    {
        const double t = a + (z - a) * i / n, u = a + (z - a) * (i + 1) / n;
        triangle(b, p(t, solidInner), p(t, solidOuter), p(u, solidOuter));
        if (inner > 0)
            triangle(b, p(t, solidInner), p(u, solidOuter), p(u, solidInner));
        if (feather > 0)
        {
            triangle(b, p(t, solidOuter), p(t, outer + feather / 2), p(u, solidOuter), 1, 0, 1);
            triangle(b, p(u, solidOuter), p(t, outer + feather / 2), p(u, outer + feather / 2), 1,
                     0, 0);
            if (inner > 0)
            {
                triangle(b, p(t, solidInner), p(u, solidInner), p(t, inner - feather / 2), 1, 1, 0);
                triangle(b, p(u, solidInner), p(u, inner - feather / 2), p(t, inner - feather / 2),
                         1, 0, 0);
            }
        }
    }
}
void instance(Batch &batch, const QRectF &rect, int shape, qreal feather = 0)
{
    if (rect.width() <= 0 || rect.height() <= 0)
        return;
    const auto color = qPremultiply(batch.color.rgba());
    batch.instances.append({float(rect.x()), float(rect.y()), float(rect.width()),
                            float(rect.height()), quint8(qRed(color)), quint8(qGreen(color)),
                            quint8(qBlue(color)), quint8(qAlpha(color)), float(shape),
                            float(feather)});
}
void marker(Batch &batch, QPointF center, double radius, ChartSeries::MarkerShape shape,
            qreal feather)
{
    instance(batch, {center - QPointF(radius, radius), QSizeF(radius * 2, radius * 2)}, int(shape),
             feather);
}
QPainterPath instancePath(const ChartInstance &value)
{
    const QRectF rect(value.x, value.y, value.width, value.height);
    QPainterPath path;
    if (value.shape == ChartSeries::Circle)
        path.addEllipse(rect);
    else if (value.shape == ChartSeries::Square || value.shape >= 4)
        path.addRect(rect);
    else
    {
        path.moveTo(rect.center().x(), rect.top());
        if (value.shape == ChartSeries::Diamond)
        {
            path.lineTo(rect.right(), rect.center().y());
            path.lineTo(rect.center().x(), rect.bottom());
            path.lineTo(rect.left(), rect.center().y());
        }
        else
        {
            path.lineTo(rect.bottomRight());
            path.lineTo(rect.bottomLeft());
        }
        path.closeSubpath();
    }
    return path;
}
bool hitContains(const Hit &hit, QPointF point)
{
    if (!hit.rect.contains(point))
        return false;
    if (!hit.polygon.isEmpty()) return hit.polygon.containsPoint(point, Qt::OddEvenFill);
    if (hit.shape < 0 || hit.shape == ChartSeries::Square ||
        hit.shape == ChartSeries::TextureMarker)
        return true;
    const auto delta = point - hit.rect.center();
    const double x = delta.x() / (hit.rect.width() / 2), y = delta.y() / (hit.rect.height() / 2);
    if (hit.shape == ChartSeries::Circle)
        return x * x + y * y <= 1;
    if (hit.shape == ChartSeries::Diamond)
        return std::abs(x) + std::abs(y) <= 1;
    return y >= -1 && y <= 1 && std::abs(x) <= (y + 1) / 2;
}
QVariantMap pointMap(ChartSeries *s, int seriesIndex, int i)
{
    auto m = s->at(i);
    m.insert("seriesIndex", seriesIndex);
    m.insert("index", i);
    m.insert("seriesName", s->name());
    return m;
}
struct DecorationFrame
{
    QVector<Batch> batches;
    QVector<Text> labels;
    QRectF bounds, plot;
    QColor background;
    QFont font;
    bool software = false, antialiasing = true;
};
// Keep inherited transform/clip/opacity state when the software renderer adds new children.
struct Root : QSGTransformNode
{
    std::shared_ptr<const DecorationFrame> frame;
    qreal dpr = 0;
    QHash<QString, QSGTexture *> textures;
    QSGTexture *softwareTexture = nullptr;
    ~Root() override
    {
        while (firstChild())
            delete firstChild();
        qDeleteAll(textures);
        delete softwareTexture;
    }
};
void addBatch(QSGNode *parent, const Batch &b)
{
    if (b.vertices.isEmpty())
        return;
    auto *node = new QSGGeometryNode;
    auto *g =
        new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), int(b.vertices.size()));
    g->setDrawingMode(QSGGeometry::DrawTriangles);
    auto *v = g->vertexDataAsColoredPoint2D();
    for (int i = 0; i < b.vertices.size(); ++i)
    {
        const auto color = batchPixel(b, i);
        v[i].set(float(b.vertices[i].x()), float(b.vertices[i].y()), qRed(color), qGreen(color),
                 qBlue(color), qAlpha(color));
    }
    node->setGeometry(g);
    node->setFlag(QSGNode::OwnsGeometry);
    auto *material = new QSGVertexColorMaterial;
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    parent->appendChildNode(node);
}
void addText(Root *root, QQuickWindow *window, const Text &t, const QFont &baseFont,
             QSet<QString> &used)
{
    if (t.text.isEmpty() || t.rect.width() < 1 || t.rect.height() < 1)
        return;
    QFont font = baseFont;
    font.setPixelSize(std::max(9, baseFont.pixelSize() + t.pixels - 12));
    const QFontMetricsF metrics(font);
    const QString text = metrics.elidedText(t.text, Qt::ElideRight, t.rect.width() - 2);
    const qreal dpr = window->effectiveDevicePixelRatio();
    const QSize logical(std::max(1, int(std::ceil(metrics.horizontalAdvance(text))) + 4),
                        font.pixelSize() + 8);
    const QString key = text + QLatin1Char('|') + t.color.name(QColor::HexArgb) + font.toString() +
                        QLatin1Char('|') + QString::number(dpr);
    used.insert(key);
    auto *texture = root->textures.value(key);
    if (!texture)
    {
        QImage tile(
            QSize(int(std::ceil(logical.width() * dpr)), int(std::ceil(logical.height() * dpr))),
            QImage::Format_ARGB32_Premultiplied);
        tile.setDevicePixelRatio(dpr);
        tile.fill(Qt::transparent);
        // Text only; chart geometry is submitted separately by the data/grid layers.
        QPainter painter(&tile);
        painter.setFont(font);
        painter.setPen(t.color);
        painter.drawText(QRectF(QPointF(0, 0), logical), Qt::AlignCenter, text);
        painter.end();
        texture = window->createTextureFromImage(tile);
        if (!texture)
            return;
        root->textures.insert(key, texture);
    }
    auto *node = new QSGSimpleTextureNode;
    node->setTexture(texture);
    node->setFiltering(QSGTexture::Linear);
    node->setRect(
        QRectF(t.rect.center() - QPointF(logical.width() / 2., logical.height() / 2.), logical));
    if (t.rotation != 0)
    {
        auto *transform = new QSGTransformNode;
        QMatrix4x4 matrix;
        matrix.translate(t.rect.center().x(), t.rect.center().y());
        matrix.rotate(t.rotation, 0, 0, 1);
        matrix.translate(-t.rect.center().x(), -t.rect.center().y());
        transform->setMatrix(matrix);
        transform->appendChildNode(node);
        root->appendChildNode(transform);
    }
    else
        root->appendChildNode(node);
}
// The render thread reads immutable drawing commands, never series or axes.
QSGNode *renderDecorations(QSGNode *old, QQuickWindow *window,
                           const std::shared_ptr<const DecorationFrame> &frame)
{
    auto *root = static_cast<Root *>(old);
    if (!window || !frame)
        return root;
    const qreal dpr = window->effectiveDevicePixelRatio();
    if (root && root->frame == frame && root->dpr == dpr)
        return root;
    if (!root)
        root = new Root;
    root->frame = frame;
    root->dpr = dpr;
    while (root->firstChild())
        delete root->firstChild();
    delete root->softwareTexture;
    root->softwareTexture = nullptr;
    if (frame->background.isValid())
        root->appendChildNode(new QSGSimpleRectNode(frame->bounds, frame->background));
    if (frame->software && !frame->batches.isEmpty())
    {
        const qreal dpr = window->effectiveDevicePixelRatio();
        QImage image(QSize(std::max(1, qCeil(frame->bounds.width() * dpr)),
                           std::max(1, qCeil(frame->bounds.height() * dpr))),
                     QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, frame->antialiasing);
        for (const auto &batch : frame->batches)
        {
            const QImage &materialTexture = batch.softwareTexture.isNull() ? batch.texture : batch.softwareTexture;
            painter.save();
            if (batch.clip)
                painter.setClipRect(frame->plot);
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            for (int i = 0; i + 2 < batch.vertices.size(); i += 3)
            {
                path.moveTo(batch.vertices[i]);
                path.lineTo(batch.vertices[i + 1]);
                path.lineTo(batch.vertices[i + 2]);
                path.closeSubpath();
            }
            if (!batch.texture.isNull() && !batch.instances.isEmpty())
            {
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                for (const auto &value : batch.instances)
                {
                    painter.setOpacity(value.a / 255.);
                    painter.drawImage(QRectF(value.x, value.y, value.width, value.height),
                                      batch.texture);
                }
                painter.setOpacity(1);
            }
            else if (!batch.endColor.isValid() &&
                     std::any_of(batch.instances.cbegin(), batch.instances.cend(),
                                 [&](const ChartInstance &value) {
                                     return qRgba(value.r, value.g, value.b, value.a) !=
                                            qPremultiply(batch.color.rgba());
                                 }))
            {
                // Instanced bars can carry different semantic colors. Decode the
                // premultiplied instance color before handing it to QPainter.
                for (const auto &value : batch.instances)
                    painter.fillPath(instancePath(value),
                                     QColor::fromRgba(qUnpremultiply(
                                         qRgba(value.r, value.g, value.b, value.a))));
            }
            else
                for (const auto &value : batch.instances)
                    path.addPath(instancePath(value));
            if (path.isEmpty())
            {
                painter.restore();
                continue;
            }
            if (!batch.texture.isNull())
            {
                QBrush brush(materialTexture);
                QTransform transform;
                transform.translate(batch.textureRect.left(), batch.textureRect.top());
                transform.scale(batch.textureRect.width() / materialTexture.width(),
                                batch.textureRect.height() / materialTexture.height());
                brush.setTransform(transform);
                painter.setOpacity(batch.color.alphaF());
                painter.fillPath(path, brush);
            }
            else if (batch.endColor.isValid())
            {
                QLinearGradient gradient(batch.gradientStart, batch.gradientEnd);
                gradient.setColorAt(0, batch.color);
                gradient.setColorAt(1, batch.endColor);
                if (!batch.stops.isEmpty())
                    gradient.setStops(batch.stops);
                painter.fillPath(path, gradient);
            }
            else
                painter.fillPath(path, batch.color);
            painter.restore();
        }
        painter.end();
        root->softwareTexture = window->createTextureFromImage(image);
        if (root->softwareTexture)
        {
            auto *node = new QSGSimpleTextureNode;
            node->setTexture(root->softwareTexture);
            node->setRect(frame->bounds);
            root->appendChildNode(node);
        }
    }
    else
    {
        auto *clip = new QSGClipNode;
        clip->setIsRectangular(true);
        clip->setClipRect(frame->plot);
        root->appendChildNode(clip);
        for (const auto &batch : frame->batches)
            addBatch(batch.clip ? clip : static_cast<QSGNode *>(root), batch);
    }
    QSet<QString> used;
    for (const auto &text : frame->labels)
        addText(root, window, text, frame->font, used);
    for (auto it = root->textures.begin(); it != root->textures.end();)
        if (!used.contains(it.key()))
        {
            delete it.value();
            it = root->textures.erase(it);
        }
        else
            ++it;
    return root;
}
class ChartOverlay final : public QQuickItem
{
  public:
    explicit ChartOverlay(QQuickItem *parent) : QQuickItem(parent)
    {
        setFlag(ItemHasContents);
        setAcceptedMouseButtons(Qt::NoButton);
        setZ(1);
    }
    void setFrame(std::shared_ptr<const DecorationFrame> frame)
    {
        m_frame = std::move(frame);
        update();
    }

  protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override
    {
        return renderDecorations(old, window(), m_frame);
    }

  private:
    std::shared_ptr<const DecorationFrame> m_frame;
};
} // namespace

class ChartItem::Impl
{
  public:
    ChartScene3D *scene3D = nullptr;
    ChartRhiItem3D *layer3D = nullptr;
    bool showing3D = false;
    int touch3DCount = 0;
    QPointF touch3DCenter, touch3DPress;
    bool touch3DMoved = false;
    qreal touch3DDistance = 0;
    ChartDelegates delegates;
    bool animations = false;
    QVariantAnimation *appearanceAnimation = nullptr, *selectionAnimation = nullptr;
    qreal appearanceProgress = 1, selectionProgress = 1;
    int leaderCandidates = 0, leaderCount = 0;
    SelectionMode selectionMode = NoSelection;
    QHash<ChartSeries *, QSet<int>> selected;
    QColor selectionColor;
    qreal selectionWidth = 2;
    QPointer<ChartSeries> hoveredSeries;
    int hoveredIndex = -1;
    ChartPickIndex hitIndex, curveIndex, bandIndex, sliceIndex;
    QHash<QPair<ChartSeries *, int>, int> hitLookup, sliceLookup;
    mutable quint64 pickNodesVisited = 0, pickCandidates = 0;
    void appendSelection(DecorationFrame &frame, const QColor &color) const
    {
        Batch outline{color, {}, true};
        const auto polygon = [&](const QVector<QPointF> &points)
        {
            for (int i = 0; i < points.size(); ++i)
                segment(outline, points[i], points[(i + 1) % points.size()], selectionWidth,
                        smoothing, false);
        };
        const auto mark = [&](QRectF rect, int shape)
        {
            if (!plot.intersects(rect))
                return;
            if (shape < 0)
                rect = rect.intersected(plot);
            rect.adjust(-selectionWidth, -selectionWidth, selectionWidth, selectionWidth);
            const auto c = rect.center();
            if (shape == ChartSeries::Circle)
            {
                const double radius = rect.width() / 2;
                sector(outline, c, std::max(0., radius - selectionWidth / 2),
                       radius + selectionWidth / 2, 0, 2 * pi, smoothing);
            }
            else if (shape == ChartSeries::Diamond)
                polygon({{c.x(), rect.top()},
                         {rect.right(), c.y()},
                         {c.x(), rect.bottom()},
                         {rect.left(), c.y()}});
            else if (shape == ChartSeries::Triangle)
                polygon({{c.x(), rect.top()}, rect.bottomRight(), rect.bottomLeft()});
            else
                polygon({rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft()});
        };
        for (auto series : series)
        {
            if (!series || !series->isVisible())
                continue;
            const auto indices = selected.value(series);
            for (int index : indices)
            {
                if (index < 0 || index >= series->count())
                    continue;
                const QPair<ChartSeries *, int> key{series, index};
                if (radial() && sliceLookup.contains(key))
                {
                    const auto &h = sliceHits[sliceLookup.value(key)];
                    sector(outline, h.center, std::max(h.inner, h.outer - selectionWidth), h.outer,
                           h.start, h.end, smoothing);
                    if (h.inner > 0)
                        sector(outline, h.center, h.inner,
                               std::min(h.outer, h.inner + selectionWidth), h.start, h.end,
                               smoothing);
                    if (h.end - h.start < 2 * pi - 1e-9)
                        for (double angle : {h.start, h.end})
                        {
                            const QPointF v(std::cos(angle), std::sin(angle));
                            segment(outline, h.center + v * h.inner, h.center + v * h.outer,
                                    selectionWidth, smoothing, false);
                        }
                }
                else if (hitLookup.contains(key))
                {
                    const auto &hit = hits[hitLookup.value(key)];
                    if (hit.polygon.isEmpty()) mark(hit.rect, hit.shape);
                    else polygon(hit.polygon);
                }
                else if (!radial() && (seriesType(series) == ChartSeries::Line ||
                                       seriesType(series) == ChartSeries::Area ||
                                       seriesType(series) == ChartSeries::RangeArea))
                {
                    auto &self = *const_cast<Impl *>(this);
                    AxisContext context(self, series);
                    const auto p = series->sampleAt(index);
                    if (!valid(p.x, p.y))
                        continue;
                    const double radius = std::max(5., series->markerSize() / 2);
                    const auto at = map(p.x, p.y);
                    mark(QRectF(at - QPointF(radius, radius), QSizeF(2 * radius, 2 * radius)),
                         ChartSeries::Circle);
                    if (seriesType(series) == ChartSeries::RangeArea &&
                        valid(p.x, series->lowerValue(index)))
                    {
                        const auto low = map(p.x, series->lowerValue(index));
                        mark(QRectF(low - QPointF(radius, radius), QSizeF(2 * radius, 2 * radius)),
                             ChartSeries::Circle);
                        if (at.x() >= plot.left() && at.x() <= plot.right())
                            segment(outline,
                                    {at.x(), std::clamp(at.y(), plot.top(), plot.bottom())},
                                    {low.x(), std::clamp(low.y(), plot.top(), plot.bottom())},
                                    selectionWidth, smoothing, false);
                    }
                }
            }
        }
        if (!outline.vertices.isEmpty())
            frame.batches.append(std::move(outline));
    }
    void buildPickIndices()
    {
        for (auto index : {&hitIndex, &curveIndex, &bandIndex, &sliceIndex})
            index->reset(plot.adjusted(-18, -18, 18, 18));
        hitLookup.clear();
        sliceLookup.clear();
        for (int i = 0; i < hits.size(); ++i)
        {
            hitIndex.add(hits[i].rect, i);
            hitLookup.insert({hits[i].series, hits[i].index}, i);
        }
        for (int i = 0; i < curveHits.size(); ++i)
        {
            const auto &h = curveHits[i];
            const double margin = 18 + h.approximation;
            curveIndex.add(QRectF(h.a, h.b).normalized().adjusted(-margin, -margin, margin, margin),
                           i);
        }
        for (int i = 0; i < bandHits.size(); ++i)
        {
            const auto &h = bandHits[i];
            const double margin = 6 + h.approximation;
            const double left = std::min({h.upperA.x(), h.upperB.x(), h.lowerA.x(), h.lowerB.x()});
            const double right = std::max({h.upperA.x(), h.upperB.x(), h.lowerA.x(), h.lowerB.x()});
            const double top = std::min({h.upperA.y(), h.upperB.y(), h.lowerA.y(), h.lowerB.y()});
            const double bottom =
                std::max({h.upperA.y(), h.upperB.y(), h.lowerA.y(), h.lowerB.y()});
            bandIndex.add(QRectF(QPointF(left, top), QPointF(right, bottom))
                              .adjusted(-margin, -margin, margin, margin),
                          i);
        }
        for (int i = 0; i < sliceHits.size(); ++i)
        {
            const auto &h = sliceHits[i];
            sliceIndex.add(
                QRectF(h.center - QPointF(h.outer, h.outer), QSizeF(2 * h.outer, 2 * h.outer)), i);
            sliceLookup.insert({h.series, h.index}, i);
        }
        for (auto index : {&hitIndex, &curveIndex, &bandIndex, &sliceIndex})
            index->finish();
    }
    QVector<int> pick(const ChartPickIndex &index, QPointF point) const
    {
        auto result = index.query(point, pickNodesVisited);
        pickCandidates += result.size();
        return result;
    }
    bool preparing = false;
    ChartAxis *x = nullptr, *y = nullptr, *defaultX = nullptr, *defaultY = nullptr;
    struct AxisLayout
    {
        QPointer<ChartAxis> axis;
        bool horizontal = false;
        double extent = 0, offset = 0;
    };
    QVector<AxisLayout> axisLayouts;
    QHash<ChartAxis *, Range> axisRanges;
    QHash<ChartAxis *, QList<QMetaObject::Connection>> axisConnections;
    ChartAxis *effectiveAxis(ChartSeries *s, bool horizontal) const
    {
        auto axis = s ? (horizontal ? s->xAxis() : s->yAxis()) : nullptr;
        auto fallback = horizontal ? defaultX : defaultY;
        auto opposite = horizontal ? defaultY : defaultX;
        return axis && axis != opposite && axis->thread() == fallback->thread() &&
                       (axis == fallback ||
                        axis->orientation() == (horizontal ? Qt::Horizontal : Qt::Vertical))
                   ? axis
                   : fallback;
    }
    struct AxisContext
    {
        QScopedValueRollback<ChartAxis *> x, y;
        QScopedValueRollback<Range> xr, yr;
        AxisContext(Impl &d, ChartSeries *s)
            : x(d.x, d.effectiveAxis(s, true)), y(d.y, d.effectiveAxis(s, false)),
              xr(d.xr, d.axisRanges.value(d.x)), yr(d.yr, d.axisRanges.value(d.y))
        {
        }
    };
    void syncAxes(const ChartItem *owner)
    {
        auto *q = const_cast<ChartItem *>(owner);
        axisLayouts = {{defaultX, true}, {defaultY, false}};
        QSet<ChartAxis *> active{defaultX, defaultY}, watched;
        if (!radial())
            for (auto series : series)
                if (series)
                {
                    for (auto requested : {series->xAxis(), series->yAxis()})
                        if (requested && requested->thread() == q->thread())
                            watched.insert(requested);
                    auto ax = effectiveAxis(series, true), ay = effectiveAxis(series, false);
                    for (auto axis : {ax, ay})
                        if (!active.contains(axis))
                        {
                            active.insert(axis);
                            axisLayouts.append({axis, axis == ax});
                        }
                }
        for (auto it = axisConnections.begin(); it != axisConnections.end();)
            if (!watched.contains(it.key()))
            {
                for (auto connection : it.value())
                    QObject::disconnect(connection);
                it = axisConnections.erase(it);
            }
            else
                ++it;
        for (auto axis : watched)
        {
            if (axis == defaultX || axis == defaultY || axisConnections.contains(axis))
                continue;
            axisConnections.insert(axis, {QObject::connect(axis, &ChartAxis::changed, q,
                                                           [q]
                                                           {
                                                               q->cancelZoomSelection();
                                                               if (!q->d->applyingTouch)
                                                                   q->cancelTouch();
                                                               q->invalidate();
                                                               emit q->viewChanged();
                                                           }),
                                          QObject::connect(axis, &QObject::destroyed, q,
                                                           [q, axis]
                                                           {
                                                               q->d->axisConnections.remove(axis);
                                                               q->invalidate();
                                                           })});
        }
    }
    QList<QPointer<ChartSeries>> series;
    QHash<ChartSeries *, QList<QMetaObject::Connection>> connections;
    ChartSeries::Type type = ChartSeries::Line;
    QVariantList radarRanges;
    QString title;
    bool legend = true, grid = true, tooltips = true, crosshair = true, interactive = true,
         dirty = true;
    ThemePalette fallback;
    QPointer<ThemePalette> theme;
    QList<QMetaObject::Connection> themeConnections;
    bool customTheme = false, customFont = false;
    QFont fontOverride, textFont;
    QColor backgroundOverride, textOverride;
    QColor background, text, gridColor, borderColor, disabledColor, tooltipColor;
    QVariantMap tokens;
    QColor palette(int i) const
    {
        const auto primary = tokens.value("primary").value<QColor>();
        const auto rotated = [&](qreal offset)
        {
            return QColor::fromHslF(std::fmod(std::max<qreal>(0., primary.hslHueF()) + offset, 1.),
                                    std::max<qreal>(.5, primary.hslSaturationF()),
                                    primary.lightnessF());
        };
        const QColor colors[] = {primary,
                                 tokens.value("success").value<QColor>(),
                                 tokens.value("warning").value<QColor>(),
                                 tokens.value("danger").value<QColor>(),
                                 rotated(.15),
                                 rotated(.3),
                                 rotated(.5),
                                 rotated(.75)};
        return colors[i % 8];
    }
    QColor seriesColor(ChartSeries *s) const
    {
        return s->color().isValid() ? s->color() : palette(std::max(0, int(series.indexOf(s))));
    }
    int budget = 2048, rendered = 0;
    qreal cellW = 1, cellH = 1, smoothing = 0, curveError = .35;
    qreal barGroupWidth = .8, barSpacing = 0;
    qreal pieStart = 0, pieEnd = 360, pieRadius = .84, hole = .58, ringGap = 4;
    bool multiRing = false, sliceLabels = true;
    SliceLabelPosition sliceLabelPosition = SliceLabelInside;
    Range xr, yr;
    QRectF plot;
    QVector<Batch> batches;
    QVector<Text> labels;
    QVector<Legend> legends;
    QVector<Hit> hits;
    QVector<CurveHit> curveHits;
    QVector<BandHit> bandHits;
    QVector<SliceHit> sliceHits;
    QHash<ChartSeries *, QVector<BarValue>> barValues;
    QHash<ChartSeries *, QVector<ChartCubic>> naturalCurves, naturalLowerCurves;
    struct CurveLayout
    {
        ChartSeries::Type type = ChartSeries::Inherit;
        ChartSeries::CurveMode mode = ChartSeries::LinearCurve;
        ChartAxis::Scale xScale = ChartAxis::Linear, yScale = ChartAxis::Linear;
        double tension = -1;
        ChartCurveLod lod, lowerLod;
        bool indexed = false;
    };
    QHash<ChartSeries *, CurveLayout> curveLayouts;
    quint64 curveCacheBuilds = 0;
    int curveLodNodesVisited = 0;
    void discardCurve(ChartSeries *s)
    {
        curveLayouts.remove(s);
        naturalCurves.remove(s);
        naturalLowerCurves.remove(s);
    }
    bool useCurveLod(ChartSeries *s) const
    {
        // Source-anchored nodes keep the same representatives during a pan.
        // Viewport-relative buckets replace interior markers as either edge moves.
        return budget > 0 && s->count() > budget && s->sortedX() &&
               (seriesType(s) == ChartSeries::RangeArea ||
                seriesType(s) == ChartSeries::Line || seriesType(s) == ChartSeries::Area);
    }
    static bool sameRanges(const QHash<ChartAxis *, Range> &a, const QHash<ChartAxis *, Range> &b)
    {
        if (a.size() != b.size())
            return false;
        for (auto it = a.cbegin(); it != a.cend(); ++it)
        {
            auto other = b.constFind(it.key());
            if (other == b.cend() || it->min != other->min || it->max != other->max)
                return false;
        }
        return true;
    }
    bool transformView(QPointF oldFraction, QPointF newFraction, QPointF scale,
                       bool horizontal = true, bool vertical = true)
    {
        if (!finite(oldFraction) || !finite(newFraction) || !finite(scale) || scale.x() <= 0 ||
            scale.y() <= 0)
            return false;
        struct Change
        {
            QPointer<ChartAxis> axis;
            Range range;
        };
        QVector<Change> changes;
        for (const auto &entry : axisLayouts)
        {
            if (entry.horizontal ? !horizontal : !vertical)
                continue;
            auto axis = entry.axis.data();
            const auto range = axisRanges.value(axis);
            double before = entry.horizontal ? oldFraction.x() : oldFraction.y();
            double after = entry.horizontal ? newFraction.x() : newFraction.y();
            const double factor = entry.horizontal ? scale.x() : scale.y();
            if (axis->reversed())
            {
                before = 1 - before;
                after = 1 - after;
            }
            const Range next{valueAt(before - after / factor, range, axis->scale()),
                             valueAt(before + (1 - after) / factor, range, axis->scale())};
            if (!std::isfinite(next.min) || !std::isfinite(next.max) || next.min >= next.max ||
                !std::isfinite(next.max - next.min) ||
                (axis->scale() == ChartAxis::Logarithmic && next.min <= 0))
                return false;
            changes.append({axis, next});
        }
        for (const auto &change : changes)
            if (change.axis)
                change.axis->setRange(change.range.min, change.range.max);
        for (const auto &change : changes)
            if (change.axis)
                axisRanges.insert(change.axis, change.range);
        xr = axisRanges.value(defaultX);
        yr = axisRanges.value(defaultY);
        if (touching)
        {
            touchRanges = axisRanges;
            touchX = xr;
            touchY = yr;
        }
        return true;
    }
    QHash<ChartSeries *, QSet<int>> hiddenSlices;
    bool sliceVisible(ChartSeries *s, int i) const
    {
        const auto it = hiddenSlices.constFind(s);
        return it == hiddenSlices.cend() || !it->contains(i);
    }
    QColor sliceColor(ChartSeries *s, int i) const
    {
        return s->sliceStyle(i).value("color", palette(i)).value<QColor>();
    }
    QPointF hover{-1, -1}, press, last;
    bool dragging = false, moved = false;
    bool touchNavigation = true, touching = false, touchMoved = false, applyingTouch = false;
    QList<int> touchIds;
    QPointF touchPress, touchCenter;
    qreal touchDistance = 0;
    QRectF touchPlot;
    Range touchX, touchY;
    QHash<ChartAxis *, Range> touchRanges, gestureRanges;
    bool touchMappingChanged() const
    {
        return touching &&
               (!sameRanges(axisRanges, touchRanges) || plot != touchPlot || xr.min != touchX.min ||
                xr.max != touchX.max || yr.min != touchY.min || yr.max != touchY.max);
    }
    DragMode dragMode = PanDrag;
    ZoomAxes zoomAxes = ZoomBothAxes;
    QRectF selection, gesturePlot;
    Range gestureX, gestureY;
    bool selecting() const
    {
        return dragging && dragMode == RectangleZoomDrag && !radial();
    }
    bool selectionMappingChanged() const
    {
        return selecting() && (!sameRanges(axisRanges, gestureRanges) || plot != gesturePlot ||
                               xr.min != gestureX.min || xr.max != gestureX.max ||
                               yr.min != gestureY.min || yr.max != gestureY.max);
    }
    QRectF selectionRect(QPointF start, QPointF end) const
    {
        QRectF rect(start, end);
        rect = rect.normalized();
        if (zoomAxes == ZoomXAxis)
        {
            rect.setTop(plot.top());
            rect.setBottom(plot.bottom());
        }
        else if (zoomAxes == ZoomYAxis)
        {
            rect.setLeft(plot.left());
            rect.setRight(plot.right());
        }
        return rect.intersected(plot);
    }
    qint64 prepareNs = 0;
    quint64 revision = 0, publishedRevision = 0;
    ChartRhiItem *dataLayer = nullptr;
    ChartOverlay *overlay = nullptr, *softwareData = nullptr;
    std::shared_ptr<const DecorationFrame> backgroundFrame;
    std::shared_ptr<ChartRenderStatistics> statistics = std::make_shared<ChartRenderStatistics>();
    QMetaObject::Connection sceneConnection, activeConnection;
    bool software = true;
    bool radial() const
    {
        return type == ChartSeries::Pie || type == ChartSeries::Doughnut ||
               type == ChartSeries::Radar;
    }
    ChartSeries::Type seriesType(ChartSeries *s) const
    {
        auto t = s->type() == ChartSeries::Inherit ? type : s->type();
        return t == ChartSeries::Pie || t == ChartSeries::Doughnut || t == ChartSeries::Radar ? type
                                                                                              : t;
    }
    QPointF mapFractions(double fx, double fy) const
    {
        return {plot.left() + (x->reversed() ? 1 - fx : fx) * plot.width(),
                plot.bottom() - (y->reversed() ? 1 - fy : fy) * plot.height()};
    }
    QPointF valueFractions(QPointF p) const
    {
        const double fx = (p.x() - plot.left()) / plot.width();
        const double fy = (plot.bottom() - p.y()) / plot.height();
        return {x->reversed() ? 1 - fx : fx, y->reversed() ? 1 - fy : fy};
    }
    QPointF mapTransformed(QPointF p) const
    {
        const double minX = Detail::transform(xr.min, x->scale());
        const double minY = Detail::transform(yr.min, y->scale());
        return mapFractions((p.x() - minX) / (Detail::transform(xr.max, x->scale()) - minX),
                            (p.y() - minY) / (Detail::transform(yr.max, y->scale()) - minY));
    }
    QPointF map(double a, double b) const
    {
        return mapFractions(fraction(a, xr, x->scale()), fraction(b, yr, y->scale()));
    }
    bool valid(double a, double b) const
    {
        return std::isfinite(a) && std::isfinite(b) &&
               (x->scale() != ChartAxis::Logarithmic || a > 0) &&
               (y->scale() != ChartAxis::Logarithmic || b > 0);
    }
    bool validCurvePoint(ChartSeries *s, int index) const
    {
        if (index < 0 || index >= s->count())
            return false;
        const auto &p = s->sampleAt(index);
        return valid(p.x, p.y) && (seriesType(s) != ChartSeries::RangeArea ||
                                   (s->hasRangeValue(index) && valid(p.x, s->lowerValue(index))));
    }
    void layoutNaturalCurves()
    {
        for (auto s : series)
        {
            AxisContext context(*this, s);
            if (!s || !s->isVisible() ||
                (s->curveMode() != ChartSeries::NaturalSpline && !useCurveLod(s)) ||
                (seriesType(s) != ChartSeries::Line && seriesType(s) != ChartSeries::Area &&
                 seriesType(s) != ChartSeries::RangeArea))
                continue;
            auto &layout = curveLayouts[s];
            const bool changed = layout.type != seriesType(s) || layout.mode != s->curveMode() ||
                                 layout.xScale != x->scale() || layout.yScale != y->scale() ||
                                 layout.tension != s->splineTension();
            if (changed)
            {
                layout = {seriesType(s), s->curveMode(), x->scale(), y->scale(),
                          s->splineTension()};
                naturalCurves.remove(s);
                naturalLowerCurves.remove(s);
                ++curveCacheBuilds;
            }
            const bool band = seriesType(s) == ChartSeries::RangeArea;
            for (int boundary = 0; changed && s->curveMode() == ChartSeries::NaturalSpline &&
                                   boundary < (band ? 2 : 1);
                 ++boundary)
            {
                auto &curves = boundary ? naturalLowerCurves[s] : naturalCurves[s];
                curves.resize(std::max(0, s->count() - 1));
                QVector<QPointF> run;
                int start = 0;
                const auto flush = [&]
                {
                    const auto fitted = naturalCubics(run);
                    for (int j = 0; j < fitted.size(); ++j)
                        curves[start + j] = fitted[j];
                    run.clear();
                };
                for (int i = 0; i < s->count(); ++i)
                {
                    if (s->breakBefore(i))
                    {
                        flush();
                        start = i;
                    }
                    if (!validCurvePoint(s, i))
                    {
                        flush();
                        start = i + 1;
                    }
                    else
                    {
                        const auto &p = s->sampleAt(i);
                        run.append(
                            {Detail::transform(p.x, x->scale()),
                             Detail::transform(boundary ? s->lowerValue(i) : p.y, y->scale())});
                    }
                }
                flush();
            }
            if (useCurveLod(s) && !layout.indexed)
            {
                layout.lod.nodes.reserve(std::max(0, s->count() * 2 - 1));
                int start = -1;
                const auto flush = [&](int last)
                {
                    if (start >= 0 && last > start)
                    {
                        layout.lod.roots.append(layout.lod.append(
                            start, last,
                            [&](int first) { return curve(s, first, first + 1, false); }));
                        if (band)
                            layout.lowerLod.roots.append(layout.lowerLod.append(
                                start, last,
                                [&](int first)
                                { return curve(s, first, first + 1, false, true); }));
                    }
                    else if (start >= 0 && last == start)
                        layout.lod.isolated.append(start);
                    start = -1;
                };
                for (int i = 0; i < s->count(); ++i)
                {
                    if (s->breakBefore(i) || !validCurvePoint(s, i))
                        flush(i - 1);
                    if (validCurvePoint(s, i) && start < 0)
                        start = i;
                }
                flush(s->count() - 1);
                layout.indexed = true;
                if (!changed)
                    ++curveCacheBuilds;
            }
        }
    }
    ChartCubic curve(ChartSeries *s, int first, int second, bool screen, bool lower = false) const
    {
        if (s->curveMode() == ChartSeries::NaturalSpline && second == first + 1)
        {
            const auto &cache = lower ? naturalLowerCurves : naturalCurves;
            const auto found = cache.constFind(s);
            if (found != cache.cend() && first < found->size())
            {
                auto c = found->at(first);
                if (!screen)
                    return c;
                return {mapTransformed(c.a), mapTransformed(c.c1), mapTransformed(c.c2),
                        mapTransformed(c.b)};
            }
        }
        const auto point = [&](int index)
        {
            const auto &p = s->sampleAt(index);
            const double value = lower ? s->lowerValue(index) : p.y;
            return QPointF(Detail::transform(p.x, x->scale()),
                           Detail::transform(value, y->scale()));
        };
        const auto neighbor = [&](int index, int fallback)
        {
            return validCurvePoint(s, index) && !s->breakBefore(std::max(index, fallback))
                       ? point(index)
                       : point(fallback);
        };
        // Fit in increasing data-axis space before reflecting the viewport. In particular,
        // reversing X must not turn a monotone spline into straight segments.
        const auto c = chartCubic(neighbor(first - 1, first), point(first), point(second),
                                  neighbor(second + 1, second), s->curveMode(), s->splineTension());
        return screen ? ChartCubic{mapTransformed(c.a), mapTransformed(c.c1), mapTransformed(c.c2),
                                   mapTransformed(c.b)}
                      : c;
    }
    void layoutBars()
    {
        barValues.clear();
        struct Groups
        {
            int count[2]{}, index[2]{};
            QMap<double, SignedTotal> totals[2];
            QMap<double, double> positive[4], negative[4];
        };
        QHash<QPair<ChartAxis *, ChartAxis *>, Groups> grouping;
        // Normalize with scaled sums so finite large source values cannot overflow the denominator.
        for (auto s : series)
            if (s && s->isVisible() && s->count())
            {
                AxisContext context(*this, s);
                auto &group = grouping[{x, y}];
                const auto t = seriesType(s);
                const int horizontal = horizontalBar(t);
                if (groupedBar(t))
                    ++group.count[horizontal];
                if (percentBar(t))
                    for (int i = 0; i < s->count(); ++i)
                    {
                        const auto &p = s->sampleAt(i);
                        if (horizontal ? valid(p.y, p.x) : valid(p.x, p.y))
                            group.totals[horizontal][p.x].add(p.y);
                    }
            }
        for (auto s : series)
        {
            AxisContext context(*this, s);
            auto &group = grouping[{x, y}];
            if (!s || !s->isVisible() || !s->count())
                continue;
            const auto t = seriesType(s);
            if (!barType(t))
                continue;
            if (t == ChartSeries::Waterfall)
            {
                auto &layout = barValues[s];
                layout.reserve(s->count());
                for (int i = 0; i < s->count(); ++i)
                {
                    const auto data = s->at(i);
                    const auto &point = s->sampleAt(i);
                    if (!data.contains("waterfallKind") || !s->hasRangeValue(i) || !valid(point.x, point.y))
                        continue;
                    layout.append({i, point.x, barGroupWidth * (1 - barSpacing), s->lowerValue(i), point.y,
                        std::numeric_limits<double>::quiet_NaN(), data.value("barColor").value<QColor>(),
                        data.value("connectorBefore").toBool(), data.value("cumulativeBefore").toDouble()});
                }
                continue;
            }
            if (t == ChartSeries::Histogram)
            {
                auto &layout = barValues[s];
                layout.reserve(s->count());
                for (int i = 0; i < s->count(); ++i)
                {
                    const auto data = s->at(i);
                    const auto &point = s->sampleAt(i);
                    if (!data.contains("binLower") || !data.contains("binUpper") ||
                        !valid(point.x, point.y))
                        continue;
                    const double lower = data.value("binLower").toDouble(),
                                 upper = data.value("binUpper").toDouble();
                    layout.append({i, lower / 2 + upper / 2, (upper - lower) * (1 - barSpacing), 0,
                                   point.y, std::numeric_limits<double>::quiet_NaN()});
                }
                continue;
            }
            const int horizontal = horizontalBar(t), stack = horizontal + (percentBar(t) ? 2 : 0);
            const double slot = groupedBar(t) ? barGroupWidth / std::max(1, group.count[horizontal])
                                              : barGroupWidth;
            const double width = slot * (1 - barSpacing);
            const double offset =
                groupedBar(t) ? -barGroupWidth / 2 + slot * (group.index[horizontal]++ + .5) : 0;
            auto &layout = barValues[s];
            layout.reserve(s->count());
            for (int i = 0; i < s->count(); ++i)
            {
                const auto &p = s->sampleAt(i);
                if (!(horizontal ? valid(p.y, p.x) : valid(p.x, p.y)))
                    continue;
                const double value =
                    percentBar(t) ? group.totals[horizontal][p.x].percentage(p.y) : p.y;
                double bottom = 0;
                if (!groupedBar(t))
                {
                    auto &sum = p.y >= 0 ? group.positive[stack][p.x] : group.negative[stack][p.x];
                    bottom = sum;
                    sum += value;
                }
                if (!std::isfinite(bottom + value))
                    continue;
                const auto metadata = s->at(i);
                layout.append({i, p.x + offset, width * metadata.value("categoryWidth", 1).toDouble(), bottom, bottom + value,
                               percentBar(t) ? value : std::numeric_limits<double>::quiet_NaN(), metadata.value("barColor").value<QColor>()});
            }
        }
    }
    void ranges()
    {
        layoutBars();
        layoutNaturalCurves();
        struct Bounds
        {
            double min = INFINITY, max = -INFINITY;
        };
        QHash<ChartAxis *, Bounds> bounds;
        bool automatic = false;
        for (const auto &entry : axisLayouts)
            automatic |= entry.axis->automatic();
        if (!automatic)
        {
            axisRanges.clear();
            for (const auto &entry : axisLayouts)
                axisRanges.insert(entry.axis, {entry.axis->minimum(), entry.axis->maximum()});
            xr = axisRanges.value(defaultX);
            yr = axisRanges.value(defaultY);
            return;
        }
        const auto include = [&](double a, double b)
        {
            if (std::isfinite(a) && (x->scale() != ChartAxis::Logarithmic || a > 0))
            {
                bounds[x].min = std::min(bounds[x].min, a);
                bounds[x].max = std::max(bounds[x].max, a);
            }
            if (std::isfinite(b) && (y->scale() != ChartAxis::Logarithmic || b > 0))
            {
                bounds[y].min = std::min(bounds[y].min, b);
                bounds[y].max = std::max(bounds[y].max, b);
            }
        };
        for (auto s : series)
        {
            AxisContext context(*this, s);
            if (!s || !s->isVisible())
                continue;
            const auto t = seriesType(s);
            if (t == ChartSeries::Grid || t == ChartSeries::Contour)
            {
                const auto v = ChartStatisticsAccess::visual(s);
                if (t == ChartSeries::Grid && v.value("rows").toInt() > 0) {
                    const auto origin=v.value("origin").toPointF(); const auto spacing=v.value("spacing").toSizeF();
                    include(origin.x()-spacing.width()/2, origin.y()-spacing.height()/2);
                    include(origin.x()+(v.value("columns").toInt()-.5)*spacing.width(),origin.y()+(v.value("rows").toInt()-.5)*spacing.height());
                } else if (v.contains("contourOrigin")) {
                    const auto a=v.value("contourOrigin").toPointF(), b=v.value("contourEnd").toPointF();include(a.x(),a.y());include(b.x(),b.y());
                }
                continue;
            }
            if (barType(t))
            {
                for (const auto &bar : barValues[s])
                {
                    // Half a category of padding keeps grouped bars inside the automatic plot.
                    const double category = s->sampleAt(bar.index).x;
                    if (horizontalBar(t))
                    {
                        include(bar.bottom, category - .5);
                        include(bar.top, category + .5);
                    }
                    else
                    {
                        const auto data = s->at(bar.index);
                        const double half = data.value("categoryWidth", 1).toDouble()/2;
                        include(t == ChartSeries::Histogram ? data.value("binLower").toDouble()
                                                            : category - half,
                                bar.bottom);
                        include(t == ChartSeries::Histogram ? data.value("binUpper").toDouble()
                                                            : category + half,
                                bar.top);
                    }
                }
                continue;
            }
            bool hasAreaValue = false;
            for (int i = 0; i < s->count(); ++i)
            {
                const auto &p = s->sampleAt(i);
                if (!valid(p.x, p.y))
                    continue;
                hasAreaValue = true;
                if (t == ChartSeries::Candlestick || t == ChartSeries::Ohlc || t == ChartSeries::BoxPlot)
                {
                    const double half = s->at(i).value("categoryWidth", 1).toDouble() / 2;
                    include(p.x - half, s->lowerValue(i));
                    include(p.x + half, p.y);
                    continue;
                }
                if (t == ChartSeries::RangeArea)
                {
                    if (s->hasRangeValue(i) && valid(p.x, s->lowerValue(i)))
                    {
                        include(p.x, s->lowerValue(i));
                        include(p.x, p.y);
                        if (i > 0 && !s->breakBefore(i) && validCurvePoint(s, i - 1) &&
                            (s->curveMode() == ChartSeries::NaturalSpline ||
                             s->curveMode() == ChartSeries::CatmullRomSpline))
                            for (bool lower : {false, true})
                                for (const auto &extremum :
                                     cubicExtrema(curve(s, i - 1, i, false, lower)))
                                    include(inverse(extremum.x(), x->scale()),
                                            inverse(extremum.y(), y->scale()));
                    }
                    continue;
                }
                const double dx = t == ChartSeries::Heatmap ? cellW / 2 : 0;
                const double dy = t == ChartSeries::Heatmap ? cellH / 2 : 0;
                include(p.x - dx, p.y - dy);
                include(p.x + dx, p.y + dy);
                if ((t == ChartSeries::Line || t == ChartSeries::Area) &&
                    (s->curveMode() == ChartSeries::CatmullRomSpline ||
                     s->curveMode() == ChartSeries::NaturalSpline) &&
                    i > 0 && !s->breakBefore(i))
                {
                    const auto &previous = s->sampleAt(i - 1);
                    if (valid(previous.x, previous.y))
                        for (const auto &extremum : cubicExtrema(curve(s, i - 1, i, false)))
                            include(inverse(extremum.x(), x->scale()),
                                    inverse(extremum.y(), y->scale()));
                }
            }
            if (t == ChartSeries::Area && hasAreaValue &&
                s->areaBaselineMode() != ChartSeries::PlotBottomBaseline)
            {
                const double baseline =
                    s->areaBaselineMode() == ChartSeries::CustomBaseline ? s->areaBaseline() : 0.;
                if (y->scale() != ChartAxis::Logarithmic || baseline > 0)
                {
                    bounds[y].min = std::min(bounds[y].min, baseline);
                    bounds[y].max = std::max(bounds[y].max, baseline);
                }
            }
        }
        axisRanges.clear();
        for (const auto &entry : axisLayouts)
        {
            auto axis = entry.axis.data();
            const auto bound = bounds.value(axis);
            auto range = axis->automatic() ? expanded(bound.min, bound.max,
                                                      axis->scale() == ChartAxis::Logarithmic)
                                           : Range{axis->minimum(), axis->maximum()};
            if (!entry.horizontal && axis->automatic() && bound.min != bound.max &&
                std::isfinite(bound.min) && axis->scale() != ChartAxis::Logarithmic)
            {
                const double pad = (range.max - range.min) * .05;
                range.min -= pad;
                range.max += pad;
            }
            axisRanges.insert(axis, range);
        }
        xr = axisRanges.value(defaultX);
        yr = axisRanges.value(defaultY);
    }
    qreal makeLegend(qreal width, qreal height)
    {
        if (!legend)
            return 0;
        struct Entry
        {
            QString name;
            QColor color;
            ChartSeries *s;
            int slice;
            bool visible;
        };
        QVector<Entry> entries;
        if (type == ChartSeries::Pie || type == ChartSeries::Doughnut)
        {
            for (auto s : series)
            {
                if (!s || !s->isVisible())
                    continue;
                for (int i = 0; i < s->count() && entries.size() < 64; ++i)
                {
                    const auto &p = s->sampleAt(i);
                    QString name = p.label.isEmpty() ? QString::number(i + 1) : p.label;
                    if (type == ChartSeries::Doughnut && multiRing && !s->name().isEmpty())
                        name.prepend(s->name() + " / ");
                    entries.append({name, sliceColor(s, i), s, i, sliceVisible(s, i)});
                }
                if (type != ChartSeries::Doughnut || !multiRing)
                    break;
            }
        }
        else
            for (auto s : series)
                if (s)
                    entries.append({s->name(), seriesColor(s), s, -1, s->isVisible()});
        const double available = std::max(40., double(width) - 24);
        double cx = 12, cy = 0;
        const QFontMetricsF metrics(textFont);
        QVector<QPair<Entry, QRectF>> layout;
        for (const auto &e : entries)
        {
            const double w =
                std::min(available, std::max(80., metrics.horizontalAdvance(e.name) + 36));
            if (cx + w > width - 12 && cx > 12)
            {
                cx = 12;
                cy += 25;
            }
            if (cy > height * .35)
                break;
            layout.append({e, QRectF(cx, cy, w, 24)});
            cx += w + 8;
        }
        const double h = layout.isEmpty() ? 0 : cy + 30;
        for (auto &pair : layout)
        {
            const auto &e = pair.first;
            auto r = pair.second.translated(0, height - h);
            legends.append({r, e.s, e.slice});
            QColor color = e.visible ? e.color : disabledColor;
            Batch b{color, {}, false};
            rectangle(b, QRectF(r.left(), r.center().y() - 4, 12, 8));
            batches.append(b);
            labels.append({e.name, QRectF(r.left() + 16, r.top(), r.width() - 18, r.height()),
                           e.visible ? text : disabledColor, 12});
        }
        return h;
    }
    void layoutAxes(const ChartItem *q, double legendHeight)
    {
        const double titleTop = title.isEmpty() ? 38 : 60;
        if (radial())
        {
            plot = {20, titleTop, std::max(1., q->width() - 40),
                    std::max(1., q->height() - titleTop - legendHeight - 15)};
            return;
        }
        QFont tickFont = textFont;
        tickFont.setPixelSize(std::max(9, textFont.pixelSize() - 1));
        const QFontMetricsF metrics(tickFont);
        double left = 0, right = 0, top = 0, bottom = 0;
        int horizontalCount = 0;
        for (const auto &entry : axisLayouts)
            horizontalCount += entry.horizontal && entry.axis->isVisible();
        for (auto &entry : axisLayouts)
        {
            entry.extent = entry.offset = 0;
            auto axis = entry.axis.data();
            if (!axis->isVisible())
                continue;
            const bool far = axis->side() == ChartAxis::FarSide;
            auto &margin = entry.horizontal ? (far ? top : bottom) : (far ? right : left);
            entry.offset = margin;
            if (entry.horizontal)
                entry.extent = horizontalCount > 1 ? 60 : 45;
            else
            {
                double extent = 76;
                const auto range = axisRanges.value(axis);
                for (double tick : ticks(range, axis->scale(), axis->tickCount()))
                    extent =
                        std::max(extent, metrics.horizontalAdvance(label(tick, axis, range)) + 16);
                entry.extent = std::min(extent, std::max(76., q->width() - 120));
            }
            margin += entry.extent;
        }
        // Keep a usable plot on narrow items with several axes; labels are elided in their lanes.
        const double available = std::max(76., q->width() - 100);
        if (left + right > available)
        {
            const double ratio = available / (left + right);
            left *= ratio;
            right *= ratio;
            for (auto &entry : axisLayouts)
                if (!entry.horizontal)
                {
                    entry.extent *= ratio;
                    entry.offset *= ratio;
                }
        }
        left = std::max(24., left);
        right = std::max(24., right);
        plot = {left, titleTop + top, std::max(1., q->width() - left - right),
                std::max(1., q->height() - titleTop - top - legendHeight - bottom - 15)};
        if (touching)
            plot = touchPlot;
    }
    void axes(qreal width)
    {
        Batch gridBatch{gridColor, {}, false}, border{borderColor, {}, false};
        QColor minorColor = gridColor;
        minorColor.setAlphaF(minorColor.alphaF() * .45);
        Batch minorGrid{minorColor, {}, false};
        QFont tickFont = textFont;
        tickFont.setPixelSize(std::max(9, textFont.pixelSize() - 1));
        const QFontMetricsF metrics(tickFont);
        double topExtent = 0;
        for (const auto &entry : axisLayouts)
            if (entry.horizontal && entry.axis->side() == ChartAxis::FarSide)
                topExtent += entry.extent;
        for (const auto &entry : axisLayouts)
        {
            auto axis = entry.axis.data();
            if (!axis->isVisible())
                continue;
            const bool far = axis->side() == ChartAxis::FarSide;
            const auto range = axisRanges.value(axis);
            const double edge =
                entry.horizontal ? (far ? plot.top() - entry.offset : plot.bottom() + entry.offset)
                                 : (far ? plot.right() + entry.offset : plot.left() - entry.offset);
            const double direction = entry.horizontal ? (far ? -1 : 1) : (far ? 1 : -1);
            const auto coordinate = [&](double value)
            {
                double f = fraction(value, range, axis->scale());
                if (axis->reversed())
                    f = 1 - f;
                return entry.horizontal ? plot.left() + f * plot.width()
                                        : plot.bottom() - f * plot.height();
            };
            const auto tick = [&](double value, bool minor)
            {
                const double p = coordinate(value), length = minor ? 2 : 4;
                if (entry.horizontal)
                {
                    if (grid)
                        segment(minor ? minorGrid : gridBatch, {p, plot.top()}, {p, plot.bottom()},
                                1, smoothing);
                    segment(border, {p, edge}, {p, edge + direction * length}, 1, smoothing);
                }
                else
                {
                    if (grid)
                        segment(minor ? minorGrid : gridBatch, {plot.left(), p}, {plot.right(), p},
                                1, smoothing);
                    segment(border, {edge, p}, {edge + direction * length, p}, 1, smoothing);
                }
            };
            for (double value :
                 minorTicks(range, axis->scale(), axis->tickCount(), axis->minorTickCount()))
                tick(value, true);
            auto values = ticks(range, axis->scale(), axis->tickCount());
            if (entry.horizontal && axis->reversed())
                std::reverse(values.begin(), values.end());
            const auto tickText = [&](double value)
            {
                QString string = label(value, axis, range);
                if (!entry.horizontal)
                {
                    const double w = std::max(1., entry.extent - 8);
                    string = metrics.elidedText(string, Qt::ElideRight, w);
                    return Text{
                        string,
                        QRectF(far ? edge + 8 : edge - entry.extent, coordinate(value) - 11, w, 22),
                        text, 11};
                }
                const double w = std::min(std::max(1., double(width) - 8),
                                          metrics.horizontalAdvance(string) + 6);
                const double left =
                    std::clamp(coordinate(value) - w / 2, 4., std::max(4., double(width) - w - 4));
                return Text{string, QRectF(left, far ? edge - 28 : edge + 6, w, 22), text, 11};
            };
            const double lastLabelLeft = values.isEmpty() ? 0 : tickText(values.last()).rect.left();
            double previousRight = -INFINITY;
            for (int i = 0; i < values.size(); ++i)
            {
                tick(values[i], false);
                const auto label = tickText(values[i]);
                if (!entry.horizontal ||
                    (label.rect.left() >= previousRight + 8 &&
                     (i == values.size() - 1 || label.rect.right() + 8 <= lastLabelLeft)))
                {
                    labels.append(label);
                    previousRight = label.rect.right();
                }
            }
            if (entry.horizontal)
            {
                segment(border, {plot.left(), edge}, {plot.right(), edge}, 1, smoothing);
                labels.append({axis->title(),
                               QRectF(plot.left(), far ? edge - 54 : edge + 30, plot.width(), 22),
                               text, 12});
            }
            else
            {
                segment(border, {edge, plot.top()}, {edge, plot.bottom()}, 1, smoothing);
                const double w = std::max(1., entry.extent - 8);
                labels.append({metrics.elidedText(axis->title(), Qt::ElideRight, w),
                               QRectF(far ? edge + 4 : edge - entry.extent + 4,
                                      plot.top() - topExtent - 26, w, 22),
                               text, 12});
            }
        }
        batches.append(minorGrid);
        batches.append(gridBatch);
        batches.append(border);
    }
    int requestDelegate(ChartSeries *s, int kind, int index, const QRectF &rect,
                        QString content = {}, QColor color = {}, qreal rotation = 0,
                        double percentage = NAN)
    {
        auto *component = kind == 0   ? s->pointDelegate()
                          : kind == 1 ? s->barDelegate()
                                      : s->labelDelegate();
        if (!component || delegates.requests.size() >= delegates.budget || !rect.intersects(plot))
            return -1;
        auto data = pointMap(s, series.indexOf(s), index);
        data.insert("kind", kind == 0 ? "point" : kind == 1 ? "bar" : "label");
        data.insert("color", color.isValid() ? color : seriesColor(s));
        data.insert("text", content);
        data.insert("selected", selected.value(s).contains(index));
        data.insert("hovered", hoveredSeries == s && hoveredIndex == index);
        if (std::isfinite(percentage))
            data.insert("percentage", percentage);
        return delegates.enqueue(s, component, kind, index, rect, data, rotation);
    }
    void pointMarker(Batch &batch, ChartSeries *s, int index, QPointF point, double radius, const QColor &color = {})
    {
        const QRectF rect(point - QPointF(radius, radius), QSizeF(2 * radius, 2 * radius));
        const int id = requestDelegate(s, 0, index, rect, {}, color);
        if (id >= 0)
            batch.delegateInstances.append({int(batch.instances.size()), id});
        marker(batch, point, radius, s->markerShape(), smoothing);
    }
    void delegateLabel(ChartSeries *s, int index, double percentage = NAN)
    {
        auto &label = labels.last();
        label.delegateId = requestDelegate(s, 2, index, label.rect, label.text, label.color,
                                           label.rotation, percentage);
    }
    Batch markerBatch(ChartSeries *s) const
    {
        Batch result{seriesColor(s), {}, true};
        if (s->markerShape() == ChartSeries::TextureMarker)
        {
            result.texture = s->markerTextureImage();
            if (!result.texture.isNull())
                result.color = QColor(255, 255, 255, result.color.alpha());
        }
        return result;
    }
    Batch areaFill(ChartSeries *s) const
    {
        Batch fill{s->fillColor().isValid() ? s->fillColor() : seriesColor(s), {}, true};
        if (s->fillStyle() == ChartSeries::TextureFill)
        {
            fill.texture = s->fillTextureImage();
            if (!fill.texture.isNull())
            {
                fill.color = Qt::white;
                fill.textureRepeats = s->fillTextureMode() == ChartSeries::TileTexture;
                fill.textureRect = QRectF(plot.topLeft(),
                                          fill.textureRepeats ? s->fillTextureSize() : plot.size());
            }
        }
        if (s->fillStyle() == ChartSeries::VerticalGradient ||
            s->fillStyle() == ChartSeries::HorizontalGradient)
        {
            fill.endColor = s->fillEndColor().isValid() ? s->fillEndColor() : fill.color;
            fill.endColor.setAlphaF(fill.endColor.alphaF() * s->fillOpacity());
            for (const auto &entry : s->fillGradientStops())
            {
                const auto map = entry.toMap();
                auto color = map.value("color").value<QColor>();
                color.setAlphaF(color.alphaF() * s->fillOpacity());
                fill.stops.append({map.value("position").toDouble(), color});
            }
            fill.gradientStart = plot.topLeft();
            fill.gradientEnd = s->fillStyle() == ChartSeries::VerticalGradient ? plot.bottomLeft()
                                                                               : plot.topRight();
        }
        fill.color.setAlphaF(fill.color.alphaF() * s->fillOpacity());
        return fill;
    }
    double areaBaseline(ChartSeries *s) const
    {
        if (s->areaBaselineMode() == ChartSeries::PlotBottomBaseline)
            return y->reversed() ? yr.max : yr.min;
        const double value =
            s->areaBaselineMode() == ChartSeries::CustomBaseline ? s->areaBaseline() : 0.;
        return y->scale() == ChartAxis::Logarithmic && value <= 0
                   ? yr.min
                   : std::clamp(value, yr.min, yr.max);
    }
    void bandPath(ChartSeries *s, int first, int second, QVector<ChartBandPoint> &path,
                  QVector<int> &owners) const
    {
        const auto &a = s->sampleAt(first), &b = s->sampleAt(second);
        const QPointF previousUpper = map(a.x, a.y), previousLower = map(a.x, s->lowerValue(first));
        const QPointF upper = map(b.x, b.y), lower = map(b.x, s->lowerValue(second));
        path = {{previousUpper, previousLower, 0}};
        owners.clear();
        const auto mode = s->curveMode();
        if (mode == ChartSeries::StepLeft)
        {
            path.append({{previousUpper.x(), upper.y()}, {previousLower.x(), lower.y()}, 1});
            path.append({upper, lower, 1});
            owners = {second, second};
        }
        else if (mode == ChartSeries::StepRight)
        {
            path.append({{upper.x(), previousUpper.y()}, {lower.x(), previousLower.y()}, 0});
            path.append({upper, lower, 1});
            owners = {first, second};
        }
        else if (mode == ChartSeries::StepCenter)
        {
            const double mid = (previousUpper.x() + upper.x()) / 2;
            path.append({{mid, previousUpper.y()}, {mid, previousLower.y()}, 0});
            path.append({{mid, upper.y()}, {mid, lower.y()}, 1});
            path.append({upper, lower, 1});
            owners = {first, second, second};
        }
        else if (mode >= ChartSeries::MonotoneSpline)
            flattenBandCubics(curve(s, first, second, true), curve(s, first, second, true, true),
                              curveError * curveError, path);
        else
            path.append({upper, lower, 1});
    }
    void lodRangeArea(ChartSeries *s)
    {
        const auto found = curveLayouts.constFind(s);
        if (found == curveLayouts.cend())
            return;
        const auto &tree = found->lod, &lowerTree = found->lowerLod;
        const double minX = Detail::transform(xr.min, x->scale());
        const double spanX = Detail::transform(xr.max, x->scale()) - minX;
        const double column = spanX / std::max(1, budget / 6);
        const double margin = (s->lineWidth() + smoothing) / plot.width() * spanX;
        Batch fill = areaFill(s),
              border{s->borderColor().isValid() ? s->borderColor() : seriesColor(s), {}, true};
        Stroke upperStroke(s, plot, smoothing), lowerStroke(s, plot, smoothing);
        QVector<ChartBandPoint> path;
        QVector<int> owners;
        const auto draw = [&](int first, int last, double approximation)
        {
            rendered += path.size();
            for (int j = 1; j < path.size(); ++j)
            {
                const auto &from = path[j - 1], &to = path[j];
                if (!finite(from.upper) || !finite(from.lower) || !finite(to.upper) ||
                    !finite(to.lower))
                    continue;
                const auto bounded = [&](QPointF p)
                {
                    return QPointF(std::clamp(p.x(), plot.left() - 1e6, plot.right() + 1e6),
                                   std::clamp(p.y(), plot.top() - 1e6, plot.bottom() + 1e6));
                };
                const auto a = bounded(from.upper), b = bounded(to.upper), c = bounded(from.lower),
                           d = bounded(to.lower);
                fillBand(fill, plot, a, b, c, d);
                upperStroke.draw(border, a, b);
                lowerStroke.draw(border, c, d);
                bandHits.append({a, b, c, d, s, owners.isEmpty() ? first : owners[j - 1],
                                 owners.isEmpty() ? last : owners[j - 1], from.parameter,
                                 to.parameter, approximation});
            }
        };
        const auto visit = [&](auto &&self, int index) -> void
        {
            const auto &node = tree.nodes[index];
            ++curveLodNodesVisited;
            if (node.maxX < minX - margin || node.minX > minX + spanX + margin)
            {
                upperStroke.reset();
                lowerStroke.reset();
                return;
            }
            if (node.left < 0)
            {
                bandPath(s, node.first, node.last, path, owners);
                draw(node.first, node.last, 0);
            }
            else if (node.maxX - node.minX <= column)
            {
                const auto &lower = lowerTree.nodes[index];
                QVector<double> orders{double(node.first), node.low.order,   node.high.order,
                                       lower.low.order,    lower.high.order, double(node.last)};
                std::sort(orders.begin(), orders.end());
                orders.erase(std::unique(orders.begin(), orders.end()), orders.end());
                path.clear();
                owners.clear();
                for (double order : orders)
                {
                    const int first = std::min(node.last - 1, int(std::floor(order)));
                    const double t = order - first;
                    path.append(
                        {mapTransformed(cubicAt(curve(s, first, first + 1, false), t)),
                         mapTransformed(cubicAt(curve(s, first, first + 1, false, true), t)),
                         order});
                }
                draw(node.first, node.last,
                     std::max(1e-9, (node.maxX - node.minX) / spanX * plot.width()));
            }
            else
            {
                self(self, node.left);
                self(self, node.right);
            }
        };
        for (int root : tree.roots)
        {
            upperStroke.reset();
            lowerStroke.reset();
            visit(visit, root);
        }
        upperStroke.finish();
        lowerStroke.finish();
        batches.append(std::move(fill));
        batches.append(std::move(border));
    }
    void rangeArea(ChartSeries *s)
    {
        if (useCurveLod(s))
        {
            lodRangeArea(s);
            return;
        }
        Batch fill = areaFill(s);
        Batch border{s->borderColor().isValid() ? s->borderColor() : seriesColor(s), {}, true};
        Stroke upperStroke(s, plot, smoothing), lowerStroke(s, plot, smoothing);
        QPointF previousUpper, previousLower;
        bool previousValid = false;
        int previousIndex = -1;
        auto indices = visibleIndices(s, xr, 0);
        if (s->curveMode() == ChartSeries::NaturalSpline ||
            s->curveMode() == ChartSeries::CatmullRomSpline)
        {
            indices.resize(s->count());
            for (int i = 0; i < s->count(); ++i)
                indices[i] = i;
        }
        for (int i : indices)
        {
            const auto &p = s->sampleAt(i);
            if (!s->hasRangeValue(i) || !valid(p.x, p.y) || !valid(p.x, s->lowerValue(i)))
            {
                previousValid = false;
                continue;
            }
            auto upper = map(p.x, p.y), lower = map(p.x, s->lowerValue(i));
            if (!finite(upper) || !finite(lower))
            {
                previousValid = false;
                continue;
            }
            const auto bounded = [&](QPointF p)
            {
                return QPointF(std::clamp(p.x(), plot.left() - 1e6, plot.right() + 1e6),
                               std::clamp(p.y(), plot.top() - 1e6, plot.bottom() + 1e6));
            };
            upper = bounded(upper);
            lower = bounded(lower);
            if (s->breakBefore(i))
                previousValid = false;
            if (!previousValid)
            {
                upperStroke.reset();
                lowerStroke.reset();
            }
            else
            {
                QVector<ChartBandPoint> path;
                QVector<int> owners;
                bandPath(s, previousIndex, i, path, owners);
                for (int j = 1; j < path.size(); ++j)
                {
                    const auto &from = path[j - 1], &to = path[j];
                    if (!finite(from.upper) || !finite(from.lower) || !finite(to.upper) ||
                        !finite(to.lower))
                        continue;
                    const auto a = bounded(from.upper), b = bounded(to.upper),
                               c = bounded(from.lower), e = bounded(to.lower);
                    fillBand(fill, plot, a, b, c, e);
                    upperStroke.draw(border, a, b);
                    lowerStroke.draw(border, c, e);
                    bandHits.append(
                        {a, b, c, e, s, owners.isEmpty() ? previousIndex : owners[j - 1],
                         owners.isEmpty() ? i : owners[j - 1], from.parameter, to.parameter});
                }
            }
            previousUpper = upper;
            previousLower = lower;
            previousIndex = i;
            previousValid = true;
            ++rendered;
        }
        upperStroke.finish();
        lowerStroke.finish();
        batches.append(std::move(fill));
        batches.append(std::move(border));
    }
    QString barLabel(ChartSeries *s, const BarValue &bar, bool horizontal) const
    {
        const auto &sample = s->sampleAt(bar.index);
        const auto categoryAxis = horizontal ? y : x;
        const auto number = [&](double value)
        { return QLocale().toString(value, 'g', s->barLabelPrecision()); };
        QString category = sample.label;
        if (category.isEmpty() && categoryAxis->scale() == ChartAxis::Category && sample.x >= 0 &&
            sample.x < categoryAxis->categories().size() && std::floor(sample.x) == sample.x)
            category = categoryAxis->categories().value(int(sample.x));
        if (category.isEmpty())
            category = number(sample.x);
        const QString format = s->barLabelFormat();
        static const QRegularExpression token(
            QStringLiteral("@(value|percentage|category|series)\\b"));
        auto matches = token.globalMatch(format);
        QString result;
        qsizetype previous = 0;
        while (matches.hasNext())
        {
            const auto match = matches.next();
            result += QStringView(format).mid(previous, match.capturedStart() - previous);
            const auto name = match.capturedView(1);
            if (name == u"value")
                result += number(seriesType(s) == ChartSeries::Waterfall ? sample.value : sample.y);
            else if (name == u"percentage")
            {
                if (!std::isfinite(bar.percentage))
                    return {};
                result += number(bar.percentage);
            }
            else if (name == u"category")
                result += category;
            else
                result += s->name();
            previous = match.capturedEnd();
        }
        result += QStringView(format).mid(previous);
        result.replace(QLatin1Char('\n'), QLatin1Char(' '));
        result.replace(QLatin1Char('\r'), QLatin1Char(' '));
        return result.size() <= 1024 ? result : QString();
    }
    void placeBarLabel(ChartSeries *s, const BarValue &bar, const QRectF &rect, bool horizontal,
                       QVector<QRectF> &occupied)
    {
        const QString content = barLabel(s, bar, horizontal);
        if (content.isEmpty())
            return;
        QFont font = textFont;
        font.setPixelSize(std::max(9, font.pixelSize()));
        const QFontMetricsF metrics(font);
        const QSizeF size(std::ceil(metrics.horizontalAdvance(content)) + 6, font.pixelSize() + 8);
        QTransform rotation;
        rotation.rotate(s->barLabelRotation());
        const QSizeF occupiedSize = rotation.mapRect(QRectF(QPointF(), size)).size();
        QPointF center = rect.center();
        const bool positive = (bar.top > bar.bottom) != (horizontal ? x : y)->reversed();
        const QPointF direction =
            horizontal ? QPointF(positive ? 1 : -1, 0) : QPointF(0, positive ? -1 : 1);
        const double half = (horizontal ? occupiedSize.width() : occupiedSize.height()) / 2;
        const double extent = (horizontal ? rect.width() : rect.height()) / 2;
        const double border =
            std::min({double(s->barBorderWidth()), rect.width() / 2, rect.height() / 2});
        const double padding = s->barLabelMargin() + border;
        const bool outside = s->barLabelPosition() == ChartSeries::BarLabelOutsideEnd;
        if (outside)
            center += direction * (extent + half + s->barLabelMargin());
        else if (s->barLabelPosition() == ChartSeries::BarLabelInsideEnd)
            center += direction * (extent - half - padding);
        else if (s->barLabelPosition() == ChartSeries::BarLabelInsideBase)
            center -= direction * (extent - half - padding);
        const QRectF label(center - QPointF(occupiedSize.width() / 2, occupiedSize.height() / 2),
                           occupiedSize);
        if (!plot.contains(label) ||
            (!outside && !rect.adjusted(border, border, -border, -border).contains(label)))
            return;
        for (const auto &other : occupied)
            if (other.intersects(label))
                return;
        occupied.append(label);
        labels.append({content, QRectF(center - QPointF(size.width() / 2, size.height() / 2), size),
                       outside ? text : contrastingText(seriesColor(s)), 12,
                       s->barLabelRotation()});
        delegateLabel(s, bar.index, bar.percentage);
    }
    void curvePath(ChartSeries *s, int first, int second, QVector<QPointF> &path,
                   QVector<int> &owners) const
    {
        const auto &a = s->sampleAt(first), &b = s->sampleAt(second);
        const auto start = map(a.x, a.y), end = map(b.x, b.y);
        path = {start};
        owners.clear();
        switch (s->curveMode())
        {
        case ChartSeries::StepLeft:
            path << QPointF(start.x(), end.y()) << end;
            owners = {second, second};
            break;
        case ChartSeries::StepRight:
            path << QPointF(end.x(), start.y()) << end;
            owners = {first, second};
            break;
        case ChartSeries::StepCenter:
        {
            const double mid = (start.x() + end.x()) / 2;
            path << QPointF(mid, start.y()) << QPointF(mid, end.y()) << end;
            owners = {first, second, second};
            break;
        }
        default:
            flattenCubic(curve(s, first, second, true), curveError * curveError, path);
            break;
        }
    }
    void lodCurve(ChartSeries *s, ChartSeries::Type type)
    {
        const auto found = curveLayouts.constFind(s);
        if (found == curveLayouts.cend())
            return;
        const auto &tree = found->lod;
        const double minX = Detail::transform(xr.min, x->scale());
        const double spanX = Detail::transform(xr.max, x->scale()) - minX;
        const auto mapped = [&](QPointF point) { return mapTransformed(point); };
        // Four representatives per screen column: first, actual minimum/maximum, last.
        const double column = spanX / std::max(1, budget / 4);
        const double margin = (s->lineWidth() + smoothing) / plot.width() * spanX;
        Batch border{type == ChartSeries::Area && s->borderColor().isValid() ? s->borderColor()
                                                                             : seriesColor(s),
                     {},
                     true};
        Batch fill = areaFill(s), markers = markerBatch(s);
        Stroke stroke(s, plot, smoothing);
        const double baselineY = map(xr.min, areaBaseline(s)).y();
        QSet<int> markerIndices;
        QVector<QPointF> path;
        QVector<int> owners;
        const auto draw = [&](int first, int last, double approximation)
        {
            rendered += int(path.size());
            for (int i = 1; i < path.size(); ++i)
            {
                const auto a = path[i - 1], b = path[i];
                if (!finite(a) || !finite(b) || a == b)
                    continue;
                const auto bounded = [&](QPointF p)
                {
                    return QPointF(std::clamp(p.x(), plot.left() - 1e6, plot.right() + 1e6),
                                   std::clamp(p.y(), plot.top() - 1e6, plot.bottom() + 1e6));
                };
                const auto from = bounded(a), to = bounded(b);
                stroke.draw(border, from, to);
                curveHits.append({a, b, s, owners.isEmpty() ? first : owners[i - 1],
                                  owners.isEmpty() ? last : owners[i - 1], approximation});
                if (type == ChartSeries::Area)
                    fillBand(fill, plot, from, to, {from.x(), baselineY}, {to.x(), baselineY});
            }
        };
        const auto visit = [&](auto &&self, int index) -> void
        {
            const auto &node = tree.nodes[index];
            ++curveLodNodesVisited;
            if (node.maxX < minX - margin || node.minX > minX + spanX + margin)
            {
                stroke.reset();
                return;
            }
            if (node.left < 0)
            {
                curvePath(s, node.first, node.last, path, owners);
                draw(node.first, node.last, 0);
                if (s->markersVisible())
                    markerIndices << node.first << node.last;
            }
            else if (node.maxX - node.minX <= column)
            {
                const auto &a = s->sampleAt(node.first), &b = s->sampleAt(node.last);
                QVector<ChartLodPoint> points{
                    {{Detail::transform(a.x, x->scale()), Detail::transform(a.y, y->scale())},
                     double(node.first)},
                    node.low,
                    node.high,
                    {{Detail::transform(b.x, x->scale()), Detail::transform(b.y, y->scale())},
                     double(node.last)}};
                std::stable_sort(points.begin(), points.end(),
                                 [](const auto &a, const auto &b) { return a.order < b.order; });
                path.clear();
                owners.clear();
                for (const auto &point : points)
                {
                    path.append(mapped(point.point));
                    if (s->markersVisible())
                        markerIndices.insert(int(std::round(point.order)));
                }
                draw(node.first, node.last,
                     std::max(1e-9, (node.maxX - node.minX) / spanX * plot.width()));
            }
            else
            {
                self(self, node.left);
                self(self, node.right);
            }
        };
        for (int root : tree.roots)
        {
            stroke.reset();
            visit(visit, root);
        }
        if (s->markersVisible())
        {
            for (int index : tree.isolated)
                markerIndices.insert(index);
            auto indices = markerIndices.values();
            std::sort(indices.begin(), indices.end());
            const double r = s->markerSize() / 2;
            for (int index : indices)
            {
                const auto &p = s->sampleAt(index);
                const auto at = map(p.x, p.y);
                const QRectF rect(at - QPointF(r, r), QSizeF(2 * r, 2 * r));
                if (!plot.intersects(rect))
                    continue;
                pointMarker(markers, s, index, at, r);
                hits.append({rect, s, index, s->markerShape()});
            }
        }
        stroke.finish();
        if (type == ChartSeries::Area)
            batches.append(std::move(fill));
        batches.append(std::move(border));
        if (s->markersVisible())
            batches.append(std::move(markers));
    }
    void mappedLegend(const QVariantMap &visual)
    {
        const auto palette = visual.value("palette").toList();
        if (!visual.value("legend").toBool() || palette.size() != 256)
            return;
        const double w = std::min(180., plot.width());
        for (int i = 0; i < 64; ++i)
        {
            Batch b{palette[qRound(i * 255. / 63)].value<QColor>(), {}, false};
            rectangle(b, {plot.right() - w + i * w / 64, plot.top() - 14, w / 64 + .5, 7});
            batches.append(b);
        }
        labels.append({QString::number(visual.value("minimum").toDouble(), 'g', 4) + "  —  " +
                           QString::number(visual.value("maximum").toDouble(), 'g', 4),
                       {plot.right() - w, plot.top() - 38, w, 22},
                       text,
                       11});
    }
    void gridSeries(ChartSeries *s)
    {
        const auto visual = ChartStatisticsAccess::visual(s);
        const auto origin = visual.value("origin").toPointF();
        const auto spacing = visual.value("spacing").toSizeF();
        const int rows = visual.value("rows").toInt(), columns = visual.value("columns").toInt();
        if (rows <= 0 || columns <= 0)
            return;
        const bool dense = visual.value("dense").toBool() && x->scale() != ChartAxis::Logarithmic &&
                           y->scale() != ChartAxis::Logarithmic;
        Batch b{Qt::white, {}, true};
        if (dense)
        {
            b.texture = visual.value("atlas").value<QImage>();
            b.softwareTexture = visual.value("image").value<QImage>();
            b.scalarTexture = true;
            b.textureId = visual.value("textureId").toULongLong();
            b.textureScale = {double(columns) / b.texture.width(),
                              double(rows) / b.texture.height()};
            b.textureRect = QRectF(
                map(origin.x() - spacing.width() / 2, origin.y() + (rows - .5) * spacing.height()),
                map(origin.x() + (columns - .5) * spacing.width(),
                    origin.y() - spacing.height() / 2));
            rectangle(b, b.textureRect.normalized().intersected(plot));
        }
        for (int i = 0; i < s->count(); ++i)
        {
            const auto p = s->sampleAt(i);
            if (!valid(p.x - spacing.width() / 2, p.y - spacing.height() / 2) ||
                !valid(p.x + spacing.width() / 2, p.y + spacing.height() / 2))
                continue;
            const QRectF r = QRectF(map(p.x - spacing.width() / 2, p.y - spacing.height() / 2),
                                    map(p.x + spacing.width() / 2, p.y + spacing.height() / 2))
                                 .normalized()
                                 .intersected(plot);
            if (r.isEmpty())
                continue;
            if (!dense)
            {
                b.color = s->at(i).value("mappedColor").value<QColor>();
                instance(b, r, 4);
            }
            hits.append({r, s, i});
            ++rendered;
        }
        batches.append(std::move(b));
        mappedLegend(visual);
    }
    void contourSeries(ChartSeries *s)
    {
        QMap<QRgb, Batch> fills, lines;
        QVector<QRectF> placed;
        QSet<double> labeled;
        for (int i = 0; i < s->count(); ++i)
        {
            const auto m = s->at(i);
            const auto vertices = m.value("vertices").toList();
            QPolygonF polygon;
            bool validPoints = true;
            for (const auto &v : vertices)
            {
                const auto p = v.toPointF();
                if (!valid(p.x(), p.y()))
                {
                    validPoints = false;
                    break;
                }
                polygon.append(map(p.x(), p.y()));
            }
            if (!validPoints || polygon.size() < 2 ||
                !polygon.boundingRect().adjusted(-3, -3, 3, 3).intersects(plot))
                continue;
            const auto color = m.value("mappedColor").value<QColor>();
            if (m.value("filled").toBool() && polygon.size() == 3)
            {
                auto &b = fills[color.rgba()];
                b.color = color;
                triangle(b, polygon[0], polygon[1], polygon[2]);
                hits.append({polygon.boundingRect(), s, i, -1, NAN, polygon});
            }
            else
            {
                auto &b = lines[color.rgba()];
                b.color = s->borderColor().isValid() ? s->borderColor() : color;
                segment(b, polygon[0], polygon[1], s->lineWidth(), smoothing);
                const auto delta = polygon[1] - polygon[0];
                const double length = std::hypot(delta.x(), delta.y());
                if (length > 0)
                {
                    const QPointF normal(-delta.y() / length * 3, delta.x() / length * 3);
                    const QPolygonF hit{polygon[0] + normal, polygon[1] + normal,
                                        polygon[1] - normal, polygon[0] - normal};
                    hits.append({hit.boundingRect(), s, i, -1, NAN, hit});
                }
                if (m.contains("contourLabel") && !labeled.contains(m.value("level").toDouble()))
                {
                    const auto at = (polygon[0] + polygon[1]) / 2;
                    const QRectF rect(at.x() - 24, at.y() - 10, 48, 20);
                    bool overlap = false;
                    for (const auto &p : placed)
                        overlap |= p.intersects(rect);
                    if (!overlap && plot.contains(rect))
                    {
                        labels.append({m.value("contourLabel").toString(), rect, text, 11});
                        placed.append(rect);
                        labeled.insert(m.value("level").toDouble());
                    }
                }
            }
            ++rendered;
        }
        for (const auto &b : fills)
            batches.append(b);
        for (const auto &b : lines)
            batches.append(b);
        mappedLegend(ChartStatisticsAccess::visual(s));
    }
    void financial(ChartSeries *s, ChartSeries::Type type)
    {
        Batch body{seriesColor(s), {}, true}, lines{seriesColor(s), {}, true};
        QMap<QRgb, Batch> lineGroups;
        for (int i = 0; i < s->count(); ++i)
        {
            const auto m = s->at(i);
            const auto p = s->sampleAt(i);
            if (!s->hasRangeValue(i) || !valid(p.x, p.y) || !valid(p.x, s->lowerValue(i)))
                continue;
            const bool box = type == ChartSeries::BoxPlot;
            if (!(box ? m.contains("q1") : m.contains("open")))
                continue;
            const double width =
                m.value("categoryWidth", 1).toDouble() * barGroupWidth * (1 - barSpacing);
            const double lo = s->lowerValue(i), hi = p.y;
            const double a = m.value(box ? "q1" : "open").toDouble(),
                         b = m.value(box ? "q3" : "close").toDouble();
            if (!valid(p.x, a) || !valid(p.x, b))
                continue;
            auto bounded = [&](double x, double y)
            {
                const auto v = map(x, y);
                return QPointF(std::clamp(v.x(), plot.left() - 1e6, plot.right() + 1e6),
                               std::clamp(v.y(), plot.top() - 1e6, plot.bottom() + 1e6));
            };
            const auto left = bounded(p.x - width / 2, a), right = bounded(p.x + width / 2, b);
            QRectF hit(bounded(p.x - width / 2, lo), bounded(p.x + width / 2, hi));
            hit = hit.normalized().adjusted(0, -2, 0, 2).intersected(plot);
            if (hit.isEmpty())
                continue;
            body.color = m.value("barColor").value<QColor>();
            if (!body.color.isValid())
                body.color = seriesColor(s);
            lines.color = s->borderColor().isValid() ? s->borderColor() : body.color;
            const double lineWidth = s->lineWidth();
            // Each item retains its own color in the flattened triangle batch.
            auto &itemLines = lineGroups[lines.color.rgba()];
            itemLines.color = lines.color;
            if (box)
            {
                segment(itemLines, bounded(p.x, lo), bounded(p.x, a), lineWidth, smoothing);
                segment(itemLines, bounded(p.x, b), bounded(p.x, hi), lineWidth, smoothing);
            }
            else
            {
                segment(itemLines, bounded(p.x, lo), bounded(p.x, hi), lineWidth, smoothing);
            }
            if (type == ChartSeries::Ohlc)
            {
                segment(itemLines, left, bounded(p.x, a), lineWidth, smoothing);
                segment(itemLines, bounded(p.x, b), right, lineWidth, smoothing);
            }
            else
            {
                const QRectF r = QRectF(left, right).normalized().intersected(plot);
                if (a == b)
                    segment(itemLines, left, right, lineWidth, smoothing);
                else
                {
                    const int delegateId = requestDelegate(s, 1, i, r, {}, body.color, 0);
                    if (delegateId >= 0)
                        body.delegateInstances.append({int(body.instances.size()), delegateId});
                    instance(body, r, 4);
                    if (box)
                    {
                        for (const auto &edge : {QLineF(r.topLeft(), r.topRight()),
                                                 QLineF(r.topRight(), r.bottomRight()),
                                                 QLineF(r.bottomRight(), r.bottomLeft()),
                                                 QLineF(r.bottomLeft(), r.topLeft())})
                            segment(itemLines, edge.p1(), edge.p2(), lineWidth, smoothing);
                    }
                }
                if (box)
                {
                    for (double v : {lo, hi, m.value("median").toDouble()})
                        segment(itemLines, bounded(p.x - width / 2, v), bounded(p.x + width / 2, v),
                                lineWidth, smoothing);
                }
            }
            hits.append({hit, s, i});
            ++rendered;
        }
        batches.append(std::move(body));
        for (const auto &batch : lineGroups)
            batches.append(batch);
    }
    void cartesian()
    {
        QVector<QRectF> barLabels;
        int labelCandidates = 0;
        for (auto s : series)
        {
            AxisContext context(*this, s);
            if (!s || !s->isVisible() || !s->count())
                continue;
            const auto t = seriesType(s);
            if (t == ChartSeries::Grid) { gridSeries(s); continue; }
            if (t == ChartSeries::Contour) { contourSeries(s); continue; }
            if (t == ChartSeries::Candlestick || t == ChartSeries::Ohlc || t == ChartSeries::BoxPlot)
            {
                financial(s,t);
                continue;
            }
            if (t == ChartSeries::RangeArea)
            {
                rangeArea(s);
                continue;
            }
            if (barType(t))
            {
                if (t == ChartSeries::Waterfall && s->lineWidth() > 0)
                {
                    Batch connectors{
                        s->borderColor().isValid() ? s->borderColor() : seriesColor(s), {}, true};
                    for (const auto &bar : barValues[s])
                    {
                        if (!bar.connectorBefore || bar.index == 0 ||
                            !valid(bar.category, bar.connectorLevel))
                            continue;
                        const auto from =
                            map(s->sampleAt(bar.index - 1).x + bar.width / 2, bar.connectorLevel);
                        const auto to = map(bar.category - bar.width / 2, bar.connectorLevel);
                        if (!finite(from) || !finite(to) || from.y() < plot.top() ||
                            from.y() > plot.bottom())
                            continue;
                        const double left = std::max(plot.left(), std::min(from.x(), to.x())),
                                     right = std::min(plot.right(), std::max(from.x(), to.x()));
                        if (left < right)
                            segment(connectors, {left, from.y()}, {right, from.y()}, s->lineWidth(),
                                    smoothing);
                    }
                    batches.append(std::move(connectors));
                }
                Batch bars{seriesColor(s), {}, true};
                Batch border{s->barBorderColor().isValid() ? s->barBorderColor() : text, {}, true};
                const bool horizontal = horizontalBar(t);
                const auto axis = horizontal ? x : y;
                const auto range = horizontal ? xr : yr;
                for (const auto &bar : barValues[s])
                {
                    bars.color = bar.color.isValid() ? bar.color : seriesColor(s);
                    double bottom = bar.bottom;
                    if (groupedBar(t) || t == ChartSeries::Histogram)
                        bottom = axis->scale() == ChartAxis::Logarithmic
                                     ? range.min
                                     : std::clamp(0., range.min, range.max);
                    else if (bottom == 0 && axis->scale() == ChartAxis::Logarithmic)
                        bottom = range.min;
                    const QPointF a = horizontal ? QPointF(bottom, bar.category - bar.width / 2)
                                                 : QPointF(bar.category - bar.width / 2, bottom);
                    const QPointF b = horizontal ? QPointF(bar.top, bar.category + bar.width / 2)
                                                 : QPointF(bar.category + bar.width / 2, bar.top);
                    if (!valid(a.x(), a.y()) || !valid(b.x(), b.y()) || bar.top == bar.bottom ||
                        bar.width <= 0)
                        continue;
                    // Bound endpoints before forming QRectF: subtracting a huge offscreen
                    // coordinate can otherwise lose the visible edge to cancellation.
                    const auto bounded = [&](QPointF point)
                    {
                        return QPointF(
                            std::clamp(point.x(), plot.left() - 1e6, plot.right() + 1e6),
                            std::clamp(point.y(), plot.top() - 1e6, plot.bottom() + 1e6));
                    };
                    const QRectF rect =
                        QRectF(bounded(map(a.x(), a.y())), bounded(map(b.x(), b.y()))).normalized();
                    if (!rect.intersects(plot))
                        continue;
                    // CPU clipping bounds large offscreen values before conversion to float.
                    const int delegateId = requestDelegate(s, 1, bar.index, rect.intersected(plot),
                                                           {}, bars.color, 0, bar.percentage);
                    if (delegateId >= 0)
                        bars.delegateInstances.append({int(bars.instances.size()), delegateId});
                    instance(bars, rect.intersected(plot), 4);
                    const double width = std::min(
                        {double(s->barBorderWidth()), rect.width() / 2, rect.height() / 2});
                    if (width > 0)
                    {
                        // Clip original border strips: a clipped edge is not a new bar boundary.
                        for (const QRectF edge :
                             {QRectF(rect.left(), rect.top(), rect.width(), width),
                              QRectF(rect.left(), rect.bottom() - width, rect.width(), width),
                              QRectF(rect.left(), rect.top() + width, width,
                                     rect.height() - 2 * width),
                              QRectF(rect.right() - width, rect.top() + width, width,
                                     rect.height() - 2 * width)})
                        {
                            const auto visible = edge.intersected(plot);
                            if (!visible.isEmpty())
                            {
                                if (delegateId >= 0)
                                    border.delegateInstances.append(
                                        {int(border.instances.size()), delegateId});
                                instance(border, visible, 4);
                            }
                        }
                    }
                    if (s->barLabelsVisible() && labelCandidates++ < 256)
                        placeBarLabel(s, bar, rect, horizontal, barLabels);
                    hits.append({rect, s, bar.index, -1, bar.percentage});
                    ++rendered;
                    if (width > 0)
                    {
                        // Keep fill/border ordering for overlapping records in the same series.
                        batches.append(std::move(bars));
                        batches.append(std::move(border));
                        bars = {seriesColor(s), {}, true};
                        border = {
                            s->barBorderColor().isValid() ? s->barBorderColor() : text, {}, true};
                    }
                }
                batches.append(std::move(bars));
                batches.append(std::move(border));
                continue;
            }
            if (useCurveLod(s))
            {
                lodCurve(s, t);
                continue;
            }
            // Only ordered curves use extrema LOD. Sampling bars independently
            // would corrupt stacks; sampling cells/markers would hide categories.
            auto indices = visibleIndices(s, xr,
                                          (t == ChartSeries::Line || t == ChartSeries::Area) &&
                                                  s->curveMode() == ChartSeries::LinearCurve
                                              ? budget
                                              : 0);
            if ((t == ChartSeries::Line || t == ChartSeries::Area) &&
                (s->curveMode() == ChartSeries::CatmullRomSpline ||
                 s->curveMode() == ChartSeries::NaturalSpline))
            {
                indices.clear();
                for (int i = 0; i < s->count(); ++i)
                    indices.append(i);
            }
            Batch b{t == ChartSeries::Area && s->borderColor().isValid() ? s->borderColor()
                                                                         : seriesColor(s),
                    {},
                    true};
            Batch fill = areaFill(s), areaMarkers = markerBatch(s);
            if (t == ChartSeries::Scatter || t == ChartSeries::Bubble)
                b = markerBatch(s);
            Stroke stroke(s, plot, smoothing);
            const double baseline = areaBaseline(s);
            bool prevValid = false;
            QPointF prev;
            int prevIndex = -1;
            double vmin = INFINITY, vmax = -INFINITY;
            QHash<int, Batch> cells;
            if (t == ChartSeries::Heatmap)
                for (int i = 0; i < s->count(); ++i)
                {
                    const auto &p = s->sampleAt(i);
                    vmin = std::min(vmin, p.value);
                    vmax = std::max(vmax, p.value);
                }
            // Heatmaps retain every cell: extrema sampling is for ordered XY curves.
            if (t == ChartSeries::Heatmap)
            {
                indices.clear();
                for (int i = 0; i < s->count(); ++i)
                    if (s->sampleAt(i).x + cellW / 2 >= xr.min &&
                        s->sampleAt(i).x - cellW / 2 <= xr.max)
                        indices.append(i);
            }
            for (int index : indices)
            {
                const auto &p = s->sampleAt(index);
                if (!valid(p.x, p.y))
                {
                    prevValid = false;
                    continue;
                }
                QPointF q = map(p.x, p.y);
                if (!std::isfinite(q.x()) || !std::isfinite(q.y()))
                {
                    prevValid = false;
                    continue;
                }
                ++rendered;
                // Bound extreme offscreen geometry before float conversion.
                q.setX(std::clamp(q.x(), plot.left() - 1e6, plot.right() + 1e6));
                q.setY(std::clamp(q.y(), plot.top() - 1e6, plot.bottom() + 1e6));
                if (t == ChartSeries::Line || t == ChartSeries::Area)
                {
                    if (s->breakBefore(index))
                        prevValid = false;
                    if (!prevValid)
                        stroke.reset();
                    if (prevValid)
                    {
                        QVector<QPointF> path{prev};
                        QVector<int> owners;
                        const auto mode = s->curveMode();
                        if (mode == ChartSeries::StepLeft)
                        {
                            path << QPointF(prev.x(), q.y()) << q;
                            owners = {index, index};
                        }
                        else if (mode == ChartSeries::StepRight)
                        {
                            path << QPointF(q.x(), prev.y()) << q;
                            owners = {prevIndex, index};
                        }
                        else if (mode == ChartSeries::StepCenter)
                        {
                            const double mid = (prev.x() + q.x()) / 2;
                            path << QPointF(mid, prev.y()) << QPointF(mid, q.y()) << q;
                            owners = {prevIndex, index, index};
                        }
                        else if (mode == ChartSeries::MonotoneSpline ||
                                 mode == ChartSeries::CatmullRomSpline ||
                                 mode == ChartSeries::NaturalSpline)
                        {
                            const auto c = curve(s, prevIndex, index, true);
                            const QRectF bounds = QRectF(c.a, c.b).normalized().united(
                                QRectF(c.c1, c.c2).normalized());
                            if (t == ChartSeries::Line &&
                                !bounds
                                     .adjusted(-s->lineWidth(), -s->lineWidth(), s->lineWidth(),
                                               s->lineWidth())
                                     .intersects(plot))
                                path.append(q);
                            else
                                flattenCubic(c, curveError * curveError, path);
                        }
                        else
                            path.append(q);
                        for (int i = 1; i < path.size(); ++i)
                        {
                            const auto from = path[i - 1], to = path[i];
                            if (!finite(from) || !finite(to))
                                continue;
                            const auto bounded = [&](QPointF point)
                            {
                                return QPointF(
                                    std::clamp(point.x(), plot.left() - 1e6, plot.right() + 1e6),
                                    std::clamp(point.y(), plot.top() - 1e6, plot.bottom() + 1e6));
                            };
                            const auto start = bounded(from), end = bounded(to);
                            stroke.draw(b, start, end);
                            if (mode != ChartSeries::LinearCurve ||
                                s->strokeStyle() == ChartSeries::DashLine)
                                curveHits.append({from, to, s,
                                                  owners.isEmpty() ? prevIndex : owners[i - 1],
                                                  owners.isEmpty() ? index : owners[i - 1]});
                            if (t == ChartSeries::Area)
                            {
                                const double by = map(p.x, baseline).y();
                                fillBand(fill, plot, start, end, {start.x(), by}, {end.x(), by});
                            }
                        }
                    }
                    if (s->markersVisible())
                    {
                        const double r = s->markerSize() / 2;
                        pointMarker(areaMarkers, s, index, q, r);
                        hits.append({QRectF(q - QPointF(r, r), QSizeF(2 * r, 2 * r)), s, index,
                                     s->markerShape()});
                    }
                    prev = q;
                    prevIndex = index;
                    prevValid = true;
                }
                else if (t == ChartSeries::Scatter || t == ChartSeries::Bubble)
                {
                    const auto metadata = t == ChartSeries::Bubble ? s->at(index) : QVariantMap();
                    const double r = t == ChartSeries::Bubble ? (metadata.contains("radius") ? metadata.value("radius").toDouble() : std::clamp(p.value, 1., 64.)) : s->markerSize() / 2;
                    if (r <= 0) continue;
                    const QColor mapped = metadata.value("mappedColor").value<QColor>();
                    if (mapped.isValid()) b.color = mapped;
                    pointMarker(b, s, index, q, r, mapped);
                    hits.append({QRectF(q - QPointF(r, r), QSizeF(2 * r, 2 * r)), s, index,
                                 s->markerShape()});
                }
                else if (t == ChartSeries::Heatmap)
                {
                    if (!valid(p.x - cellW / 2, p.y - cellH / 2))
                        continue;
                    auto r = QRectF(map(p.x - cellW / 2, p.y - cellH / 2),
                                    map(p.x + cellW / 2, p.y + cellH / 2))
                                 .normalized();
                    if (!r.intersects(plot))
                        continue;
                    const int bin =
                        vmax > vmin ? std::clamp(int((p.value - vmin) / (vmax - vmin) * 63), 0, 63)
                                    : 32;
                    auto &cell = cells[bin];
                    cell.color = heatColor(bin / 63.);
                    instance(cell, r.intersected(plot), 4);
                    hits.append({r, s, index});
                }
            }
            stroke.finish();
            if (t == ChartSeries::Area)
                batches.append(fill);
            batches.append(b);
            if (t == ChartSeries::Area || t == ChartSeries::Line)
                batches.append(std::move(areaMarkers));
            for (auto it = cells.cbegin(); it != cells.cend(); ++it)
                batches.append(it.value());
            if (t == ChartSeries::Bubble) mappedLegend(ChartStatisticsAccess::visual(s));
            if (t == ChartSeries::Heatmap)
            {
                const double w = std::min(180., plot.width());
                for (int i = 0; i < 64; ++i)
                {
                    Batch c{heatColor(i / 63.), {}, false};
                    rectangle(
                        c, QRectF(plot.right() - w + i * w / 64, plot.top() - 14, w / 64 + 0.5, 7));
                    batches.append(c);
                }
                labels.append({QString::number(vmin, 'g', 4) + "  " + ChartItem::tr("Intensity") +
                                   "  " + QString::number(vmax, 'g', 4),
                               QRectF(plot.right() - w, plot.top() - 38, w, 22), text, 11});
            }
        }
    }
    void polar()
    {
        const QPointF center = plot.center();
        const double radius = std::max(1., std::min(plot.width(), plot.height()) * .42);
        if (type == ChartSeries::Radar)
        {
            int n = 0;
            double maximum = 0;
            for (auto s : series)
                if (s && s->isVisible())
                {
                    n = std::max(n, std::min(32, s->count()));
                    for (int i = 0; i < std::min(32, s->count()); ++i)
                        maximum = std::max(maximum, s->sampleAt(i).y);
                }
            if (n < 3)
                return;
            maximum = y->automatic() ? std::max(1., maximum) : y->maximum();
            const double minimum = y->automatic() ? 0 : y->minimum();
            if (maximum <= minimum)
                return;
            auto radialPoint = [&](int i, double fraction)
            {
                const double a = -pi / 2 + 2 * pi * i / n;
                return center +
                       QPointF(std::cos(a), std::sin(a)) * radius * std::clamp(fraction, 0., 1.);
            };
            auto vertex = [&](int i, double v)
            {
                const auto range = radarRanges.value(i).toMap();
                const double lo = range.value("minimum", minimum).toDouble(),
                             hi = range.value("maximum", maximum).toDouble();
                const double span = hi - lo;
                const double fraction = v <= lo   ? 0
                                        : v >= hi ? 1
                                        : std::isfinite(span)
                                            ? (v - lo) / span
                                            : (v / 2 - lo / 2) / (hi / 2 - lo / 2);
                return radialPoint(i, fraction);
            };
            Batch gridBatch{gridColor, {}, false};
            for (int i = 0; i < n; ++i)
            {
                if (grid)
                {
                    segment(gridBatch, center, radialPoint(i, 1), 1, smoothing);
                    for (int ring = 1; ring <= 4; ++ring)
                        segment(gridBatch, radialPoint(i, ring / 4.),
                                radialPoint((i + 1) % n, ring / 4.), 1, smoothing);
                }
                const auto p = center + (radialPoint(i, 1) - center) * 1.13;
                labels.append({x->categories().value(i, QString::number(i + 1)),
                               QRectF(p.x() - 40, p.y() - 11, 80, 22), text, 12});
            }
            batches.append(gridBatch);
            for (auto s : series)
                if (s && s->isVisible() && s->count() >= n)
                {
                    Batch b{
                        s->borderColor().isValid() ? s->borderColor() : seriesColor(s), {}, true},
                        f = areaFill(s), markers = markerBatch(s);
                    Stroke stroke(s, plot, smoothing);
                    for (int i = 0; i < n; ++i)
                    {
                        const auto p = vertex(i, s->sampleAt(i).y),
                                   next = vertex((i + 1) % n, s->sampleAt((i + 1) % n).y);
                        triangle(f, center, p, next);
                        stroke.draw(b, p, next);
                        if (s->markersVisible())
                            pointMarker(markers, s, i, p, s->markerSize() / 2);
                        curveHits.append({p, next, s, i, (i + 1) % n});
                        if (i < s->count())
                            hits.append({QRectF(p - QPointF(7, 7), QSizeF(14, 14)), s, i});
                        ++rendered;
                    }
                    stroke.finish();
                    batches.append(f);
                    batches.append(b);
                    batches.append(markers);
                }
            return;
        }
        const double sweep = (pieEnd - pieStart) * pi / 180;
        if (sweep <= 0 || sweep > 2 * pi || pieRadius <= 0 ||
            (type == ChartSeries::Doughnut && hole >= 1))
            return;
        QVector<ChartSeries *> rings;
        double explode = 0;
        for (auto s : series)
        {
            if (!s || !s->isVisible())
                continue;
            // Empty series reserve no ring in multi-ring mode.
            if (s->count() || !multiRing || type != ChartSeries::Doughnut)
                rings.append(s);
            for (int i = 0; i < s->count(); ++i)
                if (sliceVisible(s, i) && s->sampleAt(i).y > 0)
                    explode =
                        std::max(explode, s->sliceStyle(i).value("explodeDistance").toDouble());
            if (type != ChartSeries::Doughnut || !multiRing)
                break;
        }
        if (rings.isEmpty())
            return;
        QFont labelFont = textFont;
        labelFont.setPixelSize(std::max(9, textFont.pixelSize()));
        const QFontMetricsF metrics(labelFont);
        const double labelHeight = labelFont.pixelSize() + 8;
        const double columnWidth = std::floor(std::min(160., plot.width() * .23));
        const bool outside = sliceLabels && sliceLabelPosition == SliceLabelOutside &&
                             columnWidth >= metrics.horizontalAdvance("100%") + 6 &&
                             plot.width() >= 180 && plot.height() >= 2 * labelHeight;
        // Outside labels get two columns. Tiny plots retain their data and omit labels.
        const double availableWidth = plot.width() - (outside ? 2 * (columnWidth + 18) : 0);
        const double availableHeight = plot.height() - (outside ? 16 : 0);
        const double outer =
            std::max(0., std::min(availableWidth, availableHeight) / 2 - explode) * pieRadius;
        const double inner = type == ChartSeries::Doughnut ? outer * hole : 0;
        const int count = int(rings.size());
        const double gap = count > 1 ? std::min(double(ringGap), (outer - inner) / (2 * count)) : 0;
        const double thickness = (outer - inner - gap * (count - 1)) / count;
        if (thickness <= 0)
            return;
        QVector<QRectF> placedLabels;
        struct OutsideLabel
        {
            QString text;
            QPointF anchor, radial;
            QColor color;
            double width;
            QPointer<ChartSeries> series;
            int index;
            double percentage;
        };
        QVector<OutsideLabel> outsideLabels;
        QVector<ChartLabelSlot> columns[2];
        for (int ring = 0; ring < count; ++ring)
        {
            auto *s = rings[ring];
            double maximum = 0, total = 0;
            for (int i = 0; i < s->count(); ++i)
                if (sliceVisible(s, i))
                    maximum = std::max(maximum, s->sampleAt(i).y);
            if (maximum <= 0)
                continue;
            for (int i = 0; i < s->count(); ++i)
                if (sliceVisible(s, i))
                    total += std::max(0., s->sampleAt(i).y) / maximum;
            const double ro = outer - ring * (thickness + gap), ri = std::max(0., ro - thickness);
            double a = pieStart * pi / 180 - pi / 2;
            for (int i = 0; i < s->count(); ++i)
            {
                const auto &p = s->sampleAt(i);
                if (!sliceVisible(s, i) || p.y <= 0)
                    continue;
                const double share = (p.y / maximum) / total, z = a + sweep * share;
                if (z <= a)
                    continue;
                const auto style = s->sliceStyle(i);
                const double mid = (a + z) / 2;
                const QPointF direction(std::cos(mid), std::sin(mid));
                const QPointF c = center + direction * style.value("explodeDistance").toDouble();
                const SliceHit hit{s, i, ring, c, ri, ro, a, z, share * 100};
                sliceHits.append(hit);
                const QColor color = sliceColor(s, i);
                Batch fill{color, {}, true};
                const double feather = std::min(double(smoothing), thickness / 2);
                sector(fill, c, ri, ro, a, z, feather);
                batches.append(std::move(fill));
                const double borderWidth =
                    std::min(style.value("borderWidth").toDouble(), thickness / 2);
                if (borderWidth > 0)
                {
                    Batch border{style.value("borderColor", text).value<QColor>(), {}, true};
                    sector(border, c, ro - borderWidth, ro, a, z, std::min(feather, borderWidth));
                    if (ri > 0)
                        sector(border, c, ri, ri + borderWidth, a, z,
                               std::min(feather, borderWidth));
                    if (z - a < 2 * pi - 1e-9)
                        for (double angle : {a, z})
                        {
                            const QPointF v(std::cos(angle), std::sin(angle));
                            segment(border, c + v * ri, c + v * ro, borderWidth, feather, false);
                        }
                    batches.append(std::move(border));
                }
                ++rendered;
                if (outside && style.value("labelVisible", true).toBool() &&
                    outsideLabels.size() < 256)
                {
                    const QString percentage = QString::number(share * 100, 'f', 0) + "%";
                    QString name = p.label.left(1024);
                    if (count > 1 && !s->name().isEmpty())
                        name = s->name().left(256) + (name.isEmpty() ? QString() : ": " + name);
                    name.replace(QRegularExpression("[\\r\\n\\t]"), " ");
                    const double nameWidth = columnWidth - metrics.horizontalAdvance(percentage) -
                                             metrics.horizontalAdvance(" ") - 7;
                    name = metrics.elidedText(name, Qt::ElideRight, std::max(0., nameWidth));
                    const QString label = name.isEmpty() ? percentage : name + " " + percentage;
                    const double width = std::ceil(metrics.horizontalAdvance(label)) + 6;
                    const QPointF anchor = c + direction * ro;
                    const QPointF radial = center + direction * (outer + explode + 6);
                    const int index = int(outsideLabels.size());
                    outsideLabels.append({label, anchor, radial, color, width, s, i, share * 100});
                    columns[direction.x() >= 0 ? 1 : 0].append({radial.y(), share, index});
                }
                else if (sliceLabels && sliceLabelPosition == SliceLabelInside &&
                         style.value("labelVisible", true).toBool())
                {
                    const QString label = QString::number(share * 100, 'f', 0) + "%";
                    const double width = std::ceil(metrics.horizontalAdvance(label)) + 6;
                    const double height = labelFont.pixelSize() + 8;
                    const QPointF at = c + direction * (ri > 0 ? (ri + ro) / 2 : ro * .65);
                    const QRectF rect(at.x() - width / 2, at.y() - height / 2, width, height);
                    bool fits = plot.contains(rect) && hit.contains(rect.topLeft()) &&
                                hit.contains(rect.topRight()) && hit.contains(rect.bottomLeft()) &&
                                hit.contains(rect.bottomRight());
                    for (const auto &placed : placedLabels)
                        fits = fits && !placed.intersects(rect);
                    if (fits)
                    {
                        labels.append({label, rect, contrastingText(color), 12});
                        delegateLabel(s, i, share * 100);
                        placedLabels.append(rect);
                    }
                }
                a = z;
            }
            const double sum = maximum * total;
            if (type == ChartSeries::Doughnut && count == 1 && sliceLabels && std::isfinite(sum))
            {
                const QString label = QString::number(sum, 'g', 6);
                QFont totalFont = textFont;
                totalFont.setPixelSize(std::max(9, textFont.pixelSize() + 8));
                const double width =
                    std::ceil(QFontMetricsF(totalFont).horizontalAdvance(label)) + 6;
                const double height = totalFont.pixelSize() + 8;
                if (std::hypot(width / 2, height / 2) < inner)
                    labels.append(
                        {label,
                         QRectF(center.x() - width / 2, center.y() - height / 2, width, height),
                         text, 20});
            }
        }
        // Lay out columns first, then keep the highest-priority non-crossing routes.
        struct LabelPlacement
        {
            int label;
            QRectF rect;
        };
        QVector<LabelPlacement> placements;
        QVector<ChartLeaderRoute> routes;
        for (int side = 0; side < 2; ++side)
            for (const auto &slot :
                 layoutChartLabelColumn(columns[side], plot.top(), plot.bottom(), labelHeight, 3))
            {
                const auto &label = outsideLabels[slot.index];
                const double edge = side ? plot.right() - columnWidth : plot.left() + columnWidth;
                const QRectF rect(side ? edge : edge - label.width, slot.y - labelHeight / 2,
                                  label.width, labelHeight);
                const QPointF end(edge + (side ? -3 : 3), slot.y),
                    bend(edge + (side ? -8 : 8), slot.y);
                routes.append({{label.anchor, label.radial, bend, end},
                               int(placements.size()),
                               label.percentage});
                placements.append({slot.index, rect});
            }
        leaderCandidates = routes.size();
        const auto accepted = nonCrossingChartLeaders(std::move(routes));
        leaderCount = accepted.size();
        for (const auto &route : accepted)
        {
            const auto &placement = placements[route.index];
            const auto &label = outsideLabels[placement.label];
            Batch leader{label.color, {}, true};
            for (int i = 1; i < route.points.size(); ++i)
                segment(leader, route.points[i - 1], route.points[i], 1, smoothing, false);
            batches.append(std::move(leader));
            labels.append({label.text, placement.rect, text, 12});
            delegateLabel(label.series, label.index, label.percentage);
        }
    }

    void prepare(const ChartItem *q)
    {
        if (preparing)
            return;
        QScopedValueRollback<bool> guard(preparing, true);
        dirty = false;
        delegates.requests.clear();
        QElapsedTimer timer;
        timer.start();
        batches.clear();
        labels.clear();
        legends.clear();
        hits.clear();
        curveHits.clear();
        bandHits.clear();
        sliceHits.clear();
        rendered = 0;
        leaderCandidates = leaderCount = 0;
        curveLodNodesVisited = 0;
        smoothing = q->antialiasing() && q->window() &&
                            q->window()->rendererInterface()->graphicsApi() !=
                                QSGRendererInterface::Software
                        ? 1 / q->window()->effectiveDevicePixelRatio()
                        : 0;
        curveError = .35 / (q->window() ? q->window()->effectiveDevicePixelRatio() : 1);
        tokens = q->tokens();
        background = q->backgroundColor();
        text = q->textColor();
        textFont = q->font();
        gridColor = tokens.value("border").value<QColor>();
        borderColor = tokens.value("textSecondary").value<QColor>();
        disabledColor = tokens.value("textDisabled").value<QColor>();
        tooltipColor = tokens.value("surfaceHover").value<QColor>();
        syncAxes(q);
        ranges();
        const double legendH = makeLegend(q->width(), q->height());
        layoutAxes(q, legendH);
        labels.append({title, QRectF(16, 6, std::max(1., q->width() - 32), 28), text, 18});
        if (radial())
            polar();
        else
        {
            axes(q->width());
            cartesian();
        }
        buildPickIndices();
        const int previousCount = delegates.entries.size();
        const auto previousError = delegates.error;
        const auto replaced = delegates.synchronize(const_cast<ChartItem *>(q), plot);
        for (auto &batch : batches)
        {
            for (auto it = batch.delegateInstances.crbegin(); it != batch.delegateInstances.crend();
                 ++it)
                if (replaced.contains(it->second))
                    batch.instances.removeAt(it->first);
            batch.delegateInstances.clear();
        }
        labels.removeIf([&](const Text &label) { return replaced.contains(label.delegateId); });
        if (previousCount != delegates.entries.size() || previousError != delegates.error)
            emit const_cast<ChartItem *>(q)->delegatesChanged();
        ++revision;
        prepareNs = timer.nsecsElapsed();
    }
};

ChartItem::ChartItem(QQuickItem *parent) : QQuickItem(parent), d(new Impl)
{
    d->scene3D = new ChartScene3D(this);
    const auto refresh3D = [this]
    {
        polish();
        update();
    };
    connect(d->scene3D, &ChartScene3D::changed, this, refresh3D);
    connect(d->scene3D, &ChartScene3D::cameraChanged, this, refresh3D);
    connect(d->scene3D, &ChartScene3D::materialChanged, this, refresh3D);
    connect(d->scene3D, &ChartScene3D::selectionChanged, this, refresh3D);
    connect(d->scene3D, &ChartScene3D::statusChanged, this, refresh3D);

    setFlag(ItemHasContents);
    d->appearanceAnimation = new QVariantAnimation(this);
    d->selectionAnimation = new QVariantAnimation(this);
    for (auto animation : {d->appearanceAnimation, d->selectionAnimation})
    {
        animation->setStartValue(0.);
        animation->setEndValue(1.);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QVariantAnimation::stateChanged, this,
                [this] { emit transitionChanged(); });
    }
    connect(d->appearanceAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value)
            {
                d->appearanceProgress = value.toReal();
                updateTransitionOpacity();
            });
    connect(d->selectionAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value)
            {
                d->selectionProgress = value.toReal();
                polish();
                update();
            });
    setImplicitSize(480, 320);
    setAntialiasing(true);
    setActiveFocusOnTab(true);
    const auto attachWindow = [this](QQuickWindow *window)
    {
        cancelDrag();
        cancelTouch();
        disconnect(d->sceneConnection);
        disconnect(d->activeConnection);
        if (window)
        {
            d->sceneConnection = connect(
                window, &QQuickWindow::sceneGraphInitialized, this,
                [this]
                {
                    polish();
                    update();
                },
                Qt::QueuedConnection);
            d->activeConnection = connect(window, &QWindow::activeChanged, this,
                                          [this, window]
                                          {
                                              if (!window->isActive())
                                              {
                                                  cancelZoomSelection();
                                                  cancelTouch();
                                              }
                                          });
        }
        invalidate();
    };
    connect(this, &QQuickItem::windowChanged, this, attachWindow);
    if (window())
        attachWindow(window());
    connect(this, &QQuickItem::activeFocusChanged, this,
            [this]
            {
                if (!hasActiveFocus())
                {
                    cancelZoomSelection();
                    cancelTouch();
                }
            });
    connect(this, &QQuickItem::visibleChanged, this,
            [this]
            {
                if (!isVisible())
                {
                    finishTransitions();
                    cancelDrag();
                    cancelTouch();
                }
            });
    connect(this, &QQuickItem::enabledChanged, this,
            [this]
            {
                if (!isEnabled())
                {
                    cancelDrag();
                    cancelTouch();
                }
            });
    setTheme(nullptr);
    connect(this, &ChartItem::pointHovered, this, &ChartItem::refreshDelegateStates);
    connect(this, &ChartItem::seriesChanged, this,
            [this]
            {
                if (!d->selected.isEmpty())
                    emit selectionChanged();
            });
    connect(this, &ChartItem::selectionChanged, this,
            [this]
            {
                QVariantMap hover;
                if (d->hoveredSeries)
                    hover = {{"seriesIndex", d->series.indexOf(d->hoveredSeries)},
                             {"index", d->hoveredIndex}};
                refreshDelegateStates(hover);
                startTransition(true);
                polish();
                update();
            });
    connect(this, &QQuickItem::antialiasingChanged, this, &ChartItem::invalidate);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);
    setClip(true);
    d->x = d->defaultX = new ChartAxis(this);
    d->y = d->defaultY = new ChartAxis(this);
    d->x->setOrientation(Qt::Horizontal);
    auto axisChanged = [this]
    {
        cancelZoomSelection();
        if (!d->applyingTouch)
            cancelTouch();
        invalidate();
        emit viewChanged();
    };
    connect(d->x, &ChartAxis::changed, this, axisChanged);
    connect(d->y, &ChartAxis::changed, this, axisChanged);
}
ChartItem::~ChartItem()
{
    disconnect(d->appearanceAnimation, nullptr, this, nullptr);
    disconnect(d->selectionAnimation, nullptr, this, nullptr);
    d->appearanceAnimation->stop();
    d->selectionAnimation->stop();
    // QQuickItem can emit focus/window/visibility changes after our Impl is destroyed.
    disconnect(this, nullptr, this, nullptr);
    disconnect(d->sceneConnection);
    disconnect(d->activeConnection);
    for (auto connection : d->themeConnections)
        disconnect(connection);
    for (auto list : d->connections)
        for (auto c : list)
            disconnect(c);
    for (auto list : d->axisConnections)
        for (auto c : list)
            disconnect(c);
    disconnect(d->defaultX, nullptr, this, nullptr);
    disconnect(d->defaultY, nullptr, this, nullptr);
    d->delegates.clear();
}
void ChartItem::componentComplete()
{
    QQuickItem::componentComplete();
    if (!d->customTheme)
        setTheme(nullptr);
    invalidate();
}
ThemePalette *ChartItem::theme() const
{
    return d->theme;
}
QVariantList ChartItem::radarRanges() const
{
    return d->radarRanges;
}
void ChartItem::setRadarRanges(const QVariantList &value)
{
    if (value.size() > 32)
        return;
    for (const auto &entry : value)
    {
        const auto m = entry.toMap();
        bool a = false, b = false;
        const double lo = m.value("minimum").toDouble(&a), hi = m.value("maximum").toDouble(&b);
        if (!a || !b || !std::isfinite(lo) || !std::isfinite(hi) || lo >= hi)
            return;
    }
    if (d->radarRanges == value)
        return;
    d->radarRanges = value;
    invalidate();
    emit appearanceChanged();
}
void ChartItem::setTheme(ThemePalette *value)
{
    if (value && value->thread() != thread())
        return;
    d->customTheme = value != nullptr;
    if (!value)
    {
        if (auto *engine = qmlEngine(this))
            value = engine->singletonInstance<ThemePalette *>("QCatUiKit", "Theme");
        if (!value)
            value = &d->fallback;
    }
    if (value == d->theme)
        return;
    for (auto connection : d->themeConnections)
        disconnect(connection);
    d->themeConnections.clear();
    d->theme = value;
    d->themeConnections.append(connect(value, &ThemePalette::tokensChanged, this,
                                       [this]
                                       {
                                           startTransition();
                                           invalidate();
                                           emit appearanceChanged();
                                       }));
    d->themeConnections.append(connect(value, &QObject::destroyed, this,
                                       [this]
                                       {
                                           // Never instantiate a singleton while its engine is
                                           // clearing singletons.
                                           setTheme(&d->fallback);
                                           d->customTheme = false;
                                       }));
    startTransition();
    invalidate();
    emit themeChanged();
    emit appearanceChanged();
}
QVariantMap ChartItem::tokens() const
{
    return (d->theme ? d->theme.data() : &d->fallback)->tokens();
}
QFont ChartItem::font() const
{
    if (d->customFont)
        return d->fontOverride;
    QFont font = QGuiApplication::font();
    font.setPixelSize(tokens().value("fontSize").toInt());
    return font;
}
void ChartItem::setFont(const QFont &value)
{
    if (d->customFont && value == d->fontOverride)
        return;
    d->customFont = true;
    d->fontOverride = value;
    if (d->fontOverride.pixelSize() < 0)
        d->fontOverride.setPixelSize(std::max(9, qRound(value.pointSizeF() * 96. / 72.)));
    invalidate();
    emit appearanceChanged();
}
void ChartItem::resetFont()
{
    d->customFont = false;
    invalidate();
    emit appearanceChanged();
}
void ChartItem::resetBackgroundColor()
{
    setBackgroundColor({});
}
void ChartItem::resetTextColor()
{
    setTextColor({});
}
void ChartItem::keyPressEvent(QKeyEvent *event)
{
    if (!d->interactive)
    {
        event->ignore();
        return;
    }
    switch (event->key())
    {
    case Qt::Key_Escape:
        if (!zoomSelecting() && !d->touching)
        {
            QQuickItem::keyPressEvent(event);
            return;
        }
        cancelZoomSelection();
        cancelTouch();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        zoomAt(1.2, plotRect().center());
        break;
    case Qt::Key_Minus:
        zoomAt(1 / 1.2, plotRect().center());
        break;
    case Qt::Key_Left:
        panBy({32, 0});
        break;
    case Qt::Key_Right:
        panBy({-32, 0});
        break;
    case Qt::Key_Up:
        panBy({0, 32});
        break;
    case Qt::Key_Down:
        panBy({0, -32});
        break;
    case Qt::Key_Home:
        resetView();
        break;
    default:
        QQuickItem::keyPressEvent(event);
        return;
    }
    event->accept();
}
void ChartItem::invalidate()
{
    d->dirty = true;
    polish();
    update();
}
ChartScene3D *ChartItem::scene3D() const
{
    return d->scene3D;
}
void ChartItem::updatePolish()
{
    const bool three = d->scene3D->enabled();
    if (three != d->showing3D)
    {
        cancelDrag();
        cancelTouch();
        finishTransitions();
        d->dirty = true;
        d->showing3D = three;
    }
    for (auto *item :
         {static_cast<QQuickItem *>(d->dataLayer), static_cast<QQuickItem *>(d->softwareData),
          d->delegates.dataHost.data(), d->delegates.labelHost.data()})
        if (item)
            item->setVisible(!three);
    if (d->layer3D)
        d->layer3D->setVisible(three);
    if (three && window())
    {
        const bool software =
            window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
        if (!d->overlay)
            d->overlay = new ChartOverlay(this);
        if (software && d->layer3D)
        {
            delete d->layer3D;
            d->layer3D = nullptr;
        }
        auto frame = ChartScene3DAccess::frame(d->scene3D, size(), software);
        if (!software)
        {
            if (!d->layer3D)
                d->layer3D = new ChartRhiItem3D(this, ChartScene3DAccess::statistics(d->scene3D));
            d->layer3D->setSampleCount(antialiasing() ? 4 : 1);
            d->layer3D->setSize(size());
            d->layer3D->setFrame(frame);
        }
        auto background = std::make_shared<DecorationFrame>();
        background->bounds = background->plot = boundingRect();
        background->background = backgroundColor();
        background->software = software;
        d->backgroundFrame = background;
        auto overlay = std::make_shared<DecorationFrame>();
        overlay->bounds = overlay->plot = boundingRect();
        overlay->font = font();
        overlay->software = software;
        overlay->labels.append(
            {title(), QRectF(16, 6, std::max(1., width() - 32), 28), textColor(), 18});
        auto labels3D = ChartScene3DAccess::labels(d->scene3D);
        std::stable_sort(labels3D.begin(), labels3D.end(),
                         [](const auto &a, const auto &b) { return a.priority > b.priority; });
        QFont labelFont = font();
        labelFont.setPixelSize(std::max(9, labelFont.pixelSize() - 1));
        QFontMetricsF metrics(labelFont);
        QVector<QRectF> occupied;
        for (const auto &label : labels3D)
        {
            if (!finite(label.position) ||
                !(label.priority ? boundingRect().adjusted(-24, -24, 24, 24) : boundingRect())
                     .contains(label.position) ||
                width() < 20 || height() < 60)
                continue;
            const auto text =
                metrics.elidedText(label.text, Qt::ElideRight, std::min(114., width() - 14));
            QRectF rect(0, 0, std::ceil(metrics.horizontalAdvance(text)) + 6,
                        labelFont.pixelSize() + 8);
            rect.moveCenter(label.position);
            rect.moveLeft(std::clamp(rect.left(), 4., std::max(4., width() - rect.width() - 4)));
            rect.moveTop(
                std::clamp(rect.top(), title().isEmpty() ? 4. : 36.,
                           std::max(title().isEmpty() ? 4. : 36., height() - rect.height() - 4)));
            bool overlap = false;
            for (const auto &other : occupied)
                if (other.intersects(rect.adjusted(-2, -1, 2, 1)))
                {
                    overlap = true;
                    break;
                }
            if (overlap)
                continue;
            occupied.append(rect);
            overlay->labels.append({text, rect, textColor(), 11});
        }
        if (!d->scene3D->error().isEmpty())
            overlay->labels.append({d->scene3D->error(),
                                    QRectF(12, height() / 2 - 35, std::max(1., width() - 24), 70),
                                    textColor(), 12});
        d->overlay->setSize(size());
        d->overlay->setFrame(overlay);
        d->plot = boundingRect();
        // A mode switch must republish the 2D background even if its data did not change.
        d->publishedRevision = 0;
        update();
        return;
    }
    if (d->dirty)
        d->prepare(this);
    if (d->selectionMappingChanged())
        cancelZoomSelection();
    if (d->touchMappingChanged())
        cancelTouch();
    if (!window())
        return;
    const bool software =
        window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    if (!d->overlay)
        d->overlay = new ChartOverlay(this);
    if (software && d->dataLayer)
    {
        delete d->dataLayer;
        d->dataLayer = nullptr;
    }
    if (software && !d->softwareData)
    {
        d->softwareData = new ChartOverlay(this);
        d->softwareData->setZ(0);
        d->publishedRevision = 0;
    }
    if (!software && d->softwareData)
    {
        delete d->softwareData;
        d->softwareData = nullptr;
    }
    if (!software && !d->dataLayer)
    {
        d->dataLayer = new ChartRhiItem(this, d->statistics);
        d->publishedRevision = 0;
    }
    if (d->publishedRevision != d->revision || software != d->software)
    {
        auto frame = std::make_shared<DecorationFrame>();
        frame->bounds = boundingRect();
        frame->plot = d->plot;
        frame->background = d->background;
        frame->software = software;
        frame->antialiasing = antialiasing();
        // Background and grid precede all data, including the software fallback.
        for (const auto &batch : d->batches)
            if (!batch.clip)
                frame->batches.append(batch);
        if (software)
        {
            auto dataFrame = std::make_shared<DecorationFrame>();
            dataFrame->bounds = boundingRect();
            dataFrame->plot = d->plot;
            dataFrame->software = true;
            dataFrame->antialiasing = antialiasing();
            for (const auto &batch : d->batches)
                if (batch.clip)
                    dataFrame->batches.append(batch);
            d->softwareData->setSize(size());
            d->softwareData->setFrame(std::move(dataFrame));
        }
        d->backgroundFrame = frame;
        if (d->dataLayer)
        {
            auto data = std::make_shared<ChartRenderData>();
            data->revision = d->revision;
            qsizetype count = 0;
            for (const auto &batch : d->batches)
                if (batch.clip)
                    count += batch.vertices.size();
            data->vertices.reserve(count);
            for (const auto &batch : d->batches)
                if (batch.clip && (!batch.vertices.isEmpty() || !batch.instances.isEmpty()))
                {
                    int texture = -1;
                    if (!batch.texture.isNull())
                    {
                        for (int i = 0; i < data->textures.size(); ++i)
                            if (data->textures[i].image.cacheKey() == batch.texture.cacheKey() &&
                                data->textures[i].repeat == batch.textureRepeats)
                            {
                                texture = i;
                                break;
                            }
                        if (texture < 0)
                        {
                            texture = data->textures.size();
                            data->textures.append({batch.texture, batch.textureRepeats, batch.textureId, batch.scalarTexture});
                        }
                    }
                    const int first = data->vertices.size();
                    for (qsizetype i = 0; i < batch.vertices.size(); ++i)
                    {
                        const auto point = batch.vertices[i] - d->plot.topLeft();
                        const auto color = batchPixel(batch, i);
                        const auto uv =
                            texture < 0
                                ? QPointF(.5, .5)
                                : QPointF((batch.vertices[i].x() - batch.textureRect.left()) /
                                              batch.textureRect.width(),
                                          (batch.vertices[i].y() - batch.textureRect.top()) /
                                              batch.textureRect.height());
                        data->vertices.append({float(point.x()), float(point.y()),
                                               quint8(qRed(color)), quint8(qGreen(color)),
                                               quint8(qBlue(color)), quint8(qAlpha(color)),
                                               float(uv.x()*batch.textureScale.x()), float(uv.y()*batch.textureScale.y()), batch.scalarTexture ? 1.f/batch.texture.width() : 0.f, batch.scalarTexture ? 1.f/batch.texture.height() : 0.f});
                    }
                    if (!batch.vertices.isEmpty())
                    {
                        if (!data->draws.isEmpty() && !data->draws.last().instanced &&
                            data->draws.last().texture == texture)
                            data->draws.last().count += int(batch.vertices.size());
                        else
                            data->draws.append({first, int(batch.vertices.size()), texture, false});
                    }
                    const int instanceFirst = data->instances.size();
                    for (auto value : batch.instances)
                    {
                        value.x -= float(d->plot.left());
                        value.y -= float(d->plot.top());
                        data->instances.append(value);
                    }
                    if (!batch.instances.isEmpty())
                    {
                        if (!data->draws.isEmpty() && data->draws.last().instanced &&
                            data->draws.last().texture == texture)
                            data->draws.last().count += int(batch.instances.size());
                        else
                            data->draws.append(
                                {instanceFirst, int(batch.instances.size()), texture, true});
                    }
                }
            d->dataLayer->setPosition(d->plot.topLeft());
            d->dataLayer->setSize(d->plot.size());
            d->dataLayer->setData(std::move(data));
        }
        d->publishedRevision = d->revision;
        d->software = software;
    }
    auto overlay = std::make_shared<DecorationFrame>();
    overlay->bounds = boundingRect();
    overlay->plot = d->plot;
    overlay->font = d->textFont;
    overlay->labels = d->labels;
    overlay->software = software;
    overlay->antialiasing = antialiasing();
    auto selectedColor = selectionColor();
    selectedColor.setAlphaF(selectedColor.alphaF() * d->selectionProgress);
    d->appendSelection(*overlay, selectedColor);
    if (d->selecting() && !d->selection.isEmpty())
    {
        const auto color = d->tokens.value("primary").value<QColor>();
        Batch fill{color, {}, true}, border{color, {}, true};
        fill.color.setAlphaF(fill.color.alphaF() * .16);
        rectangle(fill, d->selection);
        const auto r = d->selection;
        const double edge = std::min({1.5, r.width() / 2, r.height() / 2});
        rectangle(border, {r.left(), r.top(), r.width(), edge});
        rectangle(border, {r.left(), r.bottom() - edge, r.width(), edge});
        rectangle(border, {r.left(), r.top() + edge, edge, r.height() - 2 * edge});
        rectangle(border, {r.right() - edge, r.top() + edge, edge, r.height() - 2 * edge});
        overlay->batches << std::move(fill) << std::move(border);
    }
    if (d->plot.contains(d->hover) && !d->dragging && !d->touching)
    {
        if (d->crosshair && !d->radial())
        {
            Batch b{d->borderColor, {}, true};
            segment(b, {d->hover.x(), d->plot.top()}, {d->hover.x(), d->plot.bottom()}, 1,
                    d->smoothing);
            segment(b, {d->plot.left(), d->hover.y()}, {d->plot.right(), d->hover.y()}, 1,
                    d->smoothing);
            overlay->batches.append(std::move(b));
        }
        const auto hit = d->tooltips ? dataAt(d->hover) : QVariantMap();
        if (!hit.isEmpty())
        {
            QString content = hit.value("seriesName").toString() + " " +
                              hit.value("label").toString() +
                              "  x=" + QString::number(hit.value("x").toDouble(), 'g', 8) +
                              "  y=" + QString::number(hit.value("y").toDouble(), 'g', 8) +
                              "  value=" + QString::number(hit.value("value").toDouble(), 'g', 5);
            if (hit.contains("lower") && hit.contains("upper"))
                content += "  [" + QString::number(hit.value("lower").toDouble(), 'g', 6) + ", " +
                           QString::number(hit.value("upper").toDouble(), 'g', 6) + "]";
            if (hit.contains("percentage"))
                content += "  " + QString::number(hit.value("percentage").toDouble(), 'f', 1) + "%";
            if (hit.contains("sourceY"))
            {
                content = hit.value("seriesName").toString() +
                          "  x=" + QString::number(hit.value("sourceX").toDouble(), 'g', 8) +
                          "  y=" + QString::number(hit.value("sourceY").toDouble(), 'g', 8);
                if (hit.value("x").toDouble() == hit.value("sourceX").toDouble())
                    content +=
                        "  " + QString::number(hit.value("percentage").toDouble(), 'f', 1) + "%";
            }
            if (hit.contains("binCount"))
                content = hit.value("seriesName").toString() + " " + hit.value("label").toString() +
                          tr("  Count: %1  Height: %2")
                              .arg(hit.value("binCount").toInt())
                              .arg(hit.value("y").toDouble(), 0, 'g', 6);
            if (hit.contains("waterfallKind"))
                content = hit.value("seriesName").toString() + " " + hit.value("label").toString() +
                    tr("  Value: %1  Total: %2").arg(hit.value("value").toDouble(), 0, 'g', 6)
                        .arg(hit.value("cumulativeAfter").toDouble(), 0, 'g', 6);
            const double w = std::min(390., std::max(1., width() - 8)),
                         left = std::clamp(d->hover.x() + 12, 4., std::max(4., width() - w - 4));
            const QRectF rect(left, std::clamp(d->hover.y() - 38, 4., std::max(4., height() - 34)),
                              w, 30);
            Batch background{d->tooltipColor, {}, false};
            rectangle(background, rect);
            overlay->batches.append(std::move(background));
            overlay->labels.append({content, rect.adjusted(6, 0, -6, 0), d->text, 12});
        }
    }
    d->overlay->setSize(size());
    d->overlay->setFrame(std::move(overlay));
    updateTransitionOpacity();
}
void ChartItem::geometryChange(const QRectF &a, const QRectF &b)
{
    QQuickItem::geometryChange(a, b);
    if (a.size() != b.size())
    {
        cancelZoomSelection();
        cancelTouch();
        invalidate();
    }
}
void ChartItem::itemChange(ItemChange change, const ItemChangeData &data)
{
    QQuickItem::itemChange(change, data);
    if (change == ItemDevicePixelRatioHasChanged)
        invalidate();
}
ChartSeries::Type ChartItem::chartType() const
{
    return d->type;
}
void ChartItem::setChartType(ChartSeries::Type v)
{
    if (v >= ChartSeries::Line && v <= ChartSeries::Contour && v != d->type)
    {
        cancelDrag();
        cancelTouch();
        d->type = v;
        d->hiddenSlices.clear();
        invalidate();
        emit appearanceChanged();
    }
}
ChartAxis *ChartItem::xAxis() const
{
    return d->x;
}
ChartAxis *ChartItem::yAxis() const
{
    return d->y;
}
QString ChartItem::title() const
{
    return d->title;
}
void ChartItem::setTitle(const QString &v)
{
    if (d->title != v)
    {
        d->title = v;
        invalidate();
        emit appearanceChanged();
    }
}
#define BOOL_PROPERTY(Get, Set, Field)                                                             \
    bool ChartItem::Get() const                                                                    \
    {                                                                                              \
        return d->Field;                                                                           \
    }                                                                                              \
    void ChartItem::Set(bool v)                                                                    \
    {                                                                                              \
        if (d->Field != v)                                                                         \
        {                                                                                          \
            d->Field = v;                                                                          \
            invalidate();                                                                          \
            emit appearanceChanged();                                                              \
        }                                                                                          \
    }
BOOL_PROPERTY(legendVisible, setLegendVisible, legend)
BOOL_PROPERTY(gridVisible, setGridVisible, grid)
BOOL_PROPERTY(tooltipsEnabled, setTooltipsEnabled, tooltips)
BOOL_PROPERTY(crosshairEnabled, setCrosshairEnabled, crosshair)
BOOL_PROPERTY(multiRing, setMultiRing, multiRing)
BOOL_PROPERTY(sliceLabelsVisible, setSliceLabelsVisible, sliceLabels)
#undef BOOL_PROPERTY
ChartItem::SliceLabelPosition ChartItem::sliceLabelPosition() const
{
    return d->sliceLabelPosition;
}
void ChartItem::setSliceLabelPosition(SliceLabelPosition value)
{
    if (value < SliceLabelInside || value > SliceLabelOutside || value == d->sliceLabelPosition)
        return;
    d->sliceLabelPosition = value;
    invalidate();
    emit appearanceChanged();
}
bool ChartItem::interactive() const
{
    return d->interactive;
}
void ChartItem::setInteractive(bool value)
{
    if (d->interactive == value)
        return;
    d->interactive = value;
    if (!value)
    {
        cancelDrag();
        cancelTouch();
        d->hover = {-1, -1};
        emit pointHovered({});
    }
    invalidate();
    emit appearanceChanged();
}
bool ChartItem::touchNavigationEnabled() const
{
    return d->touchNavigation;
}
void ChartItem::setTouchNavigationEnabled(bool value)
{
    if (d->touchNavigation == value)
        return;
    cancelTouch();
    d->touchNavigation = value;
    emit appearanceChanged();
}
ChartItem::DragMode ChartItem::dragMode() const
{
    return d->dragMode;
}
void ChartItem::setDragMode(DragMode value)
{
    if (value < PanDrag || value > RectangleZoomDrag || value == d->dragMode)
        return;
    cancelDrag();
    d->dragMode = value;
    emit appearanceChanged();
}
ChartItem::ZoomAxes ChartItem::rectangleZoomAxes() const
{
    return d->zoomAxes;
}
void ChartItem::setRectangleZoomAxes(ZoomAxes value)
{
    if (value < ZoomBothAxes || value > ZoomYAxis || value == d->zoomAxes)
        return;
    cancelZoomSelection();
    d->zoomAxes = value;
    emit appearanceChanged();
}
QRectF ChartItem::zoomSelection() const
{
    return d->selection;
}
bool ChartItem::zoomSelecting() const
{
    return d->selecting();
}
void ChartItem::cancelDrag()
{
    const bool selecting = d->selecting();
    d->dragging = false;
    d->moved = false;
    d->selection = {};
    setKeepMouseGrab(false);
    if (selecting)
    {
        d->hover = {-1, -1};
        emit zoomSelectionChanged();
        polish();
        update();
    }
}
void ChartItem::cancelZoomSelection()
{
    if (d->selecting())
        cancelDrag();
}
bool ChartItem::zoomToRect(QRectF rect)
{
    if (d->radial() || !std::isfinite(rect.x()) || !std::isfinite(rect.y()) ||
        !std::isfinite(rect.width()) || !std::isfinite(rect.height()) ||
        !std::isfinite(rect.right()) || !std::isfinite(rect.bottom()))
        return false;
    if (d->dirty)
        d->prepare(this);
    rect = d->selectionRect(rect.topLeft(), rect.bottomRight());
    const bool horizontal = d->zoomAxes != ZoomYAxis, vertical = d->zoomAxes != ZoomXAxis;
    if (rect.isEmpty() || (horizontal && rect.width() < 6) || (vertical && rect.height() < 6))
        return false;
    const QPointF first((rect.left() - d->plot.left()) / d->plot.width(),
                        (d->plot.bottom() - rect.bottom()) / d->plot.height());
    const QPointF factors(d->plot.width() / rect.width(), d->plot.height() / rect.height());
    cancelZoomSelection();
    cancelTouch();
    return d->transformView(first, {0, 0}, factors, horizontal, vertical);
}
QColor ChartItem::backgroundColor() const
{
    return d->backgroundOverride.isValid() ? d->backgroundOverride
                                           : tokens().value("surface").value<QColor>();
}
void ChartItem::setBackgroundColor(const QColor &v)
{
    if (v != d->backgroundOverride)
    {
        d->backgroundOverride = v;
        invalidate();
        emit appearanceChanged();
    }
}
QColor ChartItem::textColor() const
{
    return d->textOverride.isValid() ? d->textOverride : tokens().value("text").value<QColor>();
}
void ChartItem::setTextColor(const QColor &v)
{
    if (v != d->textOverride)
    {
        d->textOverride = v;
        invalidate();
        emit appearanceChanged();
    }
}
bool ChartItem::animationsEnabled() const
{
    return d->animations;
}
void ChartItem::setAnimationsEnabled(bool value)
{
    if (value == d->animations)
        return;
    d->animations = value;
    if (!value)
        finishTransitions();
    emit appearanceChanged();
}
bool ChartItem::transitionRunning() const
{
    return d->appearanceAnimation->state() == QAbstractAnimation::Running ||
           d->selectionAnimation->state() == QAbstractAnimation::Running;
}
void ChartItem::updateTransitionOpacity()
{
    for (auto *item :
         {static_cast<QQuickItem *>(d->dataLayer), static_cast<QQuickItem *>(d->softwareData),
          static_cast<QQuickItem *>(d->overlay), d->delegates.dataHost.data(),
          d->delegates.labelHost.data()})
        if (item)
            item->setOpacity(d->appearanceProgress);
}
void ChartItem::finishTransitions()
{
    d->appearanceAnimation->stop();
    d->selectionAnimation->stop();
    d->appearanceProgress = d->selectionProgress = 1;
    updateTransitionOpacity();
    polish();
    update();
}
void ChartItem::startTransition(bool selectionOnly)
{
    const int duration = tokens().value("motionDuration", 160).toInt();
    if (!d->animations || (d->theme && d->theme->reducedMotion()) || duration <= 0 || !isVisible())
    {
        if (transitionRunning())
            finishTransitions();
        return;
    }
    if (!d->revision || !window() || !window()->isVisible())
        return;
    auto *animation = selectionOnly ? d->selectionAnimation : d->appearanceAnimation;
    animation->stop();
    if (selectionOnly && d->selected.isEmpty())
    {
        d->selectionProgress = 1;
        return;
    }
    animation->setDuration(std::clamp(duration, 1, 2000));
    animation->start();
}
ChartItem::SelectionMode ChartItem::selectionMode() const
{
    return d->selectionMode;
}
void ChartItem::setSelectionMode(SelectionMode value)
{
    if (value < NoSelection || value > MultiSelection || value == d->selectionMode)
        return;
    d->selectionMode = value;
    if (value != MultiSelection)
        clearSelection();
    emit appearanceChanged();
}
QVariantList ChartItem::selectedPoints() const
{
    QVariantList result;
    for (int si = 0; si < seriesCount(); ++si)
    {
        auto *series = seriesAt(si);
        if (!series)
            continue;
        auto indices = d->selected.value(series).values();
        std::sort(indices.begin(), indices.end());
        for (int index : indices)
            if (index < series->count())
            {
                auto point = pointMap(series, si, index);
                point.insert("selected", true);
                result.append(point);
            }
    }
    return result;
}
bool ChartItem::isPointSelected(int si, int index) const
{
    auto *series = seriesAt(si);
    return series && index >= 0 && index < series->count() &&
           d->selected.value(series).contains(index);
}
bool ChartItem::selectPoint(int si, int index, bool selected)
{
    auto *series = seriesAt(si);
    if (!series || index < 0 || index >= series->count())
        return false;
    const bool previous = d->selected.value(series).contains(index);
    if (previous == selected)
        return true;
    if (selected)
    {
        int count = 0;
        for (const auto &set : std::as_const(d->selected))
            count += set.size();
        if (count >= 4096 && d->selectionMode != SingleSelection)
            return false;
        if (d->selectionMode == SingleSelection)
            d->selected.clear();
        d->selected[series].insert(index);
    }
    else
    {
        d->selected[series].remove(index);
        if (d->selected[series].isEmpty())
            d->selected.remove(series);
    }
    emit selectionChanged();
    return true;
}
void ChartItem::clearSelection()
{
    if (d->selected.isEmpty())
        return;
    d->selected.clear();
    emit selectionChanged();
}
QColor ChartItem::selectionColor() const
{
    return d->selectionColor.isValid() ? d->selectionColor
                                       : tokens().value("warning").value<QColor>();
}
void ChartItem::setSelectionColor(const QColor &value)
{
    if (value == d->selectionColor)
        return;
    d->selectionColor = value;
    polish();
    update();
    emit appearanceChanged();
}
void ChartItem::resetSelectionColor()
{
    setSelectionColor({});
}
qreal ChartItem::selectionWidth() const
{
    return d->selectionWidth;
}
void ChartItem::setSelectionWidth(qreal value)
{
    if (!std::isfinite(value) || value < 1 || value > 8 || value == d->selectionWidth)
        return;
    d->selectionWidth = value;
    polish();
    update();
    emit appearanceChanged();
}
void ChartItem::handlePointClick(const QVariantMap &point)
{
    if (d->selectionMode != NoSelection)
    {
        if (point.isEmpty())
            clearSelection();
        else
        {
            const int si = point.value("seriesIndex", -1).toInt(),
                      index = point.value("index", -1).toInt();
            selectPoint(si, index,
                        d->selectionMode != MultiSelection || !isPointSelected(si, index));
        }
    }
    auto result = point;
    if (!result.isEmpty())
        result.insert("selected", isPointSelected(result.value("seriesIndex", -1).toInt(),
                                                  result.value("index", -1).toInt()));
    emit pointClicked(result);
}
void ChartItem::refreshDelegateStates(const QVariantMap &hover)
{
    d->hoveredSeries = seriesAt(hover.value("seriesIndex", -1).toInt());
    d->hoveredIndex = hover.value("index", -1).toInt();
    const auto entries = d->delegates.entries;
    for (const auto &entry : entries)
        if (entry.item && entry.series &&
            entry.item->metaObject()->indexOfProperty("chartData") >= 0)
        {
            auto data = entry.item->property("chartData").toMap();
            const int index = data.value("index", -1).toInt();
            const bool selected = d->selected.value(entry.series).contains(index);
            const bool hovered = entry.series == d->hoveredSeries && index == d->hoveredIndex;
            if (data.value("selected").toBool() != selected ||
                data.value("hovered").toBool() != hovered)
            {
                data.insert("selected", selected);
                data.insert("hovered", hovered);
                entry.item->setProperty("chartData", data);
            }
        }
}
int ChartItem::delegateBudget() const
{
    return d->delegates.budget;
}
void ChartItem::setDelegateBudget(int value)
{
    if (value < 0 || value > 1024 || value == d->delegates.budget)
        return;
    d->delegates.budget = value;
    invalidate();
    emit appearanceChanged();
}
int ChartItem::activeDelegateCount() const
{
    return d->delegates.entries.size();
}
QString ChartItem::delegateError() const
{
    return d->delegates.error;
}
int ChartItem::displayBudget() const
{
    return d->budget;
}
void ChartItem::setDisplayBudget(int v)
{
    if ((v == 0 || v >= 8) && v != d->budget)
    {
        d->budget = v;
        invalidate();
        emit appearanceChanged();
    }
}
qreal ChartItem::cellWidth() const
{
    return d->cellW;
}
void ChartItem::setCellWidth(qreal v)
{
    if (std::isfinite(v) && v > 0 && v != d->cellW)
    {
        d->cellW = v;
        invalidate();
        emit appearanceChanged();
    }
}
qreal ChartItem::cellHeight() const
{
    return d->cellH;
}
void ChartItem::setCellHeight(qreal v)
{
    if (std::isfinite(v) && v > 0 && v != d->cellH)
    {
        d->cellH = v;
        invalidate();
        emit appearanceChanged();
    }
}
#define RADIAL_PROPERTY(Get, Set, Field, Minimum, Maximum)                                         \
    qreal ChartItem::Get() const                                                                   \
    {                                                                                              \
        return d->Field;                                                                           \
    }                                                                                              \
    void ChartItem::Set(qreal v)                                                                   \
    {                                                                                              \
        if (std::isfinite(v) && v >= Minimum && v <= Maximum && v != d->Field)                     \
        {                                                                                          \
            d->Field = v;                                                                          \
            invalidate();                                                                          \
            emit appearanceChanged();                                                              \
        }                                                                                          \
    }
RADIAL_PROPERTY(barGroupWidth, setBarGroupWidth, barGroupWidth, 0, 1)
RADIAL_PROPERTY(barSpacing, setBarSpacing, barSpacing, 0, 1)
RADIAL_PROPERTY(pieStartAngle, setPieStartAngle, pieStart, -360, 360)
RADIAL_PROPERTY(pieEndAngle, setPieEndAngle, pieEnd, -360, 720)
RADIAL_PROPERTY(pieRadius, setPieRadius, pieRadius, 0, 1)
RADIAL_PROPERTY(holeSize, setHoleSize, hole, 0, 1)
RADIAL_PROPERTY(ringGap, setRingGap, ringGap, 0, 64)
#undef RADIAL_PROPERTY
bool ChartItem::setPieAngles(qreal start, qreal end)
{
    if (!std::isfinite(start) || !std::isfinite(end) || start < -360 || start > 360 ||
        end < start || end - start > 360 || end > 720)
        return false;
    if (d->pieStart != start || d->pieEnd != end)
    {
        d->pieStart = start;
        d->pieEnd = end;
        invalidate();
        emit appearanceChanged();
    }
    return true;
}
bool ChartItem::isSliceVisible(int si, int i) const
{
    auto *s = seriesAt(si);
    return s && i >= 0 && i < s->count() && d->sliceVisible(s, i);
}
bool ChartItem::setSliceVisible(int si, int i, bool visible)
{
    auto *s = seriesAt(si);
    if (!s || i < 0 || i >= s->count())
        return false;
    if (d->sliceVisible(s, i) != visible)
    {
        if (visible)
        {
            d->hiddenSlices[s].remove(i);
            if (d->hiddenSlices[s].isEmpty())
                d->hiddenSlices.remove(s);
        }
        else
            d->hiddenSlices[s].insert(i);
        invalidate();
        emit appearanceChanged();
    }
    return true;
}
QQmlListProperty<ChartSeries> ChartItem::series()
{
    return {this,
            this,
            [](QQmlListProperty<ChartSeries> *p, ChartSeries *s)
            { static_cast<ChartItem *>(p->data)->addSeries(s); },
            [](QQmlListProperty<ChartSeries> *p) -> qsizetype
            { return static_cast<ChartItem *>(p->data)->seriesCount(); },
            [](QQmlListProperty<ChartSeries> *p, qsizetype i)
            { return static_cast<ChartItem *>(p->data)->seriesAt(int(i)); },
            [](QQmlListProperty<ChartSeries> *p)
            { static_cast<ChartItem *>(p->data)->clearSeries(); }};
}
void ChartItem::addSeries(ChartSeries *s)
{
    if (!s || s->thread() != thread() || d->series.contains(s))
        return;
    d->series.append(s);
    auto &c = d->connections[s];
    c.append(connect(s, &ChartSeries::dataChanged, this,
                     [this, s]
                     {
                         d->discardCurve(s);
                         if (d->selected.remove(s))
                             emit selectionChanged();
                         const bool visibilityChanged = d->hiddenSlices.remove(s);
                         invalidate();
                         if (visibilityChanged)
                             emit appearanceChanged();
                     }));
    c.append(connect(s, &ChartSeries::styleChanged, this,
                     [this, s]
                     {
                         startTransition();
                         invalidate();
                         if (d->selected.contains(s))
                             emit selectionChanged();
                     }));
    c.append(connect(s, &QObject::destroyed, this,
                     [this, s]
                     {
                         d->connections.remove(s);
                         d->discardCurve(s);
                         if (d->selected.remove(s))
                             emit selectionChanged();
                         d->hiddenSlices.remove(s);
                         d->series.removeAll(nullptr);
                         invalidate();
                         emit seriesChanged();
                     }));
    invalidate();
    emit seriesChanged();
}
void ChartItem::removeSeries(ChartSeries *s)
{
    if (!d->series.removeAll(s))
        return;
    d->hiddenSlices.remove(s);
    d->discardCurve(s);
    if (d->selected.remove(s))
        emit selectionChanged();
    for (auto c : d->connections.take(s))
        disconnect(c);
    invalidate();
    emit seriesChanged();
}
void ChartItem::clearSeries()
{
    clearSelection();
    for (auto list : d->connections)
        for (auto c : list)
            disconnect(c);
    d->connections.clear();
    d->curveLayouts.clear();
    d->naturalCurves.clear();
    d->naturalLowerCurves.clear();
    d->series.clear();
    d->hiddenSlices.clear();
    invalidate();
    emit seriesChanged();
}
int ChartItem::seriesCount() const
{
    return int(d->series.size());
}
ChartSeries *ChartItem::seriesAt(int i) const
{
    return d->series.value(i);
}
void ChartItem::resetView()
{
    if (d->scene3D->enabled())
    {
        d->scene3D->resetView();
        return;
    }
    cancelTouch();
    cancelZoomSelection();
    if (d->dirty)
        d->prepare(this);
    const auto axes = d->axisLayouts;
    for (const auto &entry : axes)
        if (entry.axis)
            entry.axis->setAutomatic(true);
    invalidate();
}
bool ChartItem::setViewRange(double a, double b, double c, double e)
{
    if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c) || !std::isfinite(e) ||
        a >= b || c >= e || !std::isfinite(b - a) || !std::isfinite(e - c) ||
        (d->x->scale() == ChartAxis::Logarithmic && a <= 0) ||
        (d->y->scale() == ChartAxis::Logarithmic && c <= 0))
        return false;
    cancelZoomSelection();
    if (!d->applyingTouch)
        cancelTouch();
    d->x->setRange(a, b);
    d->y->setRange(c, e);
    return true;
}
void ChartItem::zoomAt(qreal factor, QPointF point)
{
    if (d->scene3D->enabled())
    {
        d->scene3D->zoomBy(factor);
        return;
    }
    if (!std::isfinite(factor) || factor <= 0 || !finite(point) || d->radial())
        return;
    cancelZoomSelection();
    cancelTouch();
    if (d->dirty)
        d->prepare(this);
    const QPointF anchor(std::clamp((point.x() - d->plot.left()) / d->plot.width(), 0., 1.),
                         std::clamp((d->plot.bottom() - point.y()) / d->plot.height(), 0., 1.));
    d->transformView(anchor, anchor, {factor, factor});
}
void ChartItem::panBy(QPointF pixels)
{
    if (d->scene3D->enabled())
    {
        d->scene3D->panBy(pixels);
        return;
    }
    if (d->radial() || !finite(pixels))
        return;
    cancelZoomSelection();
    cancelTouch();
    if (d->dirty)
        d->prepare(this);
    d->transformView(
        {.5, .5}, {.5 + pixels.x() / d->plot.width(), .5 - pixels.y() / d->plot.height()}, {1, 1});
}
QPointF ChartItem::mapToPosition(QPointF p) const
{
    if (d->dirty)
        d->prepare(this);
    return d->map(p.x(), p.y());
}
QPointF ChartItem::mapToValue(QPointF p) const
{
    if (d->dirty)
        d->prepare(this);
    const auto f = d->valueFractions(p);
    return {valueAt(f.x(), d->xr, d->x->scale()), valueAt(f.y(), d->yr, d->y->scale())};
}
QPointF ChartItem::mapToPosition(QPointF value, ChartSeries *series) const
{
    if (d->dirty)
        d->prepare(this);
    Impl::AxisContext context(*d, d->series.contains(series) ? series : nullptr);
    return d->map(value.x(), value.y());
}
QPointF ChartItem::mapToValue(QPointF point, ChartSeries *series) const
{
    if (d->dirty)
        d->prepare(this);
    Impl::AxisContext context(*d, d->series.contains(series) ? series : nullptr);
    const auto f = d->valueFractions(point);
    return {valueAt(f.x(), d->xr, d->x->scale()), valueAt(f.y(), d->yr, d->y->scale())};
}
ChartAxis *ChartItem::axisForSeries(ChartSeries *series, Qt::Orientation orientation) const
{
    return d->effectiveAxis(d->series.contains(series) ? series : nullptr,
                            orientation == Qt::Horizontal);
}
QRectF ChartItem::plotRect() const
{
    if (d->scene3D->enabled())
        return boundingRect();
    if (d->dirty)
        d->prepare(this);
    return d->plot;
}
qint64 ChartItem::preparationNanoseconds() const
{
    return d->prepareNs;
}
int ChartItem::renderedPointCount() const
{
    return d->rendered;
}
QVariantMap ChartItem::dataAt(QPointF at) const
{
    if (d->scene3D->enabled())
        return d->scene3D->dataAt(at);
    const auto record = [&](ChartSeries *series, int si, int index)
    {
        auto value = pointMap(series, si, index);
        value.insert("selected", d->selected.value(series).contains(index));
        return value;
    };
    if (d->preparing || !std::isfinite(at.x()) || !std::isfinite(at.y()))
        return {};
    if (d->dirty)
        d->prepare(this);
    // A delegate callback may replace the data while the prepared snapshot is being installed.
    if (d->dirty)
        return {};
    d->pickNodesVisited = d->pickCandidates = 0;
    if (!d->plot.contains(at))
        return {};
    if (d->type == ChartSeries::Pie || d->type == ChartSeries::Doughnut)
    {
        for (int candidate : d->pick(d->sliceIndex, at))
            if (const auto *it = &d->sliceHits[candidate]; true)
                if (it->series && it->contains(at))
                {
                    auto result = record(it->series, d->series.indexOf(it->series), it->index);
                    result.insert("ringIndex", it->ring);
                    result.insert("percentage", it->percentage);
                    return result;
                }
        return {};
    }

    for (int candidate : d->pick(d->hitIndex, at))
        if (const auto *it = &d->hits[candidate]; true)
            if (it->series && hitContains(*it, at))
            {
                auto result = record(it->series, d->series.indexOf(it->series), it->index);
                if (std::isfinite(it->percentage))
                    result.insert("percentage", it->percentage);
                return result;
            }
    const auto bandResult = [&](const BandHit &hit, double *score = nullptr) -> QVariantMap
    {
        const double dx = hit.upperB.x() - hit.upperA.x();
        const double t = dx == 0 ? 0 : (at.x() - hit.upperA.x()) / dx;
        const double upper = interpolate(hit.upperA, hit.upperB, std::clamp(t, 0., 1.)).y();
        const double lower = interpolate(hit.lowerA, hit.lowerB, std::clamp(t, 0., 1.)).y();
        const bool inside = dx != 0 && t >= 0 && t <= 1 && at.y() >= std::min(upper, lower) &&
                            at.y() <= std::max(upper, lower);
        const double upperDistance = segmentDistanceSquared(at, hit.upperA, hit.upperB);
        const double lowerDistance = segmentDistanceSquared(at, hit.lowerA, hit.lowerB);
        const double distance = std::min(upperDistance, lowerDistance);
        if (!inside && distance > std::pow(6 + hit.approximation, 2))
            return {};
        if (score)
            *score = inside ? -1 / (1 + distance) : distance;
        const bool onLower = lowerDistance < upperDistance;
        const double parameter = hit.from + std::clamp(t, 0., 1.) * (hit.to - hit.from);
        const int index = parameter < .5 ? hit.first : hit.second;
        auto result = record(hit.series, d->series.indexOf(hit.series), index);
        result.insert("boundary", onLower ? "lower" : "upper");
        if (onLower)
            result.insert("y", hit.series->lowerValue(index));
        return result;
    };
    QSet<QPair<ChartSeries *, int>> refinedBands;
    ChartSeries *bandSeries = nullptr;
    QVariantMap bandBest;
    double bandScore = std::numeric_limits<double>::infinity();
    for (int candidateIndex : d->pick(d->bandIndex, at))
    {
        const auto *it = &d->bandHits[candidateIndex];
        if (!it->series)
            continue;
        if (bandSeries != it->series)
        {
            if (!bandBest.isEmpty())
                return bandBest;
            bandSeries = it->series;
            bandScore = std::numeric_limits<double>::infinity();
        }
        double candidateScore = 0;
        auto candidate = bandResult(*it, &candidateScore);
        if (candidate.isEmpty())
            continue;
        if (it->approximation <= 0)
        {
            if (!d->useCurveLod(it->series))
                return candidate;
            if (candidateScore < bandScore)
            {
                bandBest = std::move(candidate);
                bandScore = candidateScore;
            }
            if (bandScore <= -1 + 1e-12)
                return bandBest;
            continue;
        }
        const QPair<ChartSeries *, int> key{it->series, it->first};
        if (refinedBands.contains(key))
            continue;
        refinedBands.insert(key);
        Impl::AxisContext context(*d, it->series);
        QVector<ChartBandPoint> path;
        QVector<int> owners;
        QVariantMap bestHit;
        double bestScore = std::numeric_limits<double>::infinity();
        // Aggregates only nominate original intervals; never return an aggregate endpoint as source
        // data.
        for (int i = it->first; i < it->second && bestScore > -1 + 1e-12; ++i)
        {
            d->bandPath(it->series, i, i + 1, path, owners);
            for (int j = 1; j < path.size(); ++j)
            {
                double score = 0;
                auto hit = bandResult(
                    {path[j - 1].upper, path[j].upper, path[j - 1].lower, path[j].lower, it->series,
                     owners.isEmpty() ? i : owners[j - 1], owners.isEmpty() ? i + 1 : owners[j - 1],
                     path[j - 1].parameter, path[j].parameter},
                    &score);
                if (!hit.isEmpty() && score < bestScore)
                {
                    bestHit = std::move(hit);
                    bestScore = score;
                }
            }
        }
        if (!bestHit.isEmpty() && bestScore < bandScore)
        {
            bandBest = std::move(bestHit);
            bandScore = bestScore;
        }
        if (bandScore <= -1 + 1e-12)
            return bandBest;
    }
    if (!bandBest.isEmpty())
        return bandBest;
    if (d->radial())
        return {};
    double best = 18 * 18;
    QVariantMap result;
    for (int si = 0; si < seriesCount(); ++si)
    {
        auto *s = seriesAt(si);
        Impl::AxisContext context(*d, s);
        if (!s || !s->isVisible() ||
            (d->seriesType(s) != ChartSeries::Line && d->seriesType(s) != ChartSeries::Area))
            continue;
        int start = 0, end = s->count();
        if (s->sortedX())
        {
            const double left = valueAt(d->valueFractions(at - QPointF(18, 0)).x(), d->xr,
                                        d->x->scale()),
                         right = valueAt(d->valueFractions(at + QPointF(18, 0)).x(), d->xr,
                                         d->x->scale());
            start = s->lowerBound(std::min(left, right));
            end = std::min(end, s->lowerBound(std::max(left, right)) + 1);
        }
        for (int i = start; i < end; ++i)
        {
            const auto &p = s->sampleAt(i);
            if (!d->valid(p.x, p.y))
                continue;
            const auto delta = d->map(p.x, p.y) - at;
            const double dist = delta.x() * delta.x() + delta.y() * delta.y();
            if (dist < best)
            {
                best = dist;
                result = record(s, si, i);
            }
        }
    }
    const auto consider = [&](ChartSeries *s, int first, int second, QPointF from, QPointF to)
    {
        Impl::AxisContext context(*d, s);
        const double distance = segmentDistanceSquared(at, from, to);
        if (distance < best)
        {
            const auto &a = s->sampleAt(first), &b = s->sampleAt(second);
            const auto da = at - d->map(a.x, a.y), db = at - d->map(b.x, b.y);
            const int index =
                QPointF::dotProduct(da, da) < QPointF::dotProduct(db, db) ? first : second;
            result = record(s, d->series.indexOf(s), index);
            best = distance;
        }
    };
    QSet<QPair<ChartSeries *, int>> refined;
    for (int candidate : d->pick(d->curveIndex, at))
        if (const auto *it = &d->curveHits[candidate]; true)
            if (it->series)
            {
                Impl::AxisContext context(*d, it->series);
                if (it->approximation <= 0)
                    consider(it->series, it->first, it->second, it->a, it->b);
                else
                {
                    const double radius = std::sqrt(best) + it->approximation;
                    if (segmentDistanceSquared(at, it->a, it->b) > radius * radius)
                        continue;
                    // LOD lines only nominate source spans. Pick on their original curves,
                    // so neither representative extrema nor a long aggregate edge invent records.
                    QVector<QPointF> path;
                    QVector<int> owners;
                    for (int i = it->first; i < it->second && best > 1e-12; ++i)
                    {
                        const QPair<ChartSeries *, int> key{it->series, i};
                        if (refined.contains(key))
                            continue;
                        refined.insert(key);
                        d->curvePath(it->series, i, i + 1, path, owners);
                        for (int j = 1; j < path.size(); ++j)
                            consider(it->series, owners.isEmpty() ? i : owners[j - 1],
                                     owners.isEmpty() ? i + 1 : owners[j - 1], path[j - 1],
                                     path[j]);
                    }
                }
            }
    return result;
}
QSGNode *ChartItem::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    return renderDecorations(old, window(), d->backgroundFrame);
}
QVariantMap ChartItem::renderingStatistics() const
{
    if (d->scene3D->enabled())
        return d->scene3D->renderingStatistics();
    auto result = d->statistics->values();
    result.insert("backend", !window() || !d->backgroundFrame ? "uninitialized"
                                                              : (d->software ? "software" : "rhi"));
    result.insert("preparedRevision", d->revision);
    result.insert("preparationNanoseconds", d->prepareNs);
    result.insert("renderedPointCount", d->rendered);
    result.insert("curveCacheBuilds", d->curveCacheBuilds);
    result.insert("curveLodNodesVisited", d->curveLodNodesVisited);
    result.insert("pickNodesVisited", d->pickNodesVisited);
    result.insert("pickCandidates", d->pickCandidates);
    result.insert("activeDelegateCount", activeDelegateCount());
    result.insert("appearanceTransitionProgress", d->appearanceProgress);
    result.insert("selectionTransitionProgress", d->selectionProgress);
    result.insert("outsideLabelCandidates", d->leaderCandidates);
    result.insert("outsideLabelsShown", d->leaderCount);
    return result;
}
void ChartItem::hoverMoveEvent(QHoverEvent *e)
{
    if (!d->interactive)
    {
        e->ignore();
        return;
    }
    d->hover = e->position();
    emit pointHovered(dataAt(d->hover));
    polish();
    update();
    e->accept();
}
void ChartItem::hoverLeaveEvent(QHoverEvent *e)
{
    d->hover = {-1, -1};
    emit pointHovered({});
    polish();
    update();
    e->accept();
}
void ChartItem::mousePressEvent(QMouseEvent *e)
{
    if (d->scene3D->enabled())
    {
        if (!isEnabled() || !d->interactive || e->button() != Qt::LeftButton)
        {
            e->ignore();
            return;
        }
        forceActiveFocus(Qt::MouseFocusReason);
        d->press = d->last = e->position();
        d->dragging = true;
        d->moved = false;
        setKeepMouseGrab(true);
        e->accept();
        return;
    }
    if (d->touching && e->source() != Qt::MouseEventNotSynthesized)
    {
        e->ignore();
        return;
    }
    cancelTouch();
    if (!isEnabled() || e->button() != Qt::LeftButton)
    {
        e->ignore();
        return;
    }
    if (d->dirty)
        d->prepare(this);
    if (d->interactive)
        for (const auto &l : d->legends)
            if (l.rect.contains(e->position()) && l.series)
            {
                if (l.slice >= 0)
                {
                    const int seriesIndex = d->series.indexOf(l.series);
                    setSliceVisible(seriesIndex, l.slice, !isSliceVisible(seriesIndex, l.slice));
                }
                else
                    l.series->setVisible(!l.series->isVisible());
                e->accept();
                return;
            }
    if (d->interactive && d->plot.contains(e->position()))
    {
        forceActiveFocus(Qt::MouseFocusReason);
        d->press = d->last = e->position();
        d->dragging = true;
        polish();
        d->moved = false;
        if (d->selecting())
        {
            d->gestureRanges = d->axisRanges;
            d->gesturePlot = d->plot;
            d->gestureX = d->xr;
            d->gestureY = d->yr;
            d->selection = {};
            d->hover = {-1, -1};
            emit pointHovered({});
            emit zoomSelectionChanged();
        }
        // A parent Flickable must not steal a chart gesture partway through it.
        setKeepMouseGrab(d->dragging && !d->radial());
        e->accept();
    }
    else
        e->ignore();
}
void ChartItem::mouseMoveEvent(QMouseEvent *e)
{
    if (d->scene3D->enabled())
    {
        if (d->dragging && d->interactive)
        {
            auto delta = e->position() - d->last;
            if ((e->position() - d->press).manhattanLength() > 3)
                d->moved = true;
            if (d->moved)
            {
                if (e->modifiers() & Qt::ShiftModifier)
                    d->scene3D->panBy(delta);
                else
                    d->scene3D->rotateBy(QPointF(delta.x() * .4, delta.y() * .4));
            }
            d->last = e->position();
            e->accept();
        }
        return;
    }
    if (d->selecting() && d->dirty)
        d->prepare(this);
    if (d->selectionMappingChanged())
        cancelZoomSelection();
    if (d->dragging)
    {
        const auto p = e->position();
        if ((p - d->press).manhattanLength() > 3)
            d->moved = true;
        if (!finite(p))
        {
            cancelDrag();
            e->ignore();
            return;
        }
        if (d->selecting())
        {
            const auto rect = d->selectionRect(d->press, p);
            if (rect != d->selection)
            {
                d->selection = rect;
                emit zoomSelectionChanged();
            }
        }
        else if (d->interactive && d->moved)
            panBy(p - d->last);
        d->last = p;
        d->hover = p;
        polish();
        update();
        e->accept();
    }
}
void ChartItem::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->scene3D->enabled())
    {
        bool clicked = d->interactive && d->dragging && !d->moved;
        cancelDrag();
        if (clicked)
        {
            ChartScene3DAccess::click(d->scene3D, e->position());
            emit pointClicked(dataAt(e->position()));
        }
        e->accept();
        return;
    }
    if (d->dirty)
        d->prepare(this);
    if (d->selectionMappingChanged())
        cancelZoomSelection();
    const bool selecting = d->selecting();
    const bool moved = d->moved || (selecting && (e->position() - d->press).manhattanLength() > 3);
    const bool clicked = d->interactive && d->dragging && !moved;
    const auto rect = QRectF(d->press, e->position());
    cancelDrag();
    if (selecting && moved && d->interactive)
        zoomToRect(rect);
    else if (clicked)
        handlePointClick(dataAt(e->position()));
    polish();
    update();
    e->accept();
}
void ChartItem::mouseUngrabEvent()
{
    const bool dragging = d->dragging;
    cancelDrag();
    if (!dragging)
        return;
    d->hover = {-1, -1};
    emit pointHovered({});
    polish();
    update();
}
void ChartItem::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (d->interactive)
        resetView();
    e->accept();
}
void ChartItem::cancelTouch()
{
    d->touch3DCount = 0;
    setKeepTouchGrab(false);
    if (!d->touching)
        return;
    d->touching = false;
    d->touchIds.clear();
    d->dirty = true; // Re-evaluate tick margins after releasing the viewport.
    d->hover = {-1, -1};
    setKeepTouchGrab(false);
    polish();
    update();
}
void ChartItem::touchUngrabEvent()
{
    cancelTouch();
}
void ChartItem::touchEvent(QTouchEvent *event)
{
    if (d->scene3D->enabled())
    {
        if (!isEnabled() || !d->interactive || !d->touchNavigation)
        {
            event->ignore();
            return;
        }
        QList<QPointF> points;
        for (const auto &p : event->points())
            if (p.state() != QEventPoint::Released && finite(p.position()))
                points.append(p.position());
        if (event->type() == QEvent::TouchCancel || points.isEmpty())
        {
            if (event->type() != QEvent::TouchCancel && d->touch3DCount == 1 && !d->touch3DMoved)
                ChartScene3DAccess::click(d->scene3D, d->touch3DCenter);
            d->touch3DCount = 0;
            setKeepTouchGrab(false);
            event->accept();
            return;
        }
        QPointF center = points[0];
        qreal distance = 0;
        if (points.size() > 1)
        {
            center = (points[0] + points[1]) * .5;
            distance = QLineF(points[0], points[1]).length();
        }
        if (!d->touch3DCount)
        {
            d->touch3DPress = center;
            d->touch3DMoved = false;
        }
        if (points.size() > 1 || (center - d->touch3DPress).manhattanLength() > 3)
            d->touch3DMoved = true;
        if (d->touch3DCount == points.size() && d->touch3DMoved)
        {
            if (points.size() == 1)
            {
                auto delta = center - d->touch3DCenter;
                d->scene3D->rotateBy(QPointF(delta.x() * .4, delta.y() * .4));
            }
            else
            {
                d->scene3D->panBy(center - d->touch3DCenter);
                if (distance > 1 && d->touch3DDistance > 1)
                    d->scene3D->zoomBy(distance / d->touch3DDistance);
            }
        }
        d->touch3DCount = points.size();
        d->touch3DCenter = center;
        d->touch3DDistance = distance;
        setKeepTouchGrab(true);
        event->accept();
        return;
    }
    if (event->type() == QEvent::TouchCancel)
    {
        cancelTouch();
        event->accept();
        return;
    }
    if (!isEnabled() || !d->interactive || d->radial())
    {
        event->ignore();
        return;
    }
    if (d->dirty)
        d->prepare(this);
    if (d->touchMappingChanged())
        cancelTouch();
    QList<QEventPoint> points;
    for (const auto &point : event->points())
    {
        if (!finite(point.position()))
        {
            cancelTouch();
            event->accept();
            return;
        }
        if (point.state() != QEventPoint::Released)
            points.append(point);
    }
    std::sort(points.begin(), points.end(),
              [](const QEventPoint &a, const QEventPoint &b) { return a.id() < b.id(); });
    // More than two contacts cancel navigation until a fresh touch sequence.
    if (points.size() > 2)
    {
        cancelTouch();
        event->ignore();
        return;
    }
    if (event->type() == QEvent::TouchBegin)
    {
        if (points.isEmpty() ||
            std::any_of(points.cbegin(), points.cend(),
                        [this](const QEventPoint &p) { return !d->plot.contains(p.position()); }))
        {
            event->ignore();
            return;
        }
        cancelDrag();
        forceActiveFocus(Qt::MouseFocusReason);
        d->touching = true;
        d->touchMoved = false;
        d->touchIds.clear();
        d->touchRanges = d->axisRanges;
        d->touchPlot = d->plot;
        d->touchX = d->xr;
        d->touchY = d->yr;
        d->hover = {-1, -1};
        setKeepTouchGrab(d->touchNavigation);
        emit pointHovered({});
        polish();
        update();
    }
    if (!d->touching)
    {
        event->ignore();
        return;
    }
    event->accept();
    if (points.isEmpty())
    {
        const auto released = event->points();
        const bool click = !d->touchMoved && released.size() == 1 &&
                           (released.first().position() - d->touchPress).manhattanLength() <=
                               QGuiApplication::styleHints()->startDragDistance();
        const auto at = released.isEmpty() ? QPointF() : released.first().position();
        cancelTouch();
        if (click)
            handlePointClick(dataAt(at));
        return;
    }
    QList<int> ids;
    for (const auto &point : points)
        ids.append(point.id());
    const QPointF center = points.size() == 1
                               ? points.first().position()
                               : (points.first().position() + points.last().position()) / 2;
    const qreal distance =
        points.size() == 2 ? QLineF(points.first().position(), points.last().position()).length()
                           : 0;
    if (points.size() == 2)
        d->touchMoved = true; // A pinch never produces a point click.
    if (ids != d->touchIds)
    {
        // Rebase on contact identity changes, including a lifted/replaced finger.
        d->touchIds = ids;
        d->touchPress = d->touchCenter = center;
        d->touchDistance = distance;
        return;
    }
    if ((center - d->touchPress).manhattanLength() >
        QGuiApplication::styleHints()->startDragDistance())
        d->touchMoved = true;
    if (!d->touchMoved || !d->touchNavigation)
        return;
    const qreal factor = distance >= 6 && d->touchDistance >= 6 ? distance / d->touchDistance : 1;
    if (center == d->touchCenter && factor == 1)
    {
        d->touchDistance = distance;
        return;
    }
    const QPointF oldFraction((d->touchCenter.x() - d->plot.left()) / d->plot.width(),
                              (d->plot.bottom() - d->touchCenter.y()) / d->plot.height());
    const QPointF newFraction((center.x() - d->plot.left()) / d->plot.width(),
                              (d->plot.bottom() - center.y()) / d->plot.height());
    const QScopedValueRollback<bool> applying(d->applyingTouch, true);
    if (d->transformView(oldFraction, newFraction, {factor, factor}) && d->touching)
    {
        d->touchCenter = center;
        d->touchDistance = distance;
    }
}

void ChartItem::wheelEvent(QWheelEvent *e)
{
    if (d->scene3D->enabled())
    {
        if (d->interactive)
        {
            const double delta =
                e->angleDelta().y() ? e->angleDelta().y() / 120. : e->pixelDelta().y() / 40.;
            d->scene3D->zoomBy(std::pow(1.2, delta));
            e->accept();
        }
        else
            e->ignore();
        return;
    }
    if (d->interactive && d->plot.contains(e->position()) && !d->radial())
    {
        const double delta =
            e->angleDelta().y() ? e->angleDelta().y() / 120. : e->pixelDelta().y() / 40.;
        zoomAt(std::pow(1.2, delta), e->position());
        e->accept();
    }
    else
        e->ignore();
}
} // namespace QCatUiKit
