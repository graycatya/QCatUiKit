// SPDX-License-Identifier: Apache-2.0
#include "qcatledpreview.h"
#include "qcatledsource_p.h"
#include <QElapsedTimer>
#include <QFile>
#include <QMatrix4x4>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QQuickWindow>
#include <QSGNode>
#include <QTimer>
#include <atomic>
#include <cmath>
#include <cstring>
#include <rhi/qrhi.h>

namespace
{
struct RenderStatistics
{
    std::atomic<quint64> frames{0}, frameId{0};
    QMutex mutex;
    QString info, error;
};
QShader loadShader(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return QShader::fromSerialized(file.readAll());
}
} // namespace

struct QCatLedPreview::Impl
{
    QPointer<QCatLedSource> source;
    std::shared_ptr<const QCatLedLayout> lastLayout;
    QMetaObject::Connection changedConnection, destroyedConnection;
    ViewMode mode = Spatial3D;
    qreal size = 6, brightness = 1;
    QColor background{QStringLiteral("#090d15")};
    bool showStats = true;
    float yaw = 0, pitch = 0, zoom = 1;
    QPointF pan;
    std::shared_ptr<RenderStatistics> stats = std::make_shared<RenderStatistics>();
    QElapsedTimer clock;
    quint64 previousFrames = 0, previousColors = 0;
    qreal fps = 0, hz = 0;
};

class QCatLedPreviewRenderer : public QQuickRhiItemRenderer
{
  public:
    explicit QCatLedPreviewRenderer(std::shared_ptr<RenderStatistics> stats)
        : m_stats(std::move(stats))
    {
    }
    ~QCatLedPreviewRenderer() override
    {
        release();
    }

