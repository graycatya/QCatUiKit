// SPDX-License-Identifier: MIT
#include "qcatthorvgqrhirenderer_p.h"
#include <cmath>
namespace QCatUiKit::ThorvgQrhiPrivate
{
using namespace tvg;

RenderCompositor *Renderer::target(const RenderRegion &region, ColorSpace, CompositionFlag)
{
    auto c = std::unique_ptr<Composite>(new Composite);
    c->region = region;
    c->blend = blendMethod;
    auto result = c.get();
    composites.push_back(std::move(c));
    return result;
}

bool Renderer::beginComposite(RenderCompositor *data, MaskMethod method, uint8_t opacity)
{
    auto c = static_cast<Composite *>(data);
    if (!c)
        return false;
    if (!c->begun)
    {
        c->parent = current;
        c->begun = true;
        c->opacity = opacity / 255.0f;
        current = &c->first;
    }
    else
    {
        if (method == MaskMethod::None || current != &c->first)
        {
            failed = true;
            return false;
        }
        c->masked = true;
        c->method = method;
        // Mask paint already includes its own opacity during prepare().
        c->opacity = 1.0f;
        current = &c->second;
    }
    return true;
}

bool Renderer::endComposite(RenderCompositor *data)
{
    auto c = static_cast<Composite *>(data);
    if (!c || !c->parent || (current != &c->first && current != &c->second))
    {
        failed = true;
        return false;
    }
    current = c->parent;
    current->commands.push_back({nullptr, c, c->blend});
    return true;
}

void Renderer::prepare(RenderEffect *effect, const Matrix &m)
{
    if (!effect->rd)
        effect->rd = new Effect;
    auto &e = *static_cast<Effect *>(effect->rd);
    e = {};
    e.uniform = defaults(0);
    const float scale =
        std::sqrt(std::max(m.e11 * m.e11 + m.e21 * m.e21, m.e12 * m.e12 + m.e22 * m.e22));
    auto rgb = [](float *dst, const uint8_t *src, unsigned count)
    {
        for (unsigned i = 0; i < count; ++i)
            dst[i] = src[i] / 255.0f;
    };
    auto blur = [&](float sigma)
    {
        e.uniform.p0[2] = sigma * scale;
        const int r = int(std::ceil(sigma * scale * 3));
        e.extend = {-r, -r, r, r};
    };
    effect->valid = true;
    switch (effect->type)
    {
    case SceneEffect::GaussianBlur:
    {
        auto p = static_cast<RenderEffectGaussianBlur *>(effect);
        e.uniform.options[0] = 10;
        blur(p->sigma);
        e.direction = p->direction;
        if (e.direction == 1)
            e.extend.yMin = e.extend.yMax = 0;
        if (e.direction == 2)
            e.extend.xMin = e.extend.xMax = 0;
        effect->valid = p->sigma > 0;
        break;
    }
    case SceneEffect::DropShadow:
    {
        auto p = static_cast<RenderEffectDropShadow *>(effect);
        e.uniform.options[0] = 11;
        blur(p->sigma);
        rgb(e.uniform.c0, p->color, 4);
        Point offset;
        p->update(m, offset);
        e.uniform.p1[0] = offset.x;
        e.uniform.p1[1] = offset.y;
        e.extend.xMin -= int(ceil(fabs(offset.x)));
        e.extend.xMax += int(ceil(fabs(offset.x)));
        e.extend.yMin -= int(ceil(fabs(offset.y)));
        e.extend.yMax += int(ceil(fabs(offset.y)));
        break;
    }
    case SceneEffect::Fill:
        e.uniform.options[0] = 12;
        rgb(e.uniform.c0, static_cast<RenderEffectFill *>(effect)->color, 4);
        break;
    case SceneEffect::Tint:
    {
        auto p = static_cast<RenderEffectTint *>(effect);
        e.uniform.options[0] = 13;
        rgb(e.uniform.c0, p->black, 3);
        rgb(e.uniform.c1, p->white, 3);
        e.uniform.c0[3] = p->intensity / 255.0f;
        break;
    }
    case SceneEffect::Tritone:
    {
        auto p = static_cast<RenderEffectTritone *>(effect);
        e.uniform.options[0] = 14;
        rgb(e.uniform.c0, p->shadow, 3);
        rgb(e.uniform.c1, p->midtone, 3);
        rgb(e.uniform.c2, p->highlight, 3);
        e.uniform.c0[3] = p->blender / 255.0f;
        break;
    }
    case SceneEffect::MotionBlur:
    {
        auto p = static_cast<RenderEffectMotionBlur *>(effect);
        e.uniform.options[0] = 15;
        Point offset;
        int samples = 1;
        p->update(m, offset, samples);
        e.uniform.p0[0] = offset.x;
        e.uniform.p0[1] = offset.y;
        e.uniform.p0[2] = float(samples);
        const int x = int(ceil(fabs(offset.x) * 0.5f)), y = int(ceil(fabs(offset.y) * 0.5f));
        e.extend = {-x, -y, x, y};
        break;
    }
    default:
        failed = true;
        effect->valid = false;
    }
    effect->extend = e.extend;
}

bool Renderer::region(RenderEffect *effect)
{
    if (!effect->rd)
        return false;
    effect->extend = static_cast<Effect *>(effect->rd)->extend;
    return true;
}

bool Renderer::render(RenderCompositor *, const RenderEffect *effect, bool)
{
    if (!effect->rd)
        return false;
    if (effect->valid)
        current->effects.push_back(*static_cast<Effect *>(effect->rd));
    return true;
}

void Renderer::dispose(RenderEffect *effect)
{
    delete static_cast<Effect *>(effect->rd);
    effect->rd = nullptr;
}

} // namespace QCatUiKit::ThorvgQrhiPrivate
