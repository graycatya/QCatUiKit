// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatchartitem.h>
#include <QCatUiKit/qcatchartfit.h>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

// Demonstration data is application code, never part of the installed UI library.
class GalleryChartData : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString hint READ hint NOTIFY metadataChanged)
    Q_PROPERTY(QString unitName READ unitName NOTIFY metadataChanged)
    Q_PROPERTY(int batchSize READ batchSize WRITE setBatchSize NOTIFY settingsChanged)
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY settingsChanged)
    Q_PROPERTY(int maximumCapacity READ maximumCapacity NOTIFY settingsChanged)
    Q_PROPERTY(int maximumBatchSize READ maximumBatchSize NOTIFY settingsChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY settingsChanged)
    Q_PROPERTY(int barVariant READ barVariant WRITE setBarVariant NOTIFY settingsChanged)
    Q_PROPERTY(bool running READ running NOTIFY statisticsChanged)
    Q_PROPERTY(int unitCount READ unitCount NOTIFY statisticsChanged)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY statisticsChanged)
    Q_PROPERTY(int displayedSampleCount READ displayedSampleCount NOTIFY statisticsChanged)
    Q_PROPERTY(bool aggregated READ aggregated NOTIFY statisticsChanged)
    Q_PROPERTY(QString previewDescription READ previewDescription NOTIFY metadataChanged)
    Q_PROPERTY(qint64 added READ added NOTIFY statisticsChanged)
    Q_PROPERTY(qint64 removed READ removed NOTIFY statisticsChanged)
    Q_PROPERTY(qint64 updates READ updates NOTIFY statisticsChanged)
    Q_PROPERTY(double elapsedSeconds READ elapsedSeconds NOTIFY statisticsChanged)
    Q_PROPERTY(double updatesPerSecond READ updatesPerSecond NOTIFY statisticsChanged)
    Q_PROPERTY(double maximumBatchMs READ maximumBatchMs NOTIFY statisticsChanged)
  public:
    explicit GalleryChartData(QObject *parent = nullptr);
    Q_INVOKABLE void load(int mode, QCatUiKit::ChartItem *chart, QCatUiKit::ChartSeries *first,
                          QCatUiKit::ChartSeries *second);
    Q_INVOKABLE bool populateRangeSeries(QCatUiKit::ChartSeries *series, int count, bool broken,
                                         bool crossed);
    Q_INVOKABLE void retranslate();
    Q_INVOKABLE void addData();
    Q_INVOKABLE void removeData();
    Q_INVOKABLE void clearData();
    Q_INVOKABLE void resetData();
    Q_INVOKABLE void startStress();
    Q_INVOKABLE void stopStress();
    Q_INVOKABLE void configureMillionSamples();
    Q_INVOKABLE QVariantMap sourceAt(int series, int index) const;
    QString hint() const;
    QString unitName() const;
    int batchSize() const
    {
        return m_batchSize;
    }
    void setBatchSize(int value);
    int capacity() const
    {
        return m_capacity;
    }
    void setCapacity(int value);
    int maximumCapacity() const;
    int maximumBatchSize() const;
    int interval() const
    {
        return m_timer.interval();
    }
    void setInterval(int value);
    int barVariant() const
    {
        return m_barVariant;
    }
    void setBarVariant(int value);
    bool running() const
    {
        return m_timer.isActive();
    }
    int unitCount() const;
    int sampleCount() const;
    int displayedSampleCount() const;
    bool aggregated() const;
    QString previewDescription() const;
    qint64 added() const
    {
        return m_added;
    }
    qint64 removed() const
    {
        return m_removed;
    }
    qint64 updates() const
    {
        return m_updates;
    }
    double elapsedSeconds() const
    {
        return m_elapsedMs / 1000.;
    }
    double updatesPerSecond() const
    {
        return m_elapsedMs > 0 ? m_updates * 1000. / m_elapsedMs : 0;
    }
    double maximumBatchMs() const
    {
        return m_maximumBatchMs;
    }
  signals:
    void metadataChanged();
    void settingsChanged();
    void statisticsChanged();

  private:
    QCatUiKit::ChartFit m_fit;
    int m_mode = 0;
    int m_barVariant = 0;
    bool horizontalBars() const;
    QPointer<QCatUiKit::ChartItem> m_chart;
    QPointer<QCatUiKit::ChartSeries> m_first, m_second;
    QPointer<QCatUiKit::ChartSeries> m_previewFirst, m_previewSecond;
    std::unique_ptr<QCatUiKit::ChartSeries> m_bufferFirst, m_bufferSecond;
    QString m_title, m_xTitle, m_yTitle, m_firstName, m_secondName;
    QStringList m_categories, m_firstLabels, m_secondLabels;
    QTimer m_timer;
    QElapsedTimer m_clock;
    int m_batchSize = 1, m_capacity = 100000;
    qint64 m_nextIndex = 0, m_added = 0, m_removed = 0, m_updates = 0, m_elapsedMs = 0;
    double m_maximumBatchMs = 0;
    bool categorical() const;
    bool singleSeries() const;
    int samplesPerUnit() const;
    int previewUnitLimit() const;
    void bufferSources();
    void updatePreview();
    void synchronizeMetadata();
    void updateTrend();
    void finishMutation(qint64 added, qint64 removed, const QElapsedTimer &timer);
    void resetStatistics();
    void stressTick();
};
