// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotpetcontroller.h"
#include "qcatmascotpackage_p.h"
#include "qcatmascotrenderadapter_p.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QPolygonF>
#include <QPromise>
#include <QQueue>
#include <QRectF>
#include <QThreadPool>
#include <QTimer>
#include <algorithm>
#include <atomic>
#include <cmath>

namespace QCatUiKit
{
Q_LOGGING_CATEGORY(mascotLoading, "qcatuikit.mascot.loading", QtWarningMsg)
namespace
{
QThreadPool &packageLoaderPool();
void drainLoaderPool()
{
    packageLoaderPool().waitForDone();
}
QThreadPool &packageLoaderPool()
{
    struct Pool final : QThreadPool
    {
        Pool()
        {
            setMaxThreadCount(2);
            qAddPostRoutine(drainLoaderPool);
        }
        ~Pool() override
        {
            qRemovePostRoutine(drainLoaderPool);
        }
    };
    static Pool pool;
    return pool;
}
} // namespace

class MascotPetController::Impl
{
  public:
    struct Request
    {
        QString id;
        QString name;
        int loops = -1;
    };
    MascotPetController *q;
    QUrl source;
    Status status = Null;
    QString code, error, facing = "right";
    MascotPrivate::Package package;
    QString action;
    Request active;
    QQueue<Request> queue;
    std::shared_ptr<std::atomic_bool> loadingCancelled;
    QTimer timer;
    QPointer<VectorItem> vectorItem;
    QElapsedTimer clock;
    double elapsedMs = 0;
    quint64 generation = 0, nextId = 0;
    bool paused = false, enabled = true;
    qreal scale = 1;
    bool dragEnabled = true, dragging = false, feedback = false;
    QString dragAction;
    int frame = 0;
    bool reloadVectorOnStart = false;

