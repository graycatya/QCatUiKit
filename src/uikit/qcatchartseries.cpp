// SPDX-License-Identifier: MIT
#include <QCatUiKit/qcatchartseries.h>
#include "qcatchartstatistics_p.h"
#include <QImageReader>
#include <QPointer>
#include <QQmlContext>
#include <QQmlFile>
#include <QThread>
#include <QtQml/qqml.h>
#include <algorithm>
#include <cmath>
#include <limits>
namespace QCatUiKit
{
namespace
{
QImage loadChartTexture(QObject *owner, const QUrl &source, QString &error)
{
    error.clear();
    if (source.isEmpty())
        return {};
    QUrl resolved = source;
    if (resolved.isRelative())
        if (auto *context = qmlContext(owner))
            resolved = context->resolvedUrl(resolved);
    const QString path = QQmlFile::urlToLocalFileOrQrc(resolved);
    if (path.isEmpty())
    {
        error = QStringLiteral("A local file or qrc texture URL is required.");
        return {};
    }
    QImageReader reader(path);
    const auto size = reader.size();
    if (size.isValid() && (size.width() > 4096 || size.height() > 4096))
    {
        error = QStringLiteral("Chart textures are limited to 4096 x 4096 pixels.");
        return {};
    }
    auto image = reader.read();
    if (image.isNull())
    {
        error = reader.errorString();
        return {};
    }
    if (image.width() > 4096 || image.height() > 4096)
    {
        error = QStringLiteral("Chart textures are limited to 4096 x 4096 pixels.");
        return {};
    }
    return image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
}
struct SeriesRecord
{
    ChartSeries::Sample sample;
    double lower = std::numeric_limits<double>::quiet_NaN();
    bool gap = false;
    QVariantMap sliceStyle;
    QVariantMap statistics;
};
bool validatedSliceStyle(const QVariantMap &input, QVariantMap &output)
{
    for (auto it = input.cbegin(); it != input.cend(); ++it)
    {
        const auto &key = it.key();
        if (key == "color" || key == "borderColor")
        {
            const QColor color = it.value().value<QColor>();
            if (!color.isValid())
                return false;
            output.insert(key, color);
        }
        else if (key == "borderWidth" || key == "explodeDistance")
        {
            bool ok = false;
            const double value = it.value().toDouble(&ok);
            if (!ok || !std::isfinite(value) || value < 0 ||
                value > (key == "borderWidth" ? 16 : 128))
                return false;
            output.insert(key, value);
        }
        else if (key == "labelVisible" && it.value().metaType().id() == QMetaType::Bool)
            output.insert(key, it.value());
        else
            return false;
    }
    return true;
}
bool recordsFromVariants(const QVariantList &values, QVector<SeriesRecord> &out, bool range)
{
    out.reserve(values.size());
    for (const auto &value : values)
    {
        const auto m = value.toMap();
        bool xOk, yOk, lowerOk = true, valueOk = true;
        SeriesRecord r;
        r.sample.x = m.value("x").toDouble(&xOk);
        r.sample.y = m.value(range ? "upper" : "y").toDouble(&yOk);
        if (range)
            r.lower = m.value("lower").toDouble(&lowerOk);
        if (m.contains("value") && !range)
            r.sample.value = m.value("value").toDouble(&valueOk);
        r.sample.label = m.value("label").toString();
        if (m.contains("sliceStyle") &&
            (m.value("sliceStyle").metaType().id() != QMetaType::QVariantMap ||
             !validatedSliceStyle(m.value("sliceStyle").toMap(), r.sliceStyle)))
            return false;
        if (m.contains("breakBefore"))
        {
            if (m.value("breakBefore").metaType().id() != QMetaType::Bool)
                return false;
            r.gap = m.value("breakBefore").toBool();
        }
        if (!xOk || !yOk || !lowerOk || !valueOk || !std::isfinite(r.sample.x) ||
            !std::isfinite(r.sample.y) || !std::isfinite(r.sample.value) ||
            (range && !std::isfinite(r.lower)))
            return false;
        out.append(r);
    }
    return true;
}
bool rangeRecords(const QVector<ChartSeries::RangeSample> &values, QVector<SeriesRecord> &out)
{
    for (const auto &p : values)
    {
        if (!std::isfinite(p.x) || !std::isfinite(p.lower) || !std::isfinite(p.upper))
            return false;
        out.append({{p.x, p.upper, 1, p.label}, p.lower, p.breakBefore});
    }
    return true;
}
} // namespace

class ChartSeries::Impl
{
  public:
    QVariantMap visual;
    QPointer<QQmlComponent> delegates[3];
    QList<QMetaObject::Connection> delegateConnections[3];
    QString name;
    QPointer<ChartAxis> xAxis, yAxis;
    QMetaObject::Connection xAxisDestroyed, yAxisDestroyed;
    QColor color; // Invalid means use the owning chart's semantic palette.
    bool visible = true, sorted = true;
    Type type = Inherit;
    LineCapStyle cap = AutoCap;
    LineJoinStyle join = AutoJoin;
    qreal miter = 4;
    qreal width = 2;
    CurveMode curve = LinearCurve;
    qreal tension = 0, markerSize = 8;
    MarkerShape marker = Circle;
    bool markers = false;
    QUrl markerSource;
    QImage markerImage;
    QString markerError;
    qreal labelRotation = 0;
    int capacity = 100000, start = 0;
    QVector<SeriesRecord> ring;
    int breakCount = 0;
    StrokeStyle stroke = SolidLine;
    QList<qreal> pattern{8, 4};
    qreal offset = 0, opacity = 65. / 255;
    AreaBaselineMode baselineMode = ZeroBaseline;
    double baseline = 0;
    FillStyle fillStyle = SolidFill;
    QColor fillColor, borderColor, fillEndColor;
    QVariantList gradientStops;
    QUrl textureSource;
    QImage textureImage;
    QSizeF textureSize{32, 32};
    TextureMode textureMode = StretchTexture;
    QString textureError;
    qreal barBorderWidth = 0, barLabelMargin = 4;
    QColor barBorderColor;
    bool barLabels = false;
    BarLabelPosition barLabelPosition = BarLabelCenter;
    QString barLabelFormat = QStringLiteral("@value");
    int barLabelPrecision = 6;
    bool assign(const QVector<SeriesRecord> &values, bool replace)
    {
        if (replace)
        {
            ring.clear();
            start = 0;
            sorted = true;
            breakCount = 0;
        }
        for (const auto &r : values)
        {
            if (!ring.isEmpty() &&
                r.sample.x < ring[(start + ring.size() - 1) % ring.size()].sample.x)
                sorted = false;
            if (ring.size() < capacity)
                ring.append(r);
            else
            {
                breakCount -= ring[start].gap;
                ring[start] = r;
                start = (start + 1) % capacity;
            }
            breakCount += r.gap;
        }
        return replace || !values.isEmpty();
    }
};
ChartSeries::ChartSeries(QObject *parent) : QObject(parent), d(new Impl) {}
ChartSeries::~ChartSeries() = default;
QQmlComponent *ChartSeries::pointDelegate() const
{
    return d->delegates[0];
}
void ChartSeries::setPointDelegate(QQmlComponent *value)
{
    if (value == d->delegates[0] || (value && value->thread() != thread()))
        return;
    for (auto c : d->delegateConnections[0])
        disconnect(c);
    d->delegateConnections[0].clear();
    d->delegates[0] = value;
    if (value)
    {
        d->delegateConnections[0] << connect(value, &QObject::destroyed, this,
                                             &ChartSeries::styleChanged)
                                  << connect(value, &QQmlComponent::statusChanged, this,
                                             &ChartSeries::styleChanged);
    }
    emit styleChanged();
}
QQmlComponent *ChartSeries::barDelegate() const
{
    return d->delegates[1];
}
void ChartSeries::setBarDelegate(QQmlComponent *value)
{
    if (value == d->delegates[1] || (value && value->thread() != thread()))
        return;
    for (auto c : d->delegateConnections[1])
        disconnect(c);
    d->delegateConnections[1].clear();
    d->delegates[1] = value;
    if (value)
    {
        d->delegateConnections[1] << connect(value, &QObject::destroyed, this,
                                             &ChartSeries::styleChanged)
                                  << connect(value, &QQmlComponent::statusChanged, this,
                                             &ChartSeries::styleChanged);
    }
    emit styleChanged();
}
QQmlComponent *ChartSeries::labelDelegate() const
{
    return d->delegates[2];
}
void ChartSeries::setLabelDelegate(QQmlComponent *value)
{
    if (value == d->delegates[2] || (value && value->thread() != thread()))
        return;
    for (auto c : d->delegateConnections[2])
        disconnect(c);
    d->delegateConnections[2].clear();
    d->delegates[2] = value;
    if (value)
    {
        d->delegateConnections[2] << connect(value, &QObject::destroyed, this,
                                             &ChartSeries::styleChanged)
                                  << connect(value, &QQmlComponent::statusChanged, this,
                                             &ChartSeries::styleChanged);
    }
    emit styleChanged();
}
QUrl ChartSeries::markerTexture() const
{
    return d->markerSource;
}
QString ChartSeries::markerTextureError() const
{
    return d->markerError;
}
QImage ChartSeries::markerTextureImage() const
{
    return d->markerImage;
}
void ChartSeries::setMarkerTexture(const QUrl &value)
{
    if (d->markerSource == value)
        return;
    d->markerSource = value;
    d->markerImage = loadChartTexture(this, value, d->markerError);
    emit styleChanged();
}
qreal ChartSeries::barLabelRotation() const
{
    return d->labelRotation;
}
void ChartSeries::setBarLabelRotation(qreal value)
{
    if (!std::isfinite(value) || value < -180 || value > 180 || value == d->labelRotation)
        return;
    d->labelRotation = value;
    emit styleChanged();
}
QVariantList ChartSeries::fillGradientStops() const
{
    return d->gradientStops;
}
void ChartSeries::setFillGradientStops(const QVariantList &value)
{
    if (value.size() > 32 || value.size() == 1)
        return;
    QVariantList normalized;
    double previous = -1;
    for (const auto &entry : value)
    {
        const auto map = entry.toMap();
        bool ok = false;
        const double position = map.value("position").toDouble(&ok);
        const auto color = map.value("color").value<QColor>();
        if (map.size() != 2 || !ok || !std::isfinite(position) || position < 0 || position > 1 ||
            position <= previous || !color.isValid())
            return;
        normalized.append(QVariantMap{{"position", position}, {"color", color}});
        previous = position;
    }
    if (d->gradientStops == normalized)
        return;
    d->gradientStops = normalized;
    emit styleChanged();
}
QUrl ChartSeries::fillTexture() const
{
    return d->textureSource;
}
QImage ChartSeries::fillTextureImage() const
{
    return d->textureImage;
}
QString ChartSeries::fillTextureError() const
{
    return d->textureError;
}
void ChartSeries::setFillTexture(const QUrl &value)
{
    if (value == d->textureSource)
        return;
    d->textureSource = value;
    d->textureImage = loadChartTexture(this, value, d->textureError);
    emit styleChanged();
}
QSizeF ChartSeries::fillTextureSize() const
{
    return d->textureSize;
}
void ChartSeries::setFillTextureSize(const QSizeF &value)
{
    if (!std::isfinite(value.width()) || !std::isfinite(value.height()) || value.width() < 1 ||
        value.height() < 1 || value.width() > 4096 || value.height() > 4096 ||
        d->textureSize == value)
        return;
    d->textureSize = value;
    emit styleChanged();
}
ChartSeries::TextureMode ChartSeries::fillTextureMode() const
{
    return d->textureMode;
}
void ChartSeries::setFillTextureMode(TextureMode value)
{
    if (value < StretchTexture || value > TileTexture || d->textureMode == value)
        return;
    d->textureMode = value;
    emit styleChanged();
}
ChartSeries::LineCapStyle ChartSeries::lineCap() const
{
    return d->cap;
}
void ChartSeries::setLineCap(LineCapStyle value)
{
    if (value >= AutoCap && value <= RoundCap && value != d->cap)
    {
        d->cap = value;
        emit styleChanged();
    }
}
ChartSeries::LineJoinStyle ChartSeries::lineJoin() const
{
    return d->join;
}
void ChartSeries::setLineJoin(LineJoinStyle value)
{
    if (value >= AutoJoin && value <= RoundJoin && value != d->join)
    {
        d->join = value;
        emit styleChanged();
    }
}
qreal ChartSeries::miterLimit() const
{
    return d->miter;
}
void ChartSeries::setMiterLimit(qreal value)
{
    if (!std::isfinite(value))
        return;
    value = std::clamp(value, qreal(1), qreal(16));
    if (value != d->miter)
    {
        d->miter = value;
        emit styleChanged();
    }
}
ChartAxis *ChartSeries::xAxis() const
{
    return d->xAxis;
}
ChartAxis *ChartSeries::yAxis() const
{
    return d->yAxis;
}
void ChartSeries::setXAxis(ChartAxis *axis)
{
    if (axis == d->xAxis || (axis && axis->thread() != thread()))
        return;
    disconnect(d->xAxisDestroyed);
    d->xAxis = axis;
    if (axis)
        d->xAxisDestroyed = connect(axis, &QObject::destroyed, this, &ChartSeries::styleChanged);
    emit styleChanged();
}
void ChartSeries::setYAxis(ChartAxis *axis)
{
    if (axis == d->yAxis || (axis && axis->thread() != thread()))
        return;
    disconnect(d->yAxisDestroyed);
    d->yAxis = axis;
    if (axis)
        d->yAxisDestroyed = connect(axis, &QObject::destroyed, this, &ChartSeries::styleChanged);
    emit styleChanged();
}
QString ChartSeries::name() const
{
    return d->name;
}
void ChartSeries::setName(const QString &v)
{
    if (d->name != v)
    {
        d->name = v;
        emit styleChanged();
    }
}
QColor ChartSeries::color() const
{
    return d->color;
}
void ChartSeries::setColor(const QColor &v)
{
    if (d->color != v)
    {
        d->color = v;
        emit styleChanged();
    }
}
bool ChartSeries::isVisible() const
{
    return d->visible;
}
void ChartSeries::setVisible(bool v)
{
    if (d->visible != v)
    {
        d->visible = v;
        emit styleChanged();
    }
}
ChartSeries::Type ChartSeries::type() const
{
    return d->type;
}
void ChartSeries::setType(Type v)
{
    if (v >= Inherit && v <= Contour && d->type != v)
    {
        d->type = v;
        emit styleChanged();
    }
}
qreal ChartSeries::lineWidth() const
{
    return d->width;
}
void ChartSeries::setLineWidth(qreal v)
{
    if (std::isfinite(v) && v > 0 && v <= 64 && d->width != v)
    {
        d->width = v;
        emit styleChanged();
    }
}
ChartSeries::CurveMode ChartSeries::curveMode() const
{
    return d->curve;
}
void ChartSeries::setCurveMode(CurveMode value)
{
    if (value >= LinearCurve && value <= NaturalSpline && d->curve != value)
    {
        d->curve = value;
        emit styleChanged();
    }
}
qreal ChartSeries::splineTension() const
{
    return d->tension;
}
void ChartSeries::setSplineTension(qreal value)
{
    if (std::isfinite(value) && value >= 0 && value <= 1 && d->tension != value)
    {
        d->tension = value;
        emit styleChanged();
    }
}
ChartSeries::MarkerShape ChartSeries::markerShape() const
{
    return d->marker;
}
void ChartSeries::setMarkerShape(MarkerShape value)
{
    if (value >= Circle && value <= TextureMarker && d->marker != value)
    {
        d->marker = value;
        emit styleChanged();
    }
}
qreal ChartSeries::markerSize() const
{
    return d->markerSize;
}
void ChartSeries::setMarkerSize(qreal value)
{
    if (std::isfinite(value) && value >= 1 && value <= 128 && d->markerSize != value)
    {
        d->markerSize = value;
        emit styleChanged();
    }
}
bool ChartSeries::markersVisible() const
{
    return d->markers;
}
void ChartSeries::setMarkersVisible(bool value)
{
    if (d->markers != value)
    {
        d->markers = value;
        emit styleChanged();
    }
}
ChartSeries::StrokeStyle ChartSeries::strokeStyle() const
{
    return d->stroke;
}
void ChartSeries::setStrokeStyle(StrokeStyle v)
{
    if (v >= SolidLine && v <= DashLine && v != d->stroke)
    {
        d->stroke = v;
        emit styleChanged();
    }
}
QList<qreal> ChartSeries::dashPattern() const
{
    return d->pattern;
}
void ChartSeries::setDashPattern(const QList<qreal> &v)
{
    if (v.isEmpty() || v.size() > 32 || v.size() % 2)
        return;
    for (qreal n : v)
        if (!std::isfinite(n) || n < 1 || n > 1024)
            return;
    if (v != d->pattern)
    {
        d->pattern = v;
        emit styleChanged();
    }
}
qreal ChartSeries::dashOffset() const
{
    return d->offset;
}
void ChartSeries::setDashOffset(qreal v)
{
    if (std::isfinite(v) && std::abs(v) <= 1e9 && v != d->offset)
    {
        d->offset = v;
        emit styleChanged();
    }
}
ChartSeries::AreaBaselineMode ChartSeries::areaBaselineMode() const
{
    return d->baselineMode;
}
void ChartSeries::setAreaBaselineMode(AreaBaselineMode value)
{
    if (value >= ZeroBaseline && value <= PlotBottomBaseline && d->baselineMode != value)
    {
        d->baselineMode = value;
        emit styleChanged();
    }
}
double ChartSeries::areaBaseline() const
{
    return d->baseline;
}
void ChartSeries::setAreaBaseline(double value)
{
    if (std::isfinite(value) && d->baseline != value)
    {
        d->baseline = value;
        emit styleChanged();
    }
}
QColor ChartSeries::fillColor() const
{
    return d->fillColor;
}
void ChartSeries::setFillColor(const QColor &value)
{
    if (d->fillColor != value)
    {
        d->fillColor = value;
        emit styleChanged();
    }
}
QColor ChartSeries::borderColor() const
{
    return d->borderColor;
}
void ChartSeries::setBorderColor(const QColor &value)
{
    if (d->borderColor != value)
    {
        d->borderColor = value;
        emit styleChanged();
    }
}
ChartSeries::FillStyle ChartSeries::fillStyle() const
{
    return d->fillStyle;
}
void ChartSeries::setFillStyle(FillStyle value)
{
    if (value >= SolidFill && value <= TextureFill && d->fillStyle != value)
    {
        d->fillStyle = value;
        emit styleChanged();
    }
}
QColor ChartSeries::fillEndColor() const
{
    return d->fillEndColor;
}
void ChartSeries::setFillEndColor(const QColor &value)
{
    if (d->fillEndColor != value)
    {
        d->fillEndColor = value;
        emit styleChanged();
    }
}
qreal ChartSeries::fillOpacity() const
{
    return d->opacity;
}
void ChartSeries::setFillOpacity(qreal v)
{
    if (std::isfinite(v) && v >= 0 && v <= 1 && v != d->opacity)
    {
        d->opacity = v;
        emit styleChanged();
    }
}
qreal ChartSeries::barBorderWidth() const
{
    return d->barBorderWidth;
}
void ChartSeries::setBarBorderWidth(qreal v)
{
    if (std::isfinite(v) && v >= 0 && v <= 16 && d->barBorderWidth != v)
    {
        d->barBorderWidth = v;
        emit styleChanged();
    }
}
QColor ChartSeries::barBorderColor() const
{
    return d->barBorderColor;
}
void ChartSeries::setBarBorderColor(const QColor &v)
{
    if (true && d->barBorderColor != v)
    {
        d->barBorderColor = v;
        emit styleChanged();
    }
}
bool ChartSeries::barLabelsVisible() const
{
    return d->barLabels;
}
void ChartSeries::setBarLabelsVisible(bool v)
{
    if (true && d->barLabels != v)
    {
        d->barLabels = v;
        emit styleChanged();
    }
}
ChartSeries::BarLabelPosition ChartSeries::barLabelPosition() const
{
    return d->barLabelPosition;
}
void ChartSeries::setBarLabelPosition(BarLabelPosition v)
{
    if (v >= BarLabelCenter && v <= BarLabelOutsideEnd && d->barLabelPosition != v)
    {
        d->barLabelPosition = v;
        emit styleChanged();
    }
}
QString ChartSeries::barLabelFormat() const
{
    return d->barLabelFormat;
}
void ChartSeries::setBarLabelFormat(const QString &v)
{
    if (v.size() <= 256 && !v.contains(QLatin1Char('\n')) && !v.contains(QLatin1Char('\r')) &&
        d->barLabelFormat != v)
    {
        d->barLabelFormat = v;
        emit styleChanged();
    }
}
int ChartSeries::barLabelPrecision() const
{
    return d->barLabelPrecision;
}
void ChartSeries::setBarLabelPrecision(int v)
{
    if (v >= 1 && v <= 15 && d->barLabelPrecision != v)
    {
        d->barLabelPrecision = v;
        emit styleChanged();
    }
}
qreal ChartSeries::barLabelMargin() const
{
    return d->barLabelMargin;
}
void ChartSeries::setBarLabelMargin(qreal v)
{
    if (std::isfinite(v) && v >= 0 && v <= 64 && d->barLabelMargin != v)
    {
        d->barLabelMargin = v;
        emit styleChanged();
    }
}
bool ChartSeries::hasBreaks() const
{
    return d->breakCount > 0;
}
QVariantMap ChartSeries::sliceStyle(int i) const
{
    return i >= 0 && i < count() ? d->ring[(d->start + i) % count()].sliceStyle : QVariantMap();
}
bool ChartSeries::setSliceStyle(int i, const QVariantMap &style)
{
    if (QThread::currentThread() != thread() || i < 0 || i >= count())
        return false;
    QVariantMap normalized;
    if (!validatedSliceStyle(style, normalized))
        return false;
    auto &record = d->ring[(d->start + i) % count()];
    if (record.sliceStyle != normalized)
    {
        record.sliceStyle = normalized;
        emit styleChanged();
    }
    return true;
}
bool ChartSeries::breakBefore(int i) const
{
    return i >= 0 && i < count() && d->ring[(d->start + i) % count()].gap;
}
bool ChartSeries::setBreakBefore(int i, bool v)
{
    if (QThread::currentThread() != thread() || i < 0 || i >= count())
        return false;
    auto &r = d->ring[(d->start + i) % count()];
    if (r.gap != v)
    {
        d->breakCount += v ? 1 : -1;
        r.gap = v;
        emit dataChanged();
    }
    return true;
}
double ChartSeries::lowerValue(int i) const
{
    return i >= 0 && i < count() ? d->ring[(d->start + i) % count()].lower
                                 : std::numeric_limits<double>::quiet_NaN();
}
bool ChartSeries::hasRangeValue(int i) const
{
    return std::isfinite(lowerValue(i));
}
int ChartSeries::capacity() const
{
    return d->capacity;
}
void ChartSeries::setCapacity(int v)
{
    if (v <= 0 || v == d->capacity)
        return;
    QVector<SeriesRecord> keep;
    for (int i = std::max(0, count() - v); i < count(); ++i)
        keep.append(d->ring[(d->start + i) % count()]);
    d->capacity = v;
    d->start = 0;
    d->assign(keep, true);
    emit capacityChanged();
    emit dataChanged();
}
int ChartSeries::count() const
{
    return int(d->ring.size());
}
const ChartSeries::Sample &ChartSeries::sampleAt(int i) const
{
    static const Sample empty;
    return i >= 0 && i < count() ? d->ring[(d->start + i) % count()].sample : empty;
}
bool ChartSeries::sortedX() const
{
    return d->sorted;
}
int ChartSeries::lowerBound(double x) const
{
    if (!d->sorted)
        return 0;
    int first = 0, last = count();
    while (first < last)
    {
        const int mid = first + (last - first) / 2;
        if (sampleAt(mid).x < x)
            first = mid + 1;
        else
            last = mid;
    }
    return first;
}
static bool validSamples(const QVector<ChartSeries::Sample> &values)
{
    for (const auto &p : values)
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.value))
            return false;
    return true;
}
bool ChartSeries::appendSamples(const QVector<Sample> &values)
{
    if (QThread::currentThread() != thread() || !validSamples(values))
        return false;
    QVector<SeriesRecord> records;
    records.reserve(values.size());
    for (const auto &p : values)
        records.append({p});
    if (d->assign(records, false))
        emit dataChanged();
    return true;
}
bool ChartSeries::replaceSamples(const QVector<Sample> &values)
{
    if (QThread::currentThread() != thread() || !validSamples(values))
        return false;
    QVector<SeriesRecord> records;
    records.reserve(std::min(qsizetype(d->capacity), values.size()));
    for (int i = std::max(0, int(values.size()) - d->capacity); i < values.size(); ++i)
        records.append({values[i]});
    d->assign(records, true);
    emit dataChanged();
    return true;
}
bool ChartSeries::appendRangeSamples(const QVector<RangeSample> &values)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != thread() || !rangeRecords(values, records))
        return false;
    if (d->assign(records, false))
        emit dataChanged();
    return true;
}
bool ChartSeries::replaceRangeSamples(const QVector<RangeSample> &values)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != thread() || !rangeRecords(values, records))
        return false;
    d->assign(records, true);
    emit dataChanged();
    return true;
}
bool ChartSeries::replacePoints(const QVector<QPointF> &points)
{
    QVector<Sample> values;
    values.reserve(points.size());
    for (const auto &p : points)
        values.append({p.x(), p.y(), 1, {}});
    return replaceSamples(values);
}
bool ChartSeries::append(double x, double y, double value)
{
    return appendSamples({{x, y, value, {}}});
}
bool ChartSeries::appendData(const QVariantList &values)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != thread() || !recordsFromVariants(values, records, false))
        return false;
    if (d->assign(records, false))
        emit dataChanged();
    return true;
}
bool ChartSeries::setData(const QVariantList &values)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != thread() || !recordsFromVariants(values, records, false))
        return false;
    if (d->assign(records, true))
        emit dataChanged();
    return true;
}
bool ChartSeries::appendRangeData(const QVariantList &values)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != thread() || !recordsFromVariants(values, records, true))
        return false;
    if (d->assign(records, false))
        emit dataChanged();
    return true;
}
bool ChartSeries::setRangeBoundaries(const QVariantList &lower, const QVariantList &upper,
                                     RangeAlignment alignment)
{
    if (QThread::currentThread() != thread() ||
        (alignment != ExactAlignment && alignment != LinearAlignment))
        return false;
    QVector<SeriesRecord> low, high, output;
    if (!recordsFromVariants(lower, low, false) || !recordsFromVariants(upper, high, false))
        return false;
    const auto ordered = [](const auto &records)
    {
        for (int i = 1; i < records.size(); ++i)
            if (records[i].sample.x <= records[i - 1].sample.x)
                return false;
        return true;
    };
    if (!ordered(low) || !ordered(high) || low.isEmpty() != high.isEmpty())
        return false;
    if (alignment == ExactAlignment)
    {
        if (low.size() != high.size())
            return false;
        for (int i = 0; i < low.size(); ++i)
        {
            if (low[i].sample.x != high[i].sample.x)
                return false;
            auto record = high[i];
            record.lower = low[i].sample.y;
            record.gap |= low[i].gap;
            if (record.sample.label.isEmpty())
                record.sample.label = low[i].sample.label;
            record.sample.value = 1;
            record.sliceStyle.clear();
            output.append(record);
        }
    }
    else if (!low.isEmpty())
    {
        const double start = std::max(low.first().sample.x, high.first().sample.x);
        const double end = std::min(low.last().sample.x, high.last().sample.x);
        if (start > end)
            return false;
        int li = 0, hi = 0, lp = 0, hp = 0;
        bool gap = false;
        const auto sample = [](const QVector<SeriesRecord> &records, int &index, double x,
                               double &y, bool &broken, QString &label)
        {
            while (index + 1 < records.size() && records[index + 1].sample.x <= x)
                ++index;
            const auto &a = records[index];
            if (a.sample.x == x)
            {
                y = a.sample.y;
                broken |= a.gap;
                label = a.sample.label;
                return true;
            }
            if (index + 1 >= records.size() || records[index + 1].gap)
                return false;
            const auto &b = records[index + 1];
            const double span = b.sample.x - a.sample.x;
            const double t = std::isfinite(span)
                                 ? (x - a.sample.x) / span
                                 : (x / 2 - a.sample.x / 2) / (b.sample.x / 2 - a.sample.x / 2);
            y = a.sample.y * (1 - t) + b.sample.y * t;
            return std::isfinite(y);
        };
        while (li < low.size() || hi < high.size())
        {
            const double x = std::min(
                li < low.size() ? low[li].sample.x : std::numeric_limits<double>::infinity(),
                hi < high.size() ? high[hi].sample.x : std::numeric_limits<double>::infinity());
            if (li < low.size() && low[li].sample.x == x)
                ++li;
            if (hi < high.size() && high[hi].sample.x == x)
                ++hi;
            if (x < start || x > end)
                continue;
            SeriesRecord record;
            record.sample.x = x;
            QString lowLabel;
            const bool lowerValid = sample(low, lp, x, record.lower, gap, lowLabel);
            const bool upperValid = sample(high, hp, x, record.sample.y, gap, record.sample.label);
            if (!lowerValid || !upperValid)
            {
                gap = true;
                continue;
            }
            record.gap = gap;
            gap = false;
            if (record.sample.label.isEmpty())
                record.sample.label = lowLabel;
            output.append(record);
        }
    }
    d->assign(output, true);
    emit dataChanged();
    return true;
}
bool ChartSeries::setRangeData(const QVariantList &values)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != thread() || !recordsFromVariants(values, records, true))
        return false;
    if (d->assign(records, true))
        emit dataChanged();
    return true;
}
QVariantMap ChartSeries::at(int i) const
{
    if (i < 0 || i >= count())
        return {};
    const auto p = sampleAt(i);
    QVariantMap result{{"x", p.x}, {"y", p.y}, {"value", p.value}, {"label", p.label}};
    const auto &metadata = d->ring[(d->start + i) % d->ring.size()].statistics;
    for (auto it = metadata.cbegin(); it != metadata.cend(); ++it)
        result.insert(it.key(), it.value());
    const auto style = sliceStyle(i);
    if (!style.isEmpty())
        result.insert("sliceStyle", style);
    if (breakBefore(i))
        result.insert("breakBefore", true);
    if (hasRangeValue(i))
    {
        result.insert("lower", lowerValue(i));
        result.insert("upper", p.y);
    }
    return result;
}
void ChartSeries::clear()
{
    if (d->ring.isEmpty())
        return;
    d->ring.clear();
    d->breakCount = 0;
    d->start = 0;
    d->sorted = true;
    emit dataChanged();
}
void ChartStatisticsAccess::setVisual(ChartSeries *series, const QVariantMap &visual)
{
    series->d->visual = visual;
}
QVariantMap ChartStatisticsAccess::visual(const ChartSeries *series)
{
    return series->d->visual;
}
bool ChartStatisticsAccess::publish(ChartSeries *series, const QVariantList &points, bool range)
{
    QVector<SeriesRecord> records;
    if (QThread::currentThread() != series->thread() || points.size() > 1000000 ||
        !recordsFromVariants(points, records, range))
        return false;
    for (int i = 0; i < records.size(); ++i)
    {
        const auto point = points[i].toMap();
        // RangeSample has no value field. Statistical bars still need their raw
        // delta/count independently of the two geometry boundaries.
        if (point.contains("value"))
        {
            bool ok = false;
            const double value = point.value("value").toDouble(&ok);
            if (!ok || !std::isfinite(value))
                return false;
            records[i].sample.value = value;
        }
        records[i].statistics = point.value("statistics").toMap();
    }
    const int capacity = std::max(1, int(points.size()));
    const bool changed = series->d->capacity != capacity;
    series->d->capacity = capacity;
    series->d->assign(records, true);
    if (changed)
        emit series->capacityChanged();
    emit series->dataChanged();
    return true;
}
} // namespace QCatUiKit
