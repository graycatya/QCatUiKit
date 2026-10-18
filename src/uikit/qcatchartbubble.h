// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartcolorscale.h"
#include "qcatchartseries.h"
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartBubble : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartBubble)
    Q_PROPERTY(QVariantList points READ points WRITE setPoints NOTIFY dataChanged)
    Q_PROPERTY(SizeMode sizeMode READ sizeMode WRITE setSizeMode NOTIFY appearanceChanged)
    Q_PROPERTY(double sizeScale READ sizeScale WRITE setSizeScale NOTIFY appearanceChanged)
    Q_PROPERTY(bool colorMapping READ colorMapping WRITE setColorMapping NOTIFY appearanceChanged)
    Q_PROPERTY(QCatUiKit::ChartColorScale *colorScale READ colorScale CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    enum SizeMode
    {
        Radius,
        Area
    };
    Q_ENUM(SizeMode)
    explicit ChartBubble(QObject *parent = nullptr);
    ~ChartBubble() override;
    QVariantList points() const;
    void setPoints(const QVariantList &v);
    Q_INVOKABLE bool setData(const QVariantList &v);
    SizeMode sizeMode() const;
    void setSizeMode(SizeMode v);
    double sizeScale() const;
    void setSizeScale(double v);
    bool colorMapping() const;
    void setColorMapping(bool v);
    ChartColorScale *colorScale() const;
    ChartSeries *series() const;
    QString error() const;
  signals:
    void dataChanged();
    void appearanceChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartBubble)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
