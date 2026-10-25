// SPDX-License-Identifier: MIT
#include "qcatthorvgqrhifrag_p.h"
#include "qcatthorvgqrhirenderer_p.h"
#include "qcatthorvgqrhivert_p.h"
#include <QDebug>
#include <cstring>
#include <limits>
namespace QCatUiKit::ThorvgQrhiPrivate
{
using namespace tvg;

Renderer::Target *Renderer::allocateTarget()
{
    // Keep the last canvas image alive for draw(false); never sample and render
    // into the same attachment. The rest of the pool is reused in command order.
    if (nextTarget < targets.size() && targets[nextTarget].get() == history)
        ++nextTarget;
    if (nextTarget == targets.size())
        targets.push_back(std::unique_ptr<Target>(new Target));
    auto t = targets[nextTarget++].get();
    const auto size = renderTarget->pixelSize();
    const int samples = renderTarget->sampleCount();
    if (t->texture && (t->texture->pixelSize() != size || t->target->sampleCount() != samples))
    {
        t->target.reset();
        t->pass.reset();
        t->msaa.reset();
        t->texture.reset();
    }
    if (!t->texture)
    {
        t->texture.reset(
            device->newTexture(QRhiTexture::RGBA8, size, 1, QRhiTexture::RenderTarget));
        if (!t->texture->create())
        {
            t->texture.reset();
            failed = true;
            return nullptr;
        }
        QRhiColorAttachment attachment(t->texture.get());
        if (samples > 1)
        {
            t->msaa.reset(device->newRenderBuffer(QRhiRenderBuffer::Color, size, samples, {},
                                                  QRhiTexture::RGBA8));
            if (!t->msaa->create())
            {
                failed = true;
                return nullptr;
            }
            attachment = QRhiColorAttachment(t->msaa.get());
            attachment.setResolveTexture(t->texture.get());
        }
        t->target.reset(device->newTextureRenderTarget({attachment}));
        t->pass.reset(t->target->newCompatibleRenderPassDescriptor());
        t->target->setRenderPassDescriptor(t->pass.get());
        if (!t->target->create())
        {
            failed = true;
            return nullptr;
        }
    }
    return t;
}

Renderer::Draw Renderer::fullscreen(QRhiTexture *tex, Uniform u, QRhiTexture *aux,
                                    QRhiTexture *original)
{
    if (u.options[0] == 0)
        u = defaults(5);
    const float w = float(surface.w), h = float(surface.h);
    Draw draw;
    draw.uniform = u;
    draw.texture = tex;
    draw.aux = aux;
    draw.original = original;
    draw.first = quint32(frame.size());
    draw.count = 6;
    draw.viewport = {0, 0, int32_t(surface.w), int32_t(surface.h)};
    frame.insert(frame.end(), {{0, 0, 1, 1, 1, 1},
                               {w, 0, 1, 1, 1, 1},
                               {0, h, 1, 1, 1, 1},
                               {w, 0, 1, 1, 1, 1},
                               {w, h, 1, 1, 1, 1},
                               {0, h, 1, 1, 1, 1}});
    return draw;
}

Renderer::Target *Renderer::planEffect(Target *input, const Effect &effect,
                                       const RenderRegion &region)
{
    auto onePass = [&](Target *src, Uniform u, QRhiTexture *original = nullptr) -> Target *
    {
        auto out = allocateTarget();
        if (!out)
            return nullptr;
        u.region[0] = float(region.xMin);
        u.region[1] = float(region.yMin);
        u.region[2] = float(region.xMax);
        u.region[3] = float(region.yMax);
        passes.push_back(
            {out->target.get(), {fullscreen(src->texture.get(), u, nullptr, original)}});
        return out;
    };
    const int mode = int(effect.uniform.options[0]);
    if (mode == 10 || mode == 11)
    {
        auto original = input->texture.get();
        auto u = effect.uniform;
        u.options[0] = 10;
        if (u.p0[2] > 0)
        {
            if (effect.direction != 2)
            {
                u.p0[0] = 1;
                u.p0[1] = 0;
                input = onePass(input, u);
                if (!input)
                    return nullptr;
            }
            if (effect.direction != 1)
            {
                u.p0[0] = 0;
                u.p0[1] = 1;
                input = onePass(input, u);
                if (!input)
                    return nullptr;
            }
        }
        if (mode == 11)
            input = onePass(input, effect.uniform, original);
        return input;
    }
    return onePass(input, effect.uniform);
}

Renderer::Target *Renderer::planLayer(const Layer &layer, unsigned depth)
{
    if (depth > 256)
    {
        failed = true;
        return nullptr;
    }
    auto output = allocateTarget();
    if (!output)
        return nullptr;
    std::vector<Draw> pending;
    if (&layer == &root && !clearFrame && history)
        pending.push_back(fullscreen(history->texture.get()));
    for (const auto &cmd : layer.commands)
    {
        std::vector<Draw> draws;
        if (cmd.mesh)
        {
            auto mesh = cmd.mesh;
            const quint32 first = quint32(frame.size());
            frame.insert(frame.end(), mesh->vertices.begin(), mesh->vertices.end());
            for (auto part : mesh->parts)
                if (part.count)
                {
                    auto &mat = mesh->materials[part.material];
                    Draw d;
                    d.uniform = mat.uniform;
                    d.texture = mat.texture.get();
                    d.nearest = mat.nearest;
                    d.first = first + part.first;
                    d.count = part.count;
                    d.viewport = mesh->viewport;
                    draws.push_back(d);
                }
        }
        else
        {
            auto cmp = cmd.composite;
            auto src = planLayer(cmp->first, depth + 1);
            if (!src)
                return nullptr;
            if (cmp->masked)
            {
                auto content = planLayer(cmp->second, depth + 1);
                if (!content)
                    return nullptr;
                auto out = allocateTarget();
                if (!out)
                    return nullptr;
                auto u = defaults(6);
                u.options[2] = float(cmp->method);
                passes.push_back({out->target.get(),
                                  {fullscreen(content->texture.get(), u, src->texture.get())}});
                src = out;
            }
            auto u = defaults(5);
            u.options[1] = cmp->opacity;
            draws.push_back(fullscreen(src->texture.get(), u));
        }
        if (cmd.blend == BlendMethod::Normal || cmd.blend == BlendMethod::Composition)
        {
            pending.insert(pending.end(), draws.begin(), draws.end());
        }
        else
        {
            auto source = allocateTarget();
            if (!source)
                return nullptr;
            passes.push_back({source->target.get(), std::move(draws)});
            passes.push_back({output->target.get(), std::move(pending)});
            auto out = allocateTarget();
            if (!out)
                return nullptr;
            auto u = defaults(7);
            u.options[2] = float(cmd.blend);
            pending = {fullscreen(source->texture.get(), u, output->texture.get())};
            output = out;
        }
    }
    passes.push_back({output->target.get(), std::move(pending)});
    // Full target-sized layers preserve effect tails outside geometry bounds.
    for (const auto &effect : layer.effects)
    {
        output = planEffect(output, effect, {0, 0, int32_t(surface.w), int32_t(surface.h)});
        if (!output)
            return nullptr;
    }
    return output;
}

QRhiGraphicsPipeline *Renderer::pipelineFor(QRhiRenderTarget *target,
                                            QRhiShaderResourceBindings *bindings)
{
    const auto format = target->renderPassDescriptor()->serializedFormat();
    for (auto &p : pipelines)
        if (p.samples == target->sampleCount() && p.format == format)
            return p.pipeline.get();
    Pipeline p;
    p.samples = target->sampleCount();
    p.format = format;
    p.pass.reset(target->renderPassDescriptor()->newCompatibleRenderPassDescriptor());
    p.bindings.reset(device->newShaderResourceBindings());
    p.bindings->setBindings(
        {QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(
             0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
             layoutUniform.get(), sizeof(Uniform)),
         QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
                                                   white.get(), linearSampler.get()),
         QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage,
                                                   white.get(), linearSampler.get()),
         QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage,
                                                   white.get(), linearSampler.get())});
    if (!p.bindings->create())
        return nullptr;
    p.pipeline.reset(device->newGraphicsPipeline());
    p.pipeline->setShaderStages(
        {{QRhiShaderStage::Vertex,
          QShader::fromSerialized(QByteArray(reinterpret_cast<const char *>(qcatthorvgqrhivert),
                                             sizeof(qcatthorvgqrhivert)))},
         {QRhiShaderStage::Fragment,
          QShader::fromSerialized(QByteArray(reinterpret_cast<const char *>(qcatthorvgqrhifrag),
                                             sizeof(qcatthorvgqrhifrag)))}});
    QRhiVertexInputLayout layout;
    layout.setBindings({{sizeof(Vertex)}});
    layout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                          {0, 1, QRhiVertexInputAttribute::Float4, 2 * sizeof(float)}});
    p.pipeline->setVertexInputLayout(layout);
    p.pipeline->setShaderResourceBindings(p.bindings.get());
    p.pipeline->setRenderPassDescriptor(p.pass.get());
    p.pipeline->setSampleCount(p.samples);
    p.pipeline->setFlags(QRhiGraphicsPipeline::UsesScissor);
    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = true;
    blend.srcColor = blend.srcAlpha = QRhiGraphicsPipeline::One;
    blend.dstColor = blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    p.pipeline->setTargetBlends({blend});
    if (!p.pipeline->create())
        return nullptr;
    auto result = p.pipeline.get();
    pipelines.push_back(std::move(p));
    return result;
}

