// SPDX-License-Identifier: Apache-2.0
#include "qcatchart3d_p.h"
#include <QFile>
#include <QMutexLocker>
#include <cstring>
#include <limits>
#include <rhi/qrhi.h>

namespace QCatUiKit::Detail
{
namespace
{
QShader loadShader(const char *path)
{
    QFile f(QString::fromLatin1(path));
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}
class Renderer3D final : public QQuickRhiItemRenderer
{
  public:
    explicit Renderer3D(std::shared_ptr<ChartRenderStatistics> stats) : stats(std::move(stats)) {}

  protected:
    void synchronize(QQuickRhiItem *item) override
    {
        frame = static_cast<ChartRhiItem3D *>(item)->frame();
    }
    void initialize(QRhiCommandBuffer *) override
    {
        ready = false;
        if (device != rhi())
        {
            pipeline.reset();
            resources.clear();
            sampler.reset();
            uniform.reset();
            vertices.reset();
            instances.reset();
            indices.reset();
            pass.reset();
            uploaded.reset();
            vertexCapacity = instanceCapacity = indexCapacity = 0;
            device = rhi();
        }
        if (!device->isFeatureSupported(QRhi::Instancing))
        {
            fail("3D charts require hardware instancing on this graphics backend.");
            return;
        }
        if (!uniform)
        {
            uniform.reset(device->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 128));
            if (!uniform->create())
            {
                uniform.reset();
                fail("Unable to allocate 3D uniforms.");
                return;
            }
        }
        if (!sampler)
        {
            sampler.reset(device->newSampler(QRhiSampler::Linear, QRhiSampler::Linear,
                                             QRhiSampler::None, QRhiSampler::ClampToEdge,
                                             QRhiSampler::ClampToEdge));
            if (!sampler->create())
            {
                sampler.reset();
                fail("Unable to create 3D sampler.");
                return;
            }
        }
        if (resources.empty())
        {
            QImage white(1, 1, QImage::Format_RGBA8888);
            white.fill(Qt::white);
            if (!addTexture(white))
                return;
        }
        auto *rp = renderTarget()->renderPassDescriptor();
        if (!pipeline || !pass->isCompatible(rp) || samples != renderTarget()->sampleCount())
        {
            pipeline.reset();
            pass.reset(rp->newCompatibleRenderPassDescriptor());
            samples = renderTarget()->sampleCount();
            auto p = std::unique_ptr<QRhiGraphicsPipeline>(device->newGraphicsPipeline());
            auto vs = loadShader(":/QCatUiKit/shaders/chart3d.vert.qsb"),
                 fs = loadShader(":/QCatUiKit/shaders/chart3d.frag.qsb");
            if (!vs.isValid() || !fs.isValid())
            {
                fail("Missing 3D chart shader.");
                return;
            }
            p->setShaderStages({{QRhiShaderStage::Vertex, vs}, {QRhiShaderStage::Fragment, fs}});
            QRhiVertexInputLayout layout;
            layout.setBindings({{sizeof(ChartVertex3D)},
                                {sizeof(ChartInstance3D), QRhiVertexInputBinding::PerInstance}});
            layout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float3, 0},
                                  {0, 1, QRhiVertexInputAttribute::Float3, 12},
                                  {0, 2, QRhiVertexInputAttribute::Float3, 24},
                                  {0, 3, QRhiVertexInputAttribute::Float2, 36},
                                  {1, 4, QRhiVertexInputAttribute::Float4, 0},
                                  {1, 5, QRhiVertexInputAttribute::Float4, 16},
                                  {1, 6, QRhiVertexInputAttribute::Float4, 32},
                                  {1, 7, QRhiVertexInputAttribute::Float4, 48},
                                  {1, 8, QRhiVertexInputAttribute::Float4, 64}});
            p->setVertexInputLayout(layout);
            p->setTopology(QRhiGraphicsPipeline::Triangles);
            p->setDepthTest(true);
            p->setDepthWrite(true);
            p->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
            p->setCullMode(QRhiGraphicsPipeline::None);
            p->setSampleCount(samples);
            p->setShaderResourceBindings(resources.front()->bindings.get());
            p->setRenderPassDescriptor(pass.get());
            if (!p->create())
            {
                fail("Unable to create 3D chart depth pipeline.");
                return;
            }
            pipeline = std::move(p);
        }
        ready = true;
        fail({});
    }
    void render(QRhiCommandBuffer *cb) override
    {
        bool draw = ready && frame && frame->geometry;
        QRhiResourceUpdateBatch *u = nullptr;
        if (draw)
        {
            const auto &g = *frame->geometry;
            u = device->nextResourceUpdateBatch();
            if (uploaded != frame->geometry)
            {
                draw =
                    upload(vertices, vertexCapacity, g.vertices, u, stats->uploads) &&
                    upload(instances, instanceCapacity, g.instances, u, stats->instanceUploads) &&
                    upload(indices, indexCapacity, g.indices, u, stats->uploads,
                           QRhiBuffer::IndexBuffer);
                if (draw)
                {
                    resources.resize(1);
                    for (const auto &image : g.textures)
                        if (!addTexture(image))
                        {
                            draw = false;
                            break;
                        }
                }
                if (draw)
                    uploaded = frame->geometry;
            }
            for (auto &r : resources)
                if (!r->uploaded)
                {
                    u->uploadTexture(r->texture.get(), r->image);
                    r->uploaded = true;
                    ++stats->textureUploads;
                }
            float data[32];
            const auto vp = device->clipSpaceCorrMatrix() * frame->viewProjection;
            std::memcpy(data, vp.constData(), 64);
            const QVector4D eye(frame->eye, 1), light(frame->light, 0);
            data[16] = eye.x();
            data[17] = eye.y();
            data[18] = eye.z();
            data[19] = 1;
            data[20] = light.x();
            data[21] = light.y();
            data[22] = light.z();
            data[23] = 0;
            data[24] = frame->ambient;
            data[25] = frame->diffuse;
            data[26] = frame->specular;
            data[27] = frame->shininess;
            data[28] = float(frame->lightingModel);
            data[29] = frame->ambientGroundRatio;
            data[30] = frame->exposure;
            data[31] = 0;
            u->updateDynamicBuffer(uniform.get(), 0, sizeof(data), data);
        }
        cb->beginPass(renderTarget(), Qt::transparent, {1, 0}, u);
        if (draw && vertices && instances)
        {
            const auto size = renderTarget()->pixelSize();
            cb->setViewport({0, 0, float(size.width()), float(size.height())});
            cb->setGraphicsPipeline(pipeline.get());
            for (const auto &d : frame->geometry->draws)
            {
                if (!d.instances || !d.count || (d.firstIndex >= 0 && !d.indexCount))
                    continue;
                cb->setShaderResources(resources[size_t(d.texture + 1)]->bindings.get());
                const QRhiCommandBuffer::VertexInput input[] = {
                    {vertices.get(), quint32(d.first * sizeof(ChartVertex3D))},
                    {instances.get(), quint32(d.instance * sizeof(ChartInstance3D))}};
                if (d.firstIndex >= 0)
                {
                    cb->setVertexInput(0, 2, input, indices.get(),
                                       quint32(d.firstIndex * sizeof(quint32)),
                                       QRhiCommandBuffer::IndexUInt32);
                    cb->drawIndexed(d.indexCount, d.instances);
                }
                else
                {
                    cb->setVertexInput(0, 2, input);
                    cb->draw(d.count, d.instances);
                }
            }
        }
        cb->endPass();
        ++stats->frames;
        if (draw)
        {
            stats->revision = frame->revision;
            stats->instanceCount = frame->geometry->instances.size();
        }
    }

  private:
    void fail(const QString &s)
    {
        QMutexLocker lock(&stats->mutex);
        stats->error = s;
    }
    template <class T>
    bool upload(std::unique_ptr<QRhiBuffer> &b, int &capacity, const QVector<T> &data,
                QRhiResourceUpdateBatch *u, std::atomic<quint64> &counter,
                QRhiBuffer::UsageFlags usage = QRhiBuffer::VertexBuffer)
    {
        if (data.isEmpty())
            return true;
        if (data.size() > std::numeric_limits<int>::max() / qsizetype(sizeof(T)))
        {
            fail("3D chart buffer is too large.");
            return false;
        }
        const int bytes = int(data.size() * sizeof(T));
        if (capacity < bytes)
        {
            b.reset(device->newBuffer(QRhiBuffer::Dynamic, usage, bytes));
            if (!b->create())
            {
                b.reset();
                fail("Unable to allocate 3D chart geometry.");
                return false;
            }
            capacity = bytes;
            ++stats->allocations;
        }
        u->updateDynamicBuffer(b.get(), 0, bytes, data.constData());
        ++counter;
        stats->uploadedBytes += bytes;
        return true;
    }
    struct Resource
    {
        std::unique_ptr<QRhiTexture> texture;
        std::unique_ptr<QRhiShaderResourceBindings> bindings;
        QImage image;
        bool uploaded = false;
    };
    bool addTexture(const QImage &image)
    {
        auto r = std::make_unique<Resource>();
        r->image = image.convertToFormat(QImage::Format_RGBA8888);
        r->texture.reset(device->newTexture(QRhiTexture::RGBA8, r->image.size()));
        if (!r->texture->create())
        {
            fail("Unable to allocate 3D chart texture.");
            return false;
        }
        r->bindings.reset(device->newShaderResourceBindings());
        r->bindings->setBindings(
            {QRhiShaderResourceBinding::uniformBuffer(0,
                                                      QRhiShaderResourceBinding::VertexStage |
                                                          QRhiShaderResourceBinding::FragmentStage,
                                                      uniform.get()),
             QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
                                                       r->texture.get(), sampler.get())});
        if (!r->bindings->create())
        {
            fail("Unable to bind 3D chart resources.");
            return false;
        }
        resources.push_back(std::move(r));
        return true;
    }
    std::shared_ptr<ChartRenderStatistics> stats;
    std::shared_ptr<const ChartFrame3D> frame;
    std::shared_ptr<const ChartGeometry3D> uploaded;
    QRhi *device = nullptr;
    std::unique_ptr<QRhiBuffer> uniform, vertices, instances, indices;
    std::unique_ptr<QRhiSampler> sampler;
    std::unique_ptr<QRhiGraphicsPipeline> pipeline;
    std::unique_ptr<QRhiRenderPassDescriptor> pass;
    std::vector<std::unique_ptr<Resource>> resources;
    int vertexCapacity = 0, instanceCapacity = 0, indexCapacity = 0, samples = 0;
    bool ready = false;
};
} // namespace
ChartRhiItem3D::ChartRhiItem3D(QQuickItem *parent,
                               std::shared_ptr<ChartRenderStatistics> statistics)
    : QQuickRhiItem(parent), m_statistics(std::move(statistics))
{
    setAlphaBlending(true);
    setAcceptedMouseButtons(Qt::NoButton);
}
QQuickRhiItemRenderer *ChartRhiItem3D::createRenderer()
{
    return new Renderer3D(m_statistics);
}
} // namespace QCatUiKit::Detail
