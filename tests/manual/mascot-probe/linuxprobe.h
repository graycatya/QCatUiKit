// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QPoint>
#include <QProcess>
#include <QTimer>
#include <functional>
#include <memory>
#include <vector>
class QQmlEngine;
class QQuickWindow;
class QMenu;
class QSystemTrayIcon;
class MascotCompanion;
class LinuxProbe final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString report READ report NOTIFY reportChanged)
    Q_PROPERTY(bool passthrough READ passthrough NOTIFY changed)
    Q_PROPERTY(bool topmost READ topmost NOTIFY changed)
  public:
    LinuxProbe(QQmlEngine *engine, QJsonObject environment);
    ~LinuxProbe() override;
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
    void start(const QString &output);
    Q_INVOKABLE void setPassthrough(bool enabled);
    Q_INVOKABLE void setTopmost(bool enabled);
    Q_INVOKABLE void setBubble(bool enabled);
    Q_INVOKABLE void setPetVisible(bool enabled);
    Q_INVOKABLE void resetPosition();
    Q_INVOKABLE void bodyPressed();
    Q_INVOKABLE void bodyMoved();
    Q_INVOKABLE void bodyReleased();
    Q_INVOKABLE void bubbleActivated();
    Q_INVOKABLE void save();
    Q_INVOKABLE void setDiagnosticsVisible(bool visible);
    Q_INVOKABLE void snapshotCompanion();
    bool eventFilter(QObject *watched, QEvent *event) override;
  signals:
    void changed();
    void reportChanged();
    void finished(int code);

  private:
    struct Native;
    std::unique_ptr<Native> m_native;
    std::unique_ptr<MascotCompanion> m_companion;
    QHash<quint64, QString> m_companionOperations;
    QQmlEngine *m_engine;
    QQuickWindow *m_pet = nullptr;
    QProcess m_target;
    std::unique_ptr<QMenu> m_menu;
    std::unique_ptr<QSystemTrayIcon> m_tray;
    QJsonObject m_report, m_targetState;
    QByteArray m_buffer;
    QString m_token, m_output;
    QTimer m_runner;
    QTimer m_reportTimer;
    QString m_reportText;
    bool m_diagnosticsVisible = false;
    int m_reportRefreshes = 0;
    QElapsedTimer m_dragClock;
    qint64 m_lastDragEventMs = 0, m_maxDragEventGapMs = 0;
    std::vector<std::function<void()>> m_steps;
    int m_step = 0, m_ticks = 0, m_bodyClicks = 0, m_bubbleClicks = 0, m_drags = 0;
    int m_exitRequests = 0, m_menuActions = 0;
    bool m_automatic = false, m_failed = false, m_ready = false;
    bool m_passthrough = false, m_topmost = true, m_bubble = true, m_dragging = false;
    QPoint m_dragCursor, m_dragPosition, m_origin, m_pointerPosition;
    void update();
    void refreshReport();
    void applyMask();
    void prepareChecks();
    void finishChecks();
    void runDenseDrag();
    void companionCall(const QString &operation, const QJsonObject &arguments = {});
    void updateCompanionStatus();
    void record(const QString &name, bool passed, const QJsonObject &detail = {});
    void pending(const QString &name, const QString &reason);
    bool mouse(const QPoint &local, int action = 0);
    bool key(unsigned long symbol);
    bool targetFocused() const;
    bool petFocused() const;
    QPoint nativePosition() const;
    int targetClicks() const;
    int targetKeys() const;
};
