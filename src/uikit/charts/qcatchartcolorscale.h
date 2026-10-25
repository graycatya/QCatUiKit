// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QColor>
#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <memory>
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartColorScale : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartColorScale)
    Q_PROPERTY(bool automatic READ automatic WRITE setAutomatic NOTIFY changed)
    Q_PROPERTY(double minimum READ minimum NOTIFY rangeChanged)
    Q_PROPERTY(double maximum READ maximum NOTIFY rangeChanged)
    Q_PROPERTY(QVariantList colors READ colors WRITE setColors NOTIFY changed)
    Q_PROPERTY(bool legendVisible READ legendVisible WRITE setLegendVisible NOTIFY changed)
  public:
    explicit ChartColorScale(QObject *parent = nullptr);
    ~ChartColorScale() override;
    bool automatic() const;
    void setAutomatic(bool v);
    double minimum() const;
    double maximum() const;
    Q_INVOKABLE bool setRange(double minimum, double maximum);
    QVariantList colors() const;
    void setColors(const QVariantList &v);
    bool legendVisible() const;
    void setLegendVisible(bool v);
    Q_INVOKABLE QColor colorAt(double value) const;
    Q_INVOKABLE double normalized(double value) const;
    // Adapter inputs update the automatic extent; manual ranges remain unchanged.
    void setDataRange(double minimum, double maximum);
  signals:
    void changed();
    void rangeChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartColorScale)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
