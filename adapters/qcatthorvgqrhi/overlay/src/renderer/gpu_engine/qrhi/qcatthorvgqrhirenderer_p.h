// SPDX-License-Identifier: MIT
#pragma once
#if defined(__APPLE__) && defined(__aarch64__)
// Qt's qYieldCpu() uses __yield on Apple Clang; declare it before Qt headers.
#include <arm_acle.h>
#endif
#include "tvgRender.h"
#include <QImage>
#include <QMatrix4x4>
#include <QPainterPath>
#include <memory>
#include <rhi/qrhi.h>
#include <unordered_set>
#include <vector>

namespace QCatUiKit::ThorvgQrhiPrivate
{
// QRhi commands retain resource objects until frame submission, including
// multiple canvas draws within one host frame.
struct DeferredDelete
{
    template <class T> void operator()(T *p) const
    {
        if (p)
            p->deleteLater();
    }
};
template <class T> using ResourcePtr = std::unique_ptr<T, DeferredDelete>;

struct Renderer final : tvg::RenderMethod
{
    struct Vertex
    {
        float x, y, r, g, b, a;
    };
    // Layout shared by both shaders. Every member occupies std140 vec4 slots.
    struct Uniform
    {
        float projection[16]{}, inverse[16]{};
        float options[4]{}, p0[4]{}, p1[4]{}, p2[4]{};
        float c0[4]{}, c1[4]{}, c2[4]{}, region[4]{};
    };
    struct Material
    {
        Uniform uniform;
        QImage image;
        bool nearest = false, dirty = false;
        ResourcePtr<QRhiTexture> texture;
    };
    struct Part
    {
        quint32 first, count;
        unsigned material;
    };
    struct Mesh
    {
        QPainterPath path, hitPath;
        std::vector<Vertex> vertices;
        std::vector<Part> parts;
        Material materials[2];
        tvg::RenderRegion bounds, viewport;
        bool supported = true;
    };
    struct Effect
    {
        Uniform uniform;
        tvg::RenderRegion extend;
        int direction = 0;
    };
    struct Composite;
    struct Command
    {
        Mesh *mesh = nullptr;
        Composite *composite = nullptr;
        tvg::BlendMethod blend = tvg::BlendMethod::Normal;
    };
    struct Layer
    {
        std::vector<Command> commands;
        std::vector<Effect> effects;
    };
    struct Composite : tvg::RenderCompositor
    {
        Layer first, second;
        Layer *parent = nullptr;
        tvg::RenderRegion region;
        tvg::BlendMethod blend = tvg::BlendMethod::Normal;
        float opacity = 1;
        bool masked = false, begun = false;
    };
    struct Target
    {
        ResourcePtr<QRhiTexture> texture;
        ResourcePtr<QRhiRenderBuffer> msaa;
        ResourcePtr<QRhiRenderPassDescriptor> pass;
        ResourcePtr<QRhiTextureRenderTarget> target;
    };
    struct Draw
    {
        Uniform uniform;
        QRhiTexture *texture = nullptr, *aux = nullptr, *original = nullptr;
        quint32 first = 0, count = 0;
        tvg::RenderRegion viewport;
        bool nearest = false;
    };
    struct Pass
    {
        QRhiRenderTarget *target;
        std::vector<Draw> draws;
    };
    ~Renderer() override;
    tvg::Result bind(QRhi *, QRhiCommandBuffer *, QRhiRenderTarget *);
    bool preUpdate() override
    {
        failed = false;
        return device != nullptr;
    }
    tvg::RenderData prepare(const tvg::RenderShape &, tvg::RenderData, const tvg::Matrix &,
                            const tvg::Array<tvg::RenderData> &, uint8_t, tvg::RenderUpdateFlag,
                            bool) override;
    tvg::RenderData prepare(tvg::RenderSurface *, tvg::RenderData, const tvg::Matrix &,
                            const tvg::Array<tvg::RenderData> &, uint8_t, tvg::FilterMethod,
                            tvg::RenderUpdateFlag) override;
    bool postUpdate() override
    {
        return !failed;
    }
    bool preRender() override;
    bool renderShape(tvg::RenderData) override;
    bool renderImage(tvg::RenderData data) override
    {
        return renderShape(data);
    }
    bool postRender() override;
    void dispose(tvg::RenderData) override;
    tvg::RenderRegion region(tvg::RenderData data) override
    {
        return data ? static_cast<Mesh *>(data)->bounds : tvg::RenderRegion{};
    }
    bool bounds(tvg::RenderData, tvg::Point *, const tvg::Matrix &) override;
    bool blend(tvg::BlendMethod method) override
    {
        blendMethod = method;
        return true;
    }
    tvg::ColorSpace colorSpace() override
    {
        return tvg::ColorSpace::ABGR8888;
    }
    const tvg::RenderSurface *mainSurface() override
    {
        return &surface;
    }
    bool clear() override
    {
        clearRequested = true;
        return true;
    }
    bool sync() override
    {
        commandBuffer = nullptr;
        return true;
    }
    bool intersectsShape(tvg::RenderData, const tvg::RenderRegion &) override;
    bool intersectsImage(tvg::RenderData data, const tvg::RenderRegion &r) override
    {
        return intersectsShape(data, r);
    }
    tvg::RenderCompositor *target(const tvg::RenderRegion &, tvg::ColorSpace,
                                  tvg::CompositionFlag) override;
    bool beginComposite(tvg::RenderCompositor *, tvg::MaskMethod, uint8_t) override;
    bool endComposite(tvg::RenderCompositor *) override;
    void prepare(tvg::RenderEffect *, const tvg::Matrix &) override;
    bool region(tvg::RenderEffect *) override;
    bool render(tvg::RenderCompositor *, const tvg::RenderEffect *, bool) override;
    void dispose(tvg::RenderEffect *) override;
    void damage(tvg::RenderData, const tvg::RenderRegion &) override {}
    bool partial(bool) override
    {
        return false;
    }
    uint32_t dirtyRegions() const override
    {
        return 1;
    }
    tvg::Result dirtyRegions(tvg::Region *r, uint32_t n) const override
    {
        if (!r || !n)
            return tvg::Result::InvalidArguments;
        *r = {0, 0, int32_t(surface.w), int32_t(surface.h)};
        return tvg::Result::Success;
    }

