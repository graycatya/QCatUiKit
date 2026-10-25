// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledsource.h"
#include <QVariantMap>
#include <memory>

// Share as const. Generator/model own the only mutable reference during construction.
struct QCATUIKIT_EXPORT QCatLedLayoutData
{
    QString projectId;
    quint64 revision = 0;
    QVariantMap configuration;
    QVector<QCatLedSource::Position> positions;
    int countX = 0, countY = 0, countZ = 0;
    static qsizetype index(int x, int y, int z, int nx, int ny);
};
using QCatLedLayoutPtr = std::shared_ptr<const QCatLedLayoutData>;

class QCATUIKIT_EXPORT QCatLedLayoutModel : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedLayoutModel)
    Q_PROPERTY(QVariantMap configuration READ configuration NOTIFY layoutChanged)
    Q_PROPERTY(int pointCount READ pointCount NOTIFY layoutChanged)
    Q_PROPERTY(quint64 revision READ revision NOTIFY layoutChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
  public:
    explicit QCatLedLayoutModel(QObject *parent = nullptr);
    ~QCatLedLayoutModel() override;
    QVariantMap configuration() const;
    int pointCount() const;
    quint64 revision() const;
    bool busy() const;
    QString lastError() const;
    QCatLedLayoutPtr snapshot() const;
    static constexpr int MaximumPoints = 1000000;
    static QVariantMap defaults();
    static QString validate(const QVariantMap &configuration);
    static QCatLedLayoutPtr generate(const QVariantMap &configuration, const QString &projectId,
                                     quint64 revision, QString *error = nullptr);
    Q_INVOKABLE QString validateConfiguration(const QVariantMap &configuration) const;
    Q_INVOKABLE void apply(const QVariantMap &configuration);
    Q_INVOKABLE bool applyNow(const QVariantMap &configuration);
    Q_INVOKABLE QVariantMap defaultConfiguration() const;
  Q_SIGNALS:
    void layoutChanged();
    void statusChanged();

  private:
    void launch();
    struct Impl;
    const std::unique_ptr<Impl> d;
};
