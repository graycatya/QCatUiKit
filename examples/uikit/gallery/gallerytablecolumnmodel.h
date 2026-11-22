// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QIdentityProxyModel>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Gallery-only column visibility/order adapter; the library owns no column state.
class GalleryTableColumnModel : public QIdentityProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAbstractItemModel *sourceTableModel READ sourceTableModel
                       WRITE setSourceTableModel NOTIFY sourceTableModelChanged)
    Q_PROPERTY(QVariantList allColumns READ allColumns NOTIFY columnsChanged)
    Q_PROPERTY(QVariantList visibleColumns READ visibleColumns NOTIFY columnsChanged)
    Q_PROPERTY(int visibleColumnCount READ visibleColumnCount NOTIFY columnsChanged)

  public:
    explicit GalleryTableColumnModel(QObject *parent = nullptr);

    QAbstractItemModel *sourceTableModel() const;
    void setSourceTableModel(QAbstractItemModel *model);

    QVariantList allColumns() const;
    QVariantList visibleColumns() const;
    int visibleColumnCount() const;
    Q_INVOKABLE int sourceColumn(int proxyColumn) const;
    Q_INVOKABLE bool isColumnVisible(int sourceColumn) const;
    Q_INVOKABLE void setColumnVisible(int sourceColumn, bool visible);
    Q_INVOKABLE void moveColumn(int sourceColumn, int delta);
    Q_INVOKABLE void resetColumns();

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

  signals:
    void sourceTableModelChanged();
    void columnsChanged();

  private:
    void connectSource();
    void refreshColumns();
    QList<int> visibleSourceColumns() const;

    QList<int> m_order;
    QList<int> m_hidden;
};
