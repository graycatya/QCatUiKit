// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
#include <QQmlListProperty>
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartStack : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartStack)
    Q_CLASSINFO("DefaultProperty", "sources")
    Q_PROPERTY(QQmlListProperty<QCatUiKit::ChartSeries> sources READ sources NOTIFY sourcesChanged)
    Q_PROPERTY(QQmlListProperty<QCatUiKit::ChartSeries> series READ series NOTIFY seriesChanged)
    Q_PROPERTY(bool percentage READ percentage WRITE setPercentage NOTIFY percentageChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ChartStack(QObject *parent = nullptr);
    ~ChartStack() override;
    QQmlListProperty<ChartSeries> sources();
    QQmlListProperty<ChartSeries> series();
    bool percentage() const;
    void setPercentage(bool value);
    QString error() const;
    Q_INVOKABLE void addSource(QCatUiKit::ChartSeries *source);
    Q_INVOKABLE void removeSource(QCatUiKit::ChartSeries *source);
    Q_INVOKABLE void clearSources();
    Q_INVOKABLE bool refresh();
    ChartSeries *seriesAt(int index) const;
    int count() const;
  signals:
    void sourcesChanged();
    void seriesChanged();
    void percentageChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartStack)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
