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
    QString backgroundImage;
    bool grid = false, axes = false;
    QColor background{QStringLiteral("#090d15")};
    bool showStats = true;
    bool preserveNextLayoutCamera = false;
    float yaw = 0, pitch = 0, zoom = 1;
    QPointF pan;
    PointShape shape = Circle;
    SizeMode sizeMode = ScreenPixels;
    qreal sceneSize = .8, gapX = 0, gapY = 0;
    QVector3D spacing{1, 1, 1};
    int sliceAxis = 0, maximumFps = 60;
    bool slicesVertical = false, enabled = true;
    QTimer throttle;
    QElapsedTimer updateClock;
    qint64 nextUpdateNs = 0;

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
        if (d.lastLayout != layout && !d.preserveNextLayoutCamera)
        {
            d.yaw = d.pitch = 0;
            d.zoom = 1;
            d.pan = {};
        }
        d.lastLayout = layout;
        d.preserveNextLayoutCamera = false;
        m_frame = latest;
        m_mode = d.mode;
        const QVariantMap display{{"mode", int(d.mode)},
                                  {"axis", d.sliceAxis},
                                  {"vertical", d.slicesVertical},
                                  {"spacing", QVariant::fromValue(d.spacing)}};
        if (layout && (layout != m_displaySource || display != m_displayConfig))
        {
            m_displaySource = layout;
            m_displayConfig = display;
            m_displayPositions = layout->positions;
            const auto c = layout->metadata;
            const int nx = c.value("countX").toInt(), ny = c.value("countY").toInt(),
                      nz = c.value("countZ").toInt();
            const bool grid = c.value("type").toString().startsWith("Matrix") && nx > 0 && ny > 0 &&
                              nz > 0 && qint64(nx) * ny <= m_displayPositions.size() &&
                              qint64(nx) * ny * nz == m_displayPositions.size();
            QVector3D low, high;
            bool first = true;
            for (qsizetype i = 0; i < m_displayPositions.size(); ++i)
            {
                auto &p = m_displayPositions[i];
                if (grid && d.mode == QCatLedPreview::Plane2D && d.sliceAxis > 0)
                {
                    const int x = int(i % nx), y = int((i / nx) % ny),
                              z = int(i / (qsizetype(nx) * ny));
                    const int a = d.sliceAxis;
                    const int u = a == 1 ? y : x, v = a == 3 ? y : z,
                              layer = a == 1 ? x : (a == 2 ? y : z);
                    const int nu = a == 1 ? ny : nx, nv = a == 3 ? ny : nz;
                    const float su = c.value(a == 1 ? "spacingY" : "spacingX", 1).toFloat();
                    const float sv = c.value(a == 3 ? "spacingY" : "spacingZ", 1).toFloat();
                    p = {float((u + (d.slicesVertical ? 0 : layer * (nu + 1))) * su),
                         float(-(v + (d.slicesVertical ? layer * (nv + 1) : 0)) * sv), 0};
                }
                QVector3D v(p.x, p.y, p.z);
                if (first)
                    low = high = v;
                first = false;
                for (int j = 0; j < 3; ++j)
                {
                    low[j] = qMin(low[j], v[j]);
                    high[j] = qMax(high[j], v[j]);
                }
            }
            m_displayCenter = (low + high) * .5f;
            m_displayRadius = qMax(1e-4f, (high - low).length() * .5f);
            if (high == low)
                m_displayRadius = 1;
            // Fit the unscaled layout. Display spacing must change center/diameter ratio.
            for (auto &p : m_displayPositions)
            {
                p.x = (p.x - m_displayCenter.x()) * d.spacing.x();
                p.y = (p.y - m_displayCenter.y()) * d.spacing.y();
                p.z = (p.z - m_displayCenter.z()) * d.spacing.z();
            }
            m_positionsDirty = true;
        }
        m_shape = d.shape;
        m_sizeMode = d.sizeMode;
        m_sceneSize = float(d.sceneSize);
        const bool grid = layout && layout->metadata.value("type").toString().startsWith("Matrix");
        m_gapX = grid && d.mode == QCatLedPreview::Plane2D ? float(d.gapX) : 0;
        m_gapY = grid && d.mode == QCatLedPreview::Plane2D ? float(d.gapY) : 0;
        m_yaw = d.yaw;
        m_pitch = d.pitch;
        m_zoom = d.zoom;
        m_pan = d.pan;
        m_size = float(d.size);
        m_brightness = float(d.brightness);
        m_background = d.backgroundImage.isEmpty() ? d.background : QColor(Qt::transparent);
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
                                          int(mesh().size() * sizeof(float))));
            m_uniform.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 176));
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
                {{6 * sizeof(float)},
                 {sizeof(QCatLedSource::Position), QRhiVertexInputBinding::PerInstance},
                 {4, QRhiVertexInputBinding::PerInstance}});
            input.setAttributes({{0, 0, QRhiVertexInputAttribute::Float3, 0},
                                 {0, 3, QRhiVertexInputAttribute::Float3, 3 * sizeof(float)},
                                 {1, 1, QRhiVertexInputAttribute::Float3, 0},
                                 {2, 2, QRhiVertexInputAttribute::UNormByte4, 0}});
            m_pipeline->setVertexInputLayout(input);
            m_pipeline->setTopology(QRhiGraphicsPipeline::Triangles);
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
                const auto vertices = mesh();
                updates->uploadStaticBuffer(m_quad.get(), vertices.constData());
                m_quadUploaded = true;
            }
            if (m_uploadedLayout != m_frame->layout || m_positionsDirty)
            {
                updates->uploadStaticBuffer(m_positions.get(), 0, count * 12,
                                            m_displayPositions.constData());
                m_positionsDirty = false;
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
            const float radius = m_displayRadius;
            // Also support fixed backing textures: scale logical pixels to backing pixels.

            const float margin =
                qBound(0.1f, 1.0f - m_size / qMin(logicalWidth, logicalHeight), 0.95f);
            const float extent = radius / (qMin(1.0f, aspect) * margin);
            const float distance = extent / std::sin(float(M_PI) / 8) / m_zoom;
            QMatrix4x4 projection;
            if (m_mode == QCatLedPreview::Plane2D)
                projection.ortho(-extent * aspect / m_zoom, extent * aspect / m_zoom,
                                 -extent / m_zoom, extent / m_zoom, radius * 0.001f,
                                 distance + radius * 40);
            else
                projection.perspective(45, aspect, radius * 0.001f, distance + radius * 40);
            QMatrix4x4 view;
            view.translate(float(m_pan.x()) * extent / m_zoom, float(m_pan.y()) * extent / m_zoom,
                           -distance);
            if (m_mode == QCatLedPreview::Spatial3D)
            {
                view.rotate(m_pitch, 1, 0, 0);
                view.rotate(m_yaw, 0, 1, 0);
            }
            const QMatrix4x4 projectionCorrected = m_rhi->clipSpaceCorrMatrix() * projection;
            float uniform[44]{};
            std::memcpy(uniform, projectionCorrected.constData(), 64);
            std::memcpy(uniform + 16, view.constData(), 64);
            uniform[32] = logicalWidth;
            uniform[33] = logicalHeight;
            uniform[34] = m_size;
            uniform[35] = m_brightness;
            uniform[36] = m_sceneSize;
            uniform[37] = float(m_sizeMode);
            uniform[38] = float(m_shape);
            uniform[39] = std::abs(projection(1, 1));
            uniform[40] = 1 - m_gapX;
            uniform[41] = 1 - m_gapY;
            uniform[42] = m_mode == QCatLedPreview::Plane2D ? 1 : 0;
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
            const int first =
                m_shape <= QCatLedPreview::Circle ? 0 : (m_shape == QCatLedPreview::Cube ? 6 : 42);
            const int vertices =
                m_shape <= QCatLedPreview::Circle
                    ? 6
                    : (m_shape == QCatLedPreview::Cube ? 36 : int(mesh().size() / 6) - 42);
            cb->draw(vertices, count, first);
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
    static const QVector<float> &mesh()
    {
        static const QVector<float> data = []
        {
            QVector<float> out;
            auto vertex = [&](QVector3D p, QVector3D n)
            { out << p.x() << p.y() << p.z() << n.x() << n.y() << n.z(); };
            auto triangle = [&](QVector3D a, QVector3D b, QVector3D c, QVector3D n)
            {
                vertex(a, n);
                vertex(b, n);
                vertex(c, n);
            };
            triangle({-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {0, 0, 1});
            triangle({-1, 1, 0}, {1, -1, 0}, {1, 1, 0}, {0, 0, 1});
            for (int axis = 0; axis < 3; ++axis)
                for (int sign : {-1, 1})
                {
                    QVector3D n;
                    n[axis] = float(sign);
                    QVector3D u, v;
                    u[(axis + 1) % 3] = 1;
                    v[(axis + 2) % 3] = 1;
                    triangle(n - u - v, n + u - v, n - u + v, n);
                    triangle(n - u + v, n + u - v, n + u + v, n);
                }
            const int lat = 12, lon = 24;
            auto sphere = [](int y, int x)
            {
                const double a = M_PI * y / 12, b = 2 * M_PI * x / 24;
                return QVector3D(float(std::sin(a) * std::cos(b)), float(std::cos(a)),
                                 float(std::sin(a) * std::sin(b)));
            };
            for (int y = 0; y < lat; ++y)
                for (int x = 0; x < lon; ++x)
                {
                    auto a = sphere(y, x), b = sphere(y + 1, x), c = sphere(y, x + 1),
                         e = sphere(y + 1, x + 1);
                    for (auto p : {a, b, c, c, b, e})
                        vertex(p, p);
                }
            return out;
        }();
        return data;
    }
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
    QCatLedPreview::PointShape m_shape = QCatLedPreview::Circle;
    QCatLedPreview::SizeMode m_sizeMode = QCatLedPreview::ScreenPixels;
    float m_sceneSize = .8f, m_gapX = 0, m_gapY = 0, m_displayRadius = 1;
    QVector3D m_displayCenter;
    QVector<QCatLedSource::Position> m_displayPositions;
    std::shared_ptr<const QCatLedLayout> m_displaySource;
    QVariantMap m_displayConfig;
    bool m_positionsDirty = true;
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
    d->updateClock.start();
    d->throttle.setSingleShot(true);
    d->throttle.setTimerType(Qt::PreciseTimer);
    connect(&d->throttle, &QTimer::timeout, this, &QCatLedPreview::requestUpdate);
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
    if (!d->enabled && node)
        return node;
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
            connect(source, &QCatLedSource::dataChanged, this, [this] { requestUpdate(); });
        d->destroyedConnection = connect(source, &QObject::destroyed, this,
                                         [this]
                                         {
                                             emit sourceChanged();
                                             requestUpdate();
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
    requestUpdate();
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
    requestUpdate();
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
    requestUpdate();
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
    emit cameraChanged();
    requestUpdate();
}
void QCatLedPreview::orbit(qreal dx, qreal dy)
{
    if (d->mode != Spatial3D || !std::isfinite(dx) || !std::isfinite(dy))
        return;
    d->yaw = float(std::remainder(d->yaw + dx * 0.4, 360.0));
    d->pitch = qBound(-89.0f, d->pitch + float(dy * 0.4), 89.0f);
    emit cameraChanged();
    requestUpdate();
}
void QCatLedPreview::pan(qreal dx, qreal dy)
{
    if (!std::isfinite(dx) || !std::isfinite(dy))
        return;
    d->pan += QPointF(2 * dx / qMax(1.0, height()), -2 * dy / qMax(1.0, height()));
    d->pan.setX(qBound(-100.0, d->pan.x(), 100.0));
    d->pan.setY(qBound(-100.0, d->pan.y(), 100.0));
    emit cameraChanged();
    requestUpdate();
}
void QCatLedPreview::zoom(qreal steps)
{
    if (!std::isfinite(steps))
        return;
    d->zoom = qBound(0.2f, d->zoom * float(std::pow(1.15, qBound(-20.0, steps, 20.0))), 100.0f);
    emit cameraChanged();
    requestUpdate();
}

void QCatLedPreview::requestUpdate()
{
    if (!d->enabled)
        return;
    const qint64 now = d->updateClock.nsecsElapsed();
    const qint64 period = 1'000'000'000 / d->maximumFps;
    if (now >= d->nextUpdateNs)
    {
        d->throttle.stop();
        // Keep the fractional-millisecond cadence instead of accumulating wake-up delays.
        // After inactivity or a stalled event loop, skip missed slots without a catch-up burst.
        d->nextUpdateNs = now - d->nextUpdateNs >= period ? now + period : d->nextUpdateNs + period;
        update();
    }
    else if (!d->throttle.isActive())
    {
        const auto waitMs = (d->nextUpdateNs - now + 999'999) / 1'000'000;
        d->throttle.start(int(waitMs));
    }
}
QVariantMap QCatLedPreview::camera() const
{
    return {{"yaw", d->yaw},
            {"pitch", d->pitch},
            {"zoom", d->zoom},
            {"panX", d->pan.x()},
            {"panY", d->pan.y()}};
}
bool QCatLedPreview::setCamera(const QVariantMap &c)
{
    for (auto k : {"yaw", "pitch", "zoom", "panX", "panY"})
        if (!c.contains(k) || !std::isfinite(c[k].toDouble()))
            return false;
    if (c["zoom"].toDouble() < .2 || c["zoom"].toDouble() > 100 ||
        std::abs(c["pitch"].toDouble()) > 89 || std::abs(c["panX"].toDouble()) > 100 ||
        std::abs(c["panY"].toDouble()) > 100)
        return false;
    d->preserveNextLayoutCamera = true;
    d->yaw = float(c["yaw"].toDouble());
    d->pitch = float(c["pitch"].toDouble());
    d->zoom = float(c["zoom"].toDouble());
    d->pan = {c["panX"].toDouble(), c["panY"].toDouble()};
    emit cameraChanged();
    requestUpdate();
    return true;
}
void QCatLedPreview::presetView(int preset)
{
    resetView();
    if (preset == 1)
    {
        d->yaw = 35;
        d->pitch = 25;
    }
    else if (preset == 2)
        d->yaw = 90;
    else if (preset == 3)
        d->pitch = 89;
    emit cameraChanged();
    requestUpdate();
}

QCatLedPreview::PointShape QCatLedPreview::pointShape() const
{
    return d->shape;
}
void QCatLedPreview::setPointShape(PointShape value)
{
    if (value < Square || value > Sphere)
        return;
    if (d->shape == value)
        return;
    d->shape = value;
    emit settingsChanged();
    requestUpdate();
}

QCatLedPreview::SizeMode QCatLedPreview::sizeMode() const
{
    return d->sizeMode;
}
void QCatLedPreview::setSizeMode(SizeMode value)
{
    if (value < ScreenPixels || value > SceneUnits)
        return;
    if (d->sizeMode == value)
        return;
    d->sizeMode = value;
    emit settingsChanged();
    requestUpdate();
}

qreal QCatLedPreview::sceneSize() const
{
    return d->sceneSize;
}
void QCatLedPreview::setSceneSize(qreal value)
{
    if (!std::isfinite(value) || value <= 0 || value > 1e6)
        return;
    if (d->sceneSize == value)
        return;
    d->sceneSize = value;
    emit settingsChanged();
    requestUpdate();
}

QVector3D QCatLedPreview::displaySpacing() const
{
    return d->spacing;
}
void QCatLedPreview::setDisplaySpacing(QVector3D value)
{
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(value[i]) || value[i] < .1 || value[i] > 10)
            return;
    if (d->spacing == value)
        return;
    d->spacing = value;
    emit settingsChanged();
    requestUpdate();
}

qreal QCatLedPreview::gapX() const
{
    return d->gapX;
}
void QCatLedPreview::setGapX(qreal value)
{
    if (!std::isfinite(value) || value < 0 || value > .95)
        return;
    if (d->gapX == value)
        return;
    d->gapX = value;
    emit settingsChanged();
    requestUpdate();
}

qreal QCatLedPreview::gapY() const
{
    return d->gapY;
}
void QCatLedPreview::setGapY(qreal value)
{
    if (!std::isfinite(value) || value < 0 || value > .95)
        return;
    if (d->gapY == value)
        return;
    d->gapY = value;
    emit settingsChanged();
    requestUpdate();
}

int QCatLedPreview::sliceAxis() const
{
    return d->sliceAxis;
}
void QCatLedPreview::setSliceAxis(int value)
{
    if (value < 0 || value > 3)
        return;
    if (d->sliceAxis == value)
        return;
    d->sliceAxis = value;
    emit settingsChanged();
    requestUpdate();
}

bool QCatLedPreview::slicesVertical() const
{
    return d->slicesVertical;
}
void QCatLedPreview::setSlicesVertical(bool value)
{
    if (d->slicesVertical == value)
        return;
    d->slicesVertical = value;
    emit settingsChanged();
    requestUpdate();
}

bool QCatLedPreview::previewEnabled() const
{
    return d->enabled;
}
void QCatLedPreview::setPreviewEnabled(bool value)
{
    if (d->enabled == value)
        return;
    d->enabled = value;
    d->throttle.stop();
    d->nextUpdateNs = 0;
    emit settingsChanged();
    requestUpdate();
}

int QCatLedPreview::maximumFps() const
{
    return d->maximumFps;
}
void QCatLedPreview::setMaximumFps(int value)
{
    if (value < 1 || value > 240)
        return;
    if (d->maximumFps == value)
        return;
    d->maximumFps = value;
    d->throttle.stop();
    d->nextUpdateNs = 0;
    emit settingsChanged();
    requestUpdate();
}

QString QCatLedPreview::backgroundImage() const
{
    return d->backgroundImage;
}
void QCatLedPreview::setBackgroundImage(const QString &v)
{
    d->backgroundImage = v;
    emit settingsChanged();
    requestUpdate();
}
bool QCatLedPreview::showGrid() const
{
    return d->grid;
}
void QCatLedPreview::setShowGrid(bool v)
{
    d->grid = v;
    emit settingsChanged();
}
bool QCatLedPreview::showAxes() const
{
    return d->axes;
}
void QCatLedPreview::setShowAxes(bool v)
{
    d->axes = v;
    emit settingsChanged();
}
