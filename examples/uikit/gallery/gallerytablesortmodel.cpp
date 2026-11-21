// SPDX-License-Identifier: Apache-2.0
#include "gallerytablesortmodel.h"
#include <QAbstractProxyModel>

GalleryTableSortModel::GalleryTableSortModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

QCatUiKit::TableModel *GalleryTableSortModel::sourceTableModel() const
{
    return qobject_cast<QCatUiKit::TableModel *>(sourceModel());
}

void GalleryTableSortModel::setSourceTableModel(QCatUiKit::TableModel *model)
{
    if (sourceTableModel() == model)
        return;
    clearSort();
    QSortFilterProxyModel::setSourceModel(model);
    emit sourceTableModelChanged();
    emitSortChanged();
}

int GalleryTableSortModel::sortColumn() const
{
    return QSortFilterProxyModel::sortColumn();
}

Qt::SortOrder GalleryTableSortModel::sortOrder() const
{
    return QSortFilterProxyModel::sortOrder();
}

bool GalleryTableSortModel::sorted() const
{
    return sortColumn() >= 0;
}

void GalleryTableSortModel::toggleSort(int sourceColumn)
{
    if (!sourceTableModel() || sourceColumn < 0
        || sourceColumn >= sourceTableModel()->columnCount()) {
        return;
    }
    const bool sameAscending = sortColumn() == sourceColumn
            && sortOrder() == Qt::AscendingOrder;
    sort(sourceColumn, sameAscending ? Qt::DescendingOrder : Qt::AscendingOrder);
    emitSortChanged();
}

void GalleryTableSortModel::clearSort()
{
    if (sortColumn() < 0)
        return;
    sort(-1, Qt::AscendingOrder);
    emitSortChanged();
}

int GalleryTableSortModel::proxyRowForSourceRow(int sourceRow) const
{
    if (!sourceModel() || sourceRow < 0 || sourceRow >= sourceModel()->rowCount())
        return -1;
    return mapFromSource(sourceModel()->index(sourceRow, 0)).row();
}

int GalleryTableSortModel::sourceRowForProxyRow(int proxyRow) const
{
    if (!sourceModel() || proxyRow < 0 || proxyRow >= rowCount())
        return -1;
    return mapToSource(index(proxyRow, 0)).row();
}

QVariant GalleryTableSortModel::displayAt(int proxyRow, int column) const
{
    if (proxyRow < 0 || proxyRow >= rowCount() || column < 0 || column >= columnCount())
        return {};
    return data(index(proxyRow, column), Qt::DisplayRole);
}

void GalleryTableSortModel::selectSourceRow(QItemSelectionModel *selection,
                                            QAbstractItemModel *viewModel, int sourceRow,
                                            bool rows) const
{
    if (!selection || !viewModel || selection->model() != viewModel)
        return;
    const int proxyRow = proxyRowForSourceRow(sourceRow);
    if (proxyRow < 0)
        return;
    QModelIndex viewIndex = index(proxyRow, 0);
    if (viewModel != this) {
        auto *proxy = qobject_cast<QAbstractProxyModel *>(viewModel);
        if (!proxy)
            return;
        viewIndex = proxy->mapFromSource(viewIndex);
    }
    if (!viewIndex.isValid())
        return;
    const auto flags = rows ? QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
                           : QItemSelectionModel::ClearAndSelect;
    selection->setCurrentIndex(viewIndex, flags);
}

void GalleryTableSortModel::emitSortChanged()
{
    emit sortChanged();
}
