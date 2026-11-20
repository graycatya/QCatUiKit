// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QAbstractProxyModel>
#include <QCatUiKit/qcatuikitglobal.h>
#include <QtQml/qqmlregistration.h>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ColumnReverseProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    QML_ELEMENT
  public:
    explicit ColumnReverseProxyModel(QObject *parent = nullptr);
    ~ColumnReverseProxyModel() override;

    void setSourceModel(QAbstractItemModel *model) override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &proxyIndex, const QVariant &value,
                 int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &proxyIndex) const override;
    QHash<int, QByteArray> roleNames() const override;

  private:
    void connectSource();
    void disconnectSource();
};
} // namespace QCatUiKit