  private:
    QRhi *device = nullptr;
    QRhiCommandBuffer *commandBuffer = nullptr;
    QRhiRenderTarget *renderTarget = nullptr;
    tvg::RenderSurface surface;
    std::unordered_set<Mesh *> meshes;
    ResourcePtr<QRhiBuffer> vertices, uniforms, layoutUniform;
    ResourcePtr<QRhiTexture> white;
    ResourcePtr<QRhiSampler> linearSampler, nearestSampler;
    std::vector<ResourcePtr<QRhiShaderResourceBindings>> frameBindings;
    struct Pipeline
    {
        ResourcePtr<QRhiRenderPassDescriptor> pass;
        ResourcePtr<QRhiShaderResourceBindings> bindings;
        ResourcePtr<QRhiGraphicsPipeline> pipeline;
        int samples;
        QVector<quint32> format;
    };
    std::vector<Pipeline> pipelines;
    std::vector<std::unique_ptr<Target>> targets;
    size_t nextTarget = 0;
    Target *history = nullptr;
    std::vector<Pass> passes;
    Layer root;
    Layer *current = &root;
    std::vector<std::unique_ptr<Composite>> composites;
    tvg::BlendMethod blendMethod = tvg::BlendMethod::Normal;
    bool failed = false, clearRequested = false, clearFrame = true;
    std::vector<Vertex> frame;
    Mesh *meshFor(tvg::RenderData);
    void material(Material &, const tvg::Fill *, const tvg::Matrix &, uint8_t);
    void releaseGpuResources();
    Target *allocateTarget();
    Target *planLayer(const Layer &, unsigned depth = 0);
    Target *planEffect(Target *, const Effect &, const tvg::RenderRegion &);
    Draw fullscreen(QRhiTexture *, Uniform, QRhiTexture * = nullptr, QRhiTexture * = nullptr);
    Uniform defaults(int mode) const;
    bool uploadMaterial(Material &, QRhiResourceUpdateBatch *);
    QRhiGraphicsPipeline *pipelineFor(QRhiRenderTarget *, QRhiShaderResourceBindings *);
};

} // namespace QCatUiKit::ThorvgQrhiPrivate
