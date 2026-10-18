// SPDX-License-Identifier: MIT
#include <QCatUiKit/qcatchartaxis.h>
#include <algorithm>
#include <cmath>
namespace QCatUiKit
{
class ChartAxis::Impl
{
  public:
    Scale scale = Linear;
    bool automatic = true;
    bool reversed = false;
    double minimum = 0, maximum = 1;
    QString title;
    QStringList categories;
    int ticks = 6;
    int minorTicks = 0;
    Qt::Orientation orientation = Qt::Vertical;
    Side side = NearSide;
    bool visible = true;
};
ChartAxis::ChartAxis(QObject *parent) : QObject(parent), d(new Impl) {}
ChartAxis::~ChartAxis() = default;
ChartAxis::Scale ChartAxis::scale() const
{
    return d->scale;
}
void ChartAxis::setScale(Scale v)
{
    if (v < Linear || v > Category || v == d->scale)
        return;
    d->scale = v;
    if (v == Logarithmic && d->minimum <= 0)
    {
        d->minimum = 1;
        d->maximum = 10;
    }
    emit changed();
}
bool ChartAxis::reversed() const
{
    return d->reversed;
}
void ChartAxis::setReversed(bool v)
{
    if (d->reversed != v)
    {
        d->reversed = v;
        emit changed();
    }
}
bool ChartAxis::automatic() const
{
    return d->automatic;
}
void ChartAxis::setAutomatic(bool v)
{
    if (d->automatic != v)
    {
        d->automatic = v;
        emit changed();
    }
}
double ChartAxis::minimum() const
{
    return d->minimum;
}
double ChartAxis::maximum() const
{
    return d->maximum;
}
bool ChartAxis::setRange(double a, double b)
{
    if (!std::isfinite(a) || !std::isfinite(b) || a >= b || !std::isfinite(b - a) ||
        (d->scale == Logarithmic && a <= 0))
        return false;
    if (!d->automatic && d->minimum == a && d->maximum == b)
        return true;
    d->minimum = a;
    d->maximum = b;
    d->automatic = false;
    emit changed();
    return true;
}
QString ChartAxis::title() const
{
    return d->title;
}
void ChartAxis::setTitle(const QString &v)
{
    if (d->title != v)
    {
        d->title = v;
        emit changed();
    }
}
QStringList ChartAxis::categories() const
{
    return d->categories;
}
void ChartAxis::setCategories(const QStringList &v)
{
    if (d->categories != v)
    {
        d->categories = v;
        emit changed();
    }
}
int ChartAxis::tickCount() const
{
    return d->ticks;
}
void ChartAxis::setTickCount(int v)
{
    v = std::clamp(v, 2, 12);
    if (d->ticks != v)
    {
        d->ticks = v;
        emit changed();
    }
}
Qt::Orientation ChartAxis::orientation() const
{
    return d->orientation;
}
void ChartAxis::setOrientation(Qt::Orientation value)
{
    if ((value == Qt::Horizontal || value == Qt::Vertical) && value != d->orientation)
    {
        d->orientation = value;
        emit changed();
    }
}
ChartAxis::Side ChartAxis::side() const
{
    return d->side;
}
void ChartAxis::setSide(Side value)
{
    if ((value == NearSide || value == FarSide) && value != d->side)
    {
        d->side = value;
        emit changed();
    }
}
bool ChartAxis::isVisible() const
{
    return d->visible;
}
void ChartAxis::setVisible(bool value)
{
    if (value != d->visible)
    {
        d->visible = value;
        emit changed();
    }
}
int ChartAxis::minorTickCount() const
{
    return d->minorTicks;
}
void ChartAxis::setMinorTickCount(int v)
{
    v = std::clamp(v, 0, 10);
    if (d->minorTicks != v)
    {
        d->minorTicks = v;
        emit changed();
    }
}
} // namespace QCatUiKit
