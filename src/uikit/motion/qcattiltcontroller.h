// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QObject>
#include <QQuaternion>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT TiltController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool controlAllowed READ controlAllowed WRITE setControlAllowed NOTIFY
                   controlAllowedChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool calibrated READ calibrated NOTIFY calibratedChanged)
    Q_PROPERTY(bool sampleFresh READ sampleFresh NOTIFY sampleFreshChanged)
    Q_PROPERTY(bool outputValid READ outputValid NOTIFY stateChanged)
    Q_PROPERTY(double rollInput READ rollInput NOTIFY outputChanged)
    Q_PROPERTY(double pitchInput READ pitchInput NOTIFY outputChanged)
    Q_PROPERTY(double fullScaleAngleDegrees READ fullScaleAngleDegrees WRITE
                   setFullScaleAngleDegrees NOTIFY parametersChanged)
    Q_PROPERTY(double deadzoneDegrees READ deadzoneDegrees WRITE setDeadzoneDegrees NOTIFY
                   parametersChanged)
    Q_PROPERTY(double filterTimeConstantMs READ filterTimeConstantMs WRITE setFilterTimeConstantMs
                   NOTIFY parametersChanged)
    Q_PROPERTY(double expo READ expo WRITE setExpo NOTIFY parametersChanged)
    Q_PROPERTY(bool invertRoll READ invertRoll WRITE setInvertRoll NOTIFY parametersChanged)
    Q_PROPERTY(bool invertPitch READ invertPitch WRITE setInvertPitch NOTIFY parametersChanged)
    Q_PROPERTY(
        int sampleTimeoutMs READ sampleTimeoutMs WRITE setSampleTimeoutMs NOTIFY parametersChanged)
    Q_PROPERTY(int calibrationDurationMs READ calibrationDurationMs WRITE setCalibrationDurationMs
                   NOTIFY parametersChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    enum State
    {
        Disabled,
        Uncalibrated,
        Calibrating,
        Ready,
        Active,
        Fault
    };
    Q_ENUM(State)
    explicit TiltController(QObject *parent = nullptr);
    ~TiltController() override;
    bool enabled() const;
    void setEnabled(bool value);
    bool controlAllowed() const;
    void setControlAllowed(bool value);
    State state() const;
    bool calibrated() const;
    bool sampleFresh() const;
    bool outputValid() const;
    double rollInput() const;
    double pitchInput() const;
    double fullScaleAngleDegrees() const;
    void setFullScaleAngleDegrees(double value);
    double deadzoneDegrees() const;
    void setDeadzoneDegrees(double value);
    double filterTimeConstantMs() const;
    void setFilterTimeConstantMs(double value);
    double expo() const;
    void setExpo(double value);
    bool invertRoll() const;
    void setInvertRoll(bool value);
    bool invertPitch() const;
    void setInvertPitch(bool value);
    int sampleTimeoutMs() const;
    void setSampleTimeoutMs(int value);
    int calibrationDurationMs() const;
    void setCalibrationDurationMs(int value);
    QString error() const;
    // Device/control frame -> fixed reference frame; monotonically increasing microseconds.
    // Call on this object's thread. There is no sensor or transport dependency.
    Q_INVOKABLE bool submitOrientation(const QQuaternion &orientation, qint64 timestampUs);
    Q_INVOKABLE bool calibrate();
    Q_INVOKABLE bool engage();
    Q_INVOKABLE void disengage();
    Q_INVOKABLE void resetCalibration();
  signals:
    void enabledChanged();
    void controlAllowedChanged();
    void stateChanged();
    void calibratedChanged();
    void sampleFreshChanged();
    void outputChanged();
    void parametersChanged();
    void errorChanged();
    void controlSample(double roll, double pitch, bool valid);
    void calibrationFinished(bool success, QString reason);
    void faultOccurred(QString reason);

  private:
    Q_DISABLE_COPY_MOVE(TiltController)
    class Impl;
    const std::unique_ptr<Impl> d;
    void publish(State state, double roll = 0, double pitch = 0);
    void fail(const QString &reason);
    void parametersUpdated();
};
} // namespace QCatUiKit
