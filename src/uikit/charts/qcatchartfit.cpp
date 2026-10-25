// SPDX-License-Identifier: Apache-2.0
#include "qcatchartfit.h"
#include "qcatchartstatistics_p.h"
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <limits>

namespace QCatUiKit
{
namespace
{
constexpr double nan = std::numeric_limits<double>::quiet_NaN();
double productRatio(double a, double b, double c)
{
    int ae, be, ce;
    const double am = std::frexp(a, &ae), bm = std::frexp(b, &be), cm = std::frexp(c, &ce);
    return std::scalbn(am * bm / cm, ae + be - ce);
}
} // namespace
class ChartFit::Impl
{
  public:
    QPointer<ChartSeries> source;
    QList<QMetaObject::Connection> connections;
    ChartSeries *series = nullptr, *band = nullptr;
    QTimer timer;
    double multiplier = 2, slope = nan, intercept = nan, rSquared = nan, rmse = nan;
    double originX = 0, originY = 0, scaleX = 1, scaleY = 1, meanX = 0, meanY = 0, beta = 0;
    int count = 0;
    bool valid = false;
    QString error;
    double predicted(double x) const
    {
        const double centered = (x - originX) / scaleX;
        return std::fma(scaleY, std::fma(beta, centered - meanX, meanY), originY);
    }
};
ChartFit::ChartFit(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    d->series = new ChartSeries(this);
    d->series->setType(ChartSeries::Line);
    d->band = new ChartSeries(this);
    d->band->setType(ChartSeries::RangeArea);
    d->band->setBorderColor(Qt::transparent);
    d->band->setFillOpacity(.2);
    d->timer.setParent(this);
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, &ChartFit::refresh);
}
ChartFit::~ChartFit() = default;
ChartSeries *ChartFit::source() const
{
    return d->source;
}
ChartSeries *ChartFit::series() const
{
    return d->series;
}
ChartSeries *ChartFit::band() const
{
    return d->band;
}
bool ChartFit::isValid() const
{
    return d->valid;
}
int ChartFit::sampleCount() const
{
    return d->count;
}
double ChartFit::slope() const
{
    return d->slope;
}
double ChartFit::intercept() const
{
    return d->intercept;
}
double ChartFit::rSquared() const
{
    return d->rSquared;
}
double ChartFit::rootMeanSquareError() const
{
    return d->rmse;
}
double ChartFit::residualMultiplier() const
{
    return d->multiplier;
}
QString ChartFit::error() const
{
    return d->error;
}
void ChartFit::setSource(ChartSeries *source)
{
    if (QThread::currentThread() != thread() || source == d->source || source == d->series ||
        source == d->band || (source && source->thread() != thread()))
        return;
    for (const auto &connection : d->connections)
        disconnect(connection);
    d->connections.clear();
    d->source = source;
    if (source)
    {
        d->connections = {
            connect(source, &ChartSeries::dataChanged, this, [this] { d->timer.start(0); }),
            connect(source, &ChartSeries::styleChanged, this,
                    [this]
                    {
                        if (d->series->xAxis() != d->source->xAxis() ||
                            d->series->yAxis() != d->source->yAxis())
                            d->timer.start(0);
                    }),
            connect(source, &QObject::destroyed, this,
                    [this]
                    {
                        d->source = nullptr;
                        refresh();
                        emit sourceChanged();
                    })};
    }
    d->timer.start(0);
    emit sourceChanged();
}
void ChartFit::setResidualMultiplier(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 10 ||
        value == d->multiplier)
        return;
    d->multiplier = value;
    d->timer.start(0);
    emit residualMultiplierChanged();
}
double ChartFit::predict(double x) const
{
    if (!d->valid || !std::isfinite(x))
        return nan;
    const double y = d->predicted(x);
    return std::isfinite(y) ? y : nan;
}
bool ChartFit::refresh()
{
    if (QThread::currentThread() != thread())
        return false;
    d->timer.stop();
    d->count = d->source ? d->source->count() : 0;
    const auto fail = [this](const QString &error)
    {
        d->valid = false;
        d->slope = d->intercept = d->rSquared = d->rmse = nan;
        d->series->clear();
        d->band->clear();
        if (d->error != error)
        {
            d->error = error;
            emit errorChanged();
        }
        emit statisticsChanged();
        return false;
    };
    for (auto *output : {d->series, d->band})
    {
        output->setXAxis(d->source ? d->source->xAxis() : nullptr);
        output->setYAxis(d->source ? d->source->yAxis() : nullptr);
    }
    if (!d->count)
        return fail({});
    if (d->count < 2)
        return fail(tr("A linear fit requires at least two observations."));
    double xmin = INFINITY, xmax = -INFINITY, ymin = INFINITY, ymax = -INFINITY;
    for (int i = 0; i < d->count; ++i)
    {
        const auto &p = d->source->sampleAt(i);
        xmin = std::min(xmin, p.x);
        xmax = std::max(xmax, p.x);
        ymin = std::min(ymin, p.y);
        ymax = std::max(ymax, p.y);
    }
    if (xmin == xmax)
        return fail(tr("A linear fit requires distinct X coordinates."));
    d->originX = xmin / 2 + xmax / 2;
    d->originY = ymin / 2 + ymax / 2;
    d->scaleX = std::max(xmax - d->originX, d->originX - xmin);
    d->scaleY = ymin == ymax ? 1 : std::max(ymax - d->originY, d->originY - ymin);
    d->meanX = d->meanY = 0;
    double xx = 0, xy = 0, yy = 0;
    // Center and scale before Welford covariance: timestamps retain small deltas,
    // while large finite inputs do not overflow sums of squares.
    for (int i = 0; i < d->count; ++i)
    {
        const auto &p = d->source->sampleAt(i);
        const double x = (p.x - d->originX) / d->scaleX, y = (p.y - d->originY) / d->scaleY;
        const double dx = x - d->meanX, dy = y - d->meanY;
        d->meanX += dx / (i + 1);
        d->meanY += dy / (i + 1);
        xx += dx * (x - d->meanX);
        xy += dx * (y - d->meanY);
        yy += dy * (y - d->meanY);
    }
    if (!(xx > 0))
        return fail(tr("A linear fit requires distinct X coordinates."));
    d->beta = xy / xx;
    d->slope = productRatio(d->beta, d->scaleY, d->scaleX);
    d->intercept =
        std::fma(-d->slope, d->originX,
                 std::fma(d->scaleY, std::fma(-d->beta, d->meanX, d->meanY), d->originY));
    double residuals = 0;
    for (int i = 0; i < d->count; ++i)
    {
        const auto &p = d->source->sampleAt(i);
        const double x = (p.x - d->originX) / d->scaleX, y = (p.y - d->originY) / d->scaleY;
        const double residual = y - std::fma(d->beta, x - d->meanX, d->meanY);
        residuals += residual * residual;
    }
    d->rmse = d->scaleY * std::sqrt(residuals / d->count);
    d->rSquared = yy > 0 ? std::clamp(1 - residuals / yy, 0., 1.) : 1;
    const double width = d->rmse * d->multiplier;
    if (!std::isfinite(d->slope) || !std::isfinite(d->intercept) || !std::isfinite(width))
        return fail(tr("Fit statistics exceed the finite numeric range."));
    QVariantList line, band;
    for (double x : {xmin, xmax})
    {
        const double y = d->predicted(x);
        if (!std::isfinite(y) || !std::isfinite(y - width) || !std::isfinite(y + width))
            return fail(tr("Fit boundaries exceed the finite numeric range."));
        const QVariantMap statistics{{"fitted", true},
                                     {"observationCount", d->count},
                                     {"predictedY", y},
                                     {"residualHalfWidth", width}};
        line.append(QVariantMap{{"x", x}, {"y", y}, {"statistics", statistics}});
        band.append(QVariantMap{
            {"x", x}, {"lower", y - width}, {"upper", y + width}, {"statistics", statistics}});
    }
    d->valid = true;
    ChartStatisticsAccess::publish(d->series, line, false);
    ChartStatisticsAccess::publish(d->band, band, true);
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    emit statisticsChanged();
    return true;
}
} // namespace QCatUiKit
