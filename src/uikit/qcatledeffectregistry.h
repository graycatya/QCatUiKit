// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatlightingframe.h"

// Each Deck/thumbnail owns an independent instance. render runs on a worker thread.
class QCATUIKIT_EXPORT QCatLedEffectInstance
{
  public:
    virtual ~QCatLedEffectInstance();
    virtual QByteArray render(const QCatLedLayoutData &layout, const QVariantMap &parameters,
                              QCatLighting::ColorSchema schema, double effectSeconds) = 0;
};
class QCATUIKIT_EXPORT QCatLedEffectProvider
{
  public:
    virtual ~QCatLedEffectProvider();
    virtual QString id() const = 0;
    // Each descriptor: id, version, name, schemas, reverse, parameters[{id,name,default,min,max}].
    virtual QVariantList effects() const = 0;
    virtual std::unique_ptr<QCatLedEffectInstance> create(const QString &effectId, int version,
                                                          const QCatLedLayoutData &layout,
                                                          QCatLighting::ColorSchema schema,
                                                          const QVariantMap &parameters,
                                                          QString *error) const = 0;
};
class QCATUIKIT_EXPORT QCatLedEffectRegistry : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedEffectRegistry)
    Q_PROPERTY(QVariantList effects READ effects NOTIFY effectsChanged)
  public:
    explicit QCatLedEffectRegistry(QObject *parent = nullptr);
    ~QCatLedEffectRegistry() override;
    bool registerProvider(std::shared_ptr<const QCatLedEffectProvider> provider);
    QVariantList effects() const;
    Q_INVOKABLE QVariantMap effect(const QString &providerId, const QString &effectId) const;
    Q_INVOKABLE QString validatePlace(const QVariantMap &place) const;
    std::shared_ptr<QCatLedEffectInstance> prepare(const QVariantMap &place,
                                                   const QCatLedLayoutPtr &layout,
                                                   QVariantMap *parameters, QString *error) const;
  Q_SIGNALS:
    void effectsChanged();

  private:
    struct Impl;
    const std::unique_ptr<Impl> d;
};
