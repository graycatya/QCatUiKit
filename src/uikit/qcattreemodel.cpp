// SPDX-License-Identifier: Apache-2.0
#include "qcattreemodel_p.h"

namespace QCatUiKit {
namespace {
void appendChildren(QStandardItem *parent, const QVariantList &items)
{
    for (const QVariant &value : items) {
        const QVariantMap entry = value.toMap();
        auto *item = new QStandardItem(entry.isEmpty() ? value.toString()
                                                      : entry.value("text").toString());
        item->setEditable(false);
        if (entry.contains("enabled"))
            item->setEnabled(entry.value("enabled").toBool());
        parent->appendRow(item);
        appendChildren(item, entry.value("children").toList());
    }
}
}
TreeModel::TreeModel(QObject *parent) : QStandardItemModel(parent) {}
QVariantList TreeModel::items() const { return m_items; }
void TreeModel::setItems(const QVariantList &items)
{
    if (m_items == items)
        return;
    m_items = items;
    clear();
    appendChildren(invisibleRootItem(), items);
    emit itemsChanged();
}
}
