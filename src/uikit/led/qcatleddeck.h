// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledstorage.h"

class QCATUIKIT_EXPORT QCatLedDeck : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedDeckRuntime)
    Q_PROPERTY(QCatLedLayoutModel *layout READ layout WRITE setLayout NOTIFY settingsChanged)
    Q_PROPERTY(
        QCatLedEffectRegistry *registry READ registry WRITE setRegistry NOTIFY settingsChanged)
    Q_PROPERTY(QCatLedStorageRepository *repository READ repository WRITE setRepository NOTIFY
                   settingsChanged)
    Q_PROPERTY(QCatLedSource *source READ source CONSTANT)
    Q_PROPERTY(bool playing READ playing NOTIFY stateChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY stateChanged)
    Q_PROPERTY(double effectTime READ effectTime NOTIFY stateChanged)
    Q_PROPERTY(int productionInterval READ productionInterval WRITE setProductionInterval NOTIFY
                   settingsChanged)
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY settingsChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY settingsChanged)
    Q_PROPERTY(QString browsingStorage READ browsingStorage NOTIFY stateChanged)
    Q_PROPERTY(QString activeStorage READ activeStorage NOTIFY stateChanged)
    Q_PROPERTY(QString activeRepositoryId READ activeRepositoryId NOTIFY stateChanged)
    Q_PROPERTY(QString activeLocation READ activeLocation NOTIFY stateChanged)
    Q_PROPERTY(QString activePlace READ activePlace NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap workingPlace READ workingPlace NOTIFY stateChanged)
    Q_PROPERTY(QVariantList parameterDefinitions READ parameterDefinitions NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)
  public:
    explicit QCatLedDeck(QObject *parent = nullptr);
    ~QCatLedDeck() override;
    QCatLedLayoutModel *layout() const;
    void setLayout(QCatLedLayoutModel *layout);
    QCatLedEffectRegistry *registry() const;
    void setRegistry(QCatLedEffectRegistry *registry);
    QCatLedStorageRepository *repository() const;
    void setRepository(QCatLedStorageRepository *repository);
    QCatLedSource *source() const;
    bool playing() const;
    bool loading() const;
    double effectTime() const;
    int productionInterval() const;
    void setProductionInterval(int ms);
    double speed() const;
    void setSpeed(double value);
    double brightness() const;
    void setBrightness(double value);
    QString browsingStorage() const;
    QString activeStorage() const;
    QString activePlace() const;
    QString activeLocation() const;
    QString activeRepositoryId() const;
    QVariantMap workingPlace() const;
    QVariantList parameterDefinitions() const;
    QString lastError() const;
    QCatLightingFrame latestFrame() const; // thread-safe immutable snapshot
    Q_INVOKABLE void browseStorage(const QString &storageId);
    Q_INVOKABLE bool loadPlace(const QString &storageId, const QString &placeId);
    Q_INVOKABLE bool activatePlace(const QString &storageId, const QString &placeId);
    Q_INVOKABLE bool loadConfiguration(const QVariantMap &place, bool activate = false);
    Q_INVOKABLE bool editPlace(const QVariantMap &changes);
    Q_INVOKABLE bool setParameter(const QString &name, double value);
    Q_INVOKABLE bool selectEffect(const QString &providerId, const QString &effectId, int schema);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void unload();
    Q_INVOKABLE void restart();
    Q_INVOKABLE bool previous();
    Q_INVOKABLE bool next();
    Q_INVOKABLE bool savePlace(const QString &storageId, const QString &placeId,
                               const QString &name);
    Q_INVOKABLE bool saveTo(QCatLedStorageRepository *repository, const QString &storageId,
                            const QString &placeId, const QString &name);
    Q_INVOKABLE QVariantMap configuration() const;
    Q_INVOKABLE bool restoreConfiguration(const QVariantMap &configuration);
  Q_SIGNALS:
    void settingsChanged();
    void stateChanged();
    void frameReady();

  private:
    bool prepare(const QVariantMap &place, const QString &storage, const QString &id, bool activate,
                 bool editing = false, QCatLedStorageRepository *origin = nullptr);
    bool adjacent(int direction);
    void tick();
    void advanceClock();
    void publish(const QByteArray &channels, QCatLighting::ColorSchema schema, quint64 instance);
    void layoutUpdated();
    struct Impl;
    const std::unique_ptr<Impl> d;
};
