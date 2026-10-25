// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QAbstractListModel>
#include <QCatUiKit/qcatuikitglobal.h>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <memory>
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChatModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
  public:
    enum Role
    {
        MessageIdRole = Qt::UserRole + 1,
        UsernameRole,
        SenderRole,
        RecipientRole,
        TimestampRole,
        ContentRole,
        TypeRole,
        StatusRole,
        SenderAvatarRole,
        RecipientAvatarRole
    };
    Q_ENUM(Role)
    enum MessageType
    {
        Text = 0,
        Image = 1,
        Audio = 2,
        Video = 3
    };
    Q_ENUM(MessageType)
    enum MessageStatus
    {
        Pending = 0,
        Sent = 1,
        Failed = 2
    };
    Q_ENUM(MessageStatus)
    explicit ChatModel(QObject *parent = nullptr);
    ~ChatModel() override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const;
    Q_INVOKABLE bool append(const QVariantMap &message);
    Q_INVOKABLE bool update(int row, const QVariantMap &changes);
    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE bool remove(int row);
    Q_INVOKABLE void clear();
  signals:
    void countChanged();

  private:
    Q_DISABLE_COPY_MOVE(ChatModel)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
