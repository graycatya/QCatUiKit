// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QStandardItemModel>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace QCatUiKit {
class TreeModel : public QStandardItemModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TreeModel)
    Q_PROPERTY(QVariantList items READ items WRITE setItems NOTIFY itemsChanged)
public:
    explicit TreeModel(QObject *parent = nullptr);
    QVariantList items() const;
    void setItems(const QVariantList &items);
signals:
    void itemsChanged();
private:
    QVariantList m_items;
};
}
