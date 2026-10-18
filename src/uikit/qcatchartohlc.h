// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartOhlc : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartOhlc)
    Q_PROPERTY(QVariantList records READ records WRITE setRecords NOTIFY dataChanged)
    Q_PROPERTY(bool candlesticks READ candlesticks WRITE setCandlesticks NOTIFY appearanceChanged)
    Q_PROPERTY(
        bool compressedTime READ compressedTime WRITE setCompressedTime NOTIFY appearanceChanged)
    Q_PROPERTY(QColor risingColor READ risingColor WRITE setRisingColor NOTIFY appearanceChanged)
    Q_PROPERTY(QColor fallingColor READ fallingColor WRITE setFallingColor NOTIFY appearanceChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartSeries *volumeSeries READ volumeSeries CONSTANT)
    Q_PROPERTY(QStringList categories READ categories NOTIFY dataChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ChartOhlc(QObject *parent = nullptr);
    ~ChartOhlc() override;
    QVariantList records() const;
    void setRecords(const QVariantList &value);
    Q_INVOKABLE bool setData(const QVariantList &value);
    bool candlesticks() const;
    void setCandlesticks(bool value);
    bool compressedTime() const;
    void setCompressedTime(bool value);
    QColor risingColor() const;
    void setRisingColor(const QColor &value);
    QColor fallingColor() const;
    void setFallingColor(const QColor &value);
    ChartSeries *series() const;
    ChartSeries *volumeSeries() const;
    QStringList categories() const;
    QString error() const;
  signals:
    void dataChanged();
    void appearanceChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartOhlc)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
