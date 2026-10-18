// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartFit : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartFit)
    Q_PROPERTY(QCatUiKit::ChartSeries *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartSeries *band READ band CONSTANT)
    Q_PROPERTY(double residualMultiplier READ residualMultiplier WRITE setResidualMultiplier NOTIFY
                   residualMultiplierChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY statisticsChanged)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY statisticsChanged)
    Q_PROPERTY(double slope READ slope NOTIFY statisticsChanged)
    Q_PROPERTY(double intercept READ intercept NOTIFY statisticsChanged)
    Q_PROPERTY(double rSquared READ rSquared NOTIFY statisticsChanged)
    Q_PROPERTY(double rootMeanSquareError READ rootMeanSquareError NOTIFY statisticsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ChartFit(QObject *parent = nullptr);
    ~ChartFit() override;
    ChartSeries *source() const;
    void setSource(ChartSeries *source);
    ChartSeries *series() const;
    ChartSeries *band() const;
    double residualMultiplier() const;
    void setResidualMultiplier(double value);
    bool isValid() const;
    int sampleCount() const;
    double slope() const;
    double intercept() const;
    double rSquared() const;
    double rootMeanSquareError() const;
    QString error() const;
    Q_INVOKABLE bool refresh();
    Q_INVOKABLE double predict(double x) const;
  signals:
    void sourceChanged();
    void residualMultiplierChanged();
    void statisticsChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartFit)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
