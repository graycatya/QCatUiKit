// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatmascotpetcontroller.h"
#include "qcatvectoritem.h"
#include <QPointer>

namespace QCatUiKit
{
// Implementation bridge for the private mascot visual; not an application API.
class MascotRenderAdapter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QCatUiKit::MascotPetController *controller READ controller WRITE setController NOTIFY
                   controllerChanged)
    Q_PROPERTY(QCatUiKit::VectorItem *vectorItem READ vectorItem WRITE setVectorItem NOTIFY
                   vectorItemChanged)
  public:
    explicit MascotRenderAdapter(QObject *parent = nullptr) : QObject(parent) {}
    ~MascotRenderAdapter() override;
    MascotPetController *controller() const
    {
        return m_controller;
    }
    void setController(MascotPetController *controller);
    VectorItem *vectorItem() const
    {
        return m_item;
    }
    void setVectorItem(VectorItem *item);
    Q_INVOKABLE void spriteError(const QUrl &source, const QString &message);
  signals:
    void controllerChanged();
    void vectorItemChanged();

  private:
    QPointer<MascotPetController> m_controller;
    QPointer<VectorItem> m_item;
};
} // namespace QCatUiKit
