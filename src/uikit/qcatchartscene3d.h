// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartaxis.h"
#include "qcatchartseries3d.h"
#include <QQmlListProperty>
#include <memory>

namespace QCatUiKit
{
namespace Detail
{
class ChartScene3DAccess;
}
class QCATUIKIT_EXPORT ChartScene3D : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartScene3D)
    QML_UNCREATABLE("Use CatChart.scene3D")
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY changed)
    Q_PROPERTY(QQmlListProperty<QCatUiKit::ChartSeries3D> series READ series NOTIFY changed)
    Q_PROPERTY(QCatUiKit::ChartAxis *xAxis READ xAxis CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartAxis *yAxis READ yAxis CONSTANT)
    Q_PROPERTY(QCatUiKit::ChartAxis *zAxis READ zAxis CONSTANT)
    Q_PROPERTY(double yaw READ yaw WRITE setYaw NOTIFY cameraChanged)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY cameraChanged)
    Q_PROPERTY(double distance READ distance WRITE setDistance NOTIFY cameraChanged)
    Q_PROPERTY(QVector3D target READ target WRITE setTarget NOTIFY cameraChanged)
    Q_PROPERTY(Projection projection READ projection WRITE setProjection NOTIFY cameraChanged)
    Q_PROPERTY(double nearPlane READ nearPlane WRITE setNearPlane NOTIFY cameraChanged)
    Q_PROPERTY(double farPlane READ farPlane WRITE setFarPlane NOTIFY cameraChanged)
    Q_PROPERTY(double ambient READ ambient WRITE setAmbient NOTIFY materialChanged)
    Q_PROPERTY(double diffuse READ diffuse WRITE setDiffuse NOTIFY materialChanged)
    Q_PROPERTY(double specular READ specular WRITE setSpecular NOTIFY materialChanged)
    Q_PROPERTY(double shininess READ shininess WRITE setShininess NOTIFY materialChanged)
    Q_PROPERTY(
        QVector3D lightDirection READ lightDirection WRITE setLightDirection NOTIFY materialChanged)
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible NOTIFY changed)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY changed)
    Q_PROPERTY(QVariantMap selectedPoint READ selectedPoint NOTIFY selectionChanged)
    Q_PROPERTY(SliceMode sliceMode READ sliceMode WRITE setSliceMode NOTIFY changed)
    Q_PROPERTY(int sliceIndex READ sliceIndex WRITE setSliceIndex NOTIFY changed)
    Q_PROPERTY(bool supported READ supported NOTIFY statusChanged)
    Q_PROPERTY(QString error READ error NOTIFY statusChanged)
    Q_PROPERTY(LightingModel lightingModel READ lightingModel WRITE setLightingModel NOTIFY
                   materialChanged)
    Q_PROPERTY(double ambientGroundRatio READ ambientGroundRatio WRITE setAmbientGroundRatio NOTIFY
                   materialChanged)
    Q_PROPERTY(double exposure READ exposure WRITE setExposure NOTIFY materialChanged)
  public:
    enum Projection
    {
        Perspective,
        Orthographic
    };
    Q_ENUM(Projection)
    enum Preset
    {
        Isometric,
        Front,
        Top,
        Right
    };
    Q_ENUM(Preset)
    enum SelectionMode
    {
        NoSelection,
        PointSelection,
        RowSelection,
        ColumnSelection
    };
    Q_ENUM(SelectionMode)
    enum SliceMode
    {
        NoSlice,
        RowSlice,
        ColumnSlice
    };
    Q_ENUM(SliceMode)
    enum LightingModel
    {
        Phong,
        BlinnPhong
    };
    Q_ENUM(LightingModel)
    explicit ChartScene3D(QObject *parent = nullptr);
    ~ChartScene3D() override;
    bool enabled() const;
    void setEnabled(bool value);
    QQmlListProperty<ChartSeries3D> series();
    Q_INVOKABLE void addSeries(QCatUiKit::ChartSeries3D *series);
    Q_INVOKABLE void removeSeries(QCatUiKit::ChartSeries3D *series);
    Q_INVOKABLE void clearSeries();
    int seriesCount() const;
    ChartSeries3D *seriesAt(int index) const;
    ChartAxis *xAxis() const;
    ChartAxis *yAxis() const;
    ChartAxis *zAxis() const;
    double yaw() const;
    void setYaw(double value);
    double pitch() const;
    void setPitch(double value);
    double distance() const;
    void setDistance(double value);
    QVector3D target() const;
    void setTarget(QVector3D value);
    Projection projection() const;
    void setProjection(Projection value);
    double nearPlane() const;
    void setNearPlane(double value);
    double farPlane() const;
    void setFarPlane(double value);
    double ambient() const;
    void setAmbient(double value);
    double diffuse() const;
    void setDiffuse(double value);
    double specular() const;
    void setSpecular(double value);
    double shininess() const;
    void setShininess(double value);
    QVector3D lightDirection() const;
    void setLightDirection(QVector3D value);
    bool gridVisible() const;
    void setGridVisible(bool value);
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode value);
    QVariantMap selectedPoint() const;
    Q_INVOKABLE bool selectPoint(int series, int index);
    Q_INVOKABLE void clearSelection();
    SliceMode sliceMode() const;
    void setSliceMode(SliceMode value);
    int sliceIndex() const;
    void setSliceIndex(int value);
    bool supported() const;
    QString error() const;
    Q_INVOKABLE void resetView();
    Q_INVOKABLE void setPreset(Preset preset);
    Q_INVOKABLE void rotateBy(QPointF degrees);
    Q_INVOKABLE void panBy(QPointF pixels);
    Q_INVOKABLE void zoomBy(double factor);
    Q_INVOKABLE QVariantMap dataAt(QPointF position) const;
    Q_INVOKABLE QPointF mapToPosition(double x, double y, double z) const;
    Q_INVOKABLE QVariantMap renderingStatistics() const;
    LightingModel lightingModel() const;
    void setLightingModel(LightingModel value);
    double ambientGroundRatio() const;
    void setAmbientGroundRatio(double value);
    double exposure() const;
    void setExposure(double value);
  signals:
    void changed();
    void cameraChanged();
    void materialChanged();
    void selectionChanged();
    void statusChanged();
    void pointClicked(QVariantMap point);

  private:
    friend class Detail::ChartScene3DAccess;
    Q_DISABLE_COPY_MOVE(ChartScene3D)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
