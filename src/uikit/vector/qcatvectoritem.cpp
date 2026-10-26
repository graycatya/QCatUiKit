// SPDX-License-Identifier: Apache-2.0
#include "qcatvectoritem.h"
#include "qcatvectorshape_p.h"
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSGNode>
#include <QSGRendererInterface>
#include <QTimer>
#include <rhi/qrhi.h>
#include <vector>
#ifdef QCATUIKIT_HAS_THORVG
#include <qcatthorvgqrhicanvas.h>
#endif

namespace QCatUiKit
{
namespace
{
struct Feedback
{
    QMutex mutex;
    quint64 generation = 0, frames = 0;
    VectorItem::Status status = VectorItem::Loading;
    QString error;
    QSizeF size;
    qreal duration = 0;
    int frameCount = 0;
};
bool validRect(const QRectF &r)
{
    return std::isfinite(r.x()) && std::isfinite(r.y()) && std::isfinite(r.width()) &&
           std::isfinite(r.height()) && r.width() > 0 && r.height() > 0;
}
QRectF fitted(const QSizeF &target, const QSizeF &source, VectorItem::FillMode mode)
{
    if (source.isEmpty() || target.isEmpty())
        return {};
    if (mode == VectorItem::Stretch)
        return {{0, 0}, target};
    const qreal sx = target.width() / source.width(), sy = target.height() / source.height();
    const qreal scale = mode == VectorItem::PreserveAspectCrop ? qMax(sx, sy) : qMin(sx, sy);
    const QSizeF size = source * scale;
    return {(target.width() - size.width()) / 2, (target.height() - size.height()) / 2,
            size.width(), size.height()};
}
} // namespace
struct VectorItem::Impl
{
    QUrl source;
    QByteArray sourceData, bytes, resourcePath;
    Format format = Svg;
    FillMode fillMode = PreserveAspectFit;
    QRectF viewBox{0, 0, 100, 100};
    QVariantList shapes;
    Status status = Null;
    QString error;
    QSizeF size;
    qreal duration = 0, progress = 0, rate = 1;
    int frames = 0, loops = -1, completedLoops = 0;
    bool autoPlay = false, animationEnabled = true, playing = false, paused = false,
         wantsPlay = false;
    quint64 generation = 0, renderedFrames = 0;
    QTimer timer;
    QElapsedTimer clock;
    std::shared_ptr<Feedback> feedback = std::make_shared<Feedback>();
};

#ifdef QCATUIKIT_HAS_THORVG
namespace
{
using VectorPrivate::plain;
bool applyFill(tvg::Shape *shape, const QVariant &input, bool stroke)
{
    const auto value = plain(input);
    const auto map = value.toMap();
    if (map.isEmpty())
    {
        const QColor c = value.isValid() ? VectorPrivate::color(value) : QColor(Qt::transparent);
        const auto result = stroke ? shape->strokeFill(c.red(), c.green(), c.blue(), c.alpha())
                                   : shape->fill(c.red(), c.green(), c.blue(), c.alpha());
        return result == tvg::Result::Success;
    }
    tvg::Fill *fill = nullptr;
    if (map.value("type") == "linear")
    {
        auto gradient = tvg::LinearGradient::gen();
        if (!gradient)
            return false;
        if (gradient->linear(map.value("x1").toFloat(), map.value("y1").toFloat(),
                             map.value("x2").toFloat(),
                             map.value("y2").toFloat()) != tvg::Result::Success)
        {
            delete gradient;
            return false;
        }
        fill = gradient;
    }
    else
    {
        auto gradient = tvg::RadialGradient::gen();
        if (!gradient)
            return false;
        if (gradient->radial(map.value("cx").toFloat(), map.value("cy").toFloat(),
                             map.value("radius").toFloat(), map.value("cx").toFloat(),
                             map.value("cy").toFloat(), 0) != tvg::Result::Success)
        {
            delete gradient;
            return false;
        }
        fill = gradient;
    }
    std::unique_ptr<tvg::Fill> owner(fill);
    std::vector<tvg::Fill::ColorStop> stops;
    for (const auto &inputStop : plain(map.value("stops")).toList())
    {
        const auto stop = plain(inputStop).toMap();
        const auto c = VectorPrivate::color(stop.value("color"));
        stops.push_back({stop.value("offset").toFloat(), uint8_t(c.red()), uint8_t(c.green()),
                         uint8_t(c.blue()), uint8_t(c.alpha())});
    }
    if (fill->colorStops(stops.data(), uint32_t(stops.size())) != tvg::Result::Success)
        return false;
    const auto result = stroke ? shape->strokeFill(fill) : shape->fill(fill);
    if (result == tvg::Result::Success)
        owner.release();
    return result == tvg::Result::Success;
}
std::unique_ptr<tvg::Shape> makeShape(const QVariantMap &map)
{
    std::unique_ptr<tvg::Shape> shape(tvg::Shape::gen());
    if (!shape)
        return {};
    const auto type = map.value("type").toString();
    const float x = map.value("x").toFloat(), y = map.value("y").toFloat();
    const float w = map.value("width").toFloat(), h = map.value("height").toFloat();
    tvg::Result result = tvg::Result::Success;
    if (type == "rectangle")
        result = shape->appendRect(x, y, w, h, map.value("radius", 0).toFloat(),
                                   map.value("radius", 0).toFloat());
    else if (type == "ellipse")
        result = shape->appendCircle(x + w / 2, y + h / 2, w / 2, h / 2);
    else
    {
        for (const auto &input : plain(map.value("commands")).toList())
        {
            const auto c = plain(input).toList();
            const auto op = c[0].toString();
            if (op == "M")
                result = shape->moveTo(c[1].toFloat(), c[2].toFloat());
            else if (op == "L")
                result = shape->lineTo(c[1].toFloat(), c[2].toFloat());
            else if (op == "C")
                result = shape->cubicTo(c[1].toFloat(), c[2].toFloat(), c[3].toFloat(),
                                        c[4].toFloat(), c[5].toFloat(), c[6].toFloat());
            else
                result = shape->close();
            if (result != tvg::Result::Success)
                return {};
        }
    }
    if (result != tvg::Result::Success ||
        !applyFill(shape.get(), map.value("fill", QColor(Qt::black)), false))
        return {};
    if (map.contains("stroke"))
    {
        if (!applyFill(shape.get(), map.value("stroke"), true) ||
            shape->strokeWidth(map.value("strokeWidth", 1).toFloat()) != tvg::Result::Success)
            return {};
    }
    if (shape->fillRule(map.value("fillRule") == "evenodd"
                            ? tvg::FillRule::EvenOdd
                            : tvg::FillRule::NonZero) != tvg::Result::Success ||
        shape->opacity(uint8_t(map.value("opacity", 1).toDouble() * 255)) != tvg::Result::Success)
        return {};
    return shape;
}
} // namespace
#endif

class VectorRenderer final : public QQuickRhiItemRenderer
{
  public:
    explicit VectorRenderer(std::shared_ptr<Feedback> feedback) : feedback(std::move(feedback)) {}
    ~VectorRenderer() override
    {
#ifdef QCATUIKIT_HAS_THORVG
        release();
        if (initialized)
            tvg::Initializer::term();
#endif
    }

