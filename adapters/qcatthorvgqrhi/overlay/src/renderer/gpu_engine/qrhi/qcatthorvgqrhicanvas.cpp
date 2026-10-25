// SPDX-License-Identifier: MIT
#include "qcatthorvgqrhicanvas.h"
#include "qcatthorvgqrhirenderer_p.h"
#include "tvgCanvas.h"

namespace QCatUiKit
{
using namespace tvg;

ThorvgQrhiCanvas::ThorvgQrhiCanvas() = default;
ThorvgQrhiCanvas::~ThorvgQrhiCanvas() = default;

ThorvgQrhiCanvas *ThorvgQrhiCanvas::gen() noexcept
{
    if (engineInit == 0)
        return nullptr;
    auto canvas = new ThorvgQrhiCanvas;
    auto renderer = new ThorvgQrhiPrivate::Renderer;
    renderer->ref();
    canvas->pImpl->renderer = renderer;
    return canvas;
}

Result ThorvgQrhiCanvas::target(QRhi *rhi, QRhiCommandBuffer *cb, QRhiRenderTarget *target) noexcept
{
    if (pImpl->status == Status::Drawing || pImpl->status == Status::Updating)
        return Result::InsufficientCondition;
    auto renderer = static_cast<ThorvgQrhiPrivate::Renderer *>(pImpl->renderer);
    auto result = renderer->bind(rhi, cb, target);
    if (result != Result::Success)
        return result;
    const auto surface = renderer->mainSurface();
    RenderRegion vp{0, 0, int32_t(surface->w), int32_t(surface->h)};
    renderer->viewport(vp);
    pImpl->vport = vp;
    pImpl->status = Status::Damaged;
    return Result::Success;
}

} // namespace QCatUiKit
