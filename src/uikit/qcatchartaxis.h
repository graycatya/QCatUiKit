// SPDX-License-Identifier: MIT
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartAxis : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartAxis)
    Q_PROPERTY(Scale scale READ scale WRITE setScale NOTIFY changed)
    Q_PROPERTY(bool automatic READ automatic WRITE setAutomatic NOTIFY changed)
    Q_PROPERTY(double minimum READ minimum NOTIFY changed)
    Q_PROPERTY(double maximum READ maximum NOTIFY changed)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY changed)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories NOTIFY changed)
    Q_PROPERTY(int tickCount READ tickCount WRITE setTickCount NOTIFY changed)
  public:
    enum Scale
    {
        Linear,
        Logarithmic,
        DateTime,
        Category
    };
    Q_ENUM(Scale)
    explicit ChartAxis(QObject *parent = nullptr);
    ~ChartAxis() override;
    Scale scale() const;
    void setScale(Scale value);
    bool automatic() const;
    void setAutomatic(bool value);
    double minimum() const;
    double maximum() const;
    Q_INVOKABLE bool setRange(double minimum, double maximum);
    QString title() const;
    void setTitle(const QString &value);
    QStringList categories() const;
    void setCategories(const QStringList &value);
    int tickCount() const;
    void setTickCount(int value);
  signals:
    void changed();

  private:
    Q_DISABLE_COPY_MOVE(ChartAxis)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
