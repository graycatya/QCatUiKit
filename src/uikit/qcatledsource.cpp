// SPDX-License-Identifier: Apache-2.0
#include "qcatledsource.h"
#include "qcatledsource_p.h"
#include <QMutex>
#include <QMutexLocker>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <utility>

static_assert(sizeof(QCatLedSource::Position) == 12);
struct QCatLedSource::Impl
{
    mutable QMutex mutex;
    std::shared_ptr<const QCatLedSnapshot> latest;
    QString error;
    quint64 layoutCounter = 0;
    quint64 submitted = 0;
    std::atomic_bool notificationPending{false};
};

QCatLedSource::QCatLedSource(QObject *parent) : QObject(parent), d(new Impl)
{
    auto frame = std::make_shared<QCatLedSnapshot>();
    frame->layout = std::make_shared<QCatLedLayout>();
    d->latest = frame;
}
QCatLedSource::~QCatLedSource() = default;

void QCatLedSource::scheduleNotification()
{
    if (!d->notificationPending.exchange(true))
        QMetaObject::invokeMethod(
            this,
            [this]
            {
                d->notificationPending.store(false);
                emit dataChanged();
            },
            Qt::QueuedConnection);
}

quint64 QCatLedSource::setLayout(const QVector<Position> &positions)
{
    auto layout = std::make_shared<QCatLedLayout>();
    // Own the memory even when the caller uses fromRawData or retains a pointer.
    QVector3D minimum, maximum;
    bool first = true;
    bool valid = positions.size() <= 10000000;
    if (valid)
        layout->positions.reserve(positions.size());
    for (const auto &p : positions)
    {
        if (!valid || !std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
            std::abs(p.x) > 1e12f || std::abs(p.y) > 1e12f || std::abs(p.z) > 1e12f)
        {
            valid = false;
            break;
        }
        layout->positions.append(p);
        const QVector3D v(p.x, p.y, p.z);
        if (first)
            minimum = maximum = v;
        first = false;
        minimum = {std::min(minimum.x(), p.x), std::min(minimum.y(), p.y),
                   std::min(minimum.z(), p.z)};
        maximum = {std::max(maximum.x(), p.x), std::max(maximum.y(), p.y),
                   std::max(maximum.z(), p.z)};
    }
    layout->center = (minimum + maximum) * 0.5f;
    const float radius = (maximum - minimum).length() * 0.5f;
    layout->radius = radius > 0 ? std::max(radius, 1e-4f) : 1.0f;
    auto frame = std::make_shared<QCatLedSnapshot>();
    frame->layout = layout;
    frame->rgba = QByteArray(valid ? positions.size() * 4 : 0, '\0');
    std::shared_ptr<const QCatLedSnapshot> previous;
    {
        QMutexLocker lock(&d->mutex);
        if (valid)
        {
            layout->id = ++d->layoutCounter;
            previous = std::exchange(d->latest, frame);
            d->error.clear();
        }
        else
            d->error = QStringLiteral("Layout rejected: require finite XYZ within +/-1e12 and at "
                                      "most 10 million points.");
    }
    scheduleNotification();
    return valid ? layout->id : 0;
}

bool QCatLedSource::submitFrame(const QByteArray &rgba, quint64 frameId, quint64 layoutId)
{
    const auto current = snapshot();
    QString error;
    if (rgba.size() != current->layout->positions.size() * 4)
        error = QStringLiteral("Color count mismatch: expected %1 RGBA bytes, received %2.")
                    .arg(current->layout->positions.size() * 4)
                    .arg(rgba.size());
    else if (layoutId && layoutId != current->layout->id)
        error = QStringLiteral("Frame rejected: obsolete layoutId.");
    auto frame = std::make_shared<QCatLedSnapshot>();
    if (error.isEmpty())
    {
        frame->layout = current->layout;
        frame->rgba = QByteArray(rgba.constData(), rgba.size());
        frame->frameId = frameId;
    }
    bool accepted = false;
    std::shared_ptr<const QCatLedSnapshot> previous;
    {
        QMutexLocker lock(&d->mutex);
        if (error.isEmpty() && current->layout != d->latest->layout)
            error = QStringLiteral("Frame rejected: layout changed during submission.");
        if (error.isEmpty() && frameId <= d->latest->frameId)
            error = QStringLiteral("Frame rejected: frameId must increase and be nonzero.");
        if (error.isEmpty())
        {
            previous = std::exchange(d->latest, frame);
            ++d->submitted;
            accepted = true;
        }
        d->error = error;
    }
    scheduleNotification();
    return accepted;
}
std::shared_ptr<const QCatLedSnapshot> QCatLedSource::snapshot() const
{
    QMutexLocker lock(&d->mutex);
    return d->latest;
}
int QCatLedSource::pointCount() const
{
    return int(snapshot()->layout->positions.size());
}
quint64 QCatLedSource::frameId() const
{
    return snapshot()->frameId;
}
quint64 QCatLedSource::layoutId() const
{
    return snapshot()->layout->id;
}
quint64 QCatLedSource::submittedFrames() const
{
    QMutexLocker lock(&d->mutex);
    return d->submitted;
}
QString QCatLedSource::lastError() const
{
    QMutexLocker lock(&d->mutex);
    return d->error;
}
