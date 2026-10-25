// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledmixer.h"
#include <atomic>

struct QCATUIKIT_EXPORT QCatFixtureProfile
{
    QString id, deviceId, portId;
    QCatLighting::ColorSchema schema = QCatLighting::RGB;
    QStringList channelOrder{"R", "G", "B"};
    double whiteKelvin = 4000, coldKelvin = 6500, warmKelvin = 2700;
    QCatLighting::Conversion conversion = QCatLighting::Strict;
    int firstPoint = 0, pointCount = 0, startChannel = 1;
    QVariantMap configuration() const;
    static bool parse(const QVariantMap &configuration, QCatFixtureProfile *profile,
                      QString *error);
};
struct QCATUIKIT_EXPORT QCatDeviceChannelFrame
{
    struct Port
    {
        QString deviceId, portId;
        int startChannel = 1;
        QByteArray channels;
    };
    QString projectId;
    quint64 layoutRevision = 0, mappingRevision = 0, frameId = 0;
    qint64 timestampUsec = 0;
    QVector<Port> ports;
};
using QCatDeviceFramePtr = std::shared_ptr<const QCatDeviceChannelFrame>;
class QCATUIKIT_EXPORT QCatPatchMapper
{
  public:
    static QString validate(const QVariantList &profiles, int pointCount);
    static QCatDeviceFramePtr map(const QCatLightingFrame &frame, const QCatLedLayoutPtr &layout,
                                  const QVariantList &profiles, quint64 mappingRevision,
                                  QString *error = nullptr);
};
// submit must honor cancellation before committing; one complete version per transaction.
// Called on a worker. Slow consumers receive only the latest frame on the next cycle.
class QCATUIKIT_EXPORT QCatOutputBackend
{
  public:
    virtual ~QCatOutputBackend();
    virtual bool submit(QCatDeviceFramePtr frame,
                        const std::shared_ptr<std::atomic_bool> &cancelled, QString *error) = 0;
    virtual void cancel() = 0;
};
class QCATUIKIT_EXPORT QCatMemoryOutputBackend : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MemoryOutputBackend)
    Q_PROPERTY(QVariantList ports READ ports NOTIFY diagnosticsChanged)
    Q_PROPERTY(quint64 receivedFrames READ receivedFrames NOTIFY diagnosticsChanged)
    Q_PROPERTY(quint64 frameId READ frameId NOTIFY diagnosticsChanged)
    Q_PROPERTY(quint64 mappingRevision READ mappingRevision NOTIFY diagnosticsChanged)
  public:
    explicit QCatMemoryOutputBackend(QObject *parent = nullptr);
    ~QCatMemoryOutputBackend() override;
    std::shared_ptr<QCatOutputBackend> consumer() const;
    QCatDeviceFramePtr latestFrame() const;
    QVariantList ports() const;
    quint64 receivedFrames() const;
    quint64 frameId() const;
    quint64 mappingRevision() const;
  Q_SIGNALS:
    void diagnosticsChanged();

  private:
    struct Impl;
    const std::unique_ptr<Impl> d;
};
class QCATUIKIT_EXPORT QCatLedOutput : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedOutput)
    Q_PROPERTY(QCatLedMixer *mixer READ mixer WRITE setMixer NOTIFY settingsChanged)
    Q_PROPERTY(QCatMemoryOutputBackend *memoryBackend READ memoryBackend WRITE setMemoryBackend
                   NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList profiles READ profiles NOTIFY settingsChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY settingsChanged)
    Q_PROPERTY(bool repeatFrames READ repeatFrames WRITE setRepeatFrames NOTIFY settingsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
  public:
    explicit QCatLedOutput(QObject *parent = nullptr);
    ~QCatLedOutput() override;
    QCatLedMixer *mixer() const;
    void setMixer(QCatLedMixer *mixer);
    QCatMemoryOutputBackend *memoryBackend() const;
    void setMemoryBackend(QCatMemoryOutputBackend *backend);
    void setBackend(std::shared_ptr<QCatOutputBackend> backend);
    QVariantList profiles() const;
    Q_INVOKABLE bool applyProfiles(const QVariantList &profiles);
    bool enabled() const;
    void setEnabled(bool value);
    int interval() const;
    void setInterval(int value);
    bool repeatFrames() const;
    void setRepeatFrames(bool value);
    QString lastError() const;
    Q_INVOKABLE QVariantMap configuration() const;
    Q_INVOKABLE bool restoreConfiguration(const QVariantMap &configuration);
    Q_INVOKABLE void cancel();
  Q_SIGNALS:
    void settingsChanged();
    void statusChanged();

  private:
    void tick();
    struct Impl;
    const std::unique_ptr<Impl> d;
};