  protected:
    void synchronize(QQuickRhiItem *item) override
    {
        const auto &d = *static_cast<VectorItem *>(item)->d;
        generation = d.generation;
        bytes = d.bytes;
        resourcePath = d.resourcePath;
        shapes = d.shapes;
        format = d.format;
        mode = d.fillMode;
        box = d.viewBox;
        progress = d.progress;
        valid = d.status == VectorItem::Loading || d.status == VectorItem::Ready;
    }
    void initialize(QRhiCommandBuffer *) override
    {
#ifdef QCATUIKIT_HAS_THORVG
        if (!initialized)
            initialized = tvg::Initializer::init(0) == tvg::Result::Success;
        if (owner != rhi())
        {
            release();
            owner = rhi();
        }
#endif
    }
    void render(QRhiCommandBuffer *cb) override
    {
        const auto clear = [&]
        {
            cb->beginPass(renderTarget(), Qt::transparent, {1, 0});
            cb->endPass();
        };
#ifdef QCATUIKIT_HAS_THORVG
        if (!valid || !initialized)
        {
            release();
            clear();
            if (valid && !initialized)
                report(VectorItem::Error, QStringLiteral("ThorVG initialization failed"));
            return;
        }
        if (!canvas || loadedGeneration != generation)
        {
            release();
            canvas.reset(ThorvgQrhiCanvas::gen());
            if (!canvas)
            {
                clear();
                report(VectorItem::Error, QStringLiteral("Cannot create ThorVG QRhi canvas"));
                return;
            }
            if (format == VectorItem::Canvas)
            {
                std::unique_ptr<tvg::Scene> scene(tvg::Scene::gen());
                if (!scene)
                {
                    clear();
                    report(VectorItem::Error, QStringLiteral("Cannot create scene"));
                    return;
                }
                for (const auto &value : shapes)
                {
                    auto shape = makeShape(VectorPrivate::plain(value).toMap());
                    if (!shape || scene->add(shape.get()) != tvg::Result::Success)
                    {
                        clear();
                        report(VectorItem::Error, QStringLiteral("Cannot build vector shape"));
                        return;
                    }
                    shape.release();
                }
                paint = scene.get();
                if (canvas->add(paint) != tvg::Result::Success)
                {
                    paint = nullptr;
                    clear();
                    report(VectorItem::Error, QStringLiteral("Cannot attach scene"));
                    return;
                }
                scene.release();
                nativeSize = box.size();
            }
            else
            {
                if (format == VectorItem::Lottie)
                {
                    animation.reset(tvg::Animation::gen());
                    picture = animation ? animation->picture() : nullptr;
                }
                else
                    picture = tvg::Picture::gen();
                std::unique_ptr<tvg::Picture> imageOwner(format == VectorItem::Svg ? picture
                                                                                   : nullptr);
                if (!picture ||
                    picture->load(bytes.constData(), uint32_t(bytes.size()),
                                  format == VectorItem::Svg ? "svg" : "lottie",
                                  resourcePath.isEmpty() ? nullptr : resourcePath.constData(),
                                  true) != tvg::Result::Success)
                {
                    picture = nullptr;
                    clear();
                    report(VectorItem::Error, QStringLiteral("Cannot decode SVG/Lottie data"));
                    return;
                }
                float w = 0, h = 0;
                picture->size(&w, &h);
                if (!std::isfinite(w) || !std::isfinite(h) || w <= 0 || h <= 0)
                {
                    picture = nullptr;
                    clear();
                    report(VectorItem::Error,
                           QStringLiteral("Vector source has invalid dimensions"));
                    return;
                }
                nativeSize = {w, h};
                paint = picture;
                if (canvas->add(paint) != tvg::Result::Success)
                {
                    paint = nullptr;
                    picture = nullptr;
                    clear();
                    report(VectorItem::Error, QStringLiteral("Cannot attach picture"));
                    return;
                }
                imageOwner.release();
            }
            loadedGeneration = generation;
        }
        if (!paint)
        {
            clear();
            return;
        }
        tvg::Result result = tvg::Result::Success;
        if (animation)
        {
            const float frame = float(progress * qMax(0.0f, animation->totalFrame() - 1.0f));
            if (std::abs(frame - animation->curFrame()) > 0.001f)
                result = animation->frame(frame);
        }
        const auto viewport = fitted(renderTarget()->pixelSize(), nativeSize, mode);
        const qreal sx = viewport.width() / nativeSize.width(),
                    sy = viewport.height() / nativeSize.height();
        const QPointF origin = format == VectorItem::Canvas ? box.topLeft() : QPointF();
        const tvg::Matrix matrix{float(sx), 0,         float(viewport.x() - origin.x() * sx),
                                 0,         float(sy), float(viewport.y() - origin.y() * sy),
                                 0,         0,         1};
        if (result == tvg::Result::Success)
            result = paint->transform(matrix);
        if (result == tvg::Result::Success)
            result = canvas->target(rhi(), cb, renderTarget());
        if (result == tvg::Result::Success)
            result = canvas->update();
        if (result == tvg::Result::Success)
            result = canvas->draw(true);
        const auto syncResult = canvas->sync();
        if (result == tvg::Result::Success)
            result = syncResult;
        if (result != tvg::Result::Success)
        {
            clear();
            report(VectorItem::Error,
                   QStringLiteral("ThorVG QRhi render failed (%1)").arg(int(result)));
            return;
        }
        report(VectorItem::Ready);
#else
        clear();
#endif
    }

