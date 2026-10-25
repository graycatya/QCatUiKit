// SPDX-License-Identifier: MIT
#ifndef QCATTHORVGQRHICANVAS_H
#define QCATTHORVGQRHICANVAS_H

#include "thorvg.h"

class QRhi;
class QRhiCommandBuffer;
class QRhiRenderTarget;

namespace QCatUiKit
{

/** Experimental native QRhi canvas. Requires the optional engines=qrhi build.
 * All calls and destruction must take place on the owning QRhi render thread.
 * The application owns QRhi, the target, frame submission and synchronization.
 * Supports gradients/images, nested masks/group composition, advanced blends,
 * and the built-in scene effects. There is no software fallback. draw(true)
 * clears to transparent; draw(false) preserves the last canvas image.
 * See the QCatUiKit vector documentation for tested coverage and rendering differences.
 */
struct TVG_API ThorvgQrhiCanvas final : tvg::Canvas
{
    static ThorvgQrhiCanvas *gen() noexcept;
    ~ThorvgQrhiCanvas() override;

    /** Bind before every draw, outside a render pass, inside an active frame.
     * Objects must belong to the same QRhi and outlive this canvas's use of them.
     * sync() completes CPU recording only, not GPU execution. A canvas cannot
     * move between QRhi devices; recreate it on device/window migration.
     */
    tvg::Result target(QRhi *rhi, QRhiCommandBuffer *cb, QRhiRenderTarget *target) noexcept;

  protected:
    ThorvgQrhiCanvas();
};

} // namespace QCatUiKit
#endif
