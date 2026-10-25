// SPDX-License-Identifier: MIT
#include "qcatthorvgqrhirenderer_p.h"
#include <algorithm>
#include <cmath>
#include <cstring>
namespace QCatUiKit::ThorvgQrhiPrivate
{
using namespace tvg;

Renderer::Uniform Renderer::defaults(int mode) const
{
    Uniform u;
    QMatrix4x4 id;
    memcpy(u.inverse, id.constData(), 64);
    u.options[0] = float(mode);
    u.options[1] = 1;
    u.options[3] = device && device->isYUpInFramebuffer() ? 1 : 0;
    u.p2[0] = float(surface.w);
    u.p2[1] = float(surface.h);
    u.region[2] = float(surface.w);
    u.region[3] = float(surface.h);
    return u;
}

void Renderer::material(Material &mat, const Fill *fill, const Matrix &matrix, uint8_t opacity)
{
    mat.uniform = defaults(0);
    mat.nearest = false;
    if (!fill)
        return;
    auto &u = mat.uniform;
    u.options[1] = opacity / 255.0f;
    u.options[2] = float(fill->spread());
    const QTransform model(matrix.e11, matrix.e21, matrix.e12, matrix.e22, matrix.e13, matrix.e23);
    const auto &m = fill->transform();
    const QTransform gradient(m.e11, m.e21, m.e12, m.e22, m.e13, m.e23);
    QMatrix4x4 inverse((gradient * model).inverted());
    memcpy(u.inverse, inverse.constData(), 64);
    switch (fill->type())
    {
    case Type::LinearGradient:
        u.options[0] = 1;
        static_cast<const LinearGradient *>(fill)->linear(&u.p0[0], &u.p0[1], &u.p0[2], &u.p0[3]);
        break;
    case Type::RadialGradient:
        u.options[0] = 2;
        static_cast<const RadialGradient *>(fill)->radial(&u.p0[0], &u.p0[1], &u.p0[2], &u.p1[0],
                                                          &u.p1[1], &u.p1[2]);
        break;
    case Type::ConicGradient:
        u.options[0] = 3;
        static_cast<const ConicGradient *>(fill)->conic(&u.p0[0], &u.p0[1], &u.p0[2]);
        u.p0[2] *= float(3.141592653589793 / 180.0);
        break;
    default:
        failed = true;
        return;
    }
    const Fill::ColorStop *stops = nullptr;
    const auto count = fill->colorStops(&stops);
    QImage lut(1024, 1, QImage::Format_RGBA8888_Premultiplied);
    lut.fill(Qt::transparent);
    if (count)
    {
        unsigned next = 0;
        for (int x = 0; x < lut.width(); ++x)
        {
            const float t = float(x) / (lut.width() - 1);
            while (next + 1 < count && stops[next + 1].offset <= t)
                ++next;
            const auto &a = stops[next];
            const auto &b = stops[std::min(next + 1, count - 1)];
            const auto v =
                b.offset > a.offset
                    ? std::max(0.0f, std::min(1.0f, (t - a.offset) / (b.offset - a.offset)))
                    : 0.0f;
            const float alpha = a.a + (b.a - a.a) * v;
            auto channel = [&](unsigned c, unsigned d)
            { return int(std::round((float(c) + (float(d) - float(c)) * v) * alpha / 255.0f)); };
            auto dst = lut.scanLine(0) + x * 4;
            dst[0] = channel(a.r, b.r);
            dst[1] = channel(a.g, b.g);
            dst[2] = channel(a.b, b.b);
            dst[3] = int(std::round(alpha));
        }
    }
    if (mat.image != lut)
    {
        mat.image = lut;
        mat.dirty = true;
    }
}

bool Renderer::uploadMaterial(Material &mat, QRhiResourceUpdateBatch *updates)
{
    if (mat.image.isNull())
        return true;
    if (!mat.texture || mat.dirty || mat.texture->pixelSize() != mat.image.size())
    {
        mat.texture.reset(device->newTexture(QRhiTexture::RGBA8, mat.image.size()));
        if (!mat.texture->create())
        {
            mat.texture.reset();
            return false;
        }
        mat.dirty = true;
    }
    if (mat.dirty)
    {
        updates->uploadTexture(mat.texture.get(), mat.image);
        mat.dirty = false;
    }
    return true;
}

} // namespace QCatUiKit::ThorvgQrhiPrivate
