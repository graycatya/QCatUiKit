// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QHash>
#include <QJsonObject>
#include <QObject>

// Private prototype client, deliberately outside the installed library.
class MascotCompanion final : public QObject
{
    Q_OBJECT
  public:
    explicit MascotCompanion(const QString &token, QObject *parent = nullptr,
                             const QDBusConnection &bus = QDBusConnection::sessionBus());
    void start();
    quint64 call(const QString &operation, const QJsonObject &arguments = {});
    bool ready() const
    {
        return m_state == "Ready";
    }
    QJsonObject status() const;
  signals:
    void changed();
    void completed(quint64 id, bool applied, const QString &reason, const QJsonObject &result);

  private:
    QDBusConnection m_bus;
    QDBusServiceWatcher m_watcher;
    QString m_token, m_owner, m_state = "Missing", m_reason, m_version, m_shellVersion;
    quint64 m_epoch = 0, m_nextId = 0;
    QHash<quint64, QString> m_pending;
    void connectOwner(const QString &owner);
    void handshake(quint64 epoch);
    void setState(const QString &state, const QString &reason);
};
