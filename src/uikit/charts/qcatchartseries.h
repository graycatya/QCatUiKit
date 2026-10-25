// SPDX-License-Identifier: MIT
#pragma once
#include <QCatUiKit/qcatchartaxis.h>
#include <QCatUiKit/qcatuikitglobal.h>
#include <QColor>
#include <QImage>
#include <QObject>
#include <QPointF>
#include <QQmlComponent>
#include <QSizeF>
#include <QUrl>
#include <QVariantList>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include <memory>
namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartSeries : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartSeries)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY styleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY styleChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY styleChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY styleChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY styleChanged)
    Q_PROPERTY(CurveMode curveMode READ curveMode WRITE setCurveMode NOTIFY styleChanged)
    Q_PROPERTY(qreal splineTension READ splineTension WRITE setSplineTension NOTIFY styleChanged)
    Q_PROPERTY(MarkerShape markerShape READ markerShape WRITE setMarkerShape NOTIFY styleChanged)
    Q_PROPERTY(qreal markerSize READ markerSize WRITE setMarkerSize NOTIFY styleChanged)
    Q_PROPERTY(bool markersVisible READ markersVisible WRITE setMarkersVisible NOTIFY styleChanged)
    Q_PROPERTY(StrokeStyle strokeStyle READ strokeStyle WRITE setStrokeStyle NOTIFY styleChanged)
    Q_PROPERTY(QList<qreal> dashPattern READ dashPattern WRITE setDashPattern NOTIFY styleChanged)
    Q_PROPERTY(qreal dashOffset READ dashOffset WRITE setDashOffset NOTIFY styleChanged)
    Q_PROPERTY(AreaBaselineMode areaBaselineMode READ areaBaselineMode WRITE setAreaBaselineMode
                   NOTIFY styleChanged)
    Q_PROPERTY(double areaBaseline READ areaBaseline WRITE setAreaBaseline NOTIFY styleChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY styleChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY styleChanged)
    Q_PROPERTY(FillStyle fillStyle READ fillStyle WRITE setFillStyle NOTIFY styleChanged)
    Q_PROPERTY(QColor fillEndColor READ fillEndColor WRITE setFillEndColor NOTIFY styleChanged)
    Q_PROPERTY(qreal fillOpacity READ fillOpacity WRITE setFillOpacity NOTIFY styleChanged)
    Q_PROPERTY(qreal barBorderWidth READ barBorderWidth WRITE setBarBorderWidth NOTIFY styleChanged)
    Q_PROPERTY(
        QColor barBorderColor READ barBorderColor WRITE setBarBorderColor NOTIFY styleChanged)
    Q_PROPERTY(
        bool barLabelsVisible READ barLabelsVisible WRITE setBarLabelsVisible NOTIFY styleChanged)
    Q_PROPERTY(BarLabelPosition barLabelPosition READ barLabelPosition WRITE setBarLabelPosition
                   NOTIFY styleChanged)
    Q_PROPERTY(
        QString barLabelFormat READ barLabelFormat WRITE setBarLabelFormat NOTIFY styleChanged)
    Q_PROPERTY(
        int barLabelPrecision READ barLabelPrecision WRITE setBarLabelPrecision NOTIFY styleChanged)
    Q_PROPERTY(qreal barLabelMargin READ barLabelMargin WRITE setBarLabelMargin NOTIFY styleChanged)
    Q_PROPERTY(bool hasBreaks READ hasBreaks NOTIFY dataChanged)
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY capacityChanged)
    Q_PROPERTY(int count READ count NOTIFY dataChanged)
    Q_PROPERTY(QCatUiKit::ChartAxis *xAxis READ xAxis WRITE setXAxis NOTIFY styleChanged)
    Q_PROPERTY(QCatUiKit::ChartAxis *yAxis READ yAxis WRITE setYAxis NOTIFY styleChanged)
    Q_PROPERTY(LineCapStyle lineCap READ lineCap WRITE setLineCap NOTIFY styleChanged)
    Q_PROPERTY(LineJoinStyle lineJoin READ lineJoin WRITE setLineJoin NOTIFY styleChanged)
    Q_PROPERTY(qreal miterLimit READ miterLimit WRITE setMiterLimit NOTIFY styleChanged)
    Q_PROPERTY(QVariantList fillGradientStops READ fillGradientStops WRITE setFillGradientStops
                   NOTIFY styleChanged)
    Q_PROPERTY(QUrl fillTexture READ fillTexture WRITE setFillTexture NOTIFY styleChanged)
    Q_PROPERTY(
        QSizeF fillTextureSize READ fillTextureSize WRITE setFillTextureSize NOTIFY styleChanged)
    Q_PROPERTY(TextureMode fillTextureMode READ fillTextureMode WRITE setFillTextureMode NOTIFY
                   styleChanged)
    Q_PROPERTY(QString fillTextureError READ fillTextureError NOTIFY styleChanged)
    Q_PROPERTY(QUrl markerTexture READ markerTexture WRITE setMarkerTexture NOTIFY styleChanged)
    Q_PROPERTY(QString markerTextureError READ markerTextureError NOTIFY styleChanged)
    Q_PROPERTY(
        qreal barLabelRotation READ barLabelRotation WRITE setBarLabelRotation NOTIFY styleChanged)
    Q_PROPERTY(
        QQmlComponent *pointDelegate READ pointDelegate WRITE setPointDelegate NOTIFY styleChanged)
    Q_PROPERTY(QQmlComponent *barDelegate READ barDelegate WRITE setBarDelegate NOTIFY styleChanged)
    Q_PROPERTY(
        QQmlComponent *labelDelegate READ labelDelegate WRITE setLabelDelegate NOTIFY styleChanged)
  public:
    enum RangeAlignment
    {
        ExactAlignment,
        LinearAlignment
    };
    Q_ENUM(RangeAlignment)
    enum TextureMode
    {
        StretchTexture,
        TileTexture
    };
    Q_ENUM(TextureMode)
    enum LineCapStyle
    {
        AutoCap,
        FlatCap,
        SquareCap,
        RoundCap
    };
    Q_ENUM(LineCapStyle)
    enum LineJoinStyle
    {
        AutoJoin,
        BevelJoin,
        MiterJoin,
        RoundJoin
    };
    Q_ENUM(LineJoinStyle)
    enum Type
    {
        Inherit = -1,
        Line,
        Area,
        Bar,
        StackedBar,
        Scatter,
        Bubble,
        Pie,
        Doughnut,
        Radar,
        Heatmap,
        PercentBar,
        HorizontalBar,
        HorizontalStackedBar,
        HorizontalPercentBar,
        RangeArea,
        Histogram,
        Waterfall,
        Candlestick,
        Ohlc,
        BoxPlot,
        Grid,
        Contour
    };
    Q_ENUM(Type)
    enum CurveMode
    {
        LinearCurve,
        StepLeft,
        StepRight,
        StepCenter,
        MonotoneSpline,
        CatmullRomSpline,
        NaturalSpline
    };
    Q_ENUM(CurveMode)
    enum MarkerShape
    {
        Circle,
        Square,
        Triangle,
        Diamond,
        TextureMarker
    };
    Q_ENUM(MarkerShape)
    enum StrokeStyle
    {
        SolidLine,
        DashLine
    };
    Q_ENUM(StrokeStyle)
    enum AreaBaselineMode
    {
        ZeroBaseline,
        CustomBaseline,
        PlotBottomBaseline
    };
    Q_ENUM(AreaBaselineMode)
    enum FillStyle
    {
        SolidFill,
        VerticalGradient,
        HorizontalGradient,
        TextureFill
    };
    Q_ENUM(FillStyle)
    enum BarLabelPosition
    {
        BarLabelCenter,
        BarLabelInsideEnd,
        BarLabelInsideBase,
        BarLabelOutsideEnd
    };
    Q_ENUM(BarLabelPosition)
    // value is the bubble radius (logical pixels) or heatmap intensity.
    struct Sample
    {
        double x = 0, y = 0, value = 1;
        QString label;
    };
    // Paired boundaries share X by construction. Names remain stable when boundaries cross.
    struct RangeSample
    {
        double x = 0, lower = 0, upper = 0;
        QString label;
        bool breakBefore = false;
    };
    explicit ChartSeries(QObject *parent = nullptr);
    ~ChartSeries() override;
    QQmlComponent *pointDelegate() const;
    void setPointDelegate(QQmlComponent *value);
    QQmlComponent *barDelegate() const;
    void setBarDelegate(QQmlComponent *value);
    QQmlComponent *labelDelegate() const;
    void setLabelDelegate(QQmlComponent *value);
    QUrl markerTexture() const;
    void setMarkerTexture(const QUrl &value);
    QString markerTextureError() const;
    qreal barLabelRotation() const;
    void setBarLabelRotation(qreal value);
    QVariantList fillGradientStops() const;
    void setFillGradientStops(const QVariantList &value);
    QUrl fillTexture() const;
    void setFillTexture(const QUrl &value);
    QSizeF fillTextureSize() const;
    void setFillTextureSize(const QSizeF &value);
    TextureMode fillTextureMode() const;
    void setFillTextureMode(TextureMode value);
    QString fillTextureError() const;
    LineCapStyle lineCap() const;
    void setLineCap(LineCapStyle value);
    LineJoinStyle lineJoin() const;
    void setLineJoin(LineJoinStyle value);
    qreal miterLimit() const;
    void setMiterLimit(qreal value);
    ChartAxis *xAxis() const;
    void setXAxis(ChartAxis *value);
    ChartAxis *yAxis() const;
    void setYAxis(ChartAxis *value);
    QString name() const;
    void setName(const QString &value);
    QColor color() const;
    void setColor(const QColor &value);
    bool isVisible() const;
    void setVisible(bool value);
    Type type() const;
    void setType(Type value);
    qreal lineWidth() const;
    void setLineWidth(qreal value);
    CurveMode curveMode() const;
    void setCurveMode(CurveMode value);
    qreal splineTension() const;
    void setSplineTension(qreal value);
    MarkerShape markerShape() const;
    void setMarkerShape(MarkerShape value);
    qreal markerSize() const;
    void setMarkerSize(qreal value);
    bool markersVisible() const;
    void setMarkersVisible(bool value);
    StrokeStyle strokeStyle() const;
    void setStrokeStyle(StrokeStyle value);
    QList<qreal> dashPattern() const;
    void setDashPattern(const QList<qreal> &value);
    qreal dashOffset() const;
    void setDashOffset(qreal value);
    AreaBaselineMode areaBaselineMode() const;
    void setAreaBaselineMode(AreaBaselineMode value);
    double areaBaseline() const;
    void setAreaBaseline(double value);
    QColor fillColor() const;
    void setFillColor(const QColor &value);
    QColor borderColor() const;
    void setBorderColor(const QColor &value);
    FillStyle fillStyle() const;
    void setFillStyle(FillStyle value);
    QColor fillEndColor() const;
    void setFillEndColor(const QColor &value);
    qreal fillOpacity() const;
    void setFillOpacity(qreal value);
    qreal barBorderWidth() const;
    void setBarBorderWidth(qreal value);
    QColor barBorderColor() const;
    void setBarBorderColor(const QColor &value);
    bool barLabelsVisible() const;
    void setBarLabelsVisible(bool value);
    BarLabelPosition barLabelPosition() const;
    void setBarLabelPosition(BarLabelPosition value);
    QString barLabelFormat() const;
    void setBarLabelFormat(const QString &value);
    int barLabelPrecision() const;
    void setBarLabelPrecision(int value);
    qreal barLabelMargin() const;
    void setBarLabelMargin(qreal value);
    bool hasBreaks() const;
    // Replaces the complete style; an empty map restores the chart defaults.
    Q_INVOKABLE bool setSliceStyle(int index, const QVariantMap &style);
    Q_INVOKABLE QVariantMap sliceStyle(int index) const;
    Q_INVOKABLE bool breakBefore(int index) const;
    Q_INVOKABLE bool setBreakBefore(int index, bool value);
    Q_INVOKABLE bool hasRangeValue(int index) const;
    Q_INVOKABLE double lowerValue(int index) const;
    bool appendRangeSamples(const QVector<RangeSample> &samples);
    bool replaceRangeSamples(const QVector<RangeSample> &samples);
    Q_INVOKABLE bool appendRangeData(const QVariantList &samples);
    Q_INVOKABLE bool setRangeData(const QVariantList &samples);
    Q_INVOKABLE bool setRangeBoundaries(const QVariantList &lower, const QVariantList &upper,
                                        RangeAlignment alignment = ExactAlignment);
    int capacity() const;
    void setCapacity(int value);
    int count() const;
    // Reference remains valid until the next mutation; do not retain it in consumers.
    const Sample &sampleAt(int index) const;
    bool sortedX() const;
    int lowerBound(double x) const;
    // Object-thread API. Producers on other threads must queue entire batches.
    bool appendSamples(const QVector<Sample> &samples);
    bool replaceSamples(const QVector<Sample> &samples);
    bool replacePoints(const QVector<QPointF> &points);
    Q_INVOKABLE bool append(double x, double y, double value = 1);
    Q_INVOKABLE bool appendData(const QVariantList &samples);
    Q_INVOKABLE bool setData(const QVariantList &samples);
    Q_INVOKABLE QVariantMap at(int index) const;
    Q_INVOKABLE void clear();
  signals:
    void dataChanged();
    void styleChanged();
    void capacityChanged();

  private:
    friend class ChartItem;
    friend class ChartStatisticsAccess;
    QImage fillTextureImage() const;
    QImage markerTextureImage() const;
    Q_DISABLE_COPY_MOVE(ChartSeries)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
