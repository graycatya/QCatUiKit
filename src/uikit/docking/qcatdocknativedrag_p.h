// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QHash>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtQml/qqmlregistration.h>

namespace QCatUiKit
{
// Internal Wayland adapter. No Qt Wayland Compositor dependency.
class DockNativeDrag : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool enabled READ enabled CONSTANT FINAL)
    Q_PROPERTY(bool toplevelDragSupported READ toplevelDragSupported CONSTANT FINAL)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged FINAL)
    Q_PROPERTY(bool acceptsDrop MEMBER m_acceptsDrop)
  public:
    explicit DockNativeDrag(QObject *parent = nullptr);
    ~DockNativeDrag() override;
    bool enabled() const;
    bool toplevelDragSupported() const;
    bool active() const;
    Q_INVOKABLE void watchWindow(QQuickWindow *window, const QString &id);
    Q_INVOKABLE bool prepare(QQuickItem *panel, const QString &title, const QColor &surface,
                             const QColor &text, const QColor &border, int fontSize);
    Q_INVOKABLE void start(QQuickWindow *origin, QQuickWindow *floating, const QPoint &offset);
    Q_INVOKABLE bool moveWindow(QQuickWindow *window);
    Q_INVOKABLE void cancel();
  signals:
    void activeChanged();
    void moved(const QString &windowId, const QPointF &position);
    void left();
    void dropped();
    void finished(bool moved);

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  private:
    friend class DockNativeDragTest;
    const bool m_enabled;
    const bool m_toplevelDragSupported;
    bool m_active = false;
    bool m_acceptsDrop = false;
    QByteArray m_token;
    QPixmap m_pixmap;
    QHash<QObject *, QString> m_windows;
};
} // namespace QCatUiKit
