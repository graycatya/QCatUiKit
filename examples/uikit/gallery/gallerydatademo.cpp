// SPDX-License-Identifier: Apache-2.0
#include "gallerydatademo.h"
#include <algorithm>

using namespace QCatUiKit;
GalleryDataDemo::GalleryDataDemo(QObject *parent) : QObject(parent)
{
    m_table.setObjectName("demoTableModel");
    m_chat.setObjectName("demoChatModel");
    m_table.setColumns({"id", "name", "state", "value"});
    connect(&m_timer, &QTimer::timeout, this, &GalleryDataDemo::stressTick);
    resetData();
}
void GalleryDataDemo::setMode(int value)
{
    if (value < 0 || value > 2 || value == m_mode)
        return;
    stopStress();
    m_mode = value;
    resetData();
}
void GalleryDataDemo::setCapacity(int value)
{
    if (running())
        return;
    value = std::clamp(value, std::max(1, count()), maximumCapacity());
    if (m_capacity == value)
        return;
    m_capacity = value;
    emit settingsChanged();
}
void GalleryDataDemo::setBatchSize(int value)
{
    if (running())
        return;
    m_batchSize = std::clamp(value, 1, maximumBatchSize());
    emit settingsChanged();
}
void GalleryDataDemo::setInterval(int value)
{
    if (running())
        return;
    m_timer.setInterval(std::clamp(value, 16, 2000));
    emit settingsChanged();
}
void GalleryDataDemo::setPayloadSize(int value)
{
    if (running())
        return;
    m_payloadSize = std::clamp(value, 16, 4096);
    emit settingsChanged();
}
void GalleryDataDemo::setWorkload(int value)
{
    if (running())
        return;
    m_workload = std::clamp(value, 0, 2);
    emit settingsChanged();
}
QVariantMap GalleryDataDemo::selectedRecord() const
{
    return m_mode == 0 ? m_table.get(m_selected) : m_chat.get(m_selected);
}
void GalleryDataDemo::select(int row)
{
    const int next = count() ? std::clamp(row, 0, count() - 1) : -1;
    if (next == m_selected)
        return;
    m_selected = next;
    emit dataChanged();
}
void GalleryDataDemo::retranslate()
{
    m_table.setHeaders({tr("ID"), tr("Name"), tr("Status"), tr("Value")});
    for (int i = 0; i < m_chat.count(); ++i)
    {
        const auto row = m_chat.get(i);
        if (row.value("messageId") == "welcome")
            m_chat.update(
                i, {{"content", tr("Welcome! Try switching languages while writing a message.")}});
        if (row.value("sender") == "me")
            m_chat.update(i, {{"username", tr("You")}});
        else if (row.value("sender") == "peer")
            m_chat.update(i, {{"username", tr("Demo contact")}});
    }
    emit dataChanged();
}
void GalleryDataDemo::resetStatistics()
{
    m_added = m_changed = m_removed = m_rejected = m_batches = m_elapsedMs = 0;
    m_lastBatchCount = 0;
    m_maximumBatchMs = 0;
    emit statisticsChanged();
}
void GalleryDataDemo::resetData()
{
    stopStress();
    m_table.clear();
    m_chat.clear();
    m_sequence = 0;
    m_cursor = 0;
    m_capacity = maximumCapacity();
    m_batchSize = m_mode == 0 ? 100 : m_mode == 1 ? 10 : 5;
    m_timer.setInterval(100);
    m_payloadSize = 80;
    m_workload = 0;
    if (m_mode == 0)
    {
        for (const auto &name : {"CatButton", "ThemePalette", "TableModel"})
            m_table.append({{"id", ++m_sequence},
                            {"name", name},
                            {"state", tr("Ready")},
                            {"value", m_sequence * 10}});
    }
    else if (m_mode == 1)
    {
        m_chat.append({{"messageId", "welcome"},
                       {"sender", "guide"},
                       {"username", "QCatUiKit"},
                       {"content", tr("Welcome! Try switching languages while writing a message.")},
                       {"status", ChatModel::Sent}});
    }
    m_selected = count() ? 0 : -1;
    retranslate();
    resetStatistics();
    emit settingsChanged();
    emit dataChanged();
}
void GalleryDataDemo::configureLargeData()
{
    if (running())
        return;
    m_capacity = maximumCapacity();
    m_batchSize = m_mode == 0 ? 1000 : m_mode == 1 ? 100 : 25;
    m_timer.setInterval(50);
    emit settingsChanged();
}
QString GalleryDataDemo::generatedText() const
{
    const QString phrase =
        tr("Message %1 · A local demo, with no network request. ").arg(m_sequence + 1);
    return phrase.repeated(m_payloadSize / phrase.size() + 1).left(m_payloadSize);
}
void GalleryDataDemo::appendMessage(const QString &text, bool incoming, int type, int status)
{
    if (m_chat.count() >= m_capacity)
    {
        m_chat.remove(0);
        ++m_removed;
        m_selected = std::max(-1, m_selected - 1);
    }
    m_chat.append({{"messageId", QString::number(++m_sequence)},
                   {"sender", incoming ? "peer" : "me"},
                   {"username", incoming ? tr("Demo contact") : tr("You")},
                   {"content", text},
                   {"type", type},
                   {"status", status}});
    ++m_added;
}
void GalleryDataDemo::appendOne()
{
    if (m_mode == 0)
    {
        if (m_table.count() >= m_capacity)
        {
            updateOne();
            return;
        }
        ++m_sequence;
        m_table.append({{"id", m_sequence},
                        {"name", tr("Record %1").arg(m_sequence)},
                        {"state", tr("Ready")},
                        {"value", m_sequence % 1000}});
        ++m_added;
    }
    else
    {
        appendMessage(generatedText(), m_sequence % 2 == 0, ChatModel::Text, int(m_sequence % 3));
    }
}
void GalleryDataDemo::updateOne()
{
    if (!count())
    {
        appendOne();
        return;
    }
    const int row = m_cursor++ % count();
    if (m_mode == 0)
    {
        const auto old = m_table.get(row);
        m_table.update(row, {{"value", old.value("value").toInt() + 1}, {"state", tr("Updated")}});
    }
    else
    {
        const int status = (m_chat.get(row).value("status").toInt() + 1) % 3;
        m_chat.update(row, {{"status", status}});
    }
    ++m_changed;
}
void GalleryDataDemo::finishBatch(const QElapsedTimer &timer, int operations)
{
    m_selected = count() ? std::clamp(m_selected, 0, count() - 1) : -1;
    m_lastBatchCount = operations;
    ++m_batches;
    m_maximumBatchMs = std::max(m_maximumBatchMs, timer.nsecsElapsed() / 1000000.);
    if (running())
        m_elapsedMs = m_clock.elapsed();
    emit dataChanged();
    emit statisticsChanged();
}
void GalleryDataDemo::runBatch(Operation operation)
{
    QElapsedTimer timer;
    timer.start();
    int performed = 0;
    for (; performed < m_batchSize; ++performed)
    {
        if (operation == Append)
            appendOne();
        else if (operation == Update)
            updateOne();
        else if (operation == Remove)
        {
            if (!count())
                break;
            if (m_mode == 0)
                m_table.remove(count() - 1);
            else
                m_chat.remove(count() - 1);
            ++m_removed;
        }
        else
        {
            const bool blank = (m_added + m_rejected + 1) % 5 == 0;
            const qint64 before = m_added;
            m_submitting = true;
            emit submitRequested(blank ? QStringLiteral(" \t ") : generatedText());
            m_submitting = false;
            if (blank && before == m_added)
                ++m_rejected;
        }
        // Yield between operations so large configurations remain stoppable.
        if (timer.elapsed() >= 12)
        {
            ++performed;
            break;
        }
    }
    finishBatch(timer, performed);
}
void GalleryDataDemo::addBatch()
{
    if (!running())
        runBatch(Append);
}
void GalleryDataDemo::updateBatch()
{
    if (!running())
        runBatch(Update);
}
void GalleryDataDemo::removeBatch()
{
    if (!running())
        runBatch(Remove);
}
void GalleryDataDemo::removeSelected()
{
    if (running() || m_selected < 0)
        return;
    QElapsedTimer timer;
    timer.start();
    if (m_mode == 0)
        m_table.remove(m_selected);
    else
        m_chat.remove(m_selected);
    ++m_removed;
    finishBatch(timer, 1);
}
bool GalleryDataDemo::editCell(int column, const QString &text)
{
    if (running() || m_mode != 0 || column < 1 || column > 3 || m_selected < 0)
        return false;
    QElapsedTimer timer;
    timer.start();
    if (!m_table.setCell(m_selected, column, text))
        return false;
    ++m_changed;
    finishBatch(timer, 1);
    return true;
}
bool GalleryDataDemo::setMessageStatus(int status)
{
    if (running() || m_mode == 0 || m_selected < 0 || status < 0 || status > 2)
        return false;
    QElapsedTimer timer;
    timer.start();
    m_chat.update(m_selected, {{"status", status}});
    ++m_changed;
    finishBatch(timer, 1);
    return true;
}
void GalleryDataDemo::addMessage(bool incoming, int type)
{
    if (running() || m_mode == 0 || type < 0 || type > 3)
        return;
    QElapsedTimer timer;
    timer.start();
    const QString content =
        type == 0 ? generatedText() : tr("Demo attachment %1").arg(m_sequence + 1);
    appendMessage(content, incoming, type, ChatModel::Sent);
    m_selected = count() - 1;
    finishBatch(timer, 1);
}
void GalleryDataDemo::recordSubmission(const QString &text)
{
    if (m_mode == 0 || (running() && m_mode == 2 && !m_submitting))
        return;
    const auto trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return;
    QElapsedTimer timer;
    timer.start();
    appendMessage(trimmed, false, ChatModel::Text, ChatModel::Sent);
    m_selected = count() - 1;
    if (!m_submitting)
        finishBatch(timer, 1);
}
void GalleryDataDemo::clearData()
{
    if (running())
        return;
    QElapsedTimer timer;
    timer.start();
    const int previous = count();
    if (m_mode == 0)
        m_table.clear();
    else
        m_chat.clear();
    m_removed += previous;
    finishBatch(timer, previous);
}
double GalleryDataDemo::operationsPerSecond() const
{
    return m_elapsedMs > 0 ? (m_added + m_changed + m_removed + m_rejected) * 1000. / m_elapsedMs
                           : 0;
}
void GalleryDataDemo::startStress()
{
    if (running())
        return;
    resetStatistics();
    m_clock.start();
    m_timer.start();
    emit runningChanged();
}
void GalleryDataDemo::stopStress()
{
    if (!running())
        return;
    m_elapsedMs = m_clock.elapsed();
    m_timer.stop();
    emit runningChanged();
    emit statisticsChanged();
}
void GalleryDataDemo::stressTick()
{
    if (m_mode == 2)
    {
        runBatch(Submit);
        return;
    }
    if (m_workload == 2)
        runBatch(Update);
    else if (m_workload == 1 && m_batches % 6 == 4)
        runBatch(Update);
    else if (m_workload == 1 && m_batches % 6 == 5)
        runBatch(Remove);
    else
        runBatch(Append);
}