  protected:
    void synchronize(QQuickRhiItem *item) override
    {
        auto *preview = static_cast<QCatLedPreview *>(item);
        auto &d = *preview->d;
        const auto latest = d.source ? d.source->snapshot() : nullptr;
        // Fit on layout/source changes, even when the new layout has the same count.
        const auto layout = latest ? latest->layout : nullptr;
        if (d.lastLayout != layout)
        {
            d.yaw = d.pitch = 0;
            d.zoom = 1;
            d.pan = {};
        }
        d.lastLayout = layout;
        m_frame = latest;
        m_mode = d.mode;
        m_yaw = d.yaw;
        m_pitch = d.pitch;
        m_zoom = d.zoom;
        m_pan = d.pan;
        m_size = float(d.size);
        m_brightness = float(d.brightness);
        m_background = d.background;
        m_dpr = preview->window() ? float(preview->window()->effectiveDevicePixelRatio()) : 1;
        m_logicalSize = QSizeF(preview->width(), preview->height());
    }
    void initialize(QRhiCommandBuffer *) override
    {
        m_ready = false;
        if (m_rhi != rhi())
        {
            release();
            m_rhi = rhi();
        }
        if (!m_rhi->isFeatureSupported(QRhi::Instancing))
        {
            fail(QStringLiteral("The graphics backend does not support GPU instancing."));
            return;
        }
        if (!m_quad)
        {
            m_quad.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                          8 * sizeof(float)));
            m_uniform.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 96));
            if (!m_quad->create() || !m_uniform->create())
            {
                release();
                fail(QStringLiteral("Unable to create LED vertex/uniform buffers."));
                return;
            }
            m_bindings.reset(m_rhi->newShaderResourceBindings());
            m_bindings->setBindings({QRhiShaderResourceBinding::uniformBuffer(
                0, QRhiShaderResourceBinding::VertexStage, m_uniform.get())});
            if (!m_bindings->create())
            {
                release();
                fail(QStringLiteral("Unable to create LED shader bindings."));
                return;
            }
        }
        // Keep buffers across resizes. Rebuild pipeline only if render pass/sample count changes.
        const auto *rp = renderTarget()->renderPassDescriptor();
        if (!m_pipeline || m_samples != renderTarget()->sampleCount() || !m_pass->isCompatible(rp))
        {
            m_pipeline.reset();
            m_pass.reset(rp->newCompatibleRenderPassDescriptor());
            m_samples = renderTarget()->sampleCount();
            const QShader vertex =
                loadShader(QStringLiteral(":/QCatUiKit/shaders/ledpreview.vert.qsb"));
            const QShader fragment =
                loadShader(QStringLiteral(":/QCatUiKit/shaders/ledpreview.frag.qsb"));
            if (!vertex.isValid() || !fragment.isValid())
            {
                fail(QStringLiteral("Missing or invalid LED .qsb shaders."));
                return;
            }
            m_pipeline.reset(m_rhi->newGraphicsPipeline());
            m_pipeline->setShaderStages(
                {{QRhiShaderStage::Vertex, vertex}, {QRhiShaderStage::Fragment, fragment}});
            QRhiVertexInputLayout input;
            input.setBindings(
                {{2 * sizeof(float)},
                 {sizeof(QCatLedSource::Position), QRhiVertexInputBinding::PerInstance},
                 {4, QRhiVertexInputBinding::PerInstance}});
            input.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                                 {1, 1, QRhiVertexInputAttribute::Float3, 0},
                                 {2, 2, QRhiVertexInputAttribute::UNormByte4, 0}});
            m_pipeline->setVertexInputLayout(input);
            m_pipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);
            m_pipeline->setDepthTest(true);
            m_pipeline->setDepthWrite(true);
            m_pipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
            QRhiGraphicsPipeline::TargetBlend blend;
            blend.enable = true;
            blend.srcColor = QRhiGraphicsPipeline::One;
            blend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
            m_pipeline->setTargetBlends({blend});
            m_pipeline->setSampleCount(m_samples);
            m_pipeline->setShaderResourceBindings(m_bindings.get());
            m_pipeline->setRenderPassDescriptor(m_pass.get());
            if (!m_pipeline->create())
            {
                m_pipeline.reset();
                fail(QStringLiteral("Unable to create LED graphics pipeline."));
                return;
            }
        }
        m_ready = true;
        QMutexLocker lock(&m_stats->mutex);
        m_stats->error.clear();
    }
    void render(QRhiCommandBuffer *cb) override
    {
        const QSize pixels = renderTarget()->pixelSize();
        const int count = m_frame ? int(m_frame->layout->positions.size()) : 0;
        QRhiResourceUpdateBatch *updates = nullptr;
        bool draw = m_ready && m_pipeline && count > 0;
        if (!count)
        {
            m_uploadedLayout.reset();
            m_uploadedFrame.reset();
        }
        if (draw && count > m_capacity)
        {
            m_positions.reset();
            m_colors.reset();
            m_capacity = qMax(count, 1024);
            m_positions.reset(
                m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, m_capacity * 12));
            m_colors.reset(
                m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, m_capacity * 4));
            if (!m_positions->create() || !m_colors->create())
            {
                fail(QStringLiteral("Unable to allocate LED instance buffers."));
                draw = false;
                m_capacity = 0;
            }
            m_uploadedLayout.reset();
            m_uploadedFrame.reset();
        }
        if (draw)
        {
            updates = m_rhi->nextResourceUpdateBatch();
            if (!m_quadUploaded)
            {
                const float quad[] = {-1, -1, 1, -1, -1, 1, 1, 1};
                updates->uploadStaticBuffer(m_quad.get(), quad);
                m_quadUploaded = true;
            }
            if (m_uploadedLayout != m_frame->layout)
            {
                updates->uploadStaticBuffer(m_positions.get(), 0, count * 12,
                                            m_frame->layout->positions.constData());
                m_uploadedLayout = m_frame->layout;
            }
            if (m_uploadedFrame != m_frame)
            {
                updates->updateDynamicBuffer(m_colors.get(), 0, count * 4,
                                             m_frame->rgba.constData());
                m_uploadedFrame = m_frame;
            }
            const float logicalWidth = float(qMax(1.0, m_logicalSize.width()));
            const float logicalHeight = float(qMax(1.0, m_logicalSize.height()));
            const float aspect = logicalWidth / logicalHeight;
            const float radius = m_frame->layout->radius;
            // Also support fixed backing textures: scale logical pixels to backing pixels.
            const float diameter = m_size * pixels.width() / logicalWidth;
            const float diameterY = m_size * pixels.height() / logicalHeight;
            const float margin =
                qBound(0.1f, 1.0f - m_size / qMin(logicalWidth, logicalHeight), 0.95f);
            const float extent = radius / (qMin(1.0f, aspect) * margin);
            const float distance = extent / std::sin(float(M_PI) / 8) / m_zoom;
            QMatrix4x4 projection;
            if (m_mode == QCatLedPreview::Plane2D)
                projection.ortho(-extent * aspect / m_zoom, extent * aspect / m_zoom,
                                 -extent / m_zoom, extent / m_zoom, radius * 0.001f,
                                 distance + radius * 4);
            else
                projection.perspective(45, aspect, radius * 0.001f, distance + radius * 4);
            QMatrix4x4 view;
            view.translate(float(m_pan.x()) * extent / m_zoom, float(m_pan.y()) * extent / m_zoom,
                           -distance);
            if (m_mode == QCatLedPreview::Spatial3D)
            {
                view.rotate(m_pitch, 1, 0, 0);
                view.rotate(m_yaw, 0, 1, 0);
            }
            const QMatrix4x4 mvp = m_rhi->clipSpaceCorrMatrix() * projection * view;
            float uniform[24];
            std::memcpy(uniform, mvp.constData(), 64);
            uniform[16] = float(pixels.width());
            uniform[17] = float(pixels.height());
            uniform[18] = diameter;
            uniform[19] = m_brightness;
            uniform[20] = m_frame->layout->center.x();
            uniform[21] = m_frame->layout->center.y();
            uniform[22] = m_frame->layout->center.z();
            uniform[23] = diameterY;
            updates->updateDynamicBuffer(m_uniform.get(), 0, sizeof(uniform), uniform);
        }
        const float alpha = m_background.alphaF();
        const QColor clear =
            QColor::fromRgbF(m_background.redF() * alpha, m_background.greenF() * alpha,
                             m_background.blueF() * alpha, alpha);
        cb->beginPass(renderTarget(), clear, {1.0f, 0}, updates);
        if (draw)
        {
            cb->setGraphicsPipeline(m_pipeline.get());
            cb->setViewport({0, 0, float(pixels.width()), float(pixels.height())});
            cb->setShaderResources(m_bindings.get());
            const QRhiCommandBuffer::VertexInput inputs[] = {
                {m_quad.get(), 0}, {m_positions.get(), 0}, {m_colors.get(), 0}};
            cb->setVertexInput(0, 3, inputs);
            cb->draw(4, count);
        }
        cb->endPass();
        m_stats->frameId.store(draw ? m_frame->frameId : 0);
        m_stats->frames.fetch_add(1);
        // Metadata only changes on target changes, not on each animation frame.
        if (pixels != m_reportedSize)
        {
            m_reportedSize = pixels;
            QMutexLocker lock(&m_stats->mutex);
            m_stats->info = QStringLiteral("%1 · %2 · %3 × %4 px · DPR %5")
                                .arg(QString::fromLatin1(m_rhi->backendName()))
                                .arg(QString::fromUtf8(m_rhi->driverInfo().deviceName))
                                .arg(pixels.width())
                                .arg(pixels.height())
                                .arg(m_dpr, 0, 'f', 2);
        }
    }

  private:
    void fail(const QString &error)
    {
        m_ready = false;
        QMutexLocker lock(&m_stats->mutex);
        m_stats->error = error;
    }
    void release()
    {
        m_pipeline.reset();
        m_pass.reset();
        m_bindings.reset();
        m_uniform.reset();
        m_quad.reset();
        m_positions.reset();
        m_colors.reset();
        m_capacity = 0;
        m_quadUploaded = m_ready = false;
        m_uploadedLayout.reset();
        m_uploadedFrame.reset();
        m_reportedSize = {};
    }
    QRhi *m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer> m_quad, m_uniform, m_positions, m_colors;
    std::unique_ptr<QRhiShaderResourceBindings> m_bindings;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;
    std::unique_ptr<QRhiRenderPassDescriptor> m_pass;
    std::shared_ptr<RenderStatistics> m_stats;
    std::shared_ptr<const QCatLedSnapshot> m_frame, m_uploadedFrame;
    std::shared_ptr<const QCatLedLayout> m_uploadedLayout;
    QCatLedPreview::ViewMode m_mode = QCatLedPreview::Spatial3D;
    float m_yaw = 0, m_pitch = 0, m_zoom = 1, m_size = 6, m_brightness = 1, m_dpr = 1;
    QPointF m_pan;
    QSizeF m_logicalSize;
    QColor m_background;
    QSize m_reportedSize;
    int m_capacity = 0, m_samples = 1;
    bool m_quadUploaded = false, m_ready = false;
};

