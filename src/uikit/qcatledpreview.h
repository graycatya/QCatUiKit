// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledsource.h"
#include "qcatuikitglobal.h"
#include <QColor>
#include <QQuickRhiItem>
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
  public:
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
    Q_INVOKABLE void resetView();
    Q_INVOKABLE void orbit(qreal dx, qreal dy);
    Q_INVOKABLE void pan(qreal dx, qreal dy);
    Q_INVOKABLE void zoom(qreal wheelSteps);
  Q_SIGNALS:
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
    struct Impl;
    const std::unique_ptr<Impl> d;
    friend class QCatLedPreviewRenderer;
};
