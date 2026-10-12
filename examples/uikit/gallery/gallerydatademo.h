// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/ChatModel>
#include <QCatUiKit/TableModel>
#include <QElapsedTimer>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

// Gallery-only exercises of the public models; not an installed library API.
class GalleryDataDemo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY settingsChanged)
    Q_PROPERTY(QCatUiKit::TableModel *tableModel READ tableModel CONSTANT)
    Q_PROPERTY(QCatUiKit::ChatModel *chatModel READ chatModel CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY dataChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE select NOTIFY dataChanged)
    Q_PROPERTY(QVariantMap selectedRecord READ selectedRecord NOTIFY dataChanged)
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY settingsChanged)
    Q_PROPERTY(int maximumCapacity READ maximumCapacity NOTIFY settingsChanged)
    Q_PROPERTY(int batchSize READ batchSize WRITE setBatchSize NOTIFY settingsChanged)
    Q_PROPERTY(int maximumBatchSize READ maximumBatchSize NOTIFY settingsChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY settingsChanged)
    Q_PROPERTY(int payloadSize READ payloadSize WRITE setPayloadSize NOTIFY settingsChanged)
    Q_PROPERTY(int workload READ workload WRITE setWorkload NOTIFY settingsChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(qint64 added READ added NOTIFY statisticsChanged)
    Q_PROPERTY(qint64 changed READ changed NOTIFY statisticsChanged)
    Q_PROPERTY(qint64 removed READ removed NOTIFY statisticsChanged)
    Q_PROPERTY(qint64 rejected READ rejected NOTIFY statisticsChanged)
    Q_PROPERTY(qint64 batches READ batches NOTIFY statisticsChanged)
    Q_PROPERTY(int lastBatchCount READ lastBatchCount NOTIFY statisticsChanged)
    Q_PROPERTY(double elapsedSeconds READ elapsedSeconds NOTIFY statisticsChanged)
    Q_PROPERTY(double operationsPerSecond READ operationsPerSecond NOTIFY statisticsChanged)
    Q_PROPERTY(double maximumBatchMs READ maximumBatchMs NOTIFY statisticsChanged)
  public:
    explicit GalleryDataDemo(QObject *parent = nullptr);
    int mode() const
    {
        return m_mode;
    }
    void setMode(int value);
    QCatUiKit::TableModel *tableModel()
    {
        return &m_table;
    }
    QCatUiKit::ChatModel *chatModel()
    {
        return &m_chat;
    }
    int count() const
    {
        return m_mode == 0 ? m_table.count() : m_chat.count();
    }
    int selectedIndex() const
    {
        return m_selected;
    }
    QVariantMap selectedRecord() const;
    int capacity() const
    {
        return m_capacity;
    }
    void setCapacity(int value);
    int maximumCapacity() const
    {
        return m_mode == 0 ? 100000 : 10000;
    }
    int batchSize() const
    {
        return m_batchSize;
    }
    void setBatchSize(int value);
    int maximumBatchSize() const
    {
        return m_mode == 0 ? 1000 : m_mode == 1 ? 100 : 25;
    }
    int interval() const
    {
        return m_timer.interval();
    }
    void setInterval(int value);
    int payloadSize() const
    {
        return m_payloadSize;
    }
    void setPayloadSize(int value);
    int workload() const
    {
        return m_workload;
    }
    void setWorkload(int value);
    bool running() const
    {
        return m_timer.isActive();
    }
    qint64 added() const
    {
        return m_added;
    }
    qint64 changed() const
    {
        return m_changed;
    }
    qint64 removed() const
    {
        return m_removed;
    }
    qint64 rejected() const
    {
        return m_rejected;
    }
    qint64 batches() const
    {
        return m_batches;
    }
    int lastBatchCount() const
    {
        return m_lastBatchCount;
    }
    double elapsedSeconds() const
    {
        return m_elapsedMs / 1000.;
    }
    double operationsPerSecond() const;
    double maximumBatchMs() const
    {
        return m_maximumBatchMs;
    }
    Q_INVOKABLE void select(int row);
    Q_INVOKABLE void addBatch();
    Q_INVOKABLE void updateBatch();
    Q_INVOKABLE void removeBatch();
    Q_INVOKABLE void removeSelected();
    Q_INVOKABLE bool editCell(int column, const QString &text);
    Q_INVOKABLE bool setMessageStatus(int status);
    Q_INVOKABLE void addMessage(bool incoming, int type = 0);
    Q_INVOKABLE void recordSubmission(const QString &text);
    Q_INVOKABLE void clearData();
    Q_INVOKABLE void resetData();
    Q_INVOKABLE void retranslate();
    Q_INVOKABLE void configureLargeData();
    Q_INVOKABLE void startStress();
    Q_INVOKABLE void stopStress();
  signals:
    void settingsChanged();
    void dataChanged();
    void statisticsChanged();
    void runningChanged();
    void submitRequested(const QString &text);

  private:
    enum Operation
    {
        Append,
        Update,
        Remove,
        Submit
    };
    void runBatch(Operation operation);
    void appendOne();
    void updateOne();
    void appendMessage(const QString &text, bool incoming, int type, int status);
    QString generatedText() const;
    void finishBatch(const QElapsedTimer &timer, int operations);
    void resetStatistics();
    void stressTick();
    QCatUiKit::TableModel m_table{this};
    QCatUiKit::ChatModel m_chat{this};
    QTimer m_timer;
    QElapsedTimer m_clock;
    int m_mode = 0, m_selected = -1, m_capacity = 100000, m_batchSize = 100;
    int m_payloadSize = 80, m_workload = 0, m_lastBatchCount = 0;
    qint64 m_cursor = 0;
    qint64 m_sequence = 0, m_added = 0, m_changed = 0, m_removed = 0;
    qint64 m_rejected = 0, m_batches = 0, m_elapsedMs = 0;
    double m_maximumBatchMs = 0;
    bool m_submitting = false;
};