  private:
    std::shared_ptr<Feedback> feedback;
    quint64 generation = 0;
    QByteArray bytes, resourcePath;
    QVariantList shapes;
    VectorItem::Format format = VectorItem::Svg;
    VectorItem::FillMode mode = VectorItem::PreserveAspectFit;
    QRectF box;
    qreal progress = 0;
    bool valid = false;
#ifdef QCATUIKIT_HAS_THORVG
    bool initialized = false;
    QRhi *owner = nullptr;
    quint64 loadedGeneration = 0;
    std::unique_ptr<ThorvgQrhiCanvas> canvas;
    std::unique_ptr<tvg::Animation> animation;
    tvg::Paint *paint = nullptr;
    tvg::Picture *picture = nullptr;
    QSizeF nativeSize;
    void release()
    {
        canvas
            .reset(); // Canvas releases its picture reference before Animation's owning reference.
        animation.reset();
        paint = nullptr;
        picture = nullptr;
        loadedGeneration = 0;
        nativeSize = {};
    }
    void report(VectorItem::Status status, const QString &error = {})
    {
        QMutexLocker lock(&feedback->mutex);
        feedback->generation = generation;
        feedback->status = status;
        feedback->error = error;
        feedback->size = nativeSize;
        feedback->duration = animation ? animation->duration() : 0;
        feedback->frameCount = animation ? qMax(0, int(animation->totalFrame())) : 0;
        if (status == VectorItem::Ready)
            ++feedback->frames;
    }
#endif
};

VectorItem::VectorItem(QQuickItem *parent) : QQuickRhiItem(parent), d(std::make_unique<Impl>())
{
    setAlphaBlending(true);
    setSampleCount(4);
    setClip(true);
    setImplicitSize(128, 128);
    connect(this, &QQuickItem::widthChanged, this, &VectorItem::contentRectChanged);
    connect(this, &QQuickItem::heightChanged, this, &VectorItem::contentRectChanged);
    d->timer.setInterval(16);
    connect(&d->timer, &QTimer::timeout, this, &VectorItem::tick);
    d->clock.start();
}
VectorItem::~VectorItem() = default;
QQuickRhiItemRenderer *VectorItem::createRenderer()
{
    return new VectorRenderer(d->feedback);
}
QSGNode *VectorItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
{
    // Keep the themed error/empty children usable on software without asking
    // QQuickRhiItem to construct an unsupported native rendering node.
    if (!backendAvailable() || (window() && window()->rendererInterface()->graphicsApi() ==
                                                QSGRendererInterface::Software))
    {
        delete node;
        return nullptr;
    }
    return QQuickRhiItem::updatePaintNode(node, data);
}
void VectorItem::setState(Status status, const QString &error)
{
    if (d->status == status && d->error == error)
        return;
    d->status = status;
    d->error = error;
    if (status == Error || status == Null)
    {
        d->timer.stop();
        d->playing = d->paused = false;
        emit playbackChanged();
    }
    emit statusChanged();
}
void VectorItem::prepare()
{
    if (!d->timer.isActive())
    {
        d->clock.restart();
        d->timer.start();
    }
    ++d->generation;
    d->bytes.clear();
    d->resourcePath.clear();
    d->size = {};
    d->duration = 0;
    d->frames = 0;
    d->progress = 0;
    d->completedLoops = 0;
    d->playing = d->paused = false;
    d->wantsPlay = d->autoPlay;
    emit metadataChanged();
    emit progressChanged();
    emit playbackChanged();
    emit contentRectChanged();
    update();
    if (d->format == Canvas)
    {
        const auto error = VectorPrivate::shapesError(d->shapes);
        if (!error.isEmpty())
        {
            setState(Error, error);
            return;
        }
    }
    else if (d->sourceData.isEmpty() && d->source.isEmpty())
    {
        setState(Null);
        return;
    }
    if (!backendAvailable())
    {
        setState(Error, QStringLiteral("ThorVG support is disabled in this QCatUiKit build"));
        return;
    }
    if (d->format != Canvas)
    {
        d->bytes = d->sourceData;
        if (d->bytes.isEmpty())
        {
            QUrl url = d->source;
            if (url.isRelative())
                url = qmlContext(this)
                          ? qmlContext(this)->resolvedUrl(url)
                          : QUrl::fromLocalFile(QFileInfo(url.toString()).absoluteFilePath());
            if (url.scheme() != "qrc" && !url.isLocalFile())
            {
                setState(Error,
                         QStringLiteral("Vector source supports local files and qrc URLs only"));
                return;
            }
            QFile file(url.scheme() == "qrc" ? ':' + url.path() : url.toLocalFile());
            if (!file.open(QIODevice::ReadOnly))
            {
                setState(Error,
                         QStringLiteral("Cannot open vector source: %1").arg(file.errorString()));
                return;
            }
            if (file.size() > 64 * 1024 * 1024)
            {
                setState(Error, QStringLiteral("Vector source exceeds 64 MiB"));
                return;
            }
            d->bytes = file.readAll();
            if (url.isLocalFile())
                d->resourcePath = QFileInfo(url.toLocalFile()).absolutePath().toUtf8();
        }
        if (d->bytes.isEmpty() || d->bytes.size() > 64 * 1024 * 1024)
        {
            setState(Error, QStringLiteral("Vector data is empty or exceeds 64 MiB"));
            return;
        }
    }
    setState(Loading);
}
void VectorItem::tick()
{
    const qreal elapsed = d->clock.restart() / 1000.0;
    if (window() &&
        window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software &&
        d->status == Loading)
    {
        setState(Error, QStringLiteral("ThorVG QRhi requires a Qt Quick graphics backend"));
        update();
    }
    {
        QMutexLocker lock(&d->feedback->mutex);
        const auto &f = *d->feedback;
        if (f.generation == d->generation && d->generation &&
            (d->status == Loading || d->status == Ready))
        {
            const auto generation = d->generation;
            const bool metadata =
                d->size != f.size || d->duration != f.duration || d->frames != f.frameCount;
            d->size = f.size;
            d->duration = f.duration;
            d->frames = f.frameCount;
            const bool becameReady = d->status != Ready && f.status == Ready;
            const auto status = f.status;
            const auto error = f.error;
            const auto frames = f.frames;
            lock.unlock();
            if (metadata)
            {
                setImplicitSize(d->size.width(), d->size.height());
                if (generation != d->generation)
                    return;
                emit metadataChanged();
                emit contentRectChanged();
            }
            if (generation != d->generation)
                return;
            setState(status, error);
            if (generation != d->generation)
                return;
            if (becameReady && d->wantsPlay)
                play();
            if (frames != d->renderedFrames)
            {
                d->renderedFrames = frames;
                emit renderedFramesChanged();
            }
        }
    }
    if (!d->playing || d->status != Ready || d->format != Lottie || d->duration <= 0 ||
        !d->animationEnabled || !isVisible() || !window() || !window()->isExposed())
        return;
    qreal progress = d->progress + elapsed * d->rate / d->duration;
    if (progress >= 1)
    {
        d->completedLoops += int(progress);
        if (d->loops != -1 && d->completedLoops >= d->loops)
        {
            setProgress(1);
            d->playing = d->wantsPlay = false;
            emit playbackChanged();
            emit finished();
            return;
        }
        progress = std::fmod(progress, 1.0);
    }
    setProgress(progress);
}
#define QCAT_VECTOR_GET(Type, Name, Field)                                                         \
    Type VectorItem::Name() const                                                                  \
    {                                                                                              \
        return d->Field;                                                                           \
    }
QCAT_VECTOR_GET(QUrl, source, source)
QCAT_VECTOR_GET(QByteArray, sourceData, sourceData)
QCAT_VECTOR_GET(VectorItem::Format, format, format)
QCAT_VECTOR_GET(VectorItem::FillMode, fillMode, fillMode)
QCAT_VECTOR_GET(QRectF, viewBox, viewBox)
QCAT_VECTOR_GET(QVariantList, shapes, shapes)
QCAT_VECTOR_GET(VectorItem::Status, status, status)
QCAT_VECTOR_GET(QString, errorString, error)
QCAT_VECTOR_GET(QSizeF, sourceSize, size)
QCAT_VECTOR_GET(qreal, duration, duration)
QCAT_VECTOR_GET(int, frameCount, frames)
QCAT_VECTOR_GET(qreal, progress, progress)
QCAT_VECTOR_GET(bool, autoPlay, autoPlay)
QCAT_VECTOR_GET(bool, animationEnabled, animationEnabled)
QCAT_VECTOR_GET(bool, playing, playing)
QCAT_VECTOR_GET(bool, paused, paused)
QCAT_VECTOR_GET(int, loops, loops)
QCAT_VECTOR_GET(qreal, playbackRate, rate)
QCAT_VECTOR_GET(quint64, renderedFrames, renderedFrames)
#undef QCAT_VECTOR_GET
bool VectorItem::backendAvailable() const
{
#ifdef QCATUIKIT_HAS_THORVG
    return true;
#else
    return false;
#endif
}
qreal VectorItem::currentFrame() const
{
    return d->progress * qMax(0, d->frames - 1);
}
QRectF VectorItem::contentRect() const
{
    return fitted({width(), height()}, d->size, d->fillMode);
}
void VectorItem::setSource(const QUrl &value)
{
    if (d->source == value)
        return;
    d->source = value;
    emit sourceChanged();
    prepare();
}
void VectorItem::setSourceData(const QByteArray &value)
{
    if (d->sourceData == value)
        return;
    d->sourceData = value;
    emit sourceDataChanged();
    prepare();
}
void VectorItem::setFormat(Format value)
{
    if (value < Svg || value > Canvas || d->format == value)
        return;
    d->format = value;
    emit formatChanged();
    prepare();
}
void VectorItem::setFillMode(FillMode value)
{
    if (value < Stretch || value > PreserveAspectCrop || d->fillMode == value)
        return;
    d->fillMode = value;
    emit fillModeChanged();
    emit contentRectChanged();
    update();
}
void VectorItem::setViewBox(const QRectF &value)
{
    if (!validRect(value) || d->viewBox == value)
        return;
    d->viewBox = value;
    emit viewBoxChanged();
    if (d->format == Canvas)
        prepare();
}
void VectorItem::setShapes(const QVariantList &value)
{
    if (d->shapes == value)
        return;
    d->shapes = value;
    emit shapesChanged();
    if (d->format == Canvas)
        prepare();
}
void VectorItem::setProgress(qreal value)
{
    if (!std::isfinite(value))
        return;
    value = qBound(0.0, value, 1.0);
    if (d->progress == value)
        return;
    d->progress = value;
    emit progressChanged();
    update();
}
void VectorItem::setAutoPlay(bool value)
{
    if (d->autoPlay == value)
        return;
    d->autoPlay = value;
    emit playbackChanged();
}
void VectorItem::setAnimationEnabled(bool value)
{
    if (d->animationEnabled == value)
        return;
    d->animationEnabled = value;
    emit playbackChanged();
}
void VectorItem::setLoops(int value)
{
    if (value != -1 && value < 1)
        return;
    if (d->loops == value)
        return;
    d->loops = value;
    emit playbackChanged();
}
void VectorItem::setPlaybackRate(qreal value)
{
    if (!std::isfinite(value) || value <= 0 || value > 16 || d->rate == value)
        return;
    d->rate = value;
    emit playbackChanged();
}
void VectorItem::reload()
{
    prepare();
}
void VectorItem::play()
{
    if (d->format != Lottie)
        return;
    if (d->progress >= 1)
    {
        d->completedLoops = 0;
        setProgress(0);
    }
    d->wantsPlay = true;
    d->paused = false;
    d->playing = d->status == Ready && d->duration > 0 && d->frames > 1;
    d->clock.restart();
    emit playbackChanged();
}
void VectorItem::pause()
{
    if (d->format != Lottie)
        return;
    d->wantsPlay = d->playing = false;
    d->paused = true;
    emit playbackChanged();
}
void VectorItem::stop()
{
    d->wantsPlay = d->playing = d->paused = false;
    d->completedLoops = 0;
    setProgress(0);
    emit playbackChanged();
}
void VectorItem::restart()
{
    stop();
    play();
}
bool VectorItem::appendShape(const QVariantMap &shape)
{
    auto list = d->shapes;
    list.append(shape);
    if (!VectorPrivate::shapesError(list).isEmpty())
        return false;
    setShapes(list);
    return true;
}
void VectorItem::clearShapes()
{
    setShapes({});
}
} // namespace QCatUiKit