bool Renderer::postRender()
{
    static_assert(sizeof(Uniform) == 256, "Shader layout must match");
    if (failed || current != &root || !commandBuffer)
        return false;
    frameBindings.clear();
    auto updates = device->nextResourceUpdateBatch();
    if (!layoutUniform)
    {
        layoutUniform.reset(
            device->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Uniform)));
        if (!layoutUniform->create())
        {
            layoutUniform.reset();
            updates->release();
            return false;
        }
    }
    if (!white)
    {
        white.reset(device->newTexture(QRhiTexture::RGBA8, {1, 1}));
        if (!white->create())
        {
            white.reset();
            updates->release();
            return false;
        }
        QImage image(1, 1, QImage::Format_RGBA8888);
        image.fill(Qt::white);
        updates->uploadTexture(white.get(), image);
        linearSampler.reset(device->newSampler(QRhiSampler::Linear, QRhiSampler::Linear,
                                               QRhiSampler::None, QRhiSampler::ClampToEdge,
                                               QRhiSampler::ClampToEdge));
        nearestSampler.reset(device->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest,
                                                QRhiSampler::None, QRhiSampler::ClampToEdge,
                                                QRhiSampler::ClampToEdge));
        if (!linearSampler->create() || !nearestSampler->create())
        {
            updates->release();
            return false;
        }
    }
    for (auto mesh : meshes)
        for (auto &mat : mesh->materials)
            if (!uploadMaterial(mat, updates))
            {
                updates->release();
                return false;
            }
    auto result = planLayer(root);
    if (!result || failed)
    {
        updates->release();
        return false;
    }
    passes.push_back({renderTarget, {fullscreen(result->texture.get())}});
    size_t drawCount = 0;
    for (const auto &pass : passes)
        drawCount += pass.draws.size();
    const size_t stride = device->ubufAligned(sizeof(Uniform));
    if (frame.size() * sizeof(Vertex) > size_t(INT_MAX) || drawCount * stride > size_t(INT_MAX))
    {
        updates->release();
        return false;
    }
    const quint32 bytes = quint32(frame.size() * sizeof(Vertex));
    const quint32 ubBytes = quint32(std::max(size_t(1), drawCount) * stride);
    {
        vertices.reset(
            device->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, std::max(24u, bytes)));
        if (!vertices->create())
        {
            vertices.reset();
            updates->release();
            return false;
        }
    }
    {
        uniforms.reset(device->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubBytes));
        if (!uniforms->create())
        {
            uniforms.reset();
            updates->release();
            return false;
        }
    }
    if (bytes)
        updates->updateDynamicBuffer(vertices.get(), 0, bytes, frame.data());
    size_t index = 0;
    for (auto &pass : passes)
        for (auto &draw : pass.draws)
        {
            QMatrix4x4 projection;
            projection.ortho(0.0f, float(surface.w), float(surface.h), 0.0f, -1.0f, 1.0f);
            projection = device->clipSpaceCorrMatrix() * projection;
            memcpy(draw.uniform.projection, projection.constData(), 64);
            updates->updateDynamicBuffer(uniforms.get(), quint32(index * stride), sizeof(Uniform),
                                         &draw.uniform);
            auto srb = ResourcePtr<QRhiShaderResourceBindings>(device->newShaderResourceBindings());
            auto sampler = draw.nearest ? nearestSampler.get() : linearSampler.get();
            srb->setBindings(
                {QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(
                     0,
                     QRhiShaderResourceBinding::VertexStage |
                         QRhiShaderResourceBinding::FragmentStage,
                     uniforms.get(), sizeof(Uniform)),
                 QRhiShaderResourceBinding::sampledTexture(
                     1, QRhiShaderResourceBinding::FragmentStage,
                     draw.texture ? draw.texture : white.get(), sampler),
                 QRhiShaderResourceBinding::sampledTexture(
                     2, QRhiShaderResourceBinding::FragmentStage, draw.aux ? draw.aux : white.get(),
                     linearSampler.get()),
                 QRhiShaderResourceBinding::sampledTexture(
                     3, QRhiShaderResourceBinding::FragmentStage,
                     draw.original ? draw.original : white.get(), linearSampler.get())});
            if (!srb->create() || !pipelineFor(pass.target, srb.get()))
            {
                updates->release();
                return false;
            }
            frameBindings.push_back(std::move(srb));
            ++index;
        }
    index = 0;
    for (auto &pass : passes)
    {
        commandBuffer->beginPass(pass.target, QColor(0, 0, 0, 0), {1.0f, 0}, updates);
        updates = nullptr;
        commandBuffer->setViewport({0, 0, float(surface.w), float(surface.h)});
        const QRhiCommandBuffer::VertexInput input(vertices.get(), 0);
        for (auto &draw : pass.draws)
        {
            auto clip = RenderRegion::intersect(draw.viewport,
                                                {0, 0, int32_t(surface.w), int32_t(surface.h)});
            if (clip.valid() && draw.count)
            {
                commandBuffer->setGraphicsPipeline(
                    pipelineFor(pass.target, frameBindings[index].get()));
                commandBuffer->setVertexInput(0, 1, &input);
                commandBuffer->setScissor(
                    {clip.sx(), int(surface.h) - clip.sy() - clip.sh(), clip.sw(), clip.sh()});
                const QRhiCommandBuffer::DynamicOffset offset{0, quint32(index * stride)};
                commandBuffer->setShaderResources(frameBindings[index].get(), 1, &offset);
                commandBuffer->draw(draw.count, 1, draw.first);
            }
            ++index;
        }
        commandBuffer->endPass();
    }
    history = result;
    return true;
}

} // namespace QCatUiKit::ThorvgQrhiPrivate
