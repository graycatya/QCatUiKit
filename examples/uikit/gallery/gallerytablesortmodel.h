// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/TableModel>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QtQml/qqmlregistration.h>

// Gallery-only sorting adapter; the library deliberately owns no sort state.
class GalleryTableSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QCatUiKit::TableModel *sourceTableModel READ sourceTableModel
                       WRITE setSourceTableModel NOTIFY sourceTableModelChanged)
    Q_PROPERTY(int sortColumn READ sortColumn NOTIFY sortChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder NOTIFY sortChanged)
    Q_PROPERTY(bool sorted READ sorted NOTIFY sortChanged)
  public:
    explicit GalleryTableSortModel(QObject *parent = nullptr);

    QCatUiKit::TableModel *sourceTableModel() const;
    void setSourceTableModel(QCatUiKit::TableModel *model);

    int sortColumn() const;
    Qt::SortOrder sortOrder() const;
    bool sorted() const;

    Q_INVOKABLE void toggleSort(int sourceColumn);
    Q_INVOKABLE void clearSort();
    Q_INVOKABLE int proxyRowForSourceRow(int sourceRow) const;
    Q_INVOKABLE int sourceRowForProxyRow(int proxyRow) const;
    Q_INVOKABLE QVariant displayAt(int proxyRow, int column) const;
    Q_INVOKABLE void selectSourceRow(QItemSelectionModel *selection,
                                     QAbstractItemModel *viewModel, int sourceRow,
                                     bool rows) const;

  signals:
    void sourceTableModelChanged();
    void sortChanged();

  private:
    void emitSortChanged();
};
