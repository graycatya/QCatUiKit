// SPDX-License-Identifier: Apache-2.0
#include "qcatchartboxplot.h"
#include "qcatchartohlc.h"
#include "qcatchartstatistics_p.h"
#include <QThread>
#include <algorithm>
#include <cmath>
#include <limits>
namespace QCatUiKit
{
namespace
{
bool number(const QVariant &input, double &result)
{
    bool ok = false;
    result = input.toDouble(&ok);
    return ok && std::isfinite(result);
}
double quantile(const QVector<double> &values, double p)
{
    const double at = (values.size() - 1) * p;
    const int i = int(at);
    const double f = at - i;
    return values[i] * (1 - f) + values[std::min(i + 1, int(values.size() - 1))] * f;
}
} // namespace
class ChartOhlc::Impl
{
  public:
    QVariantList records;
    QStringList categories;
    QString error;
    bool candles = true, compressed = true;
    QColor rising = QColor("#389e0d"), falling = QColor("#cf1322");
    ChartSeries *series = nullptr, *volume = nullptr;
};
ChartOhlc::ChartOhlc(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Candlestick);
    d->volume = new ChartSeries(this);
    d->volume->setType(ChartSeries::Bar);
}
ChartOhlc::~ChartOhlc() = default;
QVariantList ChartOhlc::records() const
{
    return d->records;
}
void ChartOhlc::setRecords(const QVariantList &v)
{
    setData(v);
}
bool ChartOhlc::candlesticks() const
{
    return d->candles;
}
void ChartOhlc::setCandlesticks(bool v)
{
    if (QThread::currentThread() != thread() || v == d->candles)
        return;
    d->candles = v;
    d->series->setType(v ? ChartSeries::Candlestick : ChartSeries::Ohlc);
    emit appearanceChanged();
}
bool ChartOhlc::compressedTime() const
{
    return d->compressed;
}
void ChartOhlc::setCompressedTime(bool v)
{
    if (QThread::currentThread() != thread() || v == d->compressed)
        return;
    d->compressed = v;
    setData(d->records);
    emit appearanceChanged();
}
QColor ChartOhlc::risingColor() const
{
    return d->rising;
}
void ChartOhlc::setRisingColor(const QColor &v)
{
    if (QThread::currentThread() != thread() || !v.isValid() || v == d->rising)
        return;
    d->rising = v;
    setData(d->records);
    emit appearanceChanged();
}
QColor ChartOhlc::fallingColor() const
{
    return d->falling;
}
void ChartOhlc::setFallingColor(const QColor &v)
{
    if (QThread::currentThread() != thread() || !v.isValid() || v == d->falling)
        return;
    d->falling = v;
    setData(d->records);
    emit appearanceChanged();
}
ChartSeries *ChartOhlc::series() const
{
    return d->series;
}
ChartSeries *ChartOhlc::volumeSeries() const
{
    return d->volume;
}
QStringList ChartOhlc::categories() const
{
    return d->categories;
}
QString ChartOhlc::error() const
{
    return d->error;
}
bool ChartOhlc::setData(const QVariantList &input)
{
    if (QThread::currentThread() != thread())
        return false;
    const auto fail = [this]
    {
        d->error = tr("OHLC requires increasing finite times, low ≤ open/close ≤ high, and "
                      "nonnegative volume (at most 100,000 records).");
        emit errorChanged();
        return false;
    };
    if (input.size() > 100000)
        return fail();
    QVariantList price, volume;
    QStringList categories;
    double previous = -INFINITY, spacing = INFINITY;
    for (int i = 0; i < input.size(); ++i)
    {
        const auto r = input[i].toMap();
        double time, open, high, low, close, count;
        if (!number(r.value("time"), time) || !number(r.value("open"), open) ||
            !number(r.value("high"), high) || !number(r.value("low"), low) ||
            !number(r.value("close"), close) || !number(r.value("volume", 0), count) ||
            time <= previous || low > std::min(open, close) || high < std::max(open, close) ||
            count < 0)
            return fail();
        if (i)
            spacing = std::min(spacing, time - previous);
        previous = time;
        const QString label = r.value("label", QString::number(time, 'g', 15)).toString();
        const double x = d->compressed ? i : time;
        const QVariantMap meta{
            {"sourceIndex", i}, {"time", time},
            {"open", open},     {"high", high},
            {"low", low},       {"close", close},
            {"volume", count},  {"barColor", close >= open ? d->rising : d->falling}};
        price.append(QVariantMap{{"x", x},
                                 {"lower", low},
                                 {"upper", high},
                                 {"value", close},
                                 {"label", label},
                                 {"statistics", meta}});
        volume.append(QVariantMap{{"x", x}, {"y", count}, {"label", label}, {"statistics", meta}});
        categories.append(label);
    }
    const double width = d->compressed || !std::isfinite(spacing) ? 1 : spacing;
    for (auto *list : {&price, &volume})
        for (auto &point : *list)
        {
            auto m = point.toMap(), meta = m.value("statistics").toMap();
            meta.insert("categoryWidth", width);
            m.insert("statistics", meta);
            point = m;
        }
    d->records = input;
    d->categories = categories;
    ChartStatisticsAccess::publish(d->series, price, true);
    ChartStatisticsAccess::publish(d->volume, volume, false);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    emit dataChanged();
    return true;
}
class ChartBoxPlot::Impl
{
  public:
    QVariantList groups;
    QStringList categories;
    QString error;
    double factor = 1.5;
    ChartSeries *series = nullptr, *outliers = nullptr;
};
ChartBoxPlot::ChartBoxPlot(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::BoxPlot);
    d->outliers = new ChartSeries(this);
    d->outliers->setType(ChartSeries::Scatter);
    d->outliers->setMarkerSize(6);
}
ChartBoxPlot::~ChartBoxPlot() = default;
QVariantList ChartBoxPlot::groups() const
{
    return d->groups;
}
void ChartBoxPlot::setGroups(const QVariantList &v)
{
    setData(v);
}
bool ChartBoxPlot::setData(const QVariantList &v)
{
    return update(v, d->factor);
}
double ChartBoxPlot::whiskerFactor() const
{
    return d->factor;
}
void ChartBoxPlot::setWhiskerFactor(double v)
{
    update(d->groups, v);
}
ChartSeries *ChartBoxPlot::series() const
{
    return d->series;
}
ChartSeries *ChartBoxPlot::outlierSeries() const
{
    return d->outliers;
}
QStringList ChartBoxPlot::categories() const
{
    return d->categories;
}
QString ChartBoxPlot::error() const
{
    return d->error;
}
bool ChartBoxPlot::update(const QVariantList &input, double factor)
{
    if (QThread::currentThread() != thread())
        return false;
    const auto fail = [this]
    {
        d->error = tr("Box groups require finite observations or ordered minimum, q1, median, q3, "
                      "maximum and outside outliers; factor must be 0–10.");
        emit errorChanged();
        return false;
    };
    if (!std::isfinite(factor) || factor < 0 || factor > 10 || input.size() > 4096)
        return fail();
    QVariantList boxes, outliers;
    QStringList categories;
    int total = 0;
    for (int i = 0; i < input.size(); ++i)
    {
        const auto g = input[i].toMap();
        const bool raw = g.contains("values");
        double low, q1, median, q3, high;
        QVariantList outside;
        int count = 0;
        if (raw)
        {
            if (g.contains("minimum") || g.contains("q1") || g.contains("median") ||
                g.contains("q3") || g.contains("maximum") || g.contains("outliers"))
                return fail();
            const auto values = g.value("values").toList();
            QVector<double> sorted;
            total += values.size();
            if (total > 1000000 || values.isEmpty())
                return fail();
            for (const auto &v : values)
            {
                double n;
                if (!number(v, n))
                    return fail();
                sorted.append(n);
            }
            std::sort(sorted.begin(), sorted.end());
            count = sorted.size();
            q1 = quantile(sorted, .25);
            median = quantile(sorted, .5);
            q3 = quantile(sorted, .75);
            // Compare in scaled coordinates; fences may exceed the finite range.
            const double scale = std::max({1., std::abs(q1), std::abs(q3)});
            const double spread = q3 / scale - q1 / scale, lf = q1 / scale - factor * spread,
                         hf = q3 / scale + factor * spread;
            low = INFINITY;
            high = -INFINITY;
            for (int j = 0; j < values.size(); ++j)
            {
                const double v = values[j].toDouble();
                if (v / scale < lf || v / scale > hf)
                    outside.append(QVariantMap{{"value", v}, {"sourceIndex", j}});
                else
                {
                    low = std::min(low, v);
                    high = std::max(high, v);
                }
            }
            // Factor zero can place every observation outside interpolated quartiles.
            if (!std::isfinite(low))
            {
                low = q1;
                high = q3;
            }
            low = std::min(low, q1);
            high = std::max(high, q3);
        }
        else
        {
            if (!number(g.value("minimum"), low) || !number(g.value("q1"), q1) ||
                !number(g.value("median"), median) || !number(g.value("q3"), q3) ||
                !number(g.value("maximum"), high))
                return fail();
            const auto values = g.value("outliers").toList();
            total += values.size();
            if (total > 1000000)
                return fail();
            for (const auto &value : values)
            {
                double v;
                if (!number(value, v) || (v >= low && v <= high))
                    return fail();
                outside.append(QVariantMap{{"value", v}, {"sourceIndex", -1}});
            }
        }
        if (!(low <= q1 && q1 <= median && median <= q3 && q3 <= high))
            return fail();
        const QString label = g.value("label", QString::number(i + 1)).toString();
        categories.append(label);
        const QVariantMap meta{{"groupIndex", i},
                               {"source", raw ? "observations" : "summary"},
                               {"sampleCount", count},
                               {"minimum", low},
                               {"q1", q1},
                               {"median", median},
                               {"q3", q3},
                               {"maximum", high},
                               {"outlierCount", outside.size()}};
        boxes.append(QVariantMap{{"x", i},
                                 {"lower", low},
                                 {"upper", high},
                                 {"value", median},
                                 {"label", label},
                                 {"statistics", meta}});
        for (const auto &entry : outside)
        {
            auto m = meta;
            const auto o = entry.toMap();
            m.insert("sourceIndex", o.value("sourceIndex"));
            m.insert("outlier", true);
            outliers.append(QVariantMap{
                {"x", i}, {"y", o.value("value")}, {"label", label}, {"statistics", m}});
        }
    }
    const bool changed = d->factor != factor;
    d->factor = factor;
    d->groups = input;
    d->categories = categories;
    ChartStatisticsAccess::publish(d->series, boxes, true);
    ChartStatisticsAccess::publish(d->outliers, outliers, false);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    if (changed)
        emit whiskerFactorChanged();
    emit dataChanged();
    return true;
}
} // namespace QCatUiKit
