// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickRhiItem>
#include <QVariantMap>
#include <atomic>
#include <memory>

namespace QCatUiKit::Detail
{
// Private, immutable render-thread input. No series, axis or other GUI QObjects.
struct ChartVertex
{
    float x, y;
    quint8 r, g, b, a;
    float u, v;
    float scalar = 0;
    float atlasInverseHeight = 0;
};
static_assert(sizeof(ChartVertex) == 28,
              "Chart vertices must have no padding for range comparisons.");
// Shape: circle=0, square=1, triangle=2, diamond=3, rectangle=4.
struct ChartInstance
{
    float x, y, width, height;
    quint8 r, g, b, a;
    float shape, feather;
};
static_assert(sizeof(ChartInstance) == 28, "Instance comparisons require an unpadded layout.");
struct ChartTexture
{
    QImage image;
    bool repeat = false;
    quint64 stableId = 0;
    bool nearest = false;
};
struct ChartDraw
{
    int first = 0, count = 0, texture = -1;
    bool instanced = false;
};
struct ChartRenderData
{
    QVector<ChartVertex> vertices;
    QVector<ChartInstance> instances;
    QVector<ChartTexture> textures;
    QVector<ChartDraw> draws;
    quint64 revision = 0;
};
struct ChartRenderStatistics
{
    std::atomic<quint64> frames{0}, revision{0}, uploads{0}, uploadedBytes{0}, allocations{0};
    std::atomic<quint64> textureUploads{0}, instanceUploads{0}, instanceCount{0};
    std::atomic<quint64> textureUploadedBytes{0}, textureAllocations{0};
    QMutex mutex;
    QString error;
    QVariantMap values();
};
class ChartRhiItem final : public QQuickRhiItem
{
  public:
    ChartRhiItem(QQuickItem *parent, std::shared_ptr<ChartRenderStatistics> statistics);
    void setData(std::shared_ptr<const ChartRenderData> data);
    std::shared_ptr<const ChartRenderData> data() const
    {
        return m_data;
    }

  protected:
    QQuickRhiItemRenderer *createRenderer() override;

  private:
    std::shared_ptr<const ChartRenderData> m_data;
    std::shared_ptr<ChartRenderStatistics> m_statistics;
};
} // namespace QCatUiKit::Detail
