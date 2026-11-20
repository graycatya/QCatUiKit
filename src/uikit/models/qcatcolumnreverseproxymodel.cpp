// SPDX-License-Identifier: Apache-2.0
#include "qcatcolumnreverseproxymodel.h"

#include <algorithm>

namespace QCatUiKit
{
ColumnReverseProxyModel::ColumnReverseProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

ColumnReverseProxyModel::~ColumnReverseProxyModel() = default;

void ColumnReverseProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel())
        return;

    beginResetModel();
    disconnectSource();
    QAbstractProxyModel::setSourceModel(model);
    connectSource();
    endResetModel();
}

void ColumnReverseProxyModel::connectSource()
{
    auto *source = sourceModel();
    if (!source)
        return;

    connect(source, &QAbstractItemModel::rowsAboutToBeInserted, this,
            [this](const QModelIndex &parent, int first, int last) {
                if (!parent.isValid())
                    beginInsertRows(QModelIndex(), first, last);
            });
    connect(source, &QAbstractItemModel::rowsInserted, this,
            [this](const QModelIndex &parent, int, int) {
                if (!parent.isValid())
                    endInsertRows();
            });
    connect(source, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            [this](const QModelIndex &parent, int first, int last) {
                if (!parent.isValid())
                    beginRemoveRows(QModelIndex(), first, last);
            });
    connect(source, &QAbstractItemModel::rowsRemoved, this,
            [this](const QModelIndex &parent, int, int) {
                if (!parent.isValid())
                    endRemoveRows();
            });
    connect(source, &QAbstractItemModel::rowsMoved, this,
            [this](const QModelIndex &, int, int, const QModelIndex &, int) {
                beginResetModel();
                endResetModel();
            });

    connect(source, &QAbstractItemModel::columnsAboutToBeInserted, this,
            [this, source](const QModelIndex &parent, int first, int last) {
                if (parent.isValid())
                    return;
                const int count = last - first + 1;
                const int sourceCount = source->columnCount();
                beginInsertColumns(QModelIndex(), sourceCount + count - 1 - last,
                                   sourceCount + count - 1 - first);
            });
    connect(source, &QAbstractItemModel::columnsInserted, this,
            [this](const QModelIndex &parent, int, int) {
                if (!parent.isValid())
                    endInsertColumns();
            });
    connect(source, &QAbstractItemModel::columnsAboutToBeRemoved, this,
            [this, source](const QModelIndex &parent, int first, int last) {
                if (parent.isValid())
                    return;
                const int sourceCount = source->columnCount();
                beginRemoveColumns(QModelIndex(), sourceCount - 1 - last,
                                   sourceCount - 1 - first);
            });
    connect(source, &QAbstractItemModel::columnsRemoved, this,
            [this](const QModelIndex &parent, int, int) {
                if (!parent.isValid())
                    endRemoveColumns();
            });
    connect(source, &QAbstractItemModel::columnsMoved, this,
            [this](const QModelIndex &, int, int, const QModelIndex &, int) {
                beginResetModel();
                endResetModel();
            });

    connect(source, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                   const QList<int> &roles) {
                const QModelIndex proxyTopLeft = mapFromSource(topLeft);
                const QModelIndex proxyBottomRight = mapFromSource(bottomRight);
                if (!proxyTopLeft.isValid() || !proxyBottomRight.isValid())
                    return;
                const int left = std::min(proxyTopLeft.column(), proxyBottomRight.column());
                const int right = std::max(proxyTopLeft.column(), proxyBottomRight.column());
                const int top = std::min(proxyTopLeft.row(), proxyBottomRight.row());
                const int bottom = std::max(proxyTopLeft.row(), proxyBottomRight.row());
                emit dataChanged(index(top, left), index(bottom, right), roles);
            });
    connect(source, &QAbstractItemModel::headerDataChanged, this,
            [this](Qt::Orientation orientation, int first, int last) {
                if (orientation != Qt::Horizontal) {
                    emit headerDataChanged(orientation, first, last);
                    return;
                }
                const int count = columnCount();
                const int left = std::min(count - 1 - first, count - 1 - last);
                const int right = std::max(count - 1 - first, count - 1 - last);
                emit headerDataChanged(orientation, left, right);
            });
    connect(source, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
        beginResetModel();
    });
    connect(source, &QAbstractItemModel::modelReset, this, [this] {
        endResetModel();
    });
    connect(source, &QAbstractItemModel::layoutChanged, this, [this] {
        beginResetModel();
        endResetModel();
    });
    connect(source, &QObject::destroyed, this, [this] {
        beginResetModel();
        endResetModel();
    });
}

void ColumnReverseProxyModel::disconnectSource()
{
    if (auto *source = sourceModel())
        disconnect(source, nullptr, this, nullptr);
}

QModelIndex ColumnReverseProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid() || proxyIndex.model() != this
        || proxyIndex.column() < 0 || proxyIndex.column() >= columnCount()) {
        return {};
    }
    return sourceModel()->index(proxyIndex.row(), columnCount() - 1 - proxyIndex.column());
}

QModelIndex ColumnReverseProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid() || sourceIndex.model() != sourceModel()
        || sourceIndex.column() < 0 || sourceIndex.column() >= columnCount()) {
        return {};
    }
    return index(sourceIndex.row(), columnCount() - 1 - sourceIndex.column());
}

QModelIndex ColumnReverseProxyModel::index(int row, int column,
                                          const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= rowCount()
        || column < 0 || column >= columnCount()) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex ColumnReverseProxyModel::parent(const QModelIndex &) const
{
    return {};
}

int ColumnReverseProxyModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() || !sourceModel() ? 0 : sourceModel()->rowCount();
}

int ColumnReverseProxyModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() || !sourceModel() ? 0 : sourceModel()->columnCount();
}

QVariant ColumnReverseProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceModel()->data(sourceIndex, role) : QVariant();
}

bool ColumnReverseProxyModel::setData(const QModelIndex &proxyIndex,
                                      const QVariant &value, int role)
{
    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() && sourceModel()->setData(sourceIndex, value, role);
}

QVariant ColumnReverseProxyModel::headerData(int section, Qt::Orientation orientation,
                                             int role) const
{
    if (!sourceModel() || section < 0)
        return {};
    if (orientation == Qt::Vertical)
        return sourceModel()->headerData(section, orientation, role);
    if (section >= columnCount())
        return {};
    return sourceModel()->headerData(columnCount() - 1 - section, orientation, role);
}

Qt::ItemFlags ColumnReverseProxyModel::flags(const QModelIndex &proxyIndex) const
{
    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceModel()->flags(sourceIndex) : Qt::NoItemFlags;
}

QHash<int, QByteArray> ColumnReverseProxyModel::roleNames() const
{
    return sourceModel() ? sourceModel()->roleNames() : QAbstractProxyModel::roleNames();
}
} // namespace QCatUiKit
