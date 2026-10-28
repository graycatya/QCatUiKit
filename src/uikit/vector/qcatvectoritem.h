// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QByteArray>
#include <QCatUiKit/qcatuikitglobal.h>
#include <QList>
#include <QQuickRhiItem>
#include <QRectF>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
class VectorRenderer;
class QCATUIKIT_EXPORT VectorItem : public QQuickRhiItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(
        QByteArray sourceData READ sourceData WRITE setSourceData NOTIFY sourceDataChanged FINAL)
    Q_PROPERTY(Format format READ format WRITE setFormat NOTIFY formatChanged FINAL)
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged FINAL)
    Q_PROPERTY(QRectF viewBox READ viewBox WRITE setViewBox NOTIFY viewBoxChanged FINAL)
    Q_PROPERTY(QVariantList shapes READ shapes WRITE setShapes NOTIFY shapesChanged FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged FINAL)
    Q_PROPERTY(bool backendAvailable READ backendAvailable CONSTANT FINAL)
    Q_PROPERTY(QSizeF sourceSize READ sourceSize NOTIFY metadataChanged FINAL)
    Q_PROPERTY(QRectF contentRect READ contentRect NOTIFY contentRectChanged FINAL)
    Q_PROPERTY(qreal duration READ duration NOTIFY metadataChanged FINAL)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY metadataChanged FINAL)
    Q_PROPERTY(qreal currentFrame READ currentFrame NOTIFY progressChanged FINAL)
    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged FINAL)
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY playbackChanged FINAL)
    Q_PROPERTY(bool animationEnabled READ animationEnabled WRITE setAnimationEnabled NOTIFY
                   playbackChanged FINAL)
    Q_PROPERTY(bool playing READ playing NOTIFY playbackChanged FINAL)
    Q_PROPERTY(bool paused READ paused NOTIFY playbackChanged FINAL)
    Q_PROPERTY(int loops READ loops WRITE setLoops NOTIFY playbackChanged FINAL)
    Q_PROPERTY(
        qreal playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackChanged FINAL)
    Q_PROPERTY(quint64 renderedFrames READ renderedFrames NOTIFY renderedFramesChanged FINAL)
  public:
    enum Format
    {
        Svg,
        Lottie,
        Canvas
    };
    Q_ENUM(Format)
    enum FillMode
    {
        Stretch,
        PreserveAspectFit,
        PreserveAspectCrop
    };
    Q_ENUM(FillMode)
    enum Status
    {
        Null,
        Loading,
        Ready,
        Error
    };
    Q_ENUM(Status)
    explicit VectorItem(QQuickItem *parent = nullptr);
    ~VectorItem() override;
    QUrl source() const;
    void setSource(const QUrl &value);
    QByteArray sourceData() const;
    void setSourceData(const QByteArray &value);
    Format format() const;
    void setFormat(Format value);
    FillMode fillMode() const;
    void setFillMode(FillMode value);
    QRectF viewBox() const;
    void setViewBox(const QRectF &value);
    QVariantList shapes() const;
    void setShapes(const QVariantList &value);
    Status status() const;
    QString errorString() const;
    bool backendAvailable() const;
    QSizeF sourceSize() const;
    QRectF contentRect() const;
    qreal duration() const;
    int frameCount() const;
    qreal currentFrame() const;
    qreal progress() const;
    void setProgress(qreal value);
    bool autoPlay() const;
    void setAutoPlay(bool value);
    bool animationEnabled() const;
    void setAnimationEnabled(bool value);
    bool playing() const;
    bool paused() const;
    int loops() const;
    void setLoops(int value);
    qreal playbackRate() const;
    void setPlaybackRate(qreal value);
    quint64 renderedFrames() const;
    Q_INVOKABLE void reload();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void restart();
    Q_INVOKABLE bool appendShape(const QVariantMap &shape);
    Q_INVOKABLE void clearShapes();
  signals:
    void sourceChanged();
    void sourceDataChanged();
    void formatChanged();
    void fillModeChanged();
    void viewBoxChanged();
    void shapesChanged();
    void statusChanged();
    void metadataChanged();
    void contentRectChanged();
    void progressChanged();
    void playbackChanged();
    void renderedFramesChanged();
    void finished();

  protected:
    QQuickRhiItemRenderer *createRenderer() override;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

  private:
    // Private mascot integration: validated sources only, scoped to a package generation.
    friend class MascotPetController;
    void configureMascotCache(const QList<QUrl> &sources, qint64 limitBytes, bool invalidate);
    void prepare();
    void tick();
    void setState(Status status, const QString &error = {});
    friend class VectorRenderer;
    struct Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
