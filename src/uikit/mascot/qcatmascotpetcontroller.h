// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
class VectorItem;
class MascotRenderAdapter;
class QCATUIKIT_EXPORT MascotPetController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(
        QUrl packageSource READ packageSource WRITE setPackageSource NOTIFY packageSourceChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorCode READ errorCode NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(QStringList availableActions READ availableActions NOTIFY packageChanged)
    Q_PROPERTY(QString currentAction READ currentAction NOTIFY currentActionChanged)
    Q_PROPERTY(int actionCacheLimitMiB READ actionCacheLimitMiB WRITE setActionCacheLimitMiB NOTIFY
                   actionCacheLimitMiBChanged)
    Q_PROPERTY(int queueLength READ queueLength NOTIFY queueLengthChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(bool playbackEnabled READ playbackEnabled WRITE setPlaybackEnabled NOTIFY
                   playbackEnabledChanged)
    Q_PROPERTY(QString facing READ facing WRITE setFacing NOTIFY facingChanged)
    Q_PROPERTY(qreal petScale READ petScale WRITE setPetScale NOTIFY petScaleChanged)
    Q_PROPERTY(QSize logicalSize READ logicalSize NOTIFY packageChanged)
    Q_PROPERTY(QPointF footAnchor READ footAnchor NOTIFY anchorsChanged)
    Q_PROPERTY(QPointF bubbleAnchor READ bubbleAnchor NOTIFY anchorsChanged)
    Q_PROPERTY(QVariantMap spriteFrame READ spriteFrame NOTIFY spriteFrameChanged)
    Q_PROPERTY(bool dragEnabled READ dragEnabled WRITE setDragEnabled NOTIFY dragEnabledChanged)
    Q_PROPERTY(QString dragAction READ dragAction WRITE setDragAction NOTIFY dragActionChanged)
    Q_PROPERTY(bool dragging READ dragging NOTIFY draggingChanged)
    Q_PROPERTY(QVariantList interactionRegions READ interactionRegions NOTIFY regionsChanged)
    Q_PROPERTY(QUrl fallbackSource READ fallbackSource NOTIFY packageChanged)
  public:
    enum Status
    {
        Null,
        Loading,
        Ready,
        Error
    };
    Q_ENUM(Status)
    enum RequestPolicy
    {
        Replace,
        Enqueue,
        IgnoreIfBusy
    };
    Q_ENUM(RequestPolicy)
    explicit MascotPetController(QObject *parent = nullptr);
    ~MascotPetController() override;
    QUrl packageSource() const;
    void setPackageSource(const QUrl &source);
    Status status() const;
    QString errorCode() const;
    QString errorString() const;
    QStringList availableActions() const;
    QString currentAction() const;
    int actionCacheLimitMiB() const;
    void setActionCacheLimitMiB(int limit);
    int queueLength() const;
    bool paused() const;
    bool playbackEnabled() const;
    void setPlaybackEnabled(bool enabled);
    QString facing() const;
    void setFacing(const QString &facing);
    qreal petScale() const;
    void setPetScale(qreal scale);
    QSize logicalSize() const;
    QPointF footAnchor() const;
    QPointF bubbleAnchor() const;
    QVariantMap spriteFrame() const;
    QUrl fallbackSource() const;
    bool dragEnabled() const;
    void setDragEnabled(bool enabled);
    QString dragAction() const;
    void setDragAction(const QString &action);
    bool dragging() const;
    QVariantList interactionRegions() const;
    Q_INVOKABLE QVariantMap hitTest(const QPointF &position, bool forDrag = false) const;
    Q_INVOKABLE bool activateRegion(const QPointF &position);
    Q_INVOKABLE bool beginDrag();
    Q_INVOKABLE void endDrag();
    Q_INVOKABLE QString requestAction(const QString &name, RequestPolicy policy = Replace,
                                      const QVariantMap &options = {});
    Q_INVOKABLE void pauseActions();
    Q_INVOKABLE void resumeActions();
    Q_INVOKABLE void resetToDefault();
  signals:
    void packageSourceChanged();
    void statusChanged();
    void packageChanged();
    void currentActionChanged();
    void actionCacheLimitMiBChanged();
    void queueLengthChanged();
    void pausedChanged();
    void playbackEnabledChanged();
    void facingChanged();
    void petScaleChanged();
    void spriteFrameChanged();
    void dragEnabledChanged();
    void dragActionChanged();
    void draggingChanged();
    void regionsChanged();
    void anchorsChanged();
    void regionActivated(QString name, QPointF logicalPosition);
    void dragStarted();
    void dragFinished();
    void actionStarted(QString requestId, QString name);
    void actionEnded(QString requestId, QString reason);
    void requestRejected(QString requestId, QString reason);
    void configurationRejected(QString reason);

  private:
    Q_DISABLE_COPY_MOVE(MascotPetController)
    friend class MascotRenderAdapter;
    void attachVectorItem(VectorItem *item);
    void renderError(const QUrl &source, const QString &message);
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
