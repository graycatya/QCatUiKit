// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
#include <memory>
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartWaterfall : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatWaterfall)
    Q_PROPERTY(QVariantList steps READ steps WRITE setSteps NOTIFY stepsChanged)
    Q_PROPERTY(
        double initialValue READ initialValue WRITE setInitialValue NOTIFY initialValueChanged)
    Q_PROPERTY(bool resetOnSubtotal READ resetOnSubtotal WRITE setResetOnSubtotal NOTIFY
                   resetOnSubtotalChanged)
    Q_PROPERTY(bool connectorsVisible READ connectorsVisible WRITE setConnectorsVisible NOTIFY
                   appearanceChanged)
    Q_PROPERTY(
        QColor increaseColor READ increaseColor WRITE setIncreaseColor NOTIFY appearanceChanged)
    Q_PROPERTY(
        QColor decreaseColor READ decreaseColor WRITE setDecreaseColor NOTIFY appearanceChanged)
    Q_PROPERTY(
        QColor subtotalColor READ subtotalColor WRITE setSubtotalColor NOTIFY appearanceChanged)
    Q_PROPERTY(QColor totalColor READ totalColor WRITE setTotalColor NOTIFY appearanceChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QStringList categories READ categories NOTIFY statisticsChanged)
    Q_PROPERTY(double finalValue READ finalValue NOTIFY statisticsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    enum StepKind
    {
        Change,
        Subtotal,
        Total
    };
    Q_ENUM(StepKind)
    explicit ChartWaterfall(QObject *parent = nullptr);
    ~ChartWaterfall() override;
    QVariantList steps() const;
    void setSteps(const QVariantList &steps);
    Q_INVOKABLE bool setData(const QVariantList &steps);
    double initialValue() const;
    void setInitialValue(double value);
    bool resetOnSubtotal() const;
    void setResetOnSubtotal(bool value);
    bool connectorsVisible() const;
    void setConnectorsVisible(bool value);
    QColor increaseColor() const;
    void setIncreaseColor(const QColor &value);
    QColor decreaseColor() const;
    void setDecreaseColor(const QColor &value);
    QColor subtotalColor() const;
    void setSubtotalColor(const QColor &value);
    QColor totalColor() const;
    void setTotalColor(const QColor &value);
    ChartSeries *series() const;
    QStringList categories() const;
    double finalValue() const;
    QString error() const;
  signals:
    void stepsChanged();
    void initialValueChanged();
    void resetOnSubtotalChanged();
    void appearanceChanged();
    void statisticsChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartWaterfall)
    class Impl;
    const std::unique_ptr<Impl> d;
    bool update(const QVariantList &steps, double initial, bool reset);
    void setColor(int index, const QColor &color);
};
} // namespace QCatUiKit
