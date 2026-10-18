// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartgrid.h"
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartContour : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartContour)
    Q_PROPERTY(QCatUiKit::ChartGrid *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QVariantList levels READ levels WRITE setLevels NOTIFY appearanceChanged)
    Q_PROPERTY(bool filled READ filled WRITE setFilled NOTIFY appearanceChanged)
    Q_PROPERTY(
        bool labelsVisible READ labelsVisible WRITE setLabelsVisible NOTIFY appearanceChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ChartContour(QObject *parent = nullptr);
    ~ChartContour() override;
    ChartGrid *source() const;
    void setSource(ChartGrid *v);
    QVariantList levels() const;
    void setLevels(const QVariantList &v);
    bool filled() const;
    void setFilled(bool v);
    bool labelsVisible() const;
    void setLabelsVisible(bool v);
    ChartSeries *series() const;
    QString error() const;
    Q_INVOKABLE bool refresh();
  signals:
    void sourceChanged();
    void appearanceChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartContour)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
