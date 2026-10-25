// SPDX-License-Identifier: Apache-2.0
#include "qcattiltcontroller.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace QCatUiKit
{
class TiltController::Impl
{
  public:
    bool enabled = false, allowed = false, calibrated = false, fresh = false;
    bool invertRoll = false, invertPitch = false;
    State state = Disabled;
    double fullScale = 25, deadzone = 2, tau = 100, expo = .2;
    double roll = 0, pitch = 0, filteredRoll = 0, filteredPitch = 0;
    int timeout = 250, duration = 500, count = 0;
    qint64 timestamp = -1, calibrationStart = -1;
    QQuaternion latest, zero, anchor, sum;
    QElapsedTimer arrival;
    QTimer watchdog;
    QString error;
};

TiltController::TiltController(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    // Parent the member timer so moveToThread also moves the watchdog.
    d->watchdog.setParent(this);
    d->watchdog.setSingleShot(true);
    d->watchdog.setTimerType(Qt::PreciseTimer);
    connect(&d->watchdog, &QTimer::timeout, this,
            [this] { fail(tr("Orientation input timed out.")); });
}
TiltController::~TiltController() = default;
bool TiltController::enabled() const
{
    return d->enabled;
}
bool TiltController::controlAllowed() const
{
    return d->allowed;
}
TiltController::State TiltController::state() const
{
    return d->state;
}
bool TiltController::calibrated() const
{
    return d->calibrated;
}
bool TiltController::sampleFresh() const
{
    return d->fresh;
}
bool TiltController::outputValid() const
{
    return d->state == Active;
}
double TiltController::rollInput() const
{
    return d->roll;
}
double TiltController::pitchInput() const
{
    return d->pitch;
}
double TiltController::fullScaleAngleDegrees() const
{
    return d->fullScale;
}
double TiltController::deadzoneDegrees() const
{
    return d->deadzone;
}
double TiltController::filterTimeConstantMs() const
{
    return d->tau;
}
double TiltController::expo() const
{
    return d->expo;
}
bool TiltController::invertRoll() const
{
    return d->invertRoll;
}
bool TiltController::invertPitch() const
{
    return d->invertPitch;
}
int TiltController::sampleTimeoutMs() const
{
    return d->timeout;
}
int TiltController::calibrationDurationMs() const
{
    return d->duration;
}
QString TiltController::error() const
{
    return d->error;
}

void TiltController::publish(State state, double roll, double pitch)
{
    const bool stateChangedValue = d->state != state;
    const bool outputChangedValue = d->roll != roll || d->pitch != pitch;
    d->state = state;
    d->roll = roll;
    d->pitch = pitch;
    if (stateChangedValue)
        emit stateChanged();
    if (outputChangedValue)
        emit outputChanged();
    emit controlSample(d->roll, d->pitch, outputValid());
}
void TiltController::resetCalibration()
{
    const bool cancelling = d->state == Calibrating;
    d->watchdog.stop();
    d->timestamp = -1;
    d->filteredRoll = d->filteredPitch = 0;
    if (d->fresh)
    {
        d->fresh = false;
        emit sampleFreshChanged();
    }
    if (d->calibrated)
    {
        d->calibrated = false;
        emit calibratedChanged();
    }
    if (!d->error.isEmpty())
    {
        d->error.clear();
        emit errorChanged();
    }
    publish(d->enabled ? Uncalibrated : Disabled);
    if (cancelling)
        emit calibrationFinished(false, tr("Calibration cancelled."));
}
void TiltController::setEnabled(bool value)
{
    if (d->enabled == value)
        return;
    d->enabled = value;
    resetCalibration();
    emit enabledChanged();
}
void TiltController::setControlAllowed(bool value)
{
    if (d->allowed == value)
        return;
    d->allowed = value;
    if (!value)
        disengage();
    emit controlAllowedChanged();
}
void TiltController::fail(const QString &reason)
{
    const bool cancelling = d->state == Calibrating;
    d->watchdog.stop();
    if (d->fresh)
    {
        d->fresh = false;
        emit sampleFreshChanged();
    }
    if (d->calibrated)
    {
        d->calibrated = false;
        emit calibratedChanged();
    }
    d->error = reason;
    emit errorChanged();
    publish(Fault);
    if (cancelling)
        emit calibrationFinished(false, reason);
    emit faultOccurred(reason);
}
bool TiltController::calibrate()
{
    if (!d->enabled || d->state == Active || d->state == Fault || d->state == Calibrating ||
        !d->fresh)
        return false;
    if (d->arrival.elapsed() >= d->timeout)
    {
        fail(tr("Orientation input timed out."));
        return false;
    }
    if (d->calibrated)
    {
        d->calibrated = false;
        emit calibratedChanged();
    }
    d->anchor = d->sum = d->latest;
    d->count = 1;
    d->calibrationStart = d->timestamp;
    d->filteredRoll = d->filteredPitch = 0;
    publish(Calibrating);
    return true;
}
bool TiltController::engage()
{
    if (d->state != Ready || !d->allowed || !d->calibrated || !d->fresh)
        return false;
    if (d->arrival.elapsed() >= d->timeout)
    {
        fail(tr("Orientation input timed out."));
        return false;
    }
    publish(Active, d->roll, d->pitch);
    return outputValid();
}
void TiltController::disengage()
{
    if (d->state == Active)
        publish(Ready, d->roll, d->pitch);
}
void TiltController::parametersUpdated()
{
    // Parameter changes require a new explicit engagement; no automatic resumption.
    disengage();
    d->filteredRoll = d->filteredPitch = 0;
    publish(d->state);
    emit parametersChanged();
}
void TiltController::setFullScaleAngleDegrees(double v)
{
    if (!std::isfinite(v) || v < 5 || v > 60 || v <= d->deadzone || v == d->fullScale)
        return;
    d->fullScale = v;
    parametersUpdated();
}
void TiltController::setDeadzoneDegrees(double v)
{
    if (!std::isfinite(v) || v < 0 || v >= d->fullScale || v == d->deadzone)
        return;
    d->deadzone = v;
    parametersUpdated();
}
void TiltController::setFilterTimeConstantMs(double v)
{
    if (!std::isfinite(v) || v < 0 || v > 1000 || v == d->tau)
        return;
    d->tau = v;
    parametersUpdated();
}
void TiltController::setExpo(double v)
{
    if (!std::isfinite(v) || v < 0 || v > 1 || v == d->expo)
        return;
    d->expo = v;
    parametersUpdated();
}
void TiltController::setInvertRoll(bool v)
{
    if (v == d->invertRoll)
        return;
    d->invertRoll = v;
    parametersUpdated();
}
void TiltController::setInvertPitch(bool v)
{
    if (v == d->invertPitch)
        return;
    d->invertPitch = v;
    parametersUpdated();
}
void TiltController::setSampleTimeoutMs(int v)
{
    if (v < 20 || v > 5000 || v == d->timeout)
        return;
    d->timeout = v;
    if (d->fresh)
    {
        const auto remaining = v - d->arrival.elapsed();
        if (remaining <= 0)
            fail(tr("Orientation input timed out."));
        else
            d->watchdog.start(int(remaining));
    }
    parametersUpdated();
}
void TiltController::setCalibrationDurationMs(int v)
{
    if (v < 100 || v > 3000 || v == d->duration)
        return;
    d->duration = v;
    resetCalibration();
    emit parametersChanged();
}
bool TiltController::submitOrientation(const QQuaternion &orientation, qint64 timestampUs)
{
    if (!d->enabled || d->state == Fault)
        return false;
    const double norm = double(orientation.scalar()) * orientation.scalar() +
                        double(orientation.x()) * orientation.x() +
                        double(orientation.y()) * orientation.y() +
                        double(orientation.z()) * orientation.z();
    if (!std::isfinite(norm) || norm < 1e-12 || timestampUs < 0 || timestampUs <= d->timestamp)
    {
        fail(tr("Invalid orientation or non-increasing timestamp."));
        return false;
    }
    const qint64 delta = d->timestamp < 0 ? 0 : timestampUs - d->timestamp;
    // The arrival clock also detects stalls when the event loop has not run the timer yet.
    if (d->fresh && (delta > qint64(d->timeout) * 1000 || d->arrival.elapsed() >= d->timeout))
    {
        fail(tr("Orientation input timed out."));
        return false;
    }
    const double scale = 1.0 / std::sqrt(norm);
    const QQuaternion q(float(orientation.scalar() * scale), float(orientation.x() * scale),
                        float(orientation.y() * scale), float(orientation.z() * scale));
    d->latest = q;
    d->timestamp = timestampUs;
    d->arrival.start();
    d->watchdog.start(d->timeout);
    if (!d->fresh)
    {
        d->fresh = true;
        emit sampleFreshChanged();
    }
    if (d->state == Calibrating)
    {
        const double dot = QQuaternion::dotProduct(d->anchor, q);
        // Stay within a 2-degree cone for a full calibration interval.
        if (std::abs(dot) < std::cos(qDegreesToRadians(1.0)))
        {
            d->anchor = d->sum = q;
            d->count = 1;
            d->calibrationStart = timestampUs;
        }
        else
        {
            d->sum += dot < 0 ? -q : q;
            ++d->count;
        }
        if (d->count >= 3 && timestampUs - d->calibrationStart >= qint64(d->duration) * 1000)
        {
            d->zero = d->sum.normalized();
            d->calibrated = true;
            emit calibratedChanged();
            publish(Ready);
            emit calibrationFinished(true, {});
        }
        return true;
    }
    if (!d->calibrated)
        return true;
    // +X right, +Y toward the phone's top, +Z out of its screen. Pure local Z twist
    // does not move the normal, so it produces no control input. q maps device to reference.
    // Lowering the right edge is +roll; lowering the top/forward edge is +pitch.
    const QVector3D normal = (d->zero.conjugated() * q).rotatedVector(QVector3D(0, 0, 1));
    if (normal.z() < std::cos(qDegreesToRadians(75.0)))
    {
        fail(tr("Phone tilt exceeds the 75-degree operating range."));
        return false;
    }
    const double roll = qRadiansToDegrees(std::atan2(double(normal.x()), double(normal.z())));
    const double pitch = qRadiansToDegrees(std::atan2(double(normal.y()), double(normal.z())));
    const double alpha = d->tau == 0 ? 1 : -std::expm1(-double(delta) / (1000.0 * d->tau));
    d->filteredRoll += alpha * (roll - d->filteredRoll);
    d->filteredPitch += alpha * (pitch - d->filteredPitch);
    const auto map = [this](double angle, bool invert)
    {
        const double a = std::clamp(angle, -d->fullScale, d->fullScale);
        const double v = std::copysign(
            std::max(std::abs(a) - d->deadzone, 0.0) / (d->fullScale - d->deadzone), a);
        return (invert ? -1 : 1) * ((1 - d->expo) * v + d->expo * v * v * v);
    };
    publish(d->state, map(d->filteredRoll, d->invertRoll), map(d->filteredPitch, d->invertPitch));
    return true;
}
} // namespace QCatUiKit
