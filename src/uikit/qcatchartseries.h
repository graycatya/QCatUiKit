// SPDX-License-Identifier: MIT
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QVariantList>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include <memory>
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartSeries : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartSeries)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY styleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY styleChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY styleChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY styleChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY styleChanged)
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY capacityChanged)
    Q_PROPERTY(int count READ count NOTIFY dataChanged)
  public:
    enum Type
    {
        Inherit = -1,
        Line,
        Area,
        Bar,
        StackedBar,
        Scatter,
        Bubble,
        Pie,
        Doughnut,
        Radar,
        Heatmap
    };
    Q_ENUM(Type)
    // value is the bubble radius (logical pixels) or heatmap intensity.
    struct Sample
    {
        double x = 0, y = 0, value = 1;
        QString label;
    };
    explicit ChartSeries(QObject *parent = nullptr);
    ~ChartSeries() override;
    QString name() const;
    void setName(const QString &value);
    QColor color() const;
    void setColor(const QColor &value);
    bool isVisible() const;
    void setVisible(bool value);
    Type type() const;
    void setType(Type value);
    qreal lineWidth() const;
    void setLineWidth(qreal value);
    int capacity() const;
    void setCapacity(int value);
    int count() const;
    // Reference remains valid until the next mutation; do not retain it in consumers.
    const Sample &sampleAt(int index) const;
    bool sortedX() const;
    int lowerBound(double x) const;
    // Object-thread API. Producers on other threads must queue entire batches.
    bool appendSamples(const QVector<Sample> &samples);
    bool replaceSamples(const QVector<Sample> &samples);
    bool replacePoints(const QVector<QPointF> &points);
    Q_INVOKABLE bool append(double x, double y, double value = 1);
    Q_INVOKABLE bool appendData(const QVariantList &samples);
    Q_INVOKABLE bool setData(const QVariantList &samples);
    Q_INVOKABLE QVariantMap at(int index) const;
    Q_INVOKABLE void clear();
  signals:
    void dataChanged();
    void styleChanged();
    void capacityChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChartSeries)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
