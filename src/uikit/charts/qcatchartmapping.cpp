// SPDX-License-Identifier: Apache-2.0
#include "qcatchartbubble.h"
#include "qcatchartcolorscale.h"
#include "qcatchartgrid.h"
#include "qcatchartstatistics_p.h"
#include <QThread>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
namespace QCatUiKit
{
namespace
{
bool finiteValue(const QVariant &v, double &n)
{
    bool ok = false;
    n = v.toDouble(&ok);
    return ok && std::isfinite(n);
}
QVariantMap scaleVisual(ChartColorScale *scale)
{
    QVariantList palette;
    for (int i = 0; i < 256; ++i)
        palette.append(
            scale->colorAt(scale->minimum() * (1 - i / 255.) + scale->maximum() * (i / 255.)));
    return {{"palette", palette},
            {"minimum", scale->minimum()},
            {"maximum", scale->maximum()},
            {"legend", scale->legendVisible()}};
}
} // namespace
class ChartColorScale::Impl
{
  public:
    bool automatic = true, legend = true;
    double minimum = 0, maximum = 1, dataMin = 0, dataMax = 1;
    QVariantList colors{QColor("#2445a8"), QColor("#20b5b1"), QColor("#f9d057")};
};
ChartColorScale::ChartColorScale(QObject *p) : QObject(p), d(std::make_unique<Impl>()) {}
ChartColorScale::~ChartColorScale() = default;
bool ChartColorScale::automatic() const
{
    return d->automatic;
}
void ChartColorScale::setAutomatic(bool v)
{
    if (QThread::currentThread() != thread() || v == d->automatic)
        return;
    d->automatic = v;
    setDataRange(d->dataMin, d->dataMax);
    emit changed();
}
double ChartColorScale::minimum() const
{
    return d->minimum;
}
double ChartColorScale::maximum() const
{
    return d->maximum;
}
bool ChartColorScale::setRange(double a, double b)
{
    if (QThread::currentThread() != thread() || !std::isfinite(a) || !std::isfinite(b) || a > b)
        return false;
    d->automatic = false;
    d->minimum = a;
    d->maximum = b;
    emit rangeChanged();
    emit changed();
    return true;
}
void ChartColorScale::setDataRange(double a, double b)
{
    if (QThread::currentThread() != thread())
        return;
    if (!std::isfinite(a) || !std::isfinite(b) || a > b)
    {
        a = 0;
        b = 1;
    }
    d->dataMin = a;
    d->dataMax = b;
    if (d->automatic && (d->minimum != a || d->maximum != b))
    {
        d->minimum = a;
        d->maximum = b;
        emit rangeChanged();
    }
}
QVariantList ChartColorScale::colors() const
{
    return d->colors;
}
void ChartColorScale::setColors(const QVariantList &v)
{
    if (QThread::currentThread() != thread() || v.size() < 2 || v.size() > 32)
        return;
    QVariantList colors;
    for (const auto &entry : v)
    {
        const QColor c =
            entry.canConvert<QColor>() ? entry.value<QColor>() : QColor(entry.toString());
        if (!c.isValid())
            return;
        colors.append(c);
    }
    if (colors != d->colors)
    {
        d->colors = colors;
        emit changed();
    }
}
bool ChartColorScale::legendVisible() const
{
    return d->legend;
}
void ChartColorScale::setLegendVisible(bool v)
{
    if (QThread::currentThread() == thread() && v != d->legend)
    {
        d->legend = v;
        emit changed();
    }
}
double ChartColorScale::normalized(double v) const
{
    if (!std::isfinite(v))
        return std::numeric_limits<double>::quiet_NaN();
    if (d->minimum == d->maximum)
        return .5;
    if (v <= d->minimum)
        return 0;
    if (v >= d->maximum)
        return 1;
    const double span = d->maximum - d->minimum;
    return std::isfinite(span) ? (v - d->minimum) / span
                               : (v / 2 - d->minimum / 2) / (d->maximum / 2 - d->minimum / 2);
}
QColor ChartColorScale::colorAt(double v) const
{
    const double t = normalized(v);
    if (!std::isfinite(t))
        return Qt::transparent;
    const double f = t * (d->colors.size() - 1);
    const int i = std::min(int(f), int(d->colors.size() - 2));
    const double u = f - i;
    const auto a = d->colors[i].value<QColor>(), b = d->colors[i + 1].value<QColor>();
    const double alpha = a.alphaF() * (1 - u) + b.alphaF() * u;
    if (alpha <= 0)
        return Qt::transparent;
    const auto mix = [&](double x, double y)
    { return (x * a.alphaF() * (1 - u) + y * b.alphaF() * u) / alpha; };
    return QColor::fromRgbF(mix(a.redF(), b.redF()), mix(a.greenF(), b.greenF()),
                            mix(a.blueF(), b.blueF()), alpha);
}
class ChartBubble::Impl
{
  public:
    QVariantList points;
    SizeMode mode = Radius;
    double scale = 1;
    bool mapping = true;
    ChartSeries *series = nullptr;
    ChartColorScale *colors = nullptr;
    QString error;
};
ChartBubble::ChartBubble(QObject *p) : QObject(p), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Bubble);
    d->colors = new ChartColorScale(this);
    connect(d->colors, &ChartColorScale::changed, this, [this] { setData(d->points); });
}
ChartBubble::~ChartBubble() = default;
QVariantList ChartBubble::points() const
{
    return d->points;
}
void ChartBubble::setPoints(const QVariantList &v)
{
    setData(v);
}
ChartBubble::SizeMode ChartBubble::sizeMode() const
{
    return d->mode;
}
void ChartBubble::setSizeMode(SizeMode v)
{
    if (QThread::currentThread() != thread() || v < Radius || v > Area || v == d->mode)
        return;
    d->mode = v;
    setData(d->points);
    emit appearanceChanged();
}
double ChartBubble::sizeScale() const
{
    return d->scale;
}
void ChartBubble::setSizeScale(double v)
{
    if (QThread::currentThread() != thread() || !std::isfinite(v) || v <= 0 || v > 10000 ||
        v == d->scale)
        return;
    d->scale = v;
    setData(d->points);
    emit appearanceChanged();
}
bool ChartBubble::colorMapping() const
{
    return d->mapping;
}
void ChartBubble::setColorMapping(bool v)
{
    if (QThread::currentThread() != thread() || v == d->mapping)
        return;
    d->mapping = v;
    setData(d->points);
    emit appearanceChanged();
}
ChartColorScale *ChartBubble::colorScale() const
{
    return d->colors;
}
ChartSeries *ChartBubble::series() const
{
    return d->series;
}
QString ChartBubble::error() const
{
    return d->error;
}
bool ChartBubble::setData(const QVariantList &input)
{
    if (QThread::currentThread() != thread())
        return false;
    const auto fail = [this]
    {
        d->error = tr("Bubble points require finite x, y, nonnegative size and finite colorValue "
                      "(at most 1,000,000 points).");
        emit errorChanged();
        return false;
    };
    if (input.size() > 1000000)
        return fail();
    double low = INFINITY, high = -INFINITY;
    for (const auto &entry : input)
    {
        const auto m = entry.toMap();
        double x, y, size, color;
        if (!finiteValue(m.value("x"), x) || !finiteValue(m.value("y"), y) ||
            !finiteValue(m.value("size", 1), size) || size < 0 ||
            !finiteValue(m.value("colorValue", size), color))
            return fail();
        low = std::min(low, color);
        high = std::max(high, color);
    }
    d->colors->setDataRange(low, high);
    QVariantList points;
    for (int i = 0; i < input.size(); ++i)
    {
        const auto m = input[i].toMap();
        const double size = m.value("size", 1).toDouble(),
                     color = m.value("colorValue", size).toDouble();
        const double radius =
            d->mode == Radius
                ? std::min(64., size * d->scale)
                : std::min(64., std::sqrt(size / 3.14159265358979323846) * std::sqrt(d->scale));
        QVariantMap meta{
            {"sourceIndex", i}, {"rawSize", size}, {"colorValue", color}, {"radius", radius}};
        if (d->mapping)
            meta.insert("mappedColor", d->colors->colorAt(color));
        points.append(QVariantMap{{"x", m.value("x")},
                                  {"y", m.value("y")},
                                  {"value", radius},
                                  {"label", m.value("label")},
                                  {"statistics", meta}});
    }
    auto visual = scaleVisual(d->colors);
    visual["legend"] = d->mapping && d->colors->legendVisible();
    ChartStatisticsAccess::setVisual(d->series, visual);
    d->points = input;
    ChartStatisticsAccess::publish(d->series, points, false);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    emit dataChanged();
    return true;
}
class ChartGrid::Impl
{
  public:
    int rows = 0, columns = 0;
    QVariantList values;
    QPointF origin;
    QSizeF spacing{1, 1};
    bool dense = true;
    ChartSeries *series = nullptr;
    ChartColorScale *colors = nullptr;
    QString error;
    static std::atomic<quint64> nextId;
    quint64 id = ++nextId;
};
std::atomic<quint64> ChartGrid::Impl::nextId{0};
ChartGrid::ChartGrid(QObject *p) : QObject(p), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Grid);
    d->colors = new ChartColorScale(this);
    connect(d->colors, &ChartColorScale::changed, this,
            [this] { update(d->rows, d->columns, d->values, d->origin, d->spacing); });
}
ChartGrid::~ChartGrid() = default;
int ChartGrid::rows() const
{
    return d->rows;
}
int ChartGrid::columns() const
{
    return d->columns;
}
QVariantList ChartGrid::values() const
{
    return d->values;
}
QPointF ChartGrid::origin() const
{
    return d->origin;
}
void ChartGrid::setOrigin(QPointF v)
{
    update(d->rows, d->columns, d->values, v, d->spacing);
}
QSizeF ChartGrid::spacing() const
{
    return d->spacing;
}
void ChartGrid::setSpacing(QSizeF v)
{
    update(d->rows, d->columns, d->values, d->origin, v);
}
bool ChartGrid::dense() const
{
    return d->dense;
}
void ChartGrid::setDense(bool v)
{
    if (QThread::currentThread() != thread() || v == d->dense)
        return;
    d->dense = v;
    update(d->rows, d->columns, d->values, d->origin, d->spacing);
    emit appearanceChanged();
}
ChartColorScale *ChartGrid::colorScale() const
{
    return d->colors;
}
ChartSeries *ChartGrid::series() const
{
    return d->series;
}
QString ChartGrid::error() const
{
    return d->error;
}
bool ChartGrid::setGrid(int r, int c, const QVariantList &v)
{
    return update(r, c, v, d->origin, d->spacing);
}
bool ChartGrid::setValue(int r, int c, const QVariant &v)
{
    if (r < 0 || c < 0 || r >= d->rows || c >= d->columns)
        return false;
    auto values = d->values;
    values[r * d->columns + c] = v;
    return setGrid(d->rows, d->columns, values);
}
bool ChartGrid::appendRow(const QVariantList &v)
{
    if (d->rows <= 0 || v.size() != d->columns)
        return false;
    auto values = d->values.mid(d->columns);
    values.append(v);
    return update(d->rows, d->columns, values, d->origin + QPointF(0, d->spacing.height()),
                  d->spacing);
}
bool ChartGrid::update(int rows, int columns, const QVariantList &input, QPointF origin,
                       QSizeF spacing)
{
    if (QThread::currentThread() != thread())
        return false;
    const auto fail = [this]
    {
        d->error =
            tr("Grid dimensions must match finite values or null gaps; spacing must be positive "
               "and coordinates finite (up to 2048 per dimension and 1,000,000 cells).");
        emit errorChanged();
        return false;
    };
    if (rows < 0 || columns < 0 || rows > 2048 || columns > 2048 ||
        qint64(rows) * columns > 1000000 || qint64(rows) * columns != input.size() ||
        ((rows == 0) != (columns == 0)) || !std::isfinite(origin.x()) ||
        !std::isfinite(origin.y()) || !std::isfinite(spacing.width()) ||
        !std::isfinite(spacing.height()) || spacing.width() <= 0 || spacing.height() <= 0)
        return fail();
    for (double coordinate : {origin.x() - spacing.width() / 2, origin.y() - spacing.height() / 2,
                              origin.x() + (columns - .5) * spacing.width(),
                              origin.y() + (rows - .5) * spacing.height()})
        if (!std::isfinite(coordinate))
            return fail();
    double low = INFINITY, high = -INFINITY;
    for (const auto &v : input)
    {
        if (!v.isValid() || v.isNull())
            continue;
        double n;
        if (!finiteValue(v, n))
            return fail();
        low = std::min(low, n);
        high = std::max(high, n);
    }
    d->colors->setDataRange(low, high);
    auto visual = scaleVisual(d->colors);
    const auto palette = visual.value("palette").toList();
    QImage atlas(std::max(256, columns), rows + 1, QImage::Format_RGBA8888),
        image(columns, rows, QImage::Format_ARGB32_Premultiplied);
    atlas.fill(Qt::transparent);
    image.fill(Qt::transparent);
    for (int i = 0; i < 256; ++i)
        atlas.setPixelColor(i, rows, palette[i].value<QColor>());
    QVariantList points;
    for (int i = 0; i < input.size(); ++i)
    {
        const auto &v = input[i];
        if (!v.isValid() || v.isNull())
            continue;
        const int r = i / columns, c = i % columns;
        const double n = v.toDouble(), normalized = d->colors->normalized(n);
        const int code = qRound(normalized * 65535), bin = qRound(normalized * 255);
        const QColor color = palette[bin].value<QColor>();
        atlas.setPixelColor(c, rows - 1 - r, QColor(code / 256, code % 256, 0, 255));
        image.setPixelColor(c, rows - 1 - r, color);
        points.append(QVariantMap{
            {"x", origin.x() + c * spacing.width()},
            {"y", origin.y() + r * spacing.height()},
            {"value", n},
            {"statistics",
             QVariantMap{{"sourceIndex", i}, {"row", r}, {"column", c}, {"mappedColor", color}}}});
    }
    visual.insert("atlas", atlas);
    visual.insert("image", image);
    visual.insert("textureId", QVariant::fromValue(d->id));
    visual.insert("origin", origin);
    visual.insert("spacing", spacing);
    visual.insert("rows", rows);
    visual.insert("columns", columns);
    visual.insert("dense", d->dense);
    d->rows = rows;
    d->columns = columns;
    d->values = input;
    d->origin = origin;
    d->spacing = spacing;
    ChartStatisticsAccess::setVisual(d->series, visual);
    ChartStatisticsAccess::publish(d->series, points, false);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    emit dataChanged();
    return true;
}
} // namespace QCatUiKit
