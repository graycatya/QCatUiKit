// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartHistogram : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatHistogram)
    Q_PROPERTY(QVariantList values READ values WRITE setValues NOTIFY valuesChanged)
    Q_PROPERTY(int binCount READ binCount WRITE setBinCount NOTIFY binCountChanged)
    Q_PROPERTY(QList<double> binEdges READ binEdges WRITE setBinEdges NOTIFY binEdgesChanged)
    Q_PROPERTY(Normalization normalization READ normalization WRITE setNormalization NOTIFY
                   normalizationChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries *series READ series CONSTANT)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY statisticsChanged)
    Q_PROPERTY(int includedCount READ includedCount NOTIFY statisticsChanged)
    Q_PROPERTY(int underflowCount READ underflowCount NOTIFY statisticsChanged)
    Q_PROPERTY(int overflowCount READ overflowCount NOTIFY statisticsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    enum Normalization
    {
        Count,
        Frequency,
        Density
    };
    Q_ENUM(Normalization)
    explicit ChartHistogram(QObject *parent = nullptr);
    ~ChartHistogram() override;
    QVariantList values() const;
    void setValues(const QVariantList &values);
    Q_INVOKABLE bool setData(const QVariantList &values);
    int binCount() const;
    void setBinCount(int value);
    QList<double> binEdges() const;
    void setBinEdges(const QList<double> &edges);
    Normalization normalization() const;
    void setNormalization(Normalization value);
    ChartSeries *series() const;
    int sampleCount() const;
    int includedCount() const;
    int underflowCount() const;
    int overflowCount() const;
    QString error() const;
  signals:
    void valuesChanged();
    void binCountChanged();
    void binEdgesChanged();
    void normalizationChanged();
    void statisticsChanged();
    void errorChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartHistogram)
    class Impl;
    const std::unique_ptr<Impl> d;
    bool update(const QVariantList &values, int bins, const QList<double> &edges,
                Normalization normalization);
};
} // namespace QCatUiKit
