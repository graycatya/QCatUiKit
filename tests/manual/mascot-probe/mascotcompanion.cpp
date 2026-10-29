// SPDX-License-Identifier: Apache-2.0
#include "mascotcompanion.h"
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <cmath>
namespace
{
constexpr auto service = "org.qcat.MascotProbe1";
constexpr auto path = "/org/qcat/MascotProbe1";
constexpr auto interface = "org.qcat.MascotProbe1";
QDBusMessage method(const QString &owner, const QString &name)
{
    auto message = QDBusMessage::createMethodCall(owner, path, interface, name);
    return message;
}
} // namespace
MascotCompanion::MascotCompanion(const QString &token, QObject *parent, const QDBusConnection &bus)
    : QObject(parent), m_bus(bus),
      m_watcher(service, bus, QDBusServiceWatcher::WatchForOwnerChange, this), m_token(token)
{
    connect(&m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            [this](const QString &, const QString &, const QString &owner)
            { connectOwner(owner); });
}
void MascotCompanion::start()
{
    const auto epoch = m_epoch;
    auto message = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus",
                                                  "org.freedesktop.DBus", "GetNameOwner");
    message << QString(service);
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(message, 1500), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher, epoch]
            {
                const QDBusReply<QString> reply = *watcher;
                watcher->deleteLater();
                if (epoch != m_epoch)
                    return;
                if (reply.isValid())
                    connectOwner(reply.value());
                else
                    setState("Missing",
                             "GNOME window companion is not available: " + reply.error().message());
            });
}
void MascotCompanion::setState(const QString &state, const QString &reason)
{
    m_state = state;
    m_reason = reason;
    emit changed();
}
void MascotCompanion::connectOwner(const QString &owner)
{
    const auto epoch = ++m_epoch;
    m_owner = owner;
    m_version.clear();
    m_shellVersion.clear();
    const auto pending = m_pending;
    m_pending.clear();
    setState(owner.isEmpty() ? "Disconnected" : "Connecting",
             owner.isEmpty()
                 ? "Companion disconnected; pending requests are cancelled, no automatic replay"
                 : "Checking protocol 1");
    for (auto it = pending.cbegin(); it != pending.cend(); ++it)
        emit completed(it.key(), false, "CompanionDisconnected", {});
    if (owner.isEmpty())
        return;
    auto identity = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus",
                                                   "org.freedesktop.DBus", "GetNameOwner");
    identity << QString("org.gnome.Shell");
    auto *identityWatcher = new QDBusPendingCallWatcher(m_bus.asyncCall(identity, 1500), this);
    connect(identityWatcher, &QDBusPendingCallWatcher::finished, this,
            [this, identityWatcher, epoch]
            {
                const QDBusReply<QString> reply = *identityWatcher;
                identityWatcher->deleteLater();
                if (epoch != m_epoch)
                    return;
                if (!reply.isValid() || reply.value() != m_owner)
                {
                    setState("Untrusted",
                             "Companion must be exported by the current org.gnome.Shell owner");
                    return;
                }
                handshake(epoch);
            });
}

void MascotCompanion::handshake(quint64 epoch)
{
    auto message = method(m_owner, "Hello");
    message << uint(1);
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(message, 1500), this);
    connect(
        watcher, &QDBusPendingCallWatcher::finished, this,
        [this, watcher, epoch]
        {
            const QDBusMessage reply = watcher->reply();
            watcher->deleteLater();
            if (epoch != m_epoch)
                return;
            const auto args = reply.arguments();
            if (reply.type() == QDBusMessage::ErrorMessage || args.size() != 3 ||
                args[0].metaType().id() != QMetaType::UInt || args[0].toUInt() != 1 ||
                args[1].toString().isEmpty() || args[1].toString().size() > 128 ||
                !args[2].toString().startsWith("50."))
            {
                setState("Incompatible",
                         "Protocol/Shell version rejected: " + reply.errorMessage());
                return;
            }
            m_version = args[1].toString();
            m_shellVersion = args[2].toString();
            setState(
                "Ready",
                "Protocol 1 handshake complete; window operations still require native readback");
        });
}
QJsonObject MascotCompanion::status() const
{
    return {{"state", m_state},
            {"reason", m_reason},
            {"protocol", 1},
            {"version", m_version},
            {"shellVersion", m_shellVersion},
            {"owner", m_owner}};
}
quint64 MascotCompanion::call(const QString &operation, const QJsonObject &arguments)
{
    const quint64 id = ++m_nextId;
    if (!ready() || m_pending.size() >= 32)
    {
        const QString reason = ready() ? "CompanionBusy" : "CompanionUnavailable";
        QTimer::singleShot(0, this, [this, id, reason] { emit completed(id, false, reason, {}); });
        return id;
    }
    const auto epoch = m_epoch;
    auto message = method(m_owner, "Call");
    message << uint(1) << m_token << operation
            << QString::fromUtf8(QJsonDocument(arguments).toJson(QJsonDocument::Compact));
    m_pending.insert(id, operation);
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(message, 2000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher, epoch, id]
            {
                const QDBusMessage reply = watcher->reply();
                watcher->deleteLater();
                if (epoch != m_epoch || !m_pending.remove(id))
                    return;
                QJsonParseError error;
                const auto data = reply.arguments();
                const auto document = QJsonDocument::fromJson(
                    data.size() == 1 ? data[0].toString().toUtf8() : QByteArray(), &error);
                if (reply.type() == QDBusMessage::ErrorMessage)
                    emit completed(id, false, reply.errorName() + ": " + reply.errorMessage(), {});
                else if (error.error != QJsonParseError::NoError || !document.isObject() ||
                         !document.object().value("applied").isBool())
                    emit completed(id, false, "InvalidCompanionReply", {});
                else
                {
                    const auto result = document.object();
                    const auto rect = result.value("rect").toArray();
                    bool valid =
                        result.value("pid").toInteger() == QCoreApplication::applicationPid() &&
                        result.value("nativeWayland").toBool() && result.value("above").isBool() &&
                        result.value("focused").isBool() && rect.size() == 4;
                    for (const auto &value : rect)
                        valid &= value.isDouble() && std::isfinite(value.toDouble()) &&
                                 std::floor(value.toDouble()) == value.toDouble();
                    valid &= rect.size() == 4 && rect[2].toDouble() > 0 && rect[3].toDouble() > 0;
                    if (!valid)
                        emit completed(id, false, "InvalidNativeWindowReply", {});
                    else
                        emit completed(id, result.value("applied").toBool(),
                                       result.value("applied").toBool()
                                           ? QString()
                                           : QString("NativeReadbackMismatch"),
                                       result);
                }
            });
    return id;
}
