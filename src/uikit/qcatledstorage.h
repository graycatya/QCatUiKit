// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledeffectregistry.h"
#include <QAbstractListModel>
#include <QUrl>

class QCATUIKIT_EXPORT QCatLedStorageRepository : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedStorageRepository)
    Q_PROPERTY(quint64 thumbnailRevision READ thumbnailRevision NOTIFY thumbnailsChanged)
    Q_PROPERTY(QString repositoryId READ repositoryId NOTIFY changed)
    Q_PROPERTY(QVariantList storages READ storages NOTIFY changed)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly NOTIFY changed)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
  public:
    explicit QCatLedStorageRepository(QObject *parent = nullptr);
    ~QCatLedStorageRepository() override;
    quint64 thumbnailRevision() const;
    QString repositoryId() const;
    QVariantList storages() const;
    bool readOnly() const;
    void setReadOnly(bool value);
    QString lastError() const;
    Q_INVOKABLE QVariantMap document() const;
    Q_INVOKABLE bool replaceDocument(const QVariantMap &document);
    static QString validateDocument(const QVariantMap &document);
    Q_INVOKABLE QString createStorage(const QString &name, int slots = 12);
    Q_INVOKABLE QVariantList places(const QString &storageId) const;
    Q_INVOKABLE QVariantMap place(const QString &storageId, const QString &placeId) const;
    Q_INVOKABLE bool savePlace(const QString &storageId, const QString &placeId,
                               const QVariantMap &configuration);
    Q_INVOKABLE bool clearPlace(const QString &storageId, const QString &placeId);
    Q_INVOKABLE bool renamePlace(const QString &storageId, const QString &placeId,
                                 const QString &name);
    Q_INVOKABLE bool copyPlace(const QString &storageId, const QString &placeId,
                               const QString &targetStorage, const QString &targetPlace);
    Q_INVOKABLE QString exportJson(const QString &storageId = QString(),
                                   const QString &placeId = QString()) const;
    Q_INVOKABLE bool importJson(const QString &json, const QString &targetStorage = QString(),
                                const QString &targetPlace = QString());
    Q_INVOKABLE bool saveFile(const QUrl &url, const QString &storageId = QString(),
                              const QString &placeId = QString());
    Q_INVOKABLE bool loadFile(const QUrl &url, const QString &targetStorage = QString(),
                              const QString &targetPlace = QString());
    Q_INVOKABLE QString thumbnail(const QString &storageId, const QString &placeId,
                                  QCatLedEffectRegistry *registry);
    Q_INVOKABLE void populateExamples();
  Q_SIGNALS:
    void thumbnailsChanged();
    void changed();
    void statusChanged();

  protected:
    // Database/service adapters persist the complete candidate before committing the UI model.
    // Return false to leave the previous document intact. Runs on the owning thread.
    virtual bool persist(const QVariantMap &candidate, QString *error);

  private:
    bool commit(QVariantMap candidate);
    bool fail(const QString &error);
    struct Impl;
    const std::unique_ptr<Impl> d;
};

class QCATUIKIT_EXPORT QCatLedPlaceModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedPlaceModel)
    Q_PROPERTY(QCatLedStorageRepository *repository READ repository WRITE setRepository NOTIFY
                   settingsChanged)
    Q_PROPERTY(
        QCatLedEffectRegistry *registry READ registry WRITE setRegistry NOTIFY settingsChanged)
    Q_PROPERTY(QString storageId READ storageId WRITE setStorageId NOTIFY settingsChanged)
  public:
    explicit QCatLedPlaceModel(QObject *parent = nullptr);
    ~QCatLedPlaceModel() override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    QCatLedStorageRepository *repository() const;
    void setRepository(QCatLedStorageRepository *repository);
    QCatLedEffectRegistry *registry() const;
    void setRegistry(QCatLedEffectRegistry *registry);
    QString storageId() const;
    void setStorageId(const QString &id);
    Q_INVOKABLE QVariantMap get(int row) const;
  Q_SIGNALS:
    void settingsChanged();

  private:
    void refresh();
    struct Impl;
    const std::unique_ptr<Impl> d;
};
