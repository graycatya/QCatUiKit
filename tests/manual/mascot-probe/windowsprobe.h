// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QAbstractNativeEventFilter>
#include <QJsonObject>
#include <QObject>
#include <QPoint>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QTimer>
#include <functional>
#include <memory>
#include <vector>
class QQmlEngine;
class QQuickWindow;
class QMenu;
class WindowsProbe final : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
    Q_PROPERTY(QString report READ report NOTIFY changed)
    Q_PROPERTY(bool passthrough READ passthrough NOTIFY changed)
    Q_PROPERTY(bool topmost READ topmost NOTIFY changed)
    Q_PROPERTY(int bodyClicks READ bodyClicks NOTIFY changed)
    Q_PROPERTY(int bubbleClicks READ bubbleClicks NOTIFY changed)
  public:
    WindowsProbe(QQmlEngine *engine, QJsonObject environment);
    ~WindowsProbe() override;
    static int runTarget(const QString &token);
    QString report() const;
    bool passthrough() const
    {
        return m_passthrough;
    }
    bool topmost() const
    {
        return m_topmost;
    }
    int bodyClicks() const
    {
        return m_bodyClicks;
    }
    int bubbleClicks() const
    {
        return m_bubbleClicks;
    }
    Q_INVOKABLE void start();
    Q_INVOKABLE void setPassthrough(bool enabled);
    Q_INVOKABLE void setTopmost(bool enabled);
    Q_INVOKABLE void setBubble(bool visible);
    Q_INVOKABLE void setPetVisible(bool visible);
    Q_INVOKABLE void resetPosition();
    Q_INVOKABLE void bodyPressed();
    Q_INVOKABLE void bodyMoved();
    Q_INVOKABLE void bodyReleased();
    Q_INVOKABLE void focusBubble();
    Q_INVOKABLE void bubbleActivated();
    void runChecks(const QString &path);
    bool nativeEventFilter(const QByteArray &, void *message, qintptr *result) override;
  signals:
    void changed();
    void finished(bool passed);

  private:
    QQmlEngine *m_engine;
    QQuickWindow *m_pet = nullptr;
    QProcess m_target;
    std::unique_ptr<QMenu> m_menu;
    QSystemTrayIcon m_tray;
    QJsonObject m_report;
    QString m_token, m_output;
    quintptr m_behind = 0, m_previousForeground = 0, m_native = 0;
    QPoint m_previousCursor, m_dragCursor, m_dragPosition, m_trayPoint;
    bool m_passthrough = false, m_topmost = true, m_dragging = false, m_bubble = true;
    int m_bodyClicks = 0, m_bubbleClicks = 0, m_exitRequests = 0, m_trayActions = 0;
    int m_step = 0, m_ticks = 0, m_targetClicks = 0, m_targetKeys = 0;
    quintptr m_savedFocus = 0;
    QPoint m_savedPosition;
    bool m_allPassed = true, m_finished = false, m_inputAvailable = true;
    bool m_leftDown = false, m_rightDown = false;
    QTimer m_runner;
    QTimer m_observer;
    std::vector<std::function<void()>> m_steps;
    void applyMask();
    void record(const QString &name, bool passed, const QJsonObject &detail = {});
    void finishChecks();
    QPoint physicalPoint(const QPoint &local) const;
    bool mouse(const QPoint &physical, unsigned long flags, bool shellIcon = false);
    bool key(unsigned short code);
    int targetCounter(const wchar_t *name) const;
    quintptr foreground() const;
};