QCatLedPreview::QCatLedPreview(QQuickItem *parent) : QQuickRhiItem(parent), d(new Impl)
{
    setAlphaBlending(true);
    auto *timer = new QTimer(this);
    timer->setInterval(500);
    d->clock.start();
    connect(timer, &QTimer::timeout, this,
            [this]
            {
                const qreal seconds = d->clock.restart() / 1000.0;
                const auto frames = renderedFrames();
                const auto colors = d->source ? d->source->submittedFrames() : 0;
                d->fps = (frames - d->previousFrames) / qMax(0.001, seconds);
                d->hz = colors >= d->previousColors
                            ? (colors - d->previousColors) / qMax(0.001, seconds)
                            : 0;
                d->previousFrames = frames;
                d->previousColors = colors;
                emit statisticsChanged();
            });
    timer->start();
}
QCatLedPreview::~QCatLedPreview() = default;
QSGNode *QCatLedPreview::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
{
    if (!window() || !window()->rhi())
    {
        QMutexLocker lock(&d->stats->mutex);
        d->stats->error = QStringLiteral("LED preview requires a hardware Qt Quick backend.");
        delete node;
        return nullptr;
    }
    return QQuickRhiItem::updatePaintNode(node, data);
}
QQuickRhiItemRenderer *QCatLedPreview::createRenderer()
{
    return new QCatLedPreviewRenderer(d->stats);
}
QCatLedSource *QCatLedPreview::source() const
{
    return d->source;
}
void QCatLedPreview::setSource(QCatLedSource *source)
{
    if (d->source == source)
        return;
    disconnect(d->changedConnection);
    disconnect(d->destroyedConnection);
    d->source = source;
    d->previousColors = source ? source->submittedFrames() : 0;
    if (source)
    {
        d->changedConnection =
            connect(source, &QCatLedSource::dataChanged, this, [this] { update(); });
        d->destroyedConnection = connect(source, &QObject::destroyed, this,
                                         [this]
                                         {
                                             emit sourceChanged();
                                             update();
                                         });
    }
    resetView();
    emit sourceChanged();
}
QCatLedPreview::ViewMode QCatLedPreview::viewMode() const
{
    return d->mode;
}
void QCatLedPreview::setViewMode(ViewMode mode)
{
    if (d->mode == mode || (mode != Plane2D && mode != Spatial3D))
        return;
    d->mode = mode;
    resetView();
    emit viewModeChanged();
}
qreal QCatLedPreview::pointSize() const
{
    return d->size;
}
void QCatLedPreview::setPointSize(qreal size)
{
    if (!std::isfinite(size))
        return;
    size = qBound(1.0, size, 128.0);
    if (d->size == size)
        return;
    d->size = size;
    emit pointSizeChanged();
    update();
}
qreal QCatLedPreview::previewBrightness() const
{
    return d->brightness;
}
void QCatLedPreview::setPreviewBrightness(qreal brightness)
{
    if (!std::isfinite(brightness))
        return;
    brightness = qBound(0.0, brightness, 1.0);
    if (d->brightness == brightness)
        return;
    d->brightness = brightness;
    emit previewBrightnessChanged();
    update();
}
QColor QCatLedPreview::backgroundColor() const
{
    return d->background;
}
void QCatLedPreview::setBackgroundColor(const QColor &color)
{
    if (!color.isValid() || d->background == color)
        return;
    d->background = color;
    emit backgroundColorChanged();
    update();
}
bool QCatLedPreview::showStatistics() const
{
    return d->showStats;
}
void QCatLedPreview::setShowStatistics(bool show)
{
    if (d->showStats == show)
        return;
    d->showStats = show;
    emit showStatisticsChanged();
}
quint64 QCatLedPreview::renderedFrames() const
{
    return d->stats->frames.load();
}
quint64 QCatLedPreview::renderedFrameId() const
{
    return d->stats->frameId.load();
}
qreal QCatLedPreview::renderFps() const
{
    return d->fps;
}
qreal QCatLedPreview::colorUpdateHz() const
{
    return d->hz;
}
QString QCatLedPreview::renderingInfo() const
{
    QMutexLocker lock(&d->stats->mutex);
    return d->stats->info;
}
QString QCatLedPreview::renderError() const
{
    QMutexLocker lock(&d->stats->mutex);
    return d->stats->error;
}
void QCatLedPreview::resetView()
{
    d->yaw = d->pitch = 0;
    d->zoom = 1;
    d->pan = {};
    update();
}
void QCatLedPreview::orbit(qreal dx, qreal dy)
{
    if (d->mode != Spatial3D || !std::isfinite(dx) || !std::isfinite(dy))
        return;
    d->yaw = float(std::remainder(d->yaw + dx * 0.4, 360.0));
    d->pitch = qBound(-89.0f, d->pitch + float(dy * 0.4), 89.0f);
    update();
}
void QCatLedPreview::pan(qreal dx, qreal dy)
{
    if (!std::isfinite(dx) || !std::isfinite(dy))
        return;
    d->pan += QPointF(2 * dx / qMax(1.0, height()), -2 * dy / qMax(1.0, height()));
    d->pan.setX(qBound(-100.0, d->pan.x(), 100.0));
    d->pan.setY(qBound(-100.0, d->pan.y(), 100.0));
    update();
}
void QCatLedPreview::zoom(qreal steps)
{
    if (!std::isfinite(steps))
        return;
    d->zoom = qBound(0.2f, d->zoom * float(std::pow(1.15, qBound(-20.0, steps, 20.0))), 100.0f);
    update();
}