    explicit Impl(MascotPetController *owner) : q(owner)
    {
        timer.setParent(q);
        timer.setInterval(8);
        timer.setTimerType(Qt::PreciseTimer);
        QObject::connect(&timer, &QTimer::timeout, q, [this] { tick(); });
    }
    ~Impl()
    {
        if (loadingCancelled)
            loadingCancelled->store(true);
    }
    MascotPrivate::Sprite sprite() const
    {
        const auto a = package.actions.value(action);
        if (a.directions.contains(facing))
            return a.directions.value(facing);
        return a.directions.isEmpty() ? MascotPrivate::Sprite{} : a.directions.first();
    }
    bool mirrored() const
    {
        return !action.isEmpty() && !package.actions.value(action).directions.contains(facing);
    }
    void syncTimer()
    {
        if (package.renderer == "lottie")
        {
            timer.stop();
            if (vectorItem)
                vectorItem->setAnimationEnabled(!paused && enabled);
            return;
        }
        if (status == Ready && !paused && enabled && !action.isEmpty())
        {
            clock.restart();
            timer.start();
        }
        else
            timer.stop();
    }
    void start(const Request &request)
    {
        active = request;
        action = request.name;
        frame = 0;
        elapsedMs = 0;
        if (package.renderer == "lottie")
        {
            status = Loading;
            if (vectorItem)
            {
                vectorItem->stop();
                vectorItem->setLoops(request.loops);
                if (reloadVectorOnStart && vectorItem->source() == sprite().source)
                    vectorItem->reload();
                else
                    vectorItem->setSource(sprite().source);
                reloadVectorOnStart = false;
                vectorItem->play();
            }
        }
        syncTimer();
        const auto epoch = generation;
        emit q->currentActionChanged();
        if (epoch != generation)
            return;
        emit q->anchorsChanged();
        if (epoch != generation)
            return;
        emit q->spriteFrameChanged();
        emit q->regionsChanged();
        if (epoch != generation)
            return;
        if (!active.id.isEmpty())
            emit q->actionStarted(active.id, action);
        if (epoch != generation)
            return;
        if (package.renderer == "lottie")
        {
            emit q->statusChanged();
            QTimer::singleShot(0, q,
                               [this, epoch]
                               {
                                   if (epoch == generation)
                                       vectorStatus();
                               });
        }
    }
    void vectorStatus()
    {
        if (!vectorItem || package.renderer != "lottie" || action.isEmpty() ||
            vectorItem->source() != sprite().source || status == Error)
            return;
        if (vectorItem->status() == VectorItem::Error)
        {
            q->renderError(sprite().source, vectorItem->errorString());
        }
        else if (vectorItem->status() == VectorItem::Ready && status != Ready)
        {
            status = Ready;
            emit q->regionsChanged();
            emit q->statusChanged();
        }
    }
    void complete()
    {
        const QString id = active.id;
        active = {};
        const auto epoch = generation;
        if (!id.isEmpty())
            emit q->actionEnded(id, "Completed");
        if (epoch != generation)
            return;
        if (!queue.isEmpty())
        {
            const auto next = queue.dequeue();
            emit q->queueLengthChanged();
            if (epoch != generation)
                return;
            start(next);
        }
        else
            defaultAction();
    }
    void defaultAction()
    {
        start({{}, package.defaultAction, -1});
    }
    void cancel(const QString &reason)
    {
        const auto old = active;
        const auto pending = queue;
        active = {};
        queue.clear();
        timer.stop();
        action.clear();
        frame = 0;
        elapsedMs = 0;
        if (!pending.isEmpty())
            emit q->queueLengthChanged();
        if (!old.id.isEmpty())
            emit q->actionEnded(old.id, reason);
        for (const auto &r : pending)
            emit q->actionEnded(r.id, "Cancelled");
    }
    void tick()
    {
        if (package.renderer != "sprite" || status != Ready || paused || !enabled ||
            action.isEmpty())
            return;
        elapsedMs += double(clock.nsecsElapsed()) / 1000000.0;
        clock.restart();
        const auto s = sprite();
        const double steps = std::floor(elapsedMs * s.fps / 1000.0);
        if (active.loops != -1 && steps >= double(s.frameCount) * active.loops)
        {
            complete();
            return;
        }
        const int next = int(std::fmod(steps, s.frameCount));
        if (frame != next)
        {
            frame = next;
            emit q->spriteFrameChanged();
        }
    }
};

MascotPetController::MascotPetController(QObject *parent) : QObject(parent), d(new Impl(this)) {}
MascotPetController::~MascotPetController() = default;
QUrl MascotPetController::packageSource() const
{
    return d->source;
}
MascotPetController::Status MascotPetController::status() const
{
    return d->status;
}
QString MascotPetController::errorCode() const
{
    return d->code;
}
QString MascotPetController::errorString() const
{
    return d->error;
}
QStringList MascotPetController::availableActions() const
{
    return d->package.actions.keys();
}
QString MascotPetController::currentAction() const
{
    return d->action;
}
int MascotPetController::queueLength() const
{
    return d->queue.size();
}
bool MascotPetController::paused() const
{
    return d->paused;
}
bool MascotPetController::playbackEnabled() const
{
    return d->enabled;
}
QString MascotPetController::facing() const
{
    return d->facing;
}
qreal MascotPetController::petScale() const
{
    return d->scale;
}
QSize MascotPetController::logicalSize() const
{
    return d->package.logicalSize;
}
QPointF MascotPetController::footAnchor() const
{
    const auto p = d->package.footAnchor;
    return d->mirrored() ? QPointF(d->package.logicalSize.width() - p.x(), p.y()) : p;
}
QPointF MascotPetController::bubbleAnchor() const
{
    const auto p = d->package.bubbleAnchor;
    return d->mirrored() ? QPointF(d->package.logicalSize.width() - p.x(), p.y()) : p;
}

void MascotPetController::setPackageSource(const QUrl &source)
{
    if (source == d->source)
        return;
    if (d->loadingCancelled)
        d->loadingCancelled->store(true);
    const auto epoch = ++d->generation;
    const bool wasDragging = d->dragging;
    d->dragging = d->feedback = false;
    d->source = source;
    d->reloadVectorOnStart = !source.isEmpty();
    d->status = source.isEmpty() ? Null : Loading;
    d->package = {};
    d->code.clear();
    d->error.clear();
    d->cancel("Cancelled");
    if (epoch != d->generation)
        return;
    if (d->vectorItem)
    {
        if (source.isEmpty())
        {
            d->vectorItem->stop();
            d->vectorItem->setSource({});
        }
        else
            d->vectorItem->pause();
    }
    if (epoch != d->generation)
        return;
    if (wasDragging)
    {
        emit draggingChanged();
        emit dragFinished();
    }
    if (epoch != d->generation)
        return;
    emit packageSourceChanged();
    if (epoch != d->generation)
        return;
    emit packageChanged();
    emit currentActionChanged();
    emit anchorsChanged();
    emit spriteFrameChanged();
    emit regionsChanged();
    emit statusChanged();
    if (epoch != d->generation || source.isEmpty())
        return;
    struct Result
    {
        MascotPrivate::Package package;
        QString error;
        bool ok = false;
        qint64 elapsedMs = 0;
    };
    auto cancelled = std::make_shared<std::atomic_bool>(false);
    d->loadingCancelled = cancelled;
    auto promise = std::make_shared<QPromise<Result>>();
    auto *watcher = new QFutureWatcher<Result>(this);
    connect(watcher, &QFutureWatcher<Result>::finished, this,
            [this, watcher, epoch, cancelled]
            {
                watcher->deleteLater();
                if (epoch != d->generation || cancelled->load())
                    return;
                const auto result = watcher->result();
                qCDebug(mascotLoading) << "package validation ms" << result.elapsedMs;
                if (!result.ok)
                {
                    if (d->vectorItem)
                        d->vectorItem->setSource({});
                    if (epoch != d->generation)
                        return;
                    d->status = Error;
                    d->code = "InvalidPackage";
                    d->error = result.error;
                    emit statusChanged();
                    return;
                }
                d->package = result.package;
                if (d->package.renderer == "sprite" && d->vectorItem)
                    d->vectorItem->setSource({});
                if (epoch != d->generation)
                    return;
                if (!d->dragAction.isEmpty() && !d->package.actions.contains(d->dragAction))
                {
                    d->dragAction.clear();
                    emit dragActionChanged();
                    emit configurationRejected("UnknownDragAction");
                    if (epoch != d->generation)
                        return;
                }
                d->facing = d->package.defaultFacing;
                d->status = d->package.renderer == "sprite" ? Ready : Loading;
                emit packageChanged();
                if (epoch != d->generation)
                    return;
                emit facingChanged();
                if (epoch != d->generation)
                    return;
                d->defaultAction();
                if (epoch == d->generation)
                    emit statusChanged();
            });
    watcher->setFuture(promise->future());
    packageLoaderPool().start(
        [promise, source, cancelled]
        {
            promise->start();
            QElapsedTimer elapsed;
            elapsed.start();
            Result result;
            result.ok = MascotPrivate::loadPackage(source, &result.package, &result.error,
                                                   [cancelled] { return cancelled->load(); });
            result.elapsedMs = elapsed.elapsed();
            promise->addResult(std::move(result));
            promise->finish();
        });
}

QVariantMap MascotPetController::spriteFrame() const
{
    if ((d->status != Ready && d->status != Loading) || d->action.isEmpty())
        return {};
    const auto sprite = d->sprite();
    const bool mirror = !d->package.actions.value(d->action).directions.contains(d->facing);
    if (d->package.renderer == "lottie")
        return {{"source", sprite.source}, {"mirrored", mirror}, {"renderer", "lottie"}};
    const int columns = sprite.atlasSize.width() / sprite.frameSize.width();
    const int index = sprite.startFrame + d->frame;
    return {{"renderer", "sprite"},
            {"source", sprite.source},
            {"atlasSize", sprite.atlasSize},
            {"frameSize", sprite.frameSize},
            {"x", (index % columns) * sprite.frameSize.width()},
            {"y", (index / columns) * sprite.frameSize.height()},
            {"index", d->frame},
            {"mirrored", mirror}};
}
void MascotPetController::setPlaybackEnabled(bool enabled)
{
    if (enabled == d->enabled)
        return;
    if (!enabled)
        d->tick();
    d->enabled = enabled;
    d->syncTimer();
    emit playbackEnabledChanged();
}
void MascotPetController::pauseActions()
{
    if (d->paused)
        return;
    d->tick();
    d->paused = true;
    d->syncTimer();
    emit pausedChanged();
}
void MascotPetController::resumeActions()
{
    if (!d->paused)
        return;
    d->paused = false;
    d->syncTimer();
    emit pausedChanged();
}
void MascotPetController::setFacing(const QString &value)
{
    if (value == d->facing)
        return;
    if ((value != "left" && value != "right") ||
        ((d->status == Ready || d->status == Loading) && !d->package.allowMirror &&
         std::any_of(d->package.actions.begin(), d->package.actions.end(),
                     [&](const MascotPrivate::Action &a)
                     { return !a.directions.contains(value); })))
    {
        emit configurationRejected("FacingUnavailable");
        return;
    }
    d->facing = value;
    if ((d->status == Ready || d->status == Loading) && !d->action.isEmpty())
    {
        const auto sprite = d->sprite();
        if (d->package.renderer == "sprite")
            d->frame =
                int(std::fmod(std::floor(d->elapsedMs * sprite.fps / 1000.0), sprite.frameCount));
        else if (d->vectorItem && d->vectorItem->source() != sprite.source)
        {
            const qreal progress = d->vectorItem->progress();
            d->status = Loading;
            d->vectorItem->setSource(sprite.source);
            d->vectorItem->setProgress(progress);
            d->vectorItem->play();
            emit statusChanged();
        }
    }
    emit facingChanged();
    emit anchorsChanged();
    emit spriteFrameChanged();
    emit regionsChanged();
}
void MascotPetController::setPetScale(qreal scale)
{
    if (!std::isfinite(scale) || scale <= 0)
    {
        emit configurationRejected("InvalidScale");
        return;
    }
    if (scale == d->scale)
        return;
    d->scale = scale;
    emit petScaleChanged();
    emit regionsChanged();
}
QString MascotPetController::requestAction(const QString &name, RequestPolicy policy,
                                           const QVariantMap &options)
{
    const QString id = QString::number(++d->nextId);
    const auto epoch = d->generation;
    const bool interactionBusy = d->dragging && d->feedback;
    QTimer::singleShot(
        0, this,
        [this, epoch, name, policy, options, id, interactionBusy]
        {
            if (epoch != d->generation)
            {
                emit actionEnded(id, "Cancelled");
                return;
            }
            if (interactionBusy || (d->dragging && d->feedback))
            {
                emit requestRejected(id, "InteractionBusy");
                return;
            }
            if ((d->status != Ready && !(d->status == Loading && !d->package.actions.isEmpty())) ||
                !d->package.actions.contains(name))
            {
                emit requestRejected(id, d->status == Error || d->package.actions.isEmpty()
                                             ? "NotReady"
                                             : "UnknownAction");
                return;
            }
            if (policy < Replace || policy > IgnoreIfBusy)
            {
                emit requestRejected(id, "InvalidPolicy");
                return;
            }
            if (options.size() > (options.contains("loops") ? 1 : 0))
            {
                emit requestRejected(id, "InvalidOptions");
                return;
            }
            int loops = d->package.actions.value(name).loops;
            if (options.contains("loops"))
            {
                bool ok;
                const double number = options.value("loops").toDouble(&ok);
                if (!QJsonValue::fromVariant(options.value("loops")).isDouble() || !ok ||
                    !std::isfinite(number) || std::floor(number) != number || number < -1 ||
                    number == 0 || number > 2147483647)
                {
                    emit requestRejected(id, "InvalidLoops");
                    return;
                }
                loops = int(number);
            }
            Impl::Request request{id, name, loops};
            if (!d->active.id.isEmpty() && policy == IgnoreIfBusy)
            {
                emit actionEnded(id, "Ignored");
                return;
            }
            if (!d->active.id.isEmpty() && policy == Enqueue)
            {
                if (d->queue.size() >= 64)
                {
                    emit requestRejected(id, "QueueFull");
                    return;
                }
                d->queue.enqueue(request);
                emit queueLengthChanged();
                return;
            }
            if (policy == Replace)
            {
                d->cancel("Interrupted");
                if (epoch != d->generation)
                {
                    emit actionEnded(id, "Cancelled");
                    return;
                }
            }
            if (d->dragging && d->feedback)
            {
                emit requestRejected(id, "InteractionBusy");
                return;
            }
            d->start(request);
        });
    return id;
}
void MascotPetController::resetToDefault()
{
    // Defer mutations made by action callbacks until the current transition finishes.
    const auto epoch = d->generation;
    QTimer::singleShot(
        0, this,
        [this, epoch]
        {
            if (epoch != d->generation)
                return;
            if (d->dragging && d->feedback)
            {
                emit configurationRejected("InteractionBusy");
                return;
            }
            d->cancel("Cancelled");
            if (d->dragging && d->feedback)
            {
                emit configurationRejected("InteractionBusy");
                return;
            }
            if (epoch == d->generation &&
                (d->status == Ready || (d->status == Loading && !d->package.actions.isEmpty())))
                d->defaultAction();
            else
            {
                emit currentActionChanged();
                emit spriteFrameChanged();
            }
        });
}

bool MascotPetController::dragEnabled() const
{
    return d->dragEnabled;
}
void MascotPetController::setDragEnabled(bool enabled)
{
    if (enabled == d->dragEnabled)
        return;
    d->dragEnabled = enabled;
    if (!enabled)
        endDrag();
    emit dragEnabledChanged();
}
QString MascotPetController::dragAction() const
{
    return d->dragAction;
}
void MascotPetController::setDragAction(const QString &action)
{
    if (action == d->dragAction)
        return;
    if (d->dragging)
    {
        emit configurationRejected("InteractionBusy");
        return;
    }
    if (!action.isEmpty() && !d->package.actions.isEmpty() && !d->package.actions.contains(action))
    {
        emit configurationRejected("UnknownDragAction");
        return;
    }
    d->dragAction = action;
    emit dragActionChanged();
}
bool MascotPetController::dragging() const
{
    return d->dragging;
}
bool MascotPetController::beginDrag()
{
    if (!d->dragEnabled || d->dragging || d->status == Null ||
        (d->status == Loading && d->package.actions.isEmpty()))
        return false;
    const auto epoch = d->generation;
    d->dragging = true;
    d->feedback = !d->dragAction.isEmpty() && d->package.actions.contains(d->dragAction) &&
                  (d->status == Ready || d->status == Loading);
    if (d->feedback)
    {
        d->cancel("Interrupted");
        if (epoch != d->generation)
            return false;
        d->start({{}, d->dragAction, -1});
        if (epoch != d->generation)
            return false;
    }
    emit draggingChanged();
    if (epoch != d->generation)
        return false;
    emit dragStarted();
    return true;
}
void MascotPetController::endDrag()
{
    if (!d->dragging)
        return;
    const auto epoch = d->generation;
    const bool feedback = d->feedback;
    d->dragging = d->feedback = false;
    if (feedback)
    {
        d->cancel("Cancelled");
        if (epoch != d->generation)
            return;
        if (d->status == Ready || (d->status == Loading && !d->package.actions.isEmpty()))
            d->defaultAction();
    }
    if (epoch != d->generation)
        return;
    emit draggingChanged();
    emit dragFinished();
}
QVariantList MascotPetController::interactionRegions() const
{
    QVariantList result;
    if (d->status != Ready || d->action.isEmpty())
        return result;
    for (const auto &value : d->package.actions.value(d->action).regions)
    {
        QVariantMap region = value.toMap();
        if (region.value("shape").toString() == "polygon")
        {
            QVariantList points;
            for (const auto &p : region.value("points").toList())
            {
                const auto pair = p.toList();
                const double x = d->mirrored() ? d->package.logicalSize.width() - pair[0].toDouble()
                                               : pair[0].toDouble();
                points.append(QVariant(QVariantList{x * d->scale, pair[1].toDouble() * d->scale}));
            }
            region["points"] = points;
        }
        else
        {
            auto rect = region.value("rect").toList();
            double x = rect[0].toDouble();
            if (d->mirrored())
                x = d->package.logicalSize.width() - x - rect[2].toDouble();
            region["rect"] =
                QVariantList{x * d->scale, rect[1].toDouble() * d->scale,
                             rect[2].toDouble() * d->scale, rect[3].toDouble() * d->scale};
        }
        result.append(region);
    }
    return result;
}
QVariantMap MascotPetController::hitTest(const QPointF &position, bool forDrag) const
{
    if (d->status != Ready || d->action.isEmpty() || !std::isfinite(position.x()) ||
        !std::isfinite(position.y()))
        return {};
    QPointF point = position / d->scale;
    if (!QRectF(QPointF(), d->package.logicalSize).contains(point))
        return {};
    if (d->mirrored())
        point.setX(d->package.logicalSize.width() - point.x());
    const auto regions = d->package.actions.value(d->action).regions;
    for (auto it = regions.crbegin(); it != regions.crend(); ++it)
    {
        const auto r = it->toMap();
        if (!r.value(forDrag ? "draggable" : "clickable").toBool())
            continue;
        bool hit = false;
        const QString shape = r.value("shape").toString();
        if (shape == "polygon")
        {
            QPolygonF polygon;
            for (const auto &p : r.value("points").toList())
            {
                const auto pair = p.toList();
                polygon << QPointF(pair[0].toDouble(), pair[1].toDouble());
            }
            hit = polygon.containsPoint(point, Qt::OddEvenFill);
        }
        else
        {
            const auto coords = r.value("rect").toList();
            QRectF rect(coords[0].toDouble(), coords[1].toDouble(), coords[2].toDouble(),
                        coords[3].toDouble());
            if (shape == "rect")
                hit = rect.contains(point);
            else
            {
                const auto delta = point - rect.center();
                hit = std::pow(delta.x() / (rect.width() / 2), 2) +
                          std::pow(delta.y() / (rect.height() / 2), 2) <=
                      1;
            }
        }
        if (hit)
            return {{"id", r.value("id")}, {"logicalPosition", point}};
    }
    return {};
}
bool MascotPetController::activateRegion(const QPointF &position)
{
    const auto hit = hitTest(position);
    if (hit.isEmpty())
        return false;
    emit regionActivated(hit.value("id").toString(), hit.value("logicalPosition").toPointF());
    return true;
}

QUrl MascotPetController::fallbackSource() const
{
    return d->package.fallbackImage;
}
void MascotPetController::attachVectorItem(VectorItem *item)
{
    if (d->vectorItem == item)
        return;
    if (d->vectorItem)
    {
        disconnect(d->vectorItem, nullptr, this, nullptr);
        d->vectorItem->stop();
        d->vectorItem->setSource({});
    }
    d->vectorItem = item;
    if (!item)
    {
        if (d->package.renderer == "lottie" && d->status == Ready)
        {
            d->status = Loading;
            emit statusChanged();
        }
        return;
    }
    item->setAutoPlay(false);
    connect(item, &VectorItem::statusChanged, this, [this] { d->vectorStatus(); });
    connect(item, &VectorItem::finished, this,
            [this]
            {
                if (d->package.renderer == "lottie" && d->status == Ready && d->vectorItem &&
                    !d->action.isEmpty() && d->vectorItem->source() == d->sprite().source)
                    d->complete();
            });
    if (d->package.renderer == "lottie" && !d->action.isEmpty() && d->status != Error)
    {
        item->setLoops(d->active.loops);
        if (d->reloadVectorOnStart && item->source() == d->sprite().source)
            item->reload();
        else
            item->setSource(d->sprite().source);
        d->reloadVectorOnStart = false;
        d->syncTimer();
        item->play();
        d->vectorStatus();
    }
}
void MascotPetController::renderError(const QUrl &source, const QString &message)
{
    if (d->action.isEmpty() || source != d->sprite().source || d->status == Error)
        return;
    const auto epoch = d->generation;
    d->status = Error;
    d->code = "RenderFailed";
    d->error = message;
    d->cancel("Failed");
    if (epoch != d->generation)
        return;
    if (d->vectorItem)
        d->vectorItem->stop();
    emit currentActionChanged();
    emit anchorsChanged();
    emit spriteFrameChanged();
    emit regionsChanged();
    emit statusChanged();
}
MascotRenderAdapter::~MascotRenderAdapter()
{
    if (m_controller)
        m_controller->attachVectorItem(nullptr);
}
void MascotRenderAdapter::setController(MascotPetController *controller)
{
    if (controller == m_controller)
        return;
    if (m_controller)
        m_controller->attachVectorItem(nullptr);
    m_controller = controller;
    if (m_controller)
        m_controller->attachVectorItem(m_item);
    emit controllerChanged();
}
void MascotRenderAdapter::setVectorItem(VectorItem *item)
{
    if (item == m_item)
        return;
    m_item = item;
    if (m_controller)
        m_controller->attachVectorItem(item);
    emit vectorItemChanged();
}
void MascotRenderAdapter::spriteError(const QUrl &source, const QString &message)
{
    if (!m_controller)
        return;
    const auto epoch = m_controller->d->generation;
    QPointer<MascotPetController> controller = m_controller;
    QTimer::singleShot(0, m_controller,
                       [controller, epoch, source, message]
                       {
                           if (controller && controller->d->generation == epoch)
                               controller->renderError(source, message);
                       });
}
} // namespace QCatUiKit
