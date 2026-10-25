// SPDX-License-Identifier: MIT
#include "qcatthorvgqrhirenderer_p.h"
#include <QDebug>
#include <QPainterPathStroker>
#include <QThread>
#include <QtGui/private/qtriangulator_p.h>
#include <cmath>
#include <limits>

namespace QCatUiKit::ThorvgQrhiPrivate
{
using namespace tvg;

namespace
{
QTransform transformOf(const Matrix &m)
{
    return QTransform(m.e11, m.e21, m.e12, m.e22, m.e13, m.e23);
}

QPainterPath pathOf(const RenderPath &input, FillRule rule)
{
    QPainterPath path;
    path.setFillRule(rule == FillRule::EvenOdd ? Qt::OddEvenFill : Qt::WindingFill);
    uint32_t point = 0;
    const auto &pts = input.pts;
    for (auto cmd : input.cmds)
    {
        switch (cmd)
        {
        case PathCommand::MoveTo:
            path.moveTo(pts[point].x, pts[point].y);
            ++point;
            break;
        case PathCommand::LineTo:
            path.lineTo(pts[point].x, pts[point].y);
            ++point;
            break;
        case PathCommand::CubicTo:
            path.cubicTo(pts[point].x, pts[point].y, pts[point + 1].x, pts[point + 1].y,
                         pts[point + 2].x, pts[point + 2].y);
            point += 3;
            break;
        case PathCommand::Close:
            path.closeSubpath();
            break;
        }
    }
    return path;
}

void triangulate(std::vector<Renderer::Vertex> &out, const QPainterPath &path,
                 const RenderColor &color, uint8_t opacity)
{
    if (path.isEmpty() || !color.a || !opacity)
        return;
    const auto mesh = qTriangulate(path);
    const float a = (float(color.a) / 255.0f) * (float(opacity) / 255.0f);
    for (int i = 0; i < mesh.indices.size(); ++i)
    {
        const auto index = mesh.indices.type() == QVertexIndexVector::UnsignedInt
                               ? static_cast<const quint32 *>(mesh.indices.data())[i]
                               : static_cast<const quint16 *>(mesh.indices.data())[i];
        out.push_back({float(mesh.vertices[2 * index]), float(mesh.vertices[2 * index + 1]),
                       color.r / 255.0f * a, color.g / 255.0f * a, color.b / 255.0f * a, a});
    }
}
} // namespace

Renderer::~Renderer()
{
    if (device)
        device->removeCleanupCallback(this);
    releaseGpuResources();
}

void Renderer::releaseGpuResources()
{
    frameBindings.clear();
    pipelines.clear();
    targets.clear();
    history = nullptr;
    linearSampler.reset();
    nearestSampler.reset();
    white.reset();
    uniforms.reset();
    vertices.reset();
    layoutUniform.reset();
    for (auto mesh : meshes)
        for (auto &mat : mesh->materials)
        {
            mat.texture.reset();
            mat.dirty = true;
        }
    commandBuffer = nullptr;
    renderTarget = nullptr;
    device = nullptr;
}

Renderer::Mesh *Renderer::meshFor(RenderData data)
{
    if (data)
        return static_cast<Mesh *>(data);
    auto mesh = new Mesh;
    meshes.insert(mesh);
    return mesh;
}

void Renderer::dispose(RenderData data)
{
    auto mesh = static_cast<Mesh *>(data);
    meshes.erase(mesh);
    delete mesh;
}

Result Renderer::bind(QRhi *rhi, QRhiCommandBuffer *cb, QRhiRenderTarget *target)
{
    if (!rhi || !cb || !target || target->pixelSize().isEmpty())
        return Result::InvalidArguments;
    if (cb->rhi() != rhi || target->rhi() != rhi || !target->renderPassDescriptor())
        return Result::InvalidArguments;
    if (QThread::currentThread() != rhi->thread() || !rhi->isRecordingFrame())
        return Result::InsufficientCondition;
    if (device && device != rhi)
        return Result::InvalidArguments;
    if (!device)
    {
        device = rhi;
        device->addCleanupCallback(this, [this](QRhi *) { releaseGpuResources(); });
    }
    commandBuffer = cb;
    renderTarget = target;
    if (surface.w != uint32_t(target->pixelSize().width()) ||
        surface.h != uint32_t(target->pixelSize().height()))
        history = nullptr;
    surface.w = uint32_t(target->pixelSize().width());
    surface.h = uint32_t(target->pixelSize().height());
    surface.cs = ColorSpace::ABGR8888;
    surface.channelSize = 4;
    surface.premultiplied = true;
    failed = false;
    clearRequested = false;
    return Result::Success;
}

RenderData Renderer::prepare(const RenderShape &shape, RenderData data, const Matrix &matrix,
                             const Array<RenderData> &clips, uint8_t opacity, RenderUpdateFlag,
                             bool clipper)
{
    auto mesh = meshFor(data);
    mesh->vertices.clear();
    mesh->parts.clear();
    mesh->viewport = vport;
    mesh->supported = true;
    const auto transform = transformOf(matrix);
    RenderPath trimmed;
    const auto *input = &shape.path;
    if (shape.trimpath())
    {
        if (shape.stroke->trim.trim(shape.path, trimmed))
            input = &trimmed;
        else
        {
            mesh->path = {};
            mesh->hitPath = {};
            mesh->bounds = {};
            return mesh;
        }
    }
    auto localPath = pathOf(*input, shape.rule);
    mesh->path = transform.map(localPath);
    auto clipPath = [&](QPainterPath path)
    {
        for (auto clipData : clips)
        {
            const auto clip = static_cast<Mesh *>(clipData);
            if (!clip || !clip->supported)
            {
                mesh->supported = false;
                return QPainterPath{};
            }
            path = path.intersected(clip->path);
        }
        return path;
    };
    mesh->path = clipPath(mesh->path);
    mesh->hitPath = mesh->path;
    if (mesh->supported && !clipper)
    {
        auto fill = [&]
        {
            material(mesh->materials[0], shape.fill, matrix, opacity);
            const auto first = quint32(mesh->vertices.size());
            triangulate(mesh->vertices, mesh->path,
                        shape.fill ? RenderColor{255, 255, 255, 255} : shape.color,
                        shape.fill ? 255 : opacity);
            mesh->parts.push_back({first, quint32(mesh->vertices.size()) - first, 0});
        };
        auto stroke = [&]
        {
            if (shape.strokeWidth() <= 0.0f)
                return;
            QPainterPathStroker stroker;
            stroker.setWidth(shape.strokeWidth());
            stroker.setCapStyle(shape.strokeCap() == StrokeCap::Round  ? Qt::RoundCap
                                : shape.strokeCap() == StrokeCap::Butt ? Qt::FlatCap
                                                                       : Qt::SquareCap);
            stroker.setJoinStyle(shape.strokeJoin() == StrokeJoin::Round   ? Qt::RoundJoin
                                 : shape.strokeJoin() == StrokeJoin::Bevel ? Qt::BevelJoin
                                                                           : Qt::MiterJoin);
            stroker.setMiterLimit(shape.strokeMiterlimit());
            const float *pattern = nullptr;
            float offset = 0.0f;
            const auto count = shape.strokeDash(&pattern, &offset);
            if (count)
            {
                QList<qreal> dashes;
                for (uint32_t i = 0; i < count; ++i)
                    dashes.append(pattern[i] / shape.strokeWidth());
                stroker.setDashPattern(dashes);
                stroker.setDashOffset(offset / shape.strokeWidth());
            }
            auto path = clipPath(transform.map(stroker.createStroke(localPath)));
            mesh->hitPath = mesh->hitPath.united(path);
            material(mesh->materials[1], shape.strokeFill(), matrix, opacity);
            const auto first = quint32(mesh->vertices.size());
            triangulate(mesh->vertices, path,
                        shape.strokeFill() ? RenderColor{255, 255, 255, 255} : shape.stroke->color,
                        shape.strokeFill() ? 255 : opacity);
            mesh->parts.push_back({first, quint32(mesh->vertices.size()) - first, 1});
        };
        if (shape.strokeFirst())
        {
            stroke();
            fill();
        }
        else
        {
            fill();
            stroke();
        }
    }
    auto rect = mesh->path.boundingRect();
    for (const auto &vertex : mesh->vertices)
        rect |= QRectF(vertex.x, vertex.y, 0.001, 0.001);
    mesh->bounds = {int32_t(std::floor(rect.left())), int32_t(std::floor(rect.top())),
                    int32_t(std::ceil(rect.right())), int32_t(std::ceil(rect.bottom()))};
    failed |= !mesh->supported;
    return mesh;
}

RenderData Renderer::prepare(RenderSurface *image, RenderData data, const Matrix &matrix,
                             const Array<RenderData> &clips, uint8_t opacity, FilterMethod filter,
                             RenderUpdateFlag)
{
    auto mesh = meshFor(data);
    mesh->vertices.clear();
    mesh->parts.clear();
    mesh->viewport = vport;
    mesh->supported = image && image->w && image->h && image->data && image->stride >= image->w;
    if (!mesh->supported)
    {
        failed = true;
        return mesh;
    }
    QPainterPath path;
    path.addRect(0, 0, image->w, image->h);
    mesh->path = transformOf(matrix).map(path);
    for (auto clip : clips)
    {
        if (!clip)
        {
            failed = true;
            return mesh;
        }
        mesh->path = mesh->path.intersected(static_cast<Mesh *>(clip)->path);
    }
    mesh->hitPath = mesh->path;
    auto &mat = mesh->materials[0];
    mat.uniform = defaults(4);
    mat.uniform.options[1] = opacity / 255.0f;
    mat.nearest = filter == FilterMethod::Nearest;
    mat.uniform.p0[2] = mat.nearest ? 1.0f : 0.0f;
    const auto inv = transformOf(matrix).inverted();
    QMatrix4x4 inverse(inv);
    memcpy(mat.uniform.inverse, inverse.constData(), 64);
    mat.uniform.p0[0] = float(image->w);
    mat.uniform.p0[1] = float(image->h);
    QImage::Format format;
    switch (image->cs)
    {
    case ColorSpace::ABGR8888:
        format = QImage::Format_RGBA8888_Premultiplied;
        break;
    case ColorSpace::ABGR8888S:
        format = QImage::Format_RGBA8888;
        break;
    case ColorSpace::ARGB8888:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    case ColorSpace::ARGB8888S:
        format = QImage::Format_ARGB32;
        break;
    case ColorSpace::XBGR8888:
        format = QImage::Format_RGBX8888;
        break;
    case ColorSpace::XRGB8888:
        format = QImage::Format_RGB32;
        break;
    default:
        failed = true;
        mesh->supported = false;
        return mesh;
    }
    QImage view(reinterpret_cast<const uchar *>(image->data), image->w, image->h, image->stride * 4,
                format);
    auto pixels = view.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    if (mat.image != pixels)
    {
        mat.image = pixels.copy();
        mat.dirty = true;
    }
    triangulate(mesh->vertices, mesh->path, RenderColor{255, 255, 255, 255}, 255);
    mesh->parts.push_back({0, quint32(mesh->vertices.size()), 0});
    const auto rect = mesh->path.boundingRect();
    mesh->bounds = {int32_t(floor(rect.left())), int32_t(floor(rect.top())),
                    int32_t(ceil(rect.right())), int32_t(ceil(rect.bottom()))};
    return mesh;
}

bool Renderer::preRender()
{
    frame.clear();
    passes.clear();
    composites.clear();
    root = {};
    current = &root;
    nextTarget = 0;
    blendMethod = BlendMethod::Normal;
    clearFrame = clearRequested;
    const bool valid = !failed && device && commandBuffer && renderTarget;
    clearRequested = false;
    return valid;
}

bool Renderer::renderShape(RenderData data)
{
    auto mesh = static_cast<Mesh *>(data);
    if (failed || !mesh || !mesh->supported)
        return false;
    current->commands.push_back({mesh, nullptr, blendMethod});
    return true;
}

bool Renderer::bounds(RenderData data, Point *points, const Matrix &matrix)
{
    if (!data || !points)
        return false;
    const auto rect = static_cast<Mesh *>(data)->hitPath.boundingRect();
    const QPointF corners[] = {rect.topLeft(), rect.topRight(), rect.bottomRight(),
                               rect.bottomLeft()};
    for (int i = 0; i < 4; ++i)
    {
        points[i] = {float(corners[i].x()), float(corners[i].y())};
        points[i] *= matrix;
    }
    return true;
}

bool Renderer::intersectsShape(RenderData data, const RenderRegion &region)
{
    if (!data)
        return false;
    const auto mesh = static_cast<Mesh *>(data);
    auto clipped = RenderRegion::intersect(region, mesh->viewport);
    return clipped.valid() &&
           mesh->hitPath.intersects(QRectF(clipped.sx(), clipped.sy(), clipped.sw(), clipped.sh()));
}

} // namespace QCatUiKit::ThorvgQrhiPrivate
