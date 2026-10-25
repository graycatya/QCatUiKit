// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartrhiitem_p.h"
#include "qcatchartscene3d.h"
#include <QMatrix4x4>
#include <QQuaternion>

namespace QCatUiKit::Detail
{
struct ChartVertex3D
{
    float x, y, z, nx, ny, nz, r, g, b, u, v;
};
struct ChartInstance3D
{
    float matrix[16];
    float color[4];
};
struct ChartDraw3D
{
    int first = 0, count = 0, instance = 0, instances = 0, texture = -1;
    int firstIndex = -1, indexCount = 0;
};
struct ChartObject3D
{
    QMatrix4x4 model, inverse;
    QVector3D minimum, maximum;
    int draw = 0, series = -1, index = -1;
};
struct ChartGeometry3D
{
    QVector<ChartVertex3D> vertices;
    QVector<quint32> indices;
    QVector<ChartInstance3D> instances;
    QVector<ChartDraw3D> draws;
    QVector<QImage> textures;
    QVector<ChartObject3D> objects;
    QVector<int> sourceIndices;
    quint64 revision = 0;
    QString error;
};
struct ChartFrame3D
{
    std::shared_ptr<const ChartGeometry3D> geometry;
    QMatrix4x4 viewProjection;
    QVector3D eye, light;
    float ambient = .25, diffuse = .7, specular = .5, shininess = 32;
    ChartScene3D::LightingModel lightingModel = ChartScene3D::BlinnPhong;
    float ambientGroundRatio = .6, exposure = 1;
    quint64 revision = 0;
};
struct ChartLabel3D
{
    QPointF position;
    QString text;
    int priority = 0;
};
class QCATUIKIT_EXPORT ChartScene3DAccess
{
  public:
    static std::shared_ptr<const ChartFrame3D> frame(ChartScene3D *, QSizeF size, bool software);
    static QVector<ChartLabel3D> labels(ChartScene3D *);
    static std::shared_ptr<ChartRenderStatistics> statistics(ChartScene3D *);
    static void click(ChartScene3D *, QPointF position);
};
class ChartRhiItem3D final : public QQuickRhiItem
{
  public:
    ChartRhiItem3D(QQuickItem *parent, std::shared_ptr<ChartRenderStatistics> statistics);
    void setFrame(std::shared_ptr<const ChartFrame3D> frame)
    {
        m_frame = std::move(frame);
        update();
    }
    const std::shared_ptr<const ChartFrame3D> &frame() const
    {
        return m_frame;
    }

  protected:
    QQuickRhiItemRenderer *createRenderer() override;

  private:
    std::shared_ptr<const ChartFrame3D> m_frame;
    std::shared_ptr<ChartRenderStatistics> m_statistics;
};
} // namespace QCatUiKit::Detail
