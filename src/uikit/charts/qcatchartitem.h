// SPDX-License-Identifier: MIT
#pragma once
#include "qcatchartaxis.h"
#include "qcatchartscene3d.h"
#include "qcatchartseries.h"
#include <QCatUiKit/qcatthemepalette.h>
#include <QFont>
#include <QQmlListProperty>
#include <QQuickItem>
#include <QUrl>
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartItem : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChart)
    Q_CLASSINFO("DefaultProperty", "series")
    Q_PROPERTY(QQmlListProperty<QCatUiKit::ChartSeries> series READ series NOTIFY seriesChanged)
    Q_PROPERTY(QCatUiKit::ChartSeries::Type chartType READ chartType WRITE setChartType NOTIFY
                   appearanceChanged)
    Q_PROPERTY(QCatUiKit::ChartAxis *xAxis READ xAxis CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartAxis *yAxis READ yAxis CONSTANT)
    Q_PROPERTY(QCatUiKit::ThemePalette *theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap tokens READ tokens NOTIFY appearanceChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont RESET resetFont NOTIFY appearanceChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY appearanceChanged)
    Q_PROPERTY(
        bool legendVisible READ legendVisible WRITE setLegendVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible NOTIFY appearanceChanged)
    Q_PROPERTY(
        bool tooltipsEnabled READ tooltipsEnabled WRITE setTooltipsEnabled NOTIFY appearanceChanged)
    Q_PROPERTY(bool crosshairEnabled READ crosshairEnabled WRITE setCrosshairEnabled NOTIFY
                   appearanceChanged)
    Q_PROPERTY(bool interactive READ interactive WRITE setInteractive NOTIFY appearanceChanged)
    Q_PROPERTY(bool touchNavigationEnabled READ touchNavigationEnabled WRITE
                   setTouchNavigationEnabled NOTIFY appearanceChanged)
    Q_PROPERTY(DragMode dragMode READ dragMode WRITE setDragMode NOTIFY appearanceChanged)
    Q_PROPERTY(ZoomAxes rectangleZoomAxes READ rectangleZoomAxes WRITE setRectangleZoomAxes NOTIFY
                   appearanceChanged)
    Q_PROPERTY(QRectF zoomSelection READ zoomSelection NOTIFY zoomSelectionChanged)
    Q_PROPERTY(bool zoomSelecting READ zoomSelecting NOTIFY zoomSelectionChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor RESET
                   resetBackgroundColor NOTIFY appearanceChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor RESET resetTextColor NOTIFY
                   appearanceChanged)
    Q_PROPERTY(int displayBudget READ displayBudget WRITE setDisplayBudget NOTIFY appearanceChanged)
    Q_PROPERTY(qreal cellWidth READ cellWidth WRITE setCellWidth NOTIFY appearanceChanged)
    Q_PROPERTY(qreal cellHeight READ cellHeight WRITE setCellHeight NOTIFY appearanceChanged)
    Q_PROPERTY(
        qreal barGroupWidth READ barGroupWidth WRITE setBarGroupWidth NOTIFY appearanceChanged)
    Q_PROPERTY(qreal barSpacing READ barSpacing WRITE setBarSpacing NOTIFY appearanceChanged)
    Q_PROPERTY(
        qreal pieStartAngle READ pieStartAngle WRITE setPieStartAngle NOTIFY appearanceChanged)
    Q_PROPERTY(qreal pieEndAngle READ pieEndAngle WRITE setPieEndAngle NOTIFY appearanceChanged)
    Q_PROPERTY(qreal pieRadius READ pieRadius WRITE setPieRadius NOTIFY appearanceChanged)
    Q_PROPERTY(qreal holeSize READ holeSize WRITE setHoleSize NOTIFY appearanceChanged)
    Q_PROPERTY(bool multiRing READ multiRing WRITE setMultiRing NOTIFY appearanceChanged)
    Q_PROPERTY(qreal ringGap READ ringGap WRITE setRingGap NOTIFY appearanceChanged)
    Q_PROPERTY(bool sliceLabelsVisible READ sliceLabelsVisible WRITE setSliceLabelsVisible NOTIFY
                   appearanceChanged)
    Q_PROPERTY(SliceLabelPosition sliceLabelPosition READ sliceLabelPosition WRITE
                   setSliceLabelPosition NOTIFY appearanceChanged)
    Q_PROPERTY(
        int delegateBudget READ delegateBudget WRITE setDelegateBudget NOTIFY appearanceChanged)
    Q_PROPERTY(int activeDelegateCount READ activeDelegateCount NOTIFY delegatesChanged)
    Q_PROPERTY(QString delegateError READ delegateError NOTIFY delegatesChanged)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY
                   appearanceChanged)
    Q_PROPERTY(QVariantList selectedPoints READ selectedPoints NOTIFY selectionChanged)
    Q_PROPERTY(QColor selectionColor READ selectionColor WRITE setSelectionColor RESET
                   resetSelectionColor NOTIFY appearanceChanged)
    Q_PROPERTY(
        qreal selectionWidth READ selectionWidth WRITE setSelectionWidth NOTIFY appearanceChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled NOTIFY
                   appearanceChanged)
    Q_PROPERTY(bool transitionRunning READ transitionRunning NOTIFY transitionChanged)
    Q_PROPERTY(QCatUiKit::ChartScene3D *scene3D READ scene3D CONSTANT)
    Q_PROPERTY(QVariantList radarRanges READ radarRanges WRITE setRadarRanges NOTIFY appearanceChanged)
    Q_PROPERTY(bool exportingImage READ exportingImage NOTIFY exportingImageChanged)
  public:
    enum SelectionMode
    {
        NoSelection,
        SingleSelection,
        MultiSelection
    };
    Q_ENUM(SelectionMode)
    enum SliceLabelPosition
    {
        SliceLabelInside,
        SliceLabelOutside
    };
    Q_ENUM(SliceLabelPosition)
    enum DragMode
    {
        PanDrag,
        RectangleZoomDrag
    };
    Q_ENUM(DragMode)
    enum ZoomAxes
    {
        ZoomBothAxes,
        ZoomXAxis,
        ZoomYAxis
    };
    Q_ENUM(ZoomAxes)
    explicit ChartItem(QQuickItem *parent = nullptr);
    QVariantList radarRanges() const;
    void setRadarRanges(const QVariantList &value);
    ~ChartItem() override;
    ChartSeries::Type chartType() const;
    void setChartType(ChartSeries::Type value);
    ChartAxis *xAxis() const;
    ChartAxis *yAxis() const;
    ThemePalette *theme() const;
    void setTheme(ThemePalette *value);
    QVariantMap tokens() const;
    QFont font() const;
    void setFont(const QFont &value);
    void resetFont();
    void resetBackgroundColor();
    void resetTextColor();
    QString title() const;
    void setTitle(const QString &value);
    bool legendVisible() const;
    void setLegendVisible(bool value);
    bool gridVisible() const;
    void setGridVisible(bool value);
    bool tooltipsEnabled() const;
    void setTooltipsEnabled(bool value);
    bool crosshairEnabled() const;
    void setCrosshairEnabled(bool value);
    bool interactive() const;
    void setInteractive(bool value);
    bool touchNavigationEnabled() const;
    void setTouchNavigationEnabled(bool value);
    DragMode dragMode() const;
    void setDragMode(DragMode value);
    ZoomAxes rectangleZoomAxes() const;
    void setRectangleZoomAxes(ZoomAxes value);
    QRectF zoomSelection() const;
    bool zoomSelecting() const;
    Q_INVOKABLE bool zoomToRect(QRectF rectangle);
    Q_INVOKABLE void cancelZoomSelection();
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &value);
    QColor textColor() const;
    void setTextColor(const QColor &value);
    bool animationsEnabled() const;
    void setAnimationsEnabled(bool value);
    bool transitionRunning() const;
    ChartScene3D *scene3D() const;
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode value);
    QVariantList selectedPoints() const;
    Q_INVOKABLE bool isPointSelected(int seriesIndex, int index) const;
    Q_INVOKABLE bool selectPoint(int seriesIndex, int index, bool selected = true);
    Q_INVOKABLE void clearSelection();
    QColor selectionColor() const;
    void setSelectionColor(const QColor &value);
    void resetSelectionColor();
    qreal selectionWidth() const;
    void setSelectionWidth(qreal value);
    int delegateBudget() const;
    void setDelegateBudget(int value);
    int activeDelegateCount() const;
    QString delegateError() const;
    int displayBudget() const;
    void setDisplayBudget(int value);
    qreal cellWidth() const;
    void setCellWidth(qreal value);
    qreal cellHeight() const;
    void setCellHeight(qreal value);
    qreal barGroupWidth() const;
    void setBarGroupWidth(qreal value);
    qreal barSpacing() const;
    void setBarSpacing(qreal value);
    qreal pieStartAngle() const;
    void setPieStartAngle(qreal value);
    qreal pieEndAngle() const;
    void setPieEndAngle(qreal value);
    Q_INVOKABLE bool setPieAngles(qreal start, qreal end);
    qreal pieRadius() const;
    void setPieRadius(qreal value);
    qreal holeSize() const;
    void setHoleSize(qreal value);
    bool multiRing() const;
    void setMultiRing(bool value);
    qreal ringGap() const;
    void setRingGap(qreal value);
    bool sliceLabelsVisible() const;
    void setSliceLabelsVisible(bool value);
    SliceLabelPosition sliceLabelPosition() const;
    void setSliceLabelPosition(SliceLabelPosition value);
    Q_INVOKABLE bool isSliceVisible(int seriesIndex, int sliceIndex) const;
    Q_INVOKABLE bool setSliceVisible(int seriesIndex, int sliceIndex, bool visible);
    QQmlListProperty<ChartSeries> series();
    Q_INVOKABLE void addSeries(QCatUiKit::ChartSeries *series);
    Q_INVOKABLE void removeSeries(QCatUiKit::ChartSeries *series);
    Q_INVOKABLE void clearSeries();
    int seriesCount() const;
    ChartSeries *seriesAt(int index) const;
    Q_INVOKABLE void resetView();
    Q_INVOKABLE bool setViewRange(double minX, double maxX, double minY, double maxY);
    Q_INVOKABLE void zoomAt(qreal factor, QPointF position);
    Q_INVOKABLE void panBy(QPointF pixels);
    Q_INVOKABLE QPointF mapToPosition(QPointF value) const;
    Q_INVOKABLE QPointF mapToValue(QPointF position) const;
    Q_INVOKABLE QPointF mapToPosition(QPointF value, QCatUiKit::ChartSeries *series) const;
    Q_INVOKABLE QPointF mapToValue(QPointF position, QCatUiKit::ChartSeries *series) const;
    Q_INVOKABLE QCatUiKit::ChartAxis *axisForSeries(QCatUiKit::ChartSeries *series,
                                                    Qt::Orientation orientation) const;
    Q_INVOKABLE QVariantMap dataAt(QPointF position) const;
    Q_INVOKABLE QRectF plotRect() const;
    // CPU geometry/layout preparation only; not a GPU frame duration.
    qint64 preparationNanoseconds() const;
    int renderedPointCount() const;
    Q_INVOKABLE QVariantMap renderingStatistics() const;
    // Captures the current view as PNG. An empty default size uses display pixels.
    Q_INVOKABLE bool exportImage(const QUrl &destination, QSize pixelSize = QSize());
    bool exportingImage() const;
  signals:
    void themeChanged();
    void appearanceChanged();
    void seriesChanged();
    void viewChanged();
    void pointHovered(QVariantMap point);
    void pointClicked(QVariantMap point);
    void zoomSelectionChanged();
    void delegatesChanged();
    void selectionChanged();
    void transitionChanged();
    void exportingImageChanged();
    void imageExportFinished(QUrl destination, bool success, QString error);

  protected:
    void componentComplete() override;
    void keyPressEvent(QKeyEvent *) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void updatePolish() override;
    void geometryChange(const QRectF &, const QRectF &) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
    void hoverMoveEvent(QHoverEvent *) override;
    void hoverLeaveEvent(QHoverEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseUngrabEvent() override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void touchEvent(QTouchEvent *) override;
    void touchUngrabEvent() override;

  private:
    void invalidate();
    void startTransition(bool selectionOnly = false);
    void finishTransitions();
    void updateTransitionOpacity();
    void handlePointClick(const QVariantMap &point);
    void refreshDelegateStates(const QVariantMap &hover = {});
    void cancelDrag();
    void cancelTouch();
    void finishImageExport(const QString &error);
    Q_DISABLE_COPY_MOVE(ChartItem)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
