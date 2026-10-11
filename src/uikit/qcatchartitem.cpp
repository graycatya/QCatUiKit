// SPDX-License-Identifier: MIT
#include "qcatchartmath_p.h"
#include <QCatUiKit/qcatchartitem.h>
#include <QElapsedTimer>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGClipNode>
#include <QSGGeometryNode>
#include <QSGRendererInterface>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QSGVertexColorMaterial>
#include <QSet>
#include <QWheelEvent>
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
};
struct Text
{
    QString text;
    QRectF rect;
    QColor color;
    int pixels = 12;
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
void segment(Batch &b, QPointF a, QPointF c, double width, qreal feather = 0)
{
    const auto delta = c - a;
    const double len = std::hypot(delta.x(), delta.y());
    if (len < 1e-9)
        return;
    const QPointF tangent = delta / len, normal(-tangent.y(), tangent.x());
    const QPointF n = normal * std::max(0., (width - feather) / 2);
    const QPointF cap = tangent * width / 2;
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
QVariantMap pointMap(ChartSeries *s, int seriesIndex, int i)
{
    auto m = s->at(i);
    m.insert("seriesIndex", seriesIndex);
    m.insert("index", i);
    m.insert("seriesName", s->name());
    return m;
}
struct Root : QSGNode
{
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
        const qreal alpha = b.color.alphaF() * b.coverage.value(i, 1);
        v[i].set(float(b.vertices[i].x()), float(b.vertices[i].y()), qRound(b.color.red() * alpha),
                 qRound(b.color.green() * alpha), qRound(b.color.blue() * alpha),
                 qRound(255 * alpha));
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
        // Text only. All chart shapes, grids and markers use SG geometry.
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
    root->appendChildNode(node);
}
} // namespace

class ChartItem::Impl
{
  public:
    ChartAxis *x = nullptr, *y = nullptr;
    QList<QPointer<ChartSeries>> series;
    QHash<ChartSeries *, QList<QMetaObject::Connection>> connections;
    ChartSeries::Type type = ChartSeries::Line;
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
                                    std::max<qreal>(.5, primary.hslSaturationF()), primary.lightnessF());
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
    qreal cellW = 1, cellH = 1, smoothing = 0;
    Range xr, yr;
    QRectF plot;
    QVector<Batch> batches;
    QVector<Text> labels;
    QVector<Legend> legends;
    QVector<Hit> hits;
    QSet<int> hiddenSlices;
    QPointF hover{-1, -1}, press, last;
    bool dragging = false, moved = false;
    qint64 prepareNs = 0;
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
    QPointF map(double a, double b) const
    {
        return {plot.left() + fraction(a, xr, x->scale()) * plot.width(),
                plot.bottom() - fraction(b, yr, y->scale()) * plot.height()};
    }
    bool valid(double a, double b) const
    {
        return (x->scale() != ChartAxis::Logarithmic || a > 0) &&
               (y->scale() != ChartAxis::Logarithmic || b > 0);
    }
    void ranges()
    {
        double ax = INFINITY, bx = -INFINITY, ay = INFINITY, by = -INFINITY;
        QMap<double, double> positive, negative;
        for (auto s : series)
        {
            if (!s || !s->isVisible())
                continue;
            const auto t = seriesType(s);
            for (int i = 0; i < s->count(); ++i)
            {
                const auto &p = s->sampleAt(i);
                if (!valid(p.x, p.y))
                    continue;
                const double dx = t == ChartSeries::Heatmap ? cellW / 2
                                  : (t == ChartSeries::Bar || t == ChartSeries::StackedBar) ? 0.5
                                                                                            : 0;
                const double dy = t == ChartSeries::Heatmap ? cellH / 2 : 0;
                ax = std::min(ax, p.x - dx);
                bx = std::max(bx, p.x + dx);
                ay = std::min(ay, p.y - dy);
                by = std::max(by, p.y + dy);
                if (t == ChartSeries::StackedBar)
                {
                    auto &sums = p.y >= 0 ? positive : negative;
                    sums[p.x] += p.y;
                    ay = std::min(ay, sums[p.x]);
                    by = std::max(by, sums[p.x]);
                }
            }
            if ((t == ChartSeries::Bar || t == ChartSeries::StackedBar || t == ChartSeries::Area) &&
                y->scale() != ChartAxis::Logarithmic)
            {
                ay = std::min(ay, 0.);
                by = std::max(by, 0.);
            }
        }
        if (x->scale() == ChartAxis::Logarithmic && ax <= 0)
            ax = 0.1;
        if (y->scale() == ChartAxis::Logarithmic && ay <= 0)
            ay = 0.1;
        xr = x->automatic() ? expanded(ax, bx, x->scale() == ChartAxis::Logarithmic)
                            : Range{x->minimum(), x->maximum()};
        yr = y->automatic() ? expanded(ay, by, y->scale() == ChartAxis::Logarithmic)
                            : Range{y->minimum(), y->maximum()};
        if (y->automatic() && ay != by && std::isfinite(ay) && y->scale() != ChartAxis::Logarithmic)
        {
            const double pad = (yr.max - yr.min) * .05;
            yr.min -= pad;
            yr.max += pad;
        }
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
                for (int i = 0; i < std::min(s->count(), 64); ++i)
                {
                    const auto &p = s->sampleAt(i);
                    entries.append({p.label.isEmpty() ? QString::number(i + 1) : p.label,
                                    palette(i), s, i, !hiddenSlices.contains(i)});
                }
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
    void axes()
    {
        Batch gridBatch{gridColor, {}, false}, border{borderColor, {}, false};
        for (double t : ticks(xr, x->scale(), x->tickCount()))
        {
            const double px = map(t, yr.min).x();
            if (grid)
                segment(gridBatch, {px, plot.top()}, {px, plot.bottom()}, 1, smoothing);
            segment(border, {px, plot.bottom()}, {px, plot.bottom() + 4}, 1, smoothing);
            labels.append({label(t, x, xr), QRectF(px - 45, plot.bottom() + 6, 90, 22), text, 11});
        }
        for (double t : ticks(yr, y->scale(), y->tickCount()))
        {
            const double py = map(xr.min, t).y();
            if (grid)
                segment(gridBatch, {plot.left(), py}, {plot.right(), py}, 1, smoothing);
            segment(border, {plot.left() - 4, py}, {plot.left(), py}, 1, smoothing);
            labels.append({label(t, y, yr), QRectF(0, py - 11, plot.left() - 8, 22), text, 11});
        }
        segment(border, plot.topLeft(), plot.bottomLeft(), 1, smoothing);
        segment(border, plot.bottomLeft(), plot.bottomRight(), 1, smoothing);
        batches.append(gridBatch);
        batches.append(border);
        labels.append(
            {x->title(), QRectF(plot.left(), plot.bottom() + 30, plot.width(), 22), text, 12});
        labels.append({y->title(), QRectF(4, plot.top() - 26, std::max(60., plot.left() + 100), 22),
                       text, 12});
    }
    void cartesian()
    {
        int bars = 0;
        for (auto s : series)
            if (s && s->isVisible() && seriesType(s) == ChartSeries::Bar)
                ++bars;
        int barIndex = 0;
        QMap<double, double> pos, neg;
        for (auto s : series)
        {
            if (!s || !s->isVisible() || !s->count())
                continue;
            const auto t = seriesType(s);
            // Only ordered curves use extrema LOD. Sampling bars independently
            // would corrupt stacks; sampling cells/markers would hide categories.
            auto indices = visibleIndices(
                s, xr, (t == ChartSeries::Line || t == ChartSeries::Area) ? budget : 0);
            Batch b{seriesColor(s), {}, true};
            Batch fill{seriesColor(s), {}, true};
            fill.color.setAlpha(65);
            const double baseline =
                y->scale() == ChartAxis::Logarithmic ? yr.min : std::clamp(0., yr.min, yr.max);
            bool prevValid = false;
            QPointF prev;
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
                    if (prevValid)
                    {
                        segment(b, prev, q, s->lineWidth(), smoothing);
                        if (t == ChartSeries::Area)
                        {
                            const double by = map(p.x, baseline).y();
                            triangle(fill, prev, {prev.x(), by}, q);
                            triangle(fill, q, {prev.x(), by}, {q.x(), by});
                        }
                    }
                    prev = q;
                    prevValid = true;
                }
                else if (t == ChartSeries::Scatter || t == ChartSeries::Bubble)
                {
                    const double r = t == ChartSeries::Bubble ? std::clamp(p.value, 1., 64.) : 4.;
                    sector(b, q, 0, r, 0, 2 * pi, smoothing);
                    hits.append({QRectF(q - QPointF(r, r), QSizeF(2 * r, 2 * r)), s, index});
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
                    rectangle(cell, r);
                    hits.append({r, s, index});
                }
                else if (t == ChartSeries::Bar || t == ChartSeries::StackedBar)
                {
                    double bottom = baseline, top = p.y, offset = 0, w = .8;
                    if (t == ChartSeries::Bar)
                    {
                        w = .8 / std::max(1, bars);
                        offset = -.4 + w * (barIndex + .5);
                    }
                    else
                    {
                        auto &sums = p.y >= 0 ? pos : neg;
                        bottom = sums[p.x];
                        top = bottom + p.y;
                        sums[p.x] = top;
                    }
                    if (!valid(p.x + offset - w / 2, bottom) || !valid(p.x + offset + w / 2, top))
                        continue;
                    auto r =
                        QRectF(map(p.x + offset - w / 2, bottom), map(p.x + offset + w / 2, top))
                            .normalized();
                    rectangle(b, r);
                    hits.append({r, s, index});
                }
            }
            if (t == ChartSeries::Bar)
                ++barIndex;
            if (t == ChartSeries::Area)
                batches.append(fill);
            batches.append(b);
            for (auto it = cells.cbegin(); it != cells.cend(); ++it)
                batches.append(it.value());
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
                    for (int i = 0; i < s->count(); ++i)
                        maximum = std::max(maximum, s->sampleAt(i).y);
                }
            if (n < 3)
                return;
            maximum = y->automatic() ? std::max(1., maximum) : y->maximum();
            if (maximum <= 0)
                return;
            auto vertex = [&](int i, double v)
            {
                const double a = -pi / 2 + 2 * pi * i / n;
                return center +
                       QPointF(std::cos(a), std::sin(a)) * radius * std::clamp(v / maximum, 0., 1.);
            };
            Batch gridBatch{gridColor, {}, false};
            for (int i = 0; i < n; ++i)
            {
                if (grid)
                {
                    segment(gridBatch, center, vertex(i, maximum), 1, smoothing);
                    for (int ring = 1; ring <= 4; ++ring)
                        segment(gridBatch, vertex(i, maximum * ring / 4),
                                vertex((i + 1) % n, maximum * ring / 4), 1, smoothing);
                }
                const auto p = center + (vertex(i, maximum) - center) * 1.13;
                labels.append({x->categories().value(i, QString::number(i + 1)),
                               QRectF(p.x() - 40, p.y() - 11, 80, 22), text, 12});
            }
            batches.append(gridBatch);
            for (auto s : series)
                if (s && s->isVisible())
                {
                    Batch b{seriesColor(s), {}, true}, f{seriesColor(s), {}, true};
                    f.color.setAlpha(55);
                    for (int i = 0; i < n; ++i)
                    {
                        const auto p = vertex(i, s->sampleAt(i).y),
                                   next = vertex((i + 1) % n, s->sampleAt((i + 1) % n).y);
                        triangle(f, center, p, next);
                        segment(b, p, next, 2, smoothing);
                        sector(b, p, 0, 3, 0, 2 * pi, smoothing);
                        if (i < s->count())
                            hits.append({QRectF(p - QPointF(7, 7), QSizeF(14, 14)), s, i});
                        ++rendered;
                    }
                    batches.append(f);
                    batches.append(b);
                }
            return;
        }
        for (auto s : series)
        {
            if (!s || !s->isVisible())
                continue;
            double total = 0;
            for (int i = 0; i < s->count(); ++i)
                if (!hiddenSlices.contains(i))
                    total += std::max(0., s->sampleAt(i).y);
            if (total <= 0)
                break;
            double a = -pi / 2;
            for (int i = 0; i < s->count(); ++i)
            {
                const auto &p = s->sampleAt(i);
                if (hiddenSlices.contains(i) || p.y <= 0)
                    continue;
                const double z = a + 2 * pi * p.y / total;
                Batch b{palette(i), {}, true};
                sector(b, center, type == ChartSeries::Doughnut ? radius * .58 : 0, radius, a, z,
                       smoothing);
                batches.append(b);
                ++rendered;
                if ((z - a) > .15)
                {
                    const double mid = (a + z) / 2;
                    const auto at = center + QPointF(std::cos(mid), std::sin(mid)) * radius * .8;
                    labels.append({QString::number(p.y / total * 100, 'f', 0) + "%",
                                   QRectF(at.x() - 25, at.y() - 11, 50, 22),
                                   contrastingText(palette(i)), 12});
                }
                a = z;
            }
            if (type == ChartSeries::Doughnut)
                labels.append({QString::number(total, 'g', 6),
                               QRectF(center.x() - radius * .5, center.y() - 15, radius, 30), text,
                               20});
            break;
        }
    }
    void prepare(const ChartItem *q)
    {
        QElapsedTimer timer;
        timer.start();
        batches.clear();
        labels.clear();
        legends.clear();
        hits.clear();
        rendered = 0;
        smoothing = q->antialiasing() && q->window() &&
                            q->window()->rendererInterface()->graphicsApi() !=
                                QSGRendererInterface::Software
                        ? 1 / q->window()->effectiveDevicePixelRatio()
                        : 0;
        tokens = q->tokens();
        background = q->backgroundColor();
        text = q->textColor();
        textFont = q->font();
        gridColor = tokens.value("border").value<QColor>();
        borderColor = tokens.value("textSecondary").value<QColor>();
        disabledColor = tokens.value("textDisabled").value<QColor>();
        tooltipColor = tokens.value("surfaceHover").value<QColor>();
        ranges();
        const double legendH = makeLegend(q->width(), q->height());
        const double top = title.isEmpty() ? 38 : 60;
        plot = QRectF(radial() ? 20 : 76, top, std::max(1., q->width() - (radial() ? 40 : 100)),
                      std::max(1., q->height() - top - legendH - (radial() ? 15 : 60)));
        labels.append({title, QRectF(16, 6, std::max(1., q->width() - 32), 28), text, 18});
        if (radial())
            polar();
        else
        {
            axes();
            cartesian();
        }
        dirty = false;
        prepareNs = timer.nsecsElapsed();
    }
};

