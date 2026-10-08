// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QAbstractTableModel>
#include <QCatUiKit/Export.h>
#include <QStringList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <memory>
namespace QCatUiKit
{
class QCATUIKIT_EXPORT TableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList columns READ columns WRITE setColumns NOTIFY columnsChanged FINAL)
    Q_PROPERTY(QStringList headers READ headers WRITE setHeaders NOTIFY headersChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
  public:
    explicit TableModel(QObject *parent = nullptr);
    ~TableModel() override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QStringList columns() const;
    void setColumns(const QStringList &value);
    QStringList headers() const;
    void setHeaders(const QStringList &value);
    int count() const;
    Q_INVOKABLE bool append(const QVariantMap &row);
    Q_INVOKABLE bool update(int row, const QVariantMap &changes);
    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE bool setCell(int row, int column, const QVariant &value);
    Q_INVOKABLE bool remove(int row);
    Q_INVOKABLE void clear();
  signals:
    void columnsChanged();
    void headersChanged();
    void countChanged();

  private:
    Q_DISABLE_COPY_MOVE(TableModel)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
