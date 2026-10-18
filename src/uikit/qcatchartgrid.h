// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartcolorscale.h"
#include "qcatchartseries.h"
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartGrid : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartGrid)
    Q_PROPERTY(int rows READ rows NOTIFY dataChanged)
    Q_PROPERTY(int columns READ columns NOTIFY dataChanged)
    Q_PROPERTY(QVariantList values READ values NOTIFY dataChanged)
    Q_PROPERTY(QPointF origin READ origin WRITE setOrigin NOTIFY dataChanged)
    Q_PROPERTY(QSizeF spacing READ spacing WRITE setSpacing NOTIFY dataChanged)
    Q_PROPERTY(bool dense READ dense WRITE setDense NOTIFY appearanceChanged)
    Q_PROPERTY(QCatUiKit::ChartColorScale *colorScale READ colorScale CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ChartGrid(QObject *parent = nullptr);
    ~ChartGrid() override;
    int rows() const;
    int columns() const;
    QVariantList values() const;
    QPointF origin() const;
    void setOrigin(QPointF v);
    QSizeF spacing() const;
    void setSpacing(QSizeF v);
    bool dense() const;
    void setDense(bool v);
    ChartColorScale *colorScale() const;
    ChartSeries *series() const;
    QString error() const;
    Q_INVOKABLE bool setGrid(int rows, int columns, const QVariantList &values);
    Q_INVOKABLE bool setValue(int row, int column, const QVariant &value);
    Q_INVOKABLE bool appendRow(const QVariantList &values);
  signals:
    void dataChanged();
    void appearanceChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartGrid)
    class Impl;
    const std::unique_ptr<Impl> d;
    bool update(int rows, int columns, const QVariantList &values, QPointF origin, QSizeF spacing);
};
} // namespace QCatUiKit
