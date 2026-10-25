// SPDX-License-Identifier: Apache-2.0
#include "qcattablemodel.h"
#include <QSet>
namespace QCatUiKit
{
class TableModel::Impl
{
  public:
    QStringList columns;
    QStringList headers;
    QList<QVariantMap> rows;
};
TableModel::TableModel(QObject *parent) : QAbstractTableModel(parent), d(std::make_unique<Impl>())
{
}
TableModel::~TableModel() = default;
int TableModel::count() const
{
    return int(d->rows.size());
}
int TableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : count();
}
int TableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(d->columns.size());
}
QVariant TableModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.model() != this || idx.row() < 0 || idx.row() >= count() ||
        idx.column() < 0 || idx.column() >= columnCount())
        return {};
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};
    return d->rows.at(idx.row()).value(d->columns.at(idx.column()));
}
bool TableModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role != Qt::EditRole || !idx.isValid() || idx.model() != this || idx.row() < 0 ||
        idx.row() >= count() || idx.column() < 0 || idx.column() >= columnCount())
        return false;
    const auto key = d->columns.at(idx.column());
    if (d->rows[idx.row()].value(key) == value)
        return true;
    d->rows[idx.row()][key] = value;
    emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
    return true;
}
Qt::ItemFlags TableModel::flags(const QModelIndex &idx) const
{
    return idx.isValid() && idx.model() == this
               ? QAbstractTableModel::flags(idx) | Qt::ItemIsEditable
               : Qt::NoItemFlags;
}
QHash<int, QByteArray> TableModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}, {Qt::EditRole, "edit"}};
}
QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || section < 0)
        return {};
    if (orientation == Qt::Vertical)
        return section < count() ? QVariant(section + 1) : QVariant();
    if (section >= columnCount())
        return {};
    return section < d->headers.size() ? d->headers.at(section) : d->columns.at(section);
}
QStringList TableModel::columns() const
{
    return d->columns;
}
void TableModel::setColumns(const QStringList &value)
{
    if (value == d->columns)
        return;
    QSet<QString> unique;
    for (const auto &key : value)
    {
        if (key.isEmpty() || unique.contains(key))
            return;
        unique.insert(key);
    }
    beginResetModel();
    d->columns = value;
    endResetModel();
    emit columnsChanged();
}
QStringList TableModel::headers() const
{
    return d->headers;
}
void TableModel::setHeaders(const QStringList &value)
{
    if (value == d->headers)
        return;
    d->headers = value;
    emit headersChanged();
    if (columnCount())
        emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}
bool TableModel::append(const QVariantMap &row)
{
    if (d->columns.isEmpty())
        return false;
    const int pos = count();
    beginInsertRows({}, pos, pos);
    d->rows.append(row);
    endInsertRows();
    emit countChanged();
    return true;
}
bool TableModel::update(int row, const QVariantMap &changes)
{
    if (row < 0 || row >= count())
        return false;
    bool changed = false;
    for (auto it = changes.cbegin(); it != changes.cend(); ++it)
    {
        if (d->rows[row].value(it.key()) == it.value())
            continue;
        d->rows[row][it.key()] = it.value();
        changed = true;
    }
    if (changed && columnCount())
        emit dataChanged(index(row, 0), index(row, columnCount() - 1),
                         {Qt::DisplayRole, Qt::EditRole});
    return true;
}
QVariantMap TableModel::get(int row) const
{
    return row >= 0 && row < count() ? d->rows.at(row) : QVariantMap();
}
bool TableModel::setCell(int row, int column, const QVariant &value)
{
    return setData(index(row, column), value);
}
bool TableModel::remove(int row)
{
    if (row < 0 || row >= count())
        return false;
    beginRemoveRows({}, row, row);
    d->rows.removeAt(row);
    endRemoveRows();
    emit countChanged();
    return true;
}
void TableModel::clear()
{
    if (d->rows.isEmpty())
        return;
    beginResetModel();
    d->rows.clear();
    endResetModel();
    emit countChanged();
}
} // namespace QCatUiKit
