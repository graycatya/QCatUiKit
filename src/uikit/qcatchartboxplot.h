// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartBoxPlot : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartBoxPlot)
    Q_PROPERTY(QVariantList groups READ groups WRITE setGroups NOTIFY dataChanged)
    Q_PROPERTY(
        double whiskerFactor READ whiskerFactor WRITE setWhiskerFactor NOTIFY whiskerFactorChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartSeries *outlierSeries READ outlierSeries CONSTANT)
    Q_PROPERTY(QStringList categories READ categories NOTIFY dataChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ChartBoxPlot(QObject *parent = nullptr);
    ~ChartBoxPlot() override;
    QVariantList groups() const;
    void setGroups(const QVariantList &value);
    Q_INVOKABLE bool setData(const QVariantList &value);
    double whiskerFactor() const;
    void setWhiskerFactor(double value);
    ChartSeries *series() const;
    ChartSeries *outlierSeries() const;
    QStringList categories() const;
    QString error() const;
  signals:
    void dataChanged();
    void whiskerFactorChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartBoxPlot)
    class Impl;
    const std::unique_ptr<Impl> d;
    bool update(const QVariantList &value, double factor);
};
} // namespace QCatUiKit
