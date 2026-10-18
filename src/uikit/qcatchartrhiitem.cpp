// SPDX-License-Identifier: Apache-2.0
#include "qcatchartrhiitem_p.h"
#include <QFile>
#include <QMatrix4x4>
#include <QMutexLocker>
#include <QSet>
#include <cstring>
#include <limits>
#include <rhi/qrhi.h>

namespace QCatUiKit::Detail
{
namespace
{
QShader shader(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QShader::fromSerialized(file.readAll()) : QShader();
}
class ChartRenderer final : public QQuickRhiItemRenderer
{
  public:
    explicit ChartRenderer(std::shared_ptr<ChartRenderStatistics> statistics)
        : m_statistics(std::move(statistics))
    {
    }

  protected:
    void synchronize(QQuickRhiItem *item) override
    {
        m_data = static_cast<ChartRhiItem *>(item)->data();
        m_size = item->size();
    }
    void initialize(QRhiCommandBuffer *) override
    {
        m_ready = false;
        if (m_rhi != rhi())
        {
            m_pipeline.reset();
            m_instancePipeline.reset();
            m_bindings.reset();
            m_materials.clear();
            m_white.reset();
            m_sampler.reset();
            m_quad.reset();
            m_uniform.reset();
            m_pass.reset();
            m_vertices = {};
            m_instances = {};
            m_uploaded.reset();
            m_fixedUploaded = false;
            m_rhi = rhi();
            // Private diagnostic switch exercises the non-instanced backend fallback.
            m_instancing = m_rhi->isFeatureSupported(QRhi::Instancing) &&
                           !qEnvironmentVariableIntValue("QCAT_CHART_FORCE_INSTANCE_FALLBACK");
        }
        if (!m_uniform)
        {
            m_uniform.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
            if (!m_uniform->create())
            {
                m_uniform.reset();
                fail(QStringLiteral("Unable to create chart uniform buffer."));
                return;
            }
        }
        if (!m_quad)
        {
            m_quad.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                          12 * sizeof(float)));
            if (!m_quad->create())
            {
                m_quad.reset();
                fail(QStringLiteral("Unable to create chart instance quad."));
                return;
            }
        }
        if (!m_white)
        {
            m_white.reset(m_rhi->newTexture(QRhiTexture::RGBA8, {1, 1}, 1));
            m_sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear,
                                              QRhiSampler::None, QRhiSampler::ClampToEdge,
                                              QRhiSampler::ClampToEdge));
            if (!m_white->create() || !m_sampler->create())
            {
                m_white.reset();
                m_sampler.reset();
                fail(QStringLiteral("Unable to create chart material resources."));
                return;
            }
        }
        if (!m_bindings)
        {
            m_bindings = bindings(m_white.get(), m_sampler.get());
            if (!m_bindings)
            {
                fail(QStringLiteral("Unable to create chart shader bindings."));
                return;
            }
        }
        const auto *pass = renderTarget()->renderPassDescriptor();
        const int samples = renderTarget()->sampleCount();
        if (!m_pipeline || !m_instancePipeline || m_samples != samples ||
            !m_pass->isCompatible(pass))
        {
            m_pipeline.reset();
            m_instancePipeline.reset();
            m_pass.reset(pass->newCompatibleRenderPassDescriptor());
            m_samples = samples;
            m_pipeline = pipeline(false);
            m_instancePipeline = pipeline(true);
            if (!m_pipeline || !m_instancePipeline)
            {
                fail(QStringLiteral("Unable to create chart graphics pipelines."));
                return;
            }
        }
        m_ready = true;
        QMutexLocker lock(&m_statistics->mutex);
        m_statistics->error.clear();
    }
    void render(QRhiCommandBuffer *cb) override
    {
        const bool content =
            m_data && (!m_data->vertices.isEmpty() || !m_data->instances.isEmpty());
        bool draw = m_ready && content && m_size.width() > 0 && m_size.height() > 0;
        QRhiResourceUpdateBatch *updates = nullptr;
        QVector<QRhiShaderResourceBindings *> materialBindings;
        QSet<quint64> usedMaterials;
        if (draw)
        {
            updates = m_rhi->nextResourceUpdateBatch();
            if (!m_fixedUploaded)
            {
                QImage white(1, 1, QImage::Format_RGBA8888_Premultiplied);
                white.fill(Qt::white);
                updates->uploadTexture(m_white.get(), white);
                const float quad[] = {0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1};
                updates->uploadStaticBuffer(m_quad.get(), quad);
                m_fixedUploaded = true;
            }
            for (const auto &texture : m_data->textures)
            {
                const quint64 key = texture.stableId ? (quint64(1) << 63) | texture.stableId
                                                     : quint64(texture.image.cacheKey()) ^
                                                           (quint64(texture.repeat) << 62);
                usedMaterials.insert(key);
                auto material = m_materials.value(key);
                bool created = !material || material->image.size() != texture.image.size();
                if (created)
                {
                    material = std::make_shared<Material>();
                    material->texture.reset(
                        m_rhi->newTexture(QRhiTexture::RGBA8, texture.image.size(), 1));
                    const auto mode =
                        texture.repeat ? QRhiSampler::Repeat : QRhiSampler::ClampToEdge;
                    const auto filter =
                        texture.nearest ? QRhiSampler::Nearest : QRhiSampler::Linear;
                    material->sampler.reset(
                        m_rhi->newSampler(filter, filter, QRhiSampler::None, mode, mode));
                    if (!material->texture->create() || !material->sampler->create())
                    {
                        fail(QStringLiteral("Unable to create chart texture."));
                        draw = false;
                        break;
                    }
                    material->bindings = bindings(material->texture.get(), material->sampler.get());
                    if (!material->bindings)
                    {
                        fail(QStringLiteral("Unable to bind chart texture."));
                        draw = false;
                        break;
                    }
                    ++m_statistics->textureAllocations;
                    m_materials.insert(key, material);
                }
                if (created || material->image.cacheKey() != texture.image.cacheKey())
                {
                    QRect changed = texture.image.rect();
                    if (!created && texture.stableId &&
                        material->image.format() == texture.image.format())
                    {
                        int left = texture.image.width(), top = texture.image.height(), right = -1,
                            bottom = -1;
                        for (int y = 0; y < texture.image.height(); ++y)
                        {
                            const auto *a =
                                reinterpret_cast<const quint32 *>(material->image.constScanLine(y));
                            const auto *b =
                                reinterpret_cast<const quint32 *>(texture.image.constScanLine(y));
                            if (std::memcmp(a, b, texture.image.width() * 4) == 0)
                                continue;
                            int first = 0, last = texture.image.width() - 1;
                            while (first <= last && a[first] == b[first])
                                ++first;
                            while (last >= first && a[last] == b[last])
                                --last;
                            left = std::min(left, first);
                            right = std::max(right, last);
                            top = std::min(top, y);
                            bottom = y;
                        }
                        changed = right < left ? QRect()
                                               : QRect(QPoint(left, top), QPoint(right, bottom));
                    }
                    if (!changed.isEmpty())
                    {
                        QRhiTextureSubresourceUploadDescription sub(texture.image.copy(changed));
                        sub.setDestinationTopLeft(changed.topLeft());
                        updates->uploadTexture(material->texture.get(),
                                               QRhiTextureUploadDescription({{0, 0, sub}}));
                        ++m_statistics->textureUploads;
                        m_statistics->textureUploadedBytes +=
                            quint64(changed.width()) * changed.height() * 4;
                    }
                    material->image = texture.image;
                }
                materialBindings.append(material->bindings.get());
            }
            if (draw && m_uploaded != m_data)
            {
                QVector<ChartInstance> expanded;
                const QVector<ChartInstance> *instances = &m_data->instances;
                // Backends without hardware instancing keep the same analytic shader and draw each
                // quad.
                if (!m_instancing)
                {
                    if (instances->size() >
                        std::numeric_limits<int>::max() / (6 * qsizetype(sizeof(ChartInstance))))
                        draw = false;
                    else
                    {
                        expanded.reserve(instances->size() * 6);
                        for (const auto &instance : *instances)
                            for (int i = 0; i < 6; ++i)
                                expanded.append(instance);
                        instances = &expanded;
                    }
                }
                if (draw)
                    draw = upload(m_data->vertices, m_vertices, updates, m_statistics->uploads) &&
                           upload(*instances, m_instances, updates, m_statistics->instanceUploads);
                if (draw)
                    m_uploaded = m_data;
            }
            QMatrix4x4 projection;
            projection.ortho(0, float(m_size.width()), float(m_size.height()), 0, -1, 1);
            projection = m_rhi->clipSpaceCorrMatrix() * projection;
            updates->updateDynamicBuffer(m_uniform.get(), 0, 64, projection.constData());
        }
        for (auto it = m_materials.begin(); it != m_materials.end();)
        {
            if (!usedMaterials.contains(it.key()))
                it = m_materials.erase(it);
            else
                ++it;
        }
        cb->beginPass(renderTarget(), QColor(Qt::transparent), {1, 0}, updates);
        if (draw)
        {
            const auto pixels = renderTarget()->pixelSize();
            cb->setViewport({0, 0, float(pixels.width()), float(pixels.height())});
            for (const auto &command : m_data->draws)
            {
                cb->setGraphicsPipeline(command.instanced ? m_instancePipeline.get()
                                                          : m_pipeline.get());
                cb->setShaderResources(command.texture < 0 ? m_bindings.get()
                                                           : materialBindings[command.texture]);
                if (command.instanced)
                {
                    const int repeats = m_instancing ? 1 : command.count;
                    for (int i = 0; i < repeats; ++i)
                    {
                        const quint32 offset = quint32((command.first + i) * sizeof(ChartInstance) *
                                                       (m_instancing ? 1 : 6));
                        const QRhiCommandBuffer::VertexInput input[] = {
                            {m_quad.get(), 0}, {m_instances.buffer.get(), offset}};
                        cb->setVertexInput(0, 2, input);
                        cb->draw(6, m_instancing ? command.count : 1);
                    }
                }
                else
                {
                    const QRhiCommandBuffer::VertexInput input(m_vertices.buffer.get(), 0);
                    cb->setVertexInput(0, 1, &input);
                    cb->draw(command.count, 1, command.first);
                }
            }
        }
        cb->endPass();
        ++m_statistics->frames;
        if (m_ready && (draw || !content))
        {
            m_statistics->revision = m_data ? m_data->revision : 0;
            m_statistics->instanceCount = m_data ? m_data->instances.size() : 0;
        }
    }

  private:
    template <typename T> struct Buffer
    {
        std::unique_ptr<QRhiBuffer> buffer;
        int capacity = 0;
        QVector<T> previous;
    };
    template <typename T>
    bool upload(const QVector<T> &next, Buffer<T> &state, QRhiResourceUpdateBatch *updates,
                std::atomic<quint64> &counter)
    {
        if (next.size() > std::numeric_limits<int>::max() / qsizetype(sizeof(T)))
        {
            fail(QStringLiteral("Chart buffer exceeds the supported size."));
            return false;
        }
        const int bytes = int(next.size() * sizeof(T));
        if (!bytes)
        {
            state.previous.clear();
            return true;
        }
        if (!state.buffer || state.capacity < bytes)
        {
            state.buffer.reset();
            state.previous.clear();
            state.capacity = int(std::min<qint64>(std::numeric_limits<int>::max(),
                                                  std::max<qint64>(4096, qint64(bytes) * 3 / 2)));
            state.buffer.reset(
                m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, state.capacity));
            if (!state.buffer->create())
            {
                state.buffer.reset();
                fail(QStringLiteral("Unable to create chart data buffer."));
                return false;
            }
            ++m_statistics->allocations;
        }
        qsizetype first = 0, end = next.size();
        const auto &old = state.previous;
        const auto same = [&](qsizetype i)
        { return std::memcmp(&old[i], &next[i], sizeof(T)) == 0; };
        while (first < std::min(old.size(), next.size()) && same(first))
            ++first;
        if (old.size() == next.size())
            while (end > first && same(end - 1))
                --end;
        if (end > first)
        {
            const int length = int((end - first) * sizeof(T));
            updates->updateDynamicBuffer(state.buffer.get(), int(first * sizeof(T)), length,
                                         next.constData() + first);
            ++counter;
            m_statistics->uploadedBytes += length;
        }
        state.previous = next;
        return true;
    }
    std::unique_ptr<QRhiShaderResourceBindings> bindings(QRhiTexture *texture, QRhiSampler *sampler)
    {
        std::unique_ptr<QRhiShaderResourceBindings> result(m_rhi->newShaderResourceBindings());
        result->setBindings({QRhiShaderResourceBinding::uniformBuffer(
                                 0, QRhiShaderResourceBinding::VertexStage, m_uniform.get()),
                             QRhiShaderResourceBinding::sampledTexture(
                                 1, QRhiShaderResourceBinding::FragmentStage, texture, sampler)});
        if (!result->create())
            return {};
        return result;
    }
    std::unique_ptr<QRhiGraphicsPipeline> pipeline(bool instanced)
    {
        const auto base = instanced ? QStringLiteral(":/QCatUiKit/shaders/chartinstance")
                                    : QStringLiteral(":/QCatUiKit/shaders/chart");
        const auto vert = shader(base + QStringLiteral(".vert.qsb")),
                   frag = shader(base + QStringLiteral(".frag.qsb"));
        if (!vert.isValid() || !frag.isValid())
            return {};
        std::unique_ptr<QRhiGraphicsPipeline> result(m_rhi->newGraphicsPipeline());
        result->setShaderStages(
            {{QRhiShaderStage::Vertex, vert}, {QRhiShaderStage::Fragment, frag}});
        QRhiVertexInputLayout input;
        if (instanced)
        {
            input.setBindings(
                {{2 * sizeof(float)},
                 {sizeof(ChartInstance), m_instancing ? QRhiVertexInputBinding::PerInstance
                                                      : QRhiVertexInputBinding::PerVertex}});
            input.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                                 {1, 1, QRhiVertexInputAttribute::Float4, 0},
                                 {1, 2, QRhiVertexInputAttribute::UNormByte4, 16},
                                 {1, 3, QRhiVertexInputAttribute::Float2, 20}});
        }
        else
        {
            input.setBindings({{sizeof(ChartVertex)}});
            input.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                                 {0, 1, QRhiVertexInputAttribute::UNormByte4, 8},
                                 {0, 2, QRhiVertexInputAttribute::Float2, 12},
                                 {0, 3, QRhiVertexInputAttribute::Float2, 20}});
        }
        result->setVertexInputLayout(input);
        result->setTopology(QRhiGraphicsPipeline::Triangles);
        QRhiGraphicsPipeline::TargetBlend blend;
        blend.enable = true;
        blend.srcColor = blend.srcAlpha = QRhiGraphicsPipeline::One;
        blend.dstColor = blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        result->setTargetBlends({blend});
        result->setSampleCount(m_samples);
        result->setShaderResourceBindings(m_bindings.get());
        result->setRenderPassDescriptor(m_pass.get());
        if (!result->create())
            return {};
        return result;
    }
    void fail(const QString &error)
    {
        QMutexLocker lock(&m_statistics->mutex);
        m_statistics->error = error;
    }
    struct Material
    {
        QImage image;
        std::unique_ptr<QRhiTexture> texture;
        std::unique_ptr<QRhiSampler> sampler;
        std::unique_ptr<QRhiShaderResourceBindings> bindings;
    };
    QHash<quint64, std::shared_ptr<Material>> m_materials;
    Buffer<ChartVertex> m_vertices;
    Buffer<ChartInstance> m_instances;
    std::unique_ptr<QRhiTexture> m_white;
    std::unique_ptr<QRhiSampler> m_sampler;
    std::unique_ptr<QRhiBuffer> m_quad, m_uniform;
    std::unique_ptr<QRhiShaderResourceBindings> m_bindings;
    std::unique_ptr<QRhiRenderPassDescriptor> m_pass;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline, m_instancePipeline;
    std::shared_ptr<ChartRenderStatistics> m_statistics;
    std::shared_ptr<const ChartRenderData> m_data, m_uploaded;
    QSizeF m_size;
    QRhi *m_rhi = nullptr;
    int m_samples = 0;
    bool m_ready = false, m_fixedUploaded = false, m_instancing = true;
};
} // namespace

QVariantMap ChartRenderStatistics::values()
{
    QMutexLocker lock(&mutex);
    return {{"frames", frames.load()},
            {"renderedRevision", revision.load()},
            {"vertexUploads", uploads.load()},
            {"textureUploads", textureUploads.load()},
            {"textureUploadedBytes", textureUploadedBytes.load()},
            {"textureAllocations", textureAllocations.load()},
            {"instanceUploads", instanceUploads.load()},
            {"instanceCount", instanceCount.load()},
            {"uploadedBytes", uploadedBytes.load()},
            {"bufferAllocations", allocations.load()},
            {"error", error}};
}
ChartRhiItem::ChartRhiItem(QQuickItem *parent, std::shared_ptr<ChartRenderStatistics> statistics)
    : QQuickRhiItem(parent), m_statistics(std::move(statistics))
{
    setAlphaBlending(true);
    setAcceptedMouseButtons(Qt::NoButton);
}
void ChartRhiItem::setData(std::shared_ptr<const ChartRenderData> data)
{
    if (m_data == data)
        return;
    m_data = std::move(data);
    update();
}
QQuickRhiItemRenderer *ChartRhiItem::createRenderer()
{
    return new ChartRenderer(m_statistics);
}
} // namespace QCatUiKit::Detail
