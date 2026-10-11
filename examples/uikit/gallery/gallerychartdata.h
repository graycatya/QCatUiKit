// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatchartitem.h>
#include <QPointer>
#include <QtQml/qqmlregistration.h>

// Demonstration data is application code, never part of the installed UI library.
class GalleryChartData : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString hint READ hint NOTIFY metadataChanged)
  public:
    explicit GalleryChartData(QObject *parent = nullptr);
    Q_INVOKABLE void load(int mode, QCatUiKit::ChartItem *chart, QCatUiKit::ChartSeries *first,
                          QCatUiKit::ChartSeries *second);
    Q_INVOKABLE void retranslate();
    Q_INVOKABLE void advance();
    QString hint() const;
  signals:
    void metadataChanged();

  private:
    int m_mode = 0;
    QPointer<QCatUiKit::ChartItem> m_chart;
    QPointer<QCatUiKit::ChartSeries> m_first, m_second;
    QString m_title, m_xTitle, m_yTitle, m_firstName, m_secondName;
    QStringList m_categories, m_firstLabels, m_secondLabels;
};
