// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatuikitglobal.h"
#include <QObject>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
// All operations run on the object's thread. Panel IDs are application-owned stable keys.
class QCATUIKIT_EXPORT DockController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantMap layout READ layout NOTIFY layoutChanged)
    Q_PROPERTY(QStringList panelIds READ panelIds NOTIFY panelsChanged)
    Q_PROPERTY(QVariantList floatingWindows READ floatingWindows NOTIFY layoutChanged)
  public:
    enum Position
    {
        Center,
        Left,
        Right,
        Top,
        Bottom
    };
    Q_ENUM(Position)
    explicit DockController(QObject *parent = nullptr);
    ~DockController() override;
    QVariantMap layout() const;
    QStringList panelIds() const;
    Q_INVOKABLE bool registerPanel(const QString &id);
    Q_INVOKABLE bool unregisterPanel(const QString &id);
    Q_INVOKABLE bool dock(const QString &id, const QString &target, Position position = Center);
    Q_INVOKABLE bool closePanel(const QString &id);
    Q_INVOKABLE bool openPanel(const QString &id);
    Q_INVOKABLE bool activatePanel(const QString &id);
    Q_INVOKABLE bool moveTab(const QString &id, int index);
    Q_INVOKABLE bool setSplitRatio(const QString &nodeId, double ratio);
    Q_INVOKABLE QString saveLayout() const;
    // Unknown IDs are omitted; registered IDs absent from the snapshot remain closed.
    // Malformed snapshots are rejected atomically, without changing the layout.
    Q_INVOKABLE bool restoreLayout(const QString &json);
    QVariantList floatingWindows() const;
    Q_INVOKABLE bool floatPanel(const QString &id, bool wholeGroup = false);
    Q_INVOKABLE bool dockBack(const QString &windowId);
    Q_INVOKABLE QString floatingWindowForPanel(const QString &id) const;
    Q_INVOKABLE bool setFloatingGeometry(const QString &windowId, double x, double y, double width,
                                         double height);
    Q_INVOKABLE void constrainFloatingWindows();

  signals:
    void layoutChanged();
    void panelsChanged();
    void operationRejected(QString reason);

  private:
    Q_DISABLE_COPY_MOVE(DockController)
    class Impl;
    const std::unique_ptr<Impl> d;
    bool reject(const QString &reason);
};
} // namespace QCatUiKit