ChartItem::ChartItem(QQuickItem *parent) : QQuickItem(parent), d(new Impl)
{
    setFlag(ItemHasContents);
    setImplicitSize(480, 320);
    setAntialiasing(true);
    setActiveFocusOnTab(true);
    connect(this, &QQuickItem::windowChanged, this, [this] { invalidate(); });
    setTheme(nullptr);
    connect(this, &QQuickItem::antialiasingChanged, this, &ChartItem::invalidate);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setClip(true);
    d->x = new ChartAxis(this);
    d->y = new ChartAxis(this);
    auto axisChanged = [this]
    {
        invalidate();
        emit viewChanged();
    };
    connect(d->x, &ChartAxis::changed, this, axisChanged);
    connect(d->y, &ChartAxis::changed, this, axisChanged);
}
ChartItem::~ChartItem()
{
    for (auto connection : d->themeConnections)
        disconnect(connection);
    for (auto list : d->connections)
        for (auto c : list)
            disconnect(c);
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
void ChartItem::updatePolish()
{
    if (d->dirty)
        d->prepare(this);
}
void ChartItem::geometryChange(const QRectF &a, const QRectF &b)
{
    QQuickItem::geometryChange(a, b);
    if (a.size() != b.size())
        invalidate();
}
ChartSeries::Type ChartItem::chartType() const
{
    return d->type;
}
void ChartItem::setChartType(ChartSeries::Type v)
{
    if (v >= ChartSeries::Line && v <= ChartSeries::Heatmap && v != d->type)
    {
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
BOOL_PROPERTY(interactive, setInteractive, interactive)
#undef BOOL_PROPERTY
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
    c.append(connect(s, &ChartSeries::dataChanged, this, &ChartItem::invalidate));
    c.append(connect(s, &ChartSeries::styleChanged, this, &ChartItem::invalidate));
    c.append(connect(s, &QObject::destroyed, this,
                     [this, s]
                     {
                         d->connections.remove(s);
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
    for (auto c : d->connections.take(s))
        disconnect(c);
    invalidate();
    emit seriesChanged();
}
void ChartItem::clearSeries()
{
    for (auto list : d->connections)
        for (auto c : list)
            disconnect(c);
    d->connections.clear();
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
    d->x->setAutomatic(true);
    d->y->setAutomatic(true);
    invalidate();
}
bool ChartItem::setViewRange(double a, double b, double c, double e)
{
    if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c) || !std::isfinite(e) ||
        a >= b || c >= e || !std::isfinite(b - a) || !std::isfinite(e - c) ||
        (d->x->scale() == ChartAxis::Logarithmic && a <= 0) ||
        (d->y->scale() == ChartAxis::Logarithmic && c <= 0))
        return false;
    d->x->setRange(a, b);
    d->y->setRange(c, e);
    return true;
}
void ChartItem::zoomAt(qreal f, QPointF p)
{
    if (!std::isfinite(f) || f <= 0 || d->radial())
        return;
    if (d->dirty)
        d->prepare(this);
    const double fx = std::clamp((p.x() - d->plot.left()) / d->plot.width(), 0., 1.),
                 fy = std::clamp((d->plot.bottom() - p.y()) / d->plot.height(), 0., 1.);
    setViewRange(valueAt(fx - fx / f, d->xr, d->x->scale()),
                 valueAt(fx + (1 - fx) / f, d->xr, d->x->scale()),
                 valueAt(fy - fy / f, d->yr, d->y->scale()),
                 valueAt(fy + (1 - fy) / f, d->yr, d->y->scale()));
}
void ChartItem::panBy(QPointF p)
{
    if (d->radial() || !std::isfinite(p.x()) || !std::isfinite(p.y()))
        return;
    if (d->dirty)
        d->prepare(this);
    const double dx = -p.x() / d->plot.width(), dy = p.y() / d->plot.height();
    setViewRange(valueAt(dx, d->xr, d->x->scale()), valueAt(1 + dx, d->xr, d->x->scale()),
                 valueAt(dy, d->yr, d->y->scale()), valueAt(1 + dy, d->yr, d->y->scale()));
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
    return {valueAt((p.x() - d->plot.left()) / d->plot.width(), d->xr, d->x->scale()),
            valueAt((d->plot.bottom() - p.y()) / d->plot.height(), d->yr, d->y->scale())};
}
QRectF ChartItem::plotRect() const
{
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
    if (!std::isfinite(at.x()) || !std::isfinite(at.y()))
        return {};
    if (d->dirty)
        d->prepare(this);
    if (!d->plot.contains(at))
        return {};
    if (d->type == ChartSeries::Pie || d->type == ChartSeries::Doughnut)
    {
        const auto delta = at - d->plot.center();
        const double r = std::hypot(delta.x(), delta.y()),
                     outer = std::min(d->plot.width(), d->plot.height()) * .42;
        if (r > outer || (d->type == ChartSeries::Doughnut && r < outer * .58))
            return {};
        double angle = std::atan2(delta.y(), delta.x()) + pi / 2;
        if (angle < 0)
            angle += 2 * pi;
        for (int si = 0; si < seriesCount(); ++si)
        {
            auto *s = seriesAt(si);
            if (!s || !s->isVisible())
                continue;
            double total = 0;
            for (int i = 0; i < s->count(); ++i)
                if (!d->hiddenSlices.contains(i))
                    total += std::max(0., s->sampleAt(i).y);
            double end = 0;
            if (total <= 0)
                return {};
            for (int i = 0; i < s->count(); ++i)
            {
                if (d->hiddenSlices.contains(i) || s->sampleAt(i).y <= 0)
                    continue;
                end += s->sampleAt(i).y / total * 2 * pi;
                if (angle <= end)
                    return pointMap(s, si, i);
            }
            return {};
        }
        return {};
    }
    for (auto it = d->hits.crbegin(); it != d->hits.crend(); ++it)
        if (it->series && it->rect.contains(at))
            return pointMap(it->series, d->series.indexOf(it->series), it->index);
    if (d->radial())
        return {};
    double best = 18 * 18;
    QVariantMap result;
    for (int si = 0; si < seriesCount(); ++si)
    {
        auto *s = seriesAt(si);
        if (!s || !s->isVisible() ||
            (d->seriesType(s) != ChartSeries::Line && d->seriesType(s) != ChartSeries::Area))
            continue;
        int start = 0, end = s->count();
        if (s->sortedX())
        {
            const double left = mapToValue(at - QPointF(18, 0)).x(),
                         right = mapToValue(at + QPointF(18, 0)).x();
            start = s->lowerBound(left);
            end = std::min(end, s->lowerBound(right) + 1);
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
                result = pointMap(s, si, i);
            }
        }
    }
    return result;
}
QSGNode *ChartItem::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    if (!window())
        return old;
    auto *root = static_cast<Root *>(old);
    if (!root)
        root = new Root;
    while (root->firstChild())
        delete root->firstChild();
    delete root->softwareTexture;
    root->softwareTexture = nullptr;
    root->appendChildNode(new QSGSimpleRectNode(boundingRect(), d->background));
    const bool software =
        window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    QImage softwareImage;
    QPainter painter;
    if (software)
    {
        const qreal dpr = window()->effectiveDevicePixelRatio();
        softwareImage =
            QImage(QSize(std::max(1, qCeil(width() * dpr)), std::max(1, qCeil(height() * dpr))),
                   QImage::Format_ARGB32_Premultiplied);
        softwareImage.setDevicePixelRatio(dpr);
        softwareImage.fill(Qt::transparent);
        painter.begin(&softwareImage);
        painter.setRenderHint(QPainter::Antialiasing, antialiasing());
    }
    auto renderBatch = [&](QSGNode *parent, const Batch &batch)
    {
        if (!software)
        {
            addBatch(parent, batch);
            return;
        }
        painter.save();
        if (batch.clip)
            painter.setClipRect(d->plot);
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        for (int i = 0; i + 2 < batch.vertices.size(); i += 3)
        {
            path.moveTo(batch.vertices[i]);
            path.lineTo(batch.vertices[i + 1]);
            path.lineTo(batch.vertices[i + 2]);
            path.closeSubpath();
        }
        painter.fillPath(path, batch.color);
        painter.restore();
    };
    for (const auto &b : d->batches)
        if (!b.clip)
            renderBatch(root, b);
    auto *clip = new QSGClipNode;
    clip->setIsRectangular(true);
    clip->setClipRect(d->plot);
    root->appendChildNode(clip);
    for (const auto &b : d->batches)
        if (b.clip)
            renderBatch(clip, b);
    if (software && d->crosshair && !d->radial() && d->plot.contains(d->hover) && !d->dragging)
    {
        painter.save();
        painter.setClipRect(d->plot);
        painter.setPen(d->borderColor);
        painter.drawLine(QPointF(d->hover.x(), d->plot.top()),
                         QPointF(d->hover.x(), d->plot.bottom()));
        painter.drawLine(QPointF(d->plot.left(), d->hover.y()),
                         QPointF(d->plot.right(), d->hover.y()));
        painter.restore();
    }
    if (software)
    {
        painter.end();
        root->softwareTexture = window()->createTextureFromImage(softwareImage);
        if (root->softwareTexture)
        {
            auto *node = new QSGSimpleTextureNode;
            node->setTexture(root->softwareTexture);
            node->setRect(boundingRect());
            root->appendChildNode(node);
        }
    }
    QSet<QString> used;
    for (const auto &text : d->labels)
        addText(root, window(), text, d->textFont, used);
    if (d->plot.contains(d->hover) && !d->dragging)
    {
        if (!software && d->crosshair && !d->radial())
        {
            Batch b{d->borderColor, {}, true};
            segment(b, {d->hover.x(), d->plot.top()}, {d->hover.x(), d->plot.bottom()}, 1,
                    d->smoothing);
            segment(b, {d->plot.left(), d->hover.y()}, {d->plot.right(), d->hover.y()}, 1,
                    d->smoothing);
            renderBatch(clip, b);
        }
        const auto hit = dataAt(d->hover);
        if (d->tooltips && !hit.isEmpty())
        {
            const QString content =
                hit.value("seriesName").toString() + " " + hit.value("label").toString() +
                "  x=" + QString::number(hit.value("x").toDouble(), 'g', 8) +
                "  y=" + QString::number(hit.value("y").toDouble(), 'g', 8) +
                "  value=" + QString::number(hit.value("value").toDouble(), 'g', 5);
            const double w = std::min(390., std::max(1., width() - 8)),
                         left = std::clamp(d->hover.x() + 12, 4., std::max(4., width() - w - 4));
            const QRectF rect(left, std::clamp(d->hover.y() - 38, 4., std::max(4., height() - 34)),
                              w, 30);
            root->appendChildNode(new QSGSimpleRectNode(rect, d->tooltipColor));
            addText(root, window(), {content, rect.adjusted(6, 0, -6, 0), d->text, 12}, d->textFont,
                    used);
        }
    }
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
void ChartItem::hoverMoveEvent(QHoverEvent *e)
{
    if (!d->interactive)
    {
        e->ignore();
        return;
    }
    d->hover = e->position();
    emit pointHovered(dataAt(d->hover));
    update();
    e->accept();
}
void ChartItem::hoverLeaveEvent(QHoverEvent *e)
{
    d->hover = {-1, -1};
    emit pointHovered({});
    update();
    e->accept();
}
void ChartItem::mousePressEvent(QMouseEvent *e)
{
    if (d->interactive)
        for (const auto &l : d->legends)
            if (l.rect.contains(e->position()) && l.series)
            {
                if (l.slice >= 0)
                {
                    if (d->hiddenSlices.contains(l.slice))
                        d->hiddenSlices.remove(l.slice);
                    else
                        d->hiddenSlices.insert(l.slice);
                    invalidate();
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
        d->moved = false;
        e->accept();
    }
    else
        e->ignore();
}
void ChartItem::mouseMoveEvent(QMouseEvent *e)
{
    if (d->dragging)
    {
        const auto p = e->position();
        if ((p - d->press).manhattanLength() > 3)
            d->moved = true;
        if (d->interactive && d->moved)
            panBy(p - d->last);
        d->last = p;
        d->hover = p;
        update();
        e->accept();
    }
}
void ChartItem::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->dragging && !d->moved)
        emit pointClicked(dataAt(e->position()));
    d->dragging = false;
    update();
    e->accept();
}
void ChartItem::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (d->interactive)
        resetView();
    e->accept();
}
void ChartItem::wheelEvent(QWheelEvent *e)
{
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
