// SPDX-License-Identifier: MIT
#pragma once
#include "qcatchartaxis.h"
#include "qcatchartseries.h"
#include "qcatthemepalette.h"
#include <QFont>
#include <QQmlListProperty>
#include <QQuickItem>
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
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor RESET
                   resetBackgroundColor NOTIFY appearanceChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor RESET resetTextColor NOTIFY
                   appearanceChanged)
    Q_PROPERTY(int displayBudget READ displayBudget WRITE setDisplayBudget NOTIFY appearanceChanged)
    Q_PROPERTY(qreal cellWidth READ cellWidth WRITE setCellWidth NOTIFY appearanceChanged)
    Q_PROPERTY(qreal cellHeight READ cellHeight WRITE setCellHeight NOTIFY appearanceChanged)
  public:
    explicit ChartItem(QQuickItem *parent = nullptr);
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
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &value);
    QColor textColor() const;
    void setTextColor(const QColor &value);
    int displayBudget() const;
    void setDisplayBudget(int value);
    qreal cellWidth() const;
    void setCellWidth(qreal value);
    qreal cellHeight() const;
    void setCellHeight(qreal value);
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
    Q_INVOKABLE QVariantMap dataAt(QPointF position) const;
    Q_INVOKABLE QRectF plotRect() const;
    // CPU geometry/layout preparation only; not a GPU frame duration.
    qint64 preparationNanoseconds() const;
    int renderedPointCount() const;
  signals:
    void themeChanged();
    void appearanceChanged();
    void seriesChanged();
    void viewChanged();
    void pointHovered(QVariantMap point);
    void pointClicked(QVariantMap point);

  protected:
    void componentComplete() override;
    void keyPressEvent(QKeyEvent *) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void updatePolish() override;
    void geometryChange(const QRectF &, const QRectF &) override;
    void hoverMoveEvent(QHoverEvent *) override;
    void hoverLeaveEvent(QHoverEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;

  private:
    void invalidate();
    Q_DISABLE_COPY_MOVE(ChartItem)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
