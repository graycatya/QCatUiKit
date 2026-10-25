// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledsource.h"
#include <QCatUiKit/qcatuikitglobal.h>
#include <QColor>
#include <QQuickRhiItem>
#include <QVariantMap>
#include <QVector3D>
#include <memory>

class QCatLedPreviewRenderer;
class QCATUIKIT_EXPORT QCatLedPreview : public QQuickRhiItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedPreviewRenderItem)
    Q_PROPERTY(QCatLedSource *source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged FINAL)
    Q_PROPERTY(qreal pointSize READ pointSize WRITE setPointSize NOTIFY pointSizeChanged FINAL)
    Q_PROPERTY(qreal previewBrightness READ previewBrightness WRITE setPreviewBrightness NOTIFY
                   previewBrightnessChanged FINAL)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged FINAL)
    Q_PROPERTY(bool showStatistics READ showStatistics WRITE setShowStatistics NOTIFY
                   showStatisticsChanged FINAL)
    Q_PROPERTY(quint64 renderedFrames READ renderedFrames NOTIFY statisticsChanged FINAL)
    Q_PROPERTY(quint64 renderedFrameId READ renderedFrameId NOTIFY statisticsChanged FINAL)
    Q_PROPERTY(qreal renderFps READ renderFps NOTIFY statisticsChanged FINAL)
    Q_PROPERTY(qreal colorUpdateHz READ colorUpdateHz NOTIFY statisticsChanged FINAL)
    Q_PROPERTY(QString renderingInfo READ renderingInfo NOTIFY statisticsChanged FINAL)
    Q_PROPERTY(QString renderError READ renderError NOTIFY statisticsChanged FINAL)
    Q_PROPERTY(
        PointShape pointShape READ pointShape WRITE setPointShape NOTIFY settingsChanged FINAL)
    Q_PROPERTY(SizeMode sizeMode READ sizeMode WRITE setSizeMode NOTIFY settingsChanged FINAL)
    Q_PROPERTY(qreal sceneSize READ sceneSize WRITE setSceneSize NOTIFY settingsChanged FINAL)
    Q_PROPERTY(QVector3D displaySpacing READ displaySpacing WRITE setDisplaySpacing NOTIFY
                   settingsChanged FINAL)
    Q_PROPERTY(qreal gapX READ gapX WRITE setGapX NOTIFY settingsChanged FINAL)
    Q_PROPERTY(qreal gapY READ gapY WRITE setGapY NOTIFY settingsChanged FINAL)
    Q_PROPERTY(int sliceAxis READ sliceAxis WRITE setSliceAxis NOTIFY settingsChanged FINAL)
    Q_PROPERTY(bool slicesVertical READ slicesVertical WRITE setSlicesVertical NOTIFY
                   settingsChanged FINAL)
    Q_PROPERTY(bool previewEnabled READ previewEnabled WRITE setPreviewEnabled NOTIFY
                   settingsChanged FINAL)
    Q_PROPERTY(int maximumFps READ maximumFps WRITE setMaximumFps NOTIFY settingsChanged FINAL)
    Q_PROPERTY(QString backgroundImage READ backgroundImage WRITE setBackgroundImage NOTIFY
                   settingsChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY settingsChanged)
    Q_PROPERTY(bool showAxes READ showAxes WRITE setShowAxes NOTIFY settingsChanged)
  public:
    enum PointShape
    {
        Square,
        Circle,
        Cube,
        Sphere
    };
    Q_ENUM(PointShape)
    enum SizeMode
    {
        ScreenPixels,
        SceneUnits
    };
    Q_ENUM(SizeMode)
    enum ViewMode
    {
        Plane2D,
        Spatial3D
    };
    Q_ENUM(ViewMode)
    explicit QCatLedPreview(QQuickItem *parent = nullptr);
    ~QCatLedPreview() override;
    QCatLedSource *source() const;
    void setSource(QCatLedSource *source);
    ViewMode viewMode() const;
    void setViewMode(ViewMode mode);
    qreal pointSize() const;
    void setPointSize(qreal size);
    qreal previewBrightness() const;
    void setPreviewBrightness(qreal brightness);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    bool showStatistics() const;
    void setShowStatistics(bool show);
    quint64 renderedFrames() const;
    quint64 renderedFrameId() const;
    qreal renderFps() const;
    qreal colorUpdateHz() const;
    QString renderingInfo() const;
    QString renderError() const;
    PointShape pointShape() const;
    void setPointShape(PointShape value);
    SizeMode sizeMode() const;
    void setSizeMode(SizeMode value);
    qreal sceneSize() const;
    void setSceneSize(qreal value);
    QVector3D displaySpacing() const;
    void setDisplaySpacing(QVector3D value);
    qreal gapX() const;
    void setGapX(qreal value);
    qreal gapY() const;
    void setGapY(qreal value);
    int sliceAxis() const;
    void setSliceAxis(int value);
    bool slicesVertical() const;
    void setSlicesVertical(bool value);
    bool previewEnabled() const;
    void setPreviewEnabled(bool value);
    int maximumFps() const;
    void setMaximumFps(int value);
    QString backgroundImage() const;
    void setBackgroundImage(const QString &value);
    bool showGrid() const;
    void setShowGrid(bool value);
    bool showAxes() const;
    void setShowAxes(bool value);
    Q_INVOKABLE QVariantMap camera() const;
    Q_INVOKABLE bool setCamera(const QVariantMap &camera);
    Q_INVOKABLE void presetView(int preset);
    Q_INVOKABLE void resetView();
    Q_INVOKABLE void orbit(qreal dx, qreal dy);
    Q_INVOKABLE void pan(qreal dx, qreal dy);
    Q_INVOKABLE void zoom(qreal wheelSteps);
  Q_SIGNALS:
    void cameraChanged();
    void settingsChanged();
    void sourceChanged();
    void viewModeChanged();
    void pointSizeChanged();
    void previewBrightnessChanged();
    void backgroundColorChanged();
    void showStatisticsChanged();
    void statisticsChanged();

  protected:
    QQuickRhiItemRenderer *createRenderer() override;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

  private:
    void requestUpdate();
    struct Impl;
    const std::unique_ptr<Impl> d;
    friend class QCatLedPreviewRenderer;
};
