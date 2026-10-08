// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/ChatModel.h>
#include <QDateTime>
namespace QCatUiKit
{
class ChatModel::Impl
{
  public:
    QList<QVariantMap> messages;
};
ChatModel::ChatModel(QObject *parent) : QAbstractListModel(parent), d(std::make_unique<Impl>()) {}
ChatModel::~ChatModel() = default;
int ChatModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : count();
}
int ChatModel::count() const
{
    return int(d->messages.size());
}
QHash<int, QByteArray> ChatModel::roleNames() const
{
    return {{MessageIdRole, "messageId"},
            {UsernameRole, "username"},
            {SenderRole, "sender"},
            {RecipientRole, "recipient"},
            {TimestampRole, "timestamp"},
            {ContentRole, "content"},
            {TypeRole, "type"},
            {StatusRole, "status"},
            {SenderAvatarRole, "senderAvatar"},
            {RecipientAvatarRole, "recipientAvatar"}};
}
QVariant ChatModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this || index.column() != 0 || index.row() < 0 ||
        index.row() >= count())
        return {};
    const auto key = role == Qt::DisplayRole ? QByteArray("content") : roleNames().value(role);
    return d->messages.at(index.row()).value(QString::fromUtf8(key));
}
static bool validMessage(const QVariantMap &message)
{
    for (const auto &key : {QStringLiteral("type"), QStringLiteral("status")})
    {
        if (!message.contains(key))
            continue;
        bool ok = false;
        const int value = message.value(key).toInt(&ok);
        if (!ok || value < 0 || value > (key == "type" ? 3 : 2))
            return false;
    }
    return true;
}
bool ChatModel::append(const QVariantMap &message)
{
    if (!validMessage(message))
        return false;
    QVariantMap entry{{"messageId", QString()},
                      {"username", QString()},
                      {"sender", QString()},
                      {"recipient", QString()},
                      {"timestamp", QDateTime::currentMSecsSinceEpoch()},
                      {"content", QString()},
                      {"type", int(Text)},
                      {"status", int(Pending)},
                      {"senderAvatar", QString()},
                      {"recipientAvatar", QString()}};
    const auto roles = roleNames();
    for (const auto &key : roles)
        if (message.contains(QString::fromUtf8(key)))
            entry[QString::fromUtf8(key)] = message.value(QString::fromUtf8(key));
    const int row = count();
    beginInsertRows({}, row, row);
    d->messages.append(entry);
    endInsertRows();
    emit countChanged();
    return true;
}
bool ChatModel::update(int row, const QVariantMap &changes)
{
    if (row < 0 || row >= count() || !validMessage(changes))
        return false;
    QList<int> changed;
    const auto roles = roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it)
    {
        const auto key = QString::fromUtf8(it.value());
        if (changes.contains(key) && d->messages[row].value(key) != changes.value(key))
        {
            d->messages[row][key] = changes.value(key);
            changed.append(it.key());
            if (it.key() == ContentRole)
                changed.append(Qt::DisplayRole);
        }
    }
    if (!changed.isEmpty())
        emit dataChanged(index(row), index(row), changed);
    return true;
}
QVariantMap ChatModel::get(int row) const
{
    return row >= 0 && row < count() ? d->messages.at(row) : QVariantMap();
}
bool ChatModel::remove(int row)
{
    if (row < 0 || row >= count())
        return false;
    beginRemoveRows({}, row, row);
    d->messages.removeAt(row);
    endRemoveRows();
    emit countChanged();
    return true;
}
void ChatModel::clear()
{
    if (d->messages.isEmpty())
        return;
    beginResetModel();
    d->messages.clear();
    endResetModel();
    emit countChanged();
}
} // namespace QCatUiKit
