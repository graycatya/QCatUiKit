// SPDX-License-Identifier: Apache-2.0
#include "gallerytablecolumnmodel.h"

GalleryTableColumnModel::GalleryTableColumnModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

QAbstractItemModel *GalleryTableColumnModel::sourceTableModel() const
{
    return sourceModel();
}

void GalleryTableColumnModel::setSourceTableModel(QAbstractItemModel *model)
{
    if (sourceModel() == model)
        return;
    if (sourceModel())
        disconnect(sourceModel(), nullptr, this, nullptr);
    QIdentityProxyModel::setSourceModel(model);
    connectSource();
    refreshColumns();
    emit sourceTableModelChanged();
}

void GalleryTableColumnModel::connectSource()
{
    if (!sourceModel())
        return;
    connect(sourceModel(), &QAbstractItemModel::modelReset, this,
            &GalleryTableColumnModel::refreshColumns, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::columnsInserted, this,
            &GalleryTableColumnModel::refreshColumns, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::columnsRemoved, this,
            &GalleryTableColumnModel::refreshColumns, Qt::UniqueConnection);
}

void GalleryTableColumnModel::refreshColumns()
{
    m_order.clear();
    m_hidden.clear();
    if (sourceModel()) {
        for (int column = 0; column < sourceModel()->columnCount(); ++column)
            m_order.append(column);
    }
    emit columnsChanged();
}

QList<int> GalleryTableColumnModel::visibleSourceColumns() const
{
    QList<int> visible;
    visible.reserve(m_order.size());
    for (int column : m_order) {
        if (!m_hidden.contains(column))
            visible.append(column);
    }
    return visible;
}

QVariantList GalleryTableColumnModel::allColumns() const
{
    QVariantList result;
    for (int column : m_order)
        result.append(column);
    return result;
}

QVariantList GalleryTableColumnModel::visibleColumns() const
{
    QVariantList result;
    for (int column : visibleSourceColumns())
        result.append(column);
    return result;
}

int GalleryTableColumnModel::visibleColumnCount() const
{
    return visibleSourceColumns().size();
}

int GalleryTableColumnModel::sourceColumn(int proxyColumn) const
{
    const auto visible = visibleSourceColumns();
    if (proxyColumn < 0 || proxyColumn >= visible.size())
        return -1;
    return visible.at(proxyColumn);
}

bool GalleryTableColumnModel::isColumnVisible(int sourceColumn) const
{
    return m_order.contains(sourceColumn) && !m_hidden.contains(sourceColumn);
}

void GalleryTableColumnModel::setColumnVisible(int sourceColumn, bool visible)
{
    if (!sourceModel() || !m_order.contains(sourceColumn))
        return;
    if (!visible && visibleSourceColumns().size() <= 1)
        return;
    const bool wasVisible = !m_hidden.contains(sourceColumn);
    if (wasVisible == visible)
        return;
    beginResetModel();
    if (visible)
        m_hidden.removeAll(sourceColumn);
    else
        m_hidden.append(sourceColumn);
    endResetModel();
    emit columnsChanged();
}

void GalleryTableColumnModel::moveColumn(int sourceColumn, int delta)
{
    const int from = m_order.indexOf(sourceColumn);
    if (from < 0 || delta == 0)
        return;
    const int to = qBound(0, from + delta, m_order.size() - 1);
    if (to == from)
        return;
    beginResetModel();
    m_order.move(from, to);
    endResetModel();
    emit columnsChanged();
}

void GalleryTableColumnModel::resetColumns()
{
    if (!sourceModel())
        return;
    beginResetModel();
    m_order.clear();
    m_hidden.clear();
    for (int column = 0; column < sourceModel()->columnCount(); ++column)
        m_order.append(column);
    endResetModel();
    emit columnsChanged();
}

QModelIndex GalleryTableColumnModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return {};
    const auto visible = visibleSourceColumns();
    if (proxyIndex.column() < 0 || proxyIndex.column() >= visible.size())
        return {};
    return sourceModel()->index(proxyIndex.row(), visible.at(proxyIndex.column()));
}

QModelIndex GalleryTableColumnModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid()
            || sourceIndex.model() != sourceModel())
        return {};
    const int proxyColumn = visibleSourceColumns().indexOf(sourceIndex.column());
    if (proxyColumn < 0)
        return {};
    return createIndex(sourceIndex.row(), proxyColumn);
}

QModelIndex GalleryTableColumnModel::index(int row, int column,
                                           const QModelIndex &parent) const
{
    if (!sourceModel() || parent.isValid() || row < 0 || column < 0
            || row >= rowCount() || column >= columnCount())
        return {};
    return createIndex(row, column);
}

QModelIndex GalleryTableColumnModel::parent(const QModelIndex &) const
{
    return {};
}

QModelIndex GalleryTableColumnModel::sibling(int row, int column,
                                             const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.model() != this)
        return {};
    return index(row, column);
}

int GalleryTableColumnModel::columnCount(const QModelIndex &parent) const
{
    if (!sourceModel() || parent.isValid())
        return 0;
    return visibleSourceColumns().size();
}

QVariant GalleryTableColumnModel::headerData(int section, Qt::Orientation orientation,
                                             int role) const
{
    if (orientation == Qt::Vertical)
        return sourceModel() ? sourceModel()->headerData(section, orientation, role)
                             : QVariant();
    const auto visible = visibleSourceColumns();
    if (section < 0 || section >= visible.size() || !sourceModel())
        return {};
    return sourceModel()->headerData(visible.at(section), orientation, role);
}
