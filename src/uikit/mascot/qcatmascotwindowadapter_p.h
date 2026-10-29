// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatmascotpetcontroller.h"
#include <QElapsedTimer>
#include <QPointer>
#include <QQuickWindow>
#include <QTimer>

namespace QCatUiKit
{
// Private window integration. No desktop service or extension connection.
class MascotWindowAdapter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Internal window integration")
    Q_PROPERTY(QQuickWindow *window READ window WRITE setWindow NOTIFY changed)
    Q_PROPERTY(QCatUiKit::MascotPetController *controller READ controller WRITE setController NOTIFY
                   changed)
    Q_PROPERTY(QVariantMap capabilities READ capabilities NOTIFY changed)
    Q_PROPERTY(QVariantList screens READ screens NOTIFY changed)
    Q_PROPERTY(bool inputRegionsEnabled MEMBER m_regionsEnabled NOTIFY changed)
    Q_PROPERTY(bool inputTransparent READ inputTransparent WRITE setInputTransparent NOTIFY changed)
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY changed)
    Q_PROPERTY(bool constrainToAvailableGeometry MEMBER m_constrain NOTIFY changed)
    Q_PROPERTY(QRect bubbleRect MEMBER m_bubbleRect NOTIFY changed)
    Q_PROPERTY(QPoint characterOffset READ characterOffset WRITE setCharacterOffset NOTIFY
                   characterOffsetChanged)
    Q_PROPERTY(QRect availableGeometry READ availableGeometry NOTIFY screenGeometryChanged)
    Q_PROPERTY(QPoint characterPosition READ characterPosition NOTIFY characterPositionChanged)
  public:
    explicit MascotWindowAdapter(QObject *parent = nullptr);
    ~MascotWindowAdapter() override;
    QQuickWindow *window() const
    {
        return m_window;
    }
    MascotPetController *controller() const
    {
        return m_controller;
    }
    void setWindow(QQuickWindow *window);
    void setController(MascotPetController *controller);
    bool inputTransparent() const
    {
        return m_transparent;
    }
    bool alwaysOnTop() const
    {
        return m_above;
    }
    void setInputTransparent(bool value);
    void setAlwaysOnTop(bool value);
    QPoint characterOffset() const
    {
        return m_characterOffset;
    }
    void setCharacterOffset(const QPoint &offset);
    QRect availableGeometry() const;
    QPoint characterPosition() const
    {
        return m_layoutTimer.isActive() ? m_layoutAnchor
               : m_window               ? m_window->position() + m_characterOffset
                                        : QPoint();
    }
    QVariantMap capabilities() const;
    QVariantList screens() const;
    Q_INVOKABLE bool setPosition(const QString &screenId, const QPointF &point);
    Q_INVOKABLE QString moveTo(const QString &screenId, const QPointF &point, int durationMs);
    Q_INVOKABLE void cancelMove();
    Q_INVOKABLE bool requestKeyboardFocus();
    Q_INVOKABLE QVariantMap exportState() const;
    Q_INVOKABLE QVariantMap restoreState(const QVariantMap &state);
  signals:
    void changed();
    void characterOffsetChanged();
    void characterPositionChanged();
    void screenGeometryChanged();
    void moveEnded(QString id, QString reason);
    void platformError(QString code, QString detail);

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  private:
    void refresh();
    void applyWindowPolicy();
    void applyMask();
    void syncGeometry();
    QPointF m_previousFoot;
    bool m_haveFoot = false;
    bool m_geometryPending = false;
    void finishMove(const QString &reason);
    bool canPosition() const;
    QScreen *findScreen(const QString &id) const;
    QPoint bounded(QScreen *screen, const QPointF &point) const;
    QPointer<QQuickWindow> m_window;
    QPointer<MascotPetController> m_controller;
    bool m_regionsEnabled = false;
    bool m_transparent = false, m_above = true, m_constrain = true;
    bool m_pressed = false, m_dragging = false;
    QRect m_bubbleRect;
    QPoint m_characterOffset;
    QPointF m_pressGlobal, m_pressLocal;
    QPoint m_pressWindow, m_from, m_to;
    QElapsedTimer m_clock;
    QTimer m_timer;
    QTimer m_layoutTimer;
    QPoint m_layoutAnchor;
    int m_duration = 0;
    quint64 m_serial = 0;
    QString m_move;
};
// Creatable internal QML name; not installed as a public C++ API.
class MascotDesktop : public MascotWindowAdapter
{
    Q_OBJECT
    QML_ELEMENT
  public:
    using MascotWindowAdapter::MascotWindowAdapter;
};
} // namespace QCatUiKit
