// SPDX-License-Identifier: Apache-2.0
#include "qcatleddeck.h"
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QThreadPool>
#include <QTimer>
#include <cmath>

namespace
{
struct DeckSelection
{
    QVariantMap place, parameters;
    QString storage, id, location, repositoryId;
    QPointer<QCatLedStorageRepository> repository;
    std::shared_ptr<QCatLedEffectInstance> instance;
    quint64 token = 0;
    double time = 0;
    bool activate = false, editing = false, keepStopped = false;
};
struct DeckJob
{
    QMutex mutex;
    bool done = false;
    QByteArray bytes;
    QString error;
    QCatLedLayoutPtr layout;
    std::shared_ptr<DeckSelection> selection;
    quint64 token = 0;
};
} // namespace
struct QCatLedDeck::Impl
{
    QPointer<QCatLedLayoutModel> layout;
    QPointer<QCatLedEffectRegistry> registry;
    QPointer<QCatLedStorageRepository> repo;
    QCatPreviewColorAdapter *adapter = nullptr;
    std::shared_ptr<DeckSelection> active, pending;
    std::shared_ptr<DeckJob> job;
    QString browsing, error;
    double time = 0, speed = 1, brightness = 1;
    bool playing = false, stopped = true, dirty = true;
    quint64 token = 0, sequence = 0;
    QTimer timer;
    QElapsedTimer clock;
    mutable QMutex frameMutex;
    QCatLightingFrame frame;
};
QCatLedDeck::QCatLedDeck(QObject *p) : QObject(p), d(new Impl)
{
    d->adapter = new QCatPreviewColorAdapter(this);
    d->clock.start();
    d->timer.setInterval(16);
    connect(&d->timer, &QTimer::timeout, this, &QCatLedDeck::tick);
    d->timer.start();
}
QCatLedDeck::~QCatLedDeck() = default;
QCatLedLayoutModel *QCatLedDeck::layout() const
{
    return d->layout;
}
void QCatLedDeck::setLayout(QCatLedLayoutModel *l)
{
    if (l == d->layout)
        return;
    if (d->layout)
        disconnect(d->layout, nullptr, this, nullptr);
    d->layout = l;
    if (l)
    {
        connect(l, &QCatLedLayoutModel::layoutChanged, this, &QCatLedDeck::layoutUpdated);
        connect(l, &QObject::destroyed, this, &QCatLedDeck::layoutUpdated);
    }
    layoutUpdated();
    emit settingsChanged();
}
QCatLedEffectRegistry *QCatLedDeck::registry() const
{
    return d->registry;
}
void QCatLedDeck::setRegistry(QCatLedEffectRegistry *r)
{
    d->registry = r;
    emit settingsChanged();
}
QCatLedStorageRepository *QCatLedDeck::repository() const
{
    return d->repo;
}
void QCatLedDeck::setRepository(QCatLedStorageRepository *r)
{
    if (r == d->repo)
        return;
    d->repo = r;
    if (d->active && r && d->active->repositoryId == r->repositoryId())
        d->active->repository = r;
    d->browsing =
        r && !r->storages().isEmpty() ? r->storages()[0].toMap()["id"].toString() : QString{};
    emit settingsChanged();
    emit stateChanged();
}
QCatLedSource *QCatLedDeck::source() const
{
    return d->adapter->source();
}
bool QCatLedDeck::playing() const
{
    return d->playing;
}
bool QCatLedDeck::loading() const
{
    return bool(d->pending);
}
double QCatLedDeck::effectTime() const
{
    return d->time;
}
double QCatLedDeck::speed() const
{
    return d->speed;
}
double QCatLedDeck::brightness() const
{
    return d->brightness;
}
void QCatLedDeck::setSpeed(double v)
{
    if (!std::isfinite(v) || v < -8 || v > 8)
        return;
    if (v < 0 && d->active && d->registry &&
        !d->registry
             ->effect(d->active->place["providerId"].toString(),
                      d->active->place["effectId"].toString())["reverse"]
             .toBool())
    {
        d->error = QStringLiteral("Provider does not support reverse playback");
        emit stateChanged();
        return;
    }
    advanceClock();
    d->speed = v;
    emit settingsChanged();
}
void QCatLedDeck::setBrightness(double v)
{
    if (!std::isfinite(v) || v < 0 || v > 1)
        return;
    d->brightness = v;
    ++d->token;
    d->dirty = true;
    if (d->pending)
    {
        auto replacement = std::make_shared<DeckSelection>(*d->pending);
        replacement->token = d->token;
        d->pending = replacement;
    }
    emit settingsChanged();
}
QString QCatLedDeck::browsingStorage() const
{
    return d->browsing;
}
QString QCatLedDeck::activeStorage() const
{
    return d->active ? d->active->storage : QString{};
}
QString QCatLedDeck::activePlace() const
{
    return d->active ? d->active->id : QString{};
}
QVariantMap QCatLedDeck::workingPlace() const
{
    return d->active ? d->active->place : QVariantMap{};
}
QVariantList QCatLedDeck::parameterDefinitions() const
{
    if (!d->active || !d->registry)
        return {};
    const auto p = d->active->place;
    auto list =
        d->registry->effect(p["providerId"].toString(), p["effectId"].toString())["parameters"]
            .toList();
    QVariantList out;
    for (const auto &v : list)
    {
        auto m = v.toMap();
        const auto id = m["id"].toString();
        const int s = p["schema"].toInt();
        if ((id == "W" && s != 4) || ((id == "CW" || id == "WW") && s != 5))
            continue;
        if (id == "channel")
            m["max"] = s - 1;
        out << m;
    }
    return out;
}
QString QCatLedDeck::lastError() const
{
    return d->error;
}
QCatLightingFrame QCatLedDeck::latestFrame() const
{
    QMutexLocker lock(&d->frameMutex);
    return d->frame;
}
void QCatLedDeck::browseStorage(const QString &s)
{
    if (d->repo)
        for (const auto &v : d->repo->storages())
            if (v.toMap()["id"] == s)
            {
                d->browsing = s;
                emit stateChanged();
                return;
            }
}
bool QCatLedDeck::prepare(const QVariantMap &p, const QString &s, const QString &id, bool activate,
                          bool editing, QCatLedStorageRepository *origin)
{
    if (!d->registry || !d->layout)
    {
        d->error = QStringLiteral("Deck requires a layout and EffectRegistry");
        emit stateChanged();
        return false;
    }
    if (p.value("configVersion", 1).toInt() != 1)
    {
        d->error = QStringLiteral("Unsupported Place configuration version");
        emit stateChanged();
        return false;
    }
    if (d->speed < 0 && !d->registry->effect(p["providerId"].toString(), p["effectId"].toString())
                             .value("reverse", false)
                             .toBool())
    {
        d->error = QStringLiteral("Provider does not support negative Deck speed");
        emit stateChanged();
        return false;
    }
    QVariantMap params;
    QString error;
    auto instance = d->registry->prepare(p, d->layout->snapshot(), &params, &error);
    if (!instance)
    {
        d->error = error;
        emit stateChanged();
        return false;
    }
    advanceClock();
    auto selection = std::make_shared<DeckSelection>();
    selection->place = p;
    selection->parameters = params;
    selection->place["parameters"] = params;
    selection->repository =
        origin ? origin : (editing && d->active ? d->active->repository.data() : d->repo.data());
    selection->repositoryId =
        editing && d->active
            ? d->active->repositoryId
            : (selection->repository ? selection->repository->repositoryId() : QString{});
    if (editing && d->active)
        selection->location = d->active->location;
    else if (selection->repository && !s.isEmpty())
    {
        const auto storages = selection->repository->storages();
        const auto places = selection->repository->places(s);
        for (int i = 0; i < storages.size(); ++i)
            if (storages[i].toMap()["id"] == s)
                for (int j = 0; j < places.size(); ++j)
                    if (places[j].toMap()["id"] == id)
                        selection->location = tr("Storage %1: %2 · Place %3")
                                                  .arg(i + 1)
                                                  .arg(storages[i].toMap()["name"].toString())
                                                  .arg(j + 1);
    }
    selection->storage = s;
    selection->id = id;
    selection->activate = activate;
    selection->editing = editing;
    selection->keepStopped = editing && d->stopped;
    selection->time = editing ? d->time : 0;
    selection->instance = std::move(instance);
    selection->token = ++d->token;
    d->pending = selection;
    d->error.clear();
    emit stateChanged();
    tick();
    return true;
}
bool QCatLedDeck::loadPlace(const QString &s, const QString &p)
{
    return prepare(d->repo ? d->repo->place(s, p) : QVariantMap{}, s, p, false);
}
bool QCatLedDeck::activatePlace(const QString &s, const QString &p)
{
    return prepare(d->repo ? d->repo->place(s, p) : QVariantMap{}, s, p, true);
}
bool QCatLedDeck::loadConfiguration(const QVariantMap &p, bool activate)
{
    return prepare(p, {}, p["id"].toString(), activate);
}
bool QCatLedDeck::editPlace(const QVariantMap &changes)
{
    if (!d->active)
        return false;
    auto p = d->pending && d->pending->editing ? d->pending->place : d->active->place;
    for (auto it = changes.begin(); it != changes.end(); ++it)
        p[it.key()] = it.value();
    return prepare(p, activeStorage(), activePlace(), d->playing, true);
}
bool QCatLedDeck::setParameter(const QString &name, double value)
{
    if (!d->active)
        return false;
    auto params = d->pending ? d->pending->parameters : d->active->parameters;
    params[name] = value;
    return editPlace({{"parameters", params}});
}
bool QCatLedDeck::selectEffect(const QString &p, const QString &e, int schema)
{
    if (!d->registry)
        return false;
    const auto effect = d->registry->effect(p, e);
    return prepare({{"name", effect["name"]},
                    {"description", QString()},
                    {"providerId", p},
                    {"effectId", e},
                    {"effectVersion", effect["version"]},
                    {"schema", schema},
                    {"parameters", QVariantMap{}},
                    {"brightness", 1},
                    {"speed", 1},
                    {"conversion", 2},
                    {"empty", false},
                    {"configVersion", 1}},
                   {}, {}, true);
}
void QCatLedDeck::advanceClock()
{
    const double dt = d->clock.nsecsElapsed() / 1e9;
    d->clock.restart();
    if (d->playing && d->active)
    {
        const double increment = dt * d->speed * d->active->place.value("speed", 1).toDouble();
        if (increment != 0)
        {
            d->time += increment;
            d->dirty = true;
        }
    }
}
void QCatLedDeck::play()
{
    advanceClock();
    if (!d->active)
        return;
    d->playing = true;
    d->stopped = false;
    d->dirty = true;
    emit stateChanged();
}
void QCatLedDeck::pause()
{
    advanceClock();
    d->playing = false;
    ++d->token;
    d->dirty = true;
    if (d->pending)
    {
        auto replacement = std::make_shared<DeckSelection>(*d->pending);
        replacement->activate = false;
        replacement->token = d->token;
        d->pending = replacement;
    }
    emit stateChanged();
}
void QCatLedDeck::stop()
{
    advanceClock();
    d->playing = false;
    d->stopped = true;
    d->time = 0;
    d->pending.reset();
    ++d->token;
    d->dirty = false;
    const auto schema = d->active ? QCatLighting::ColorSchema(d->active->place["schema"].toInt())
                                  : QCatLighting::RGB_CW_WW;
    if (d->layout)
        publish(QByteArray(d->layout->pointCount() * int(schema), '\0'), schema, d->token);
    emit stateChanged();
}
void QCatLedDeck::unload()
{
    stop();
    d->active.reset();
    emit stateChanged();
}
void QCatLedDeck::restart()
{
    if (!d->active)
        return;
    d->time = 0;
    d->clock.restart();
    d->playing = true;
    d->stopped = false;
    ++d->token;
    d->dirty = true;
    emit stateChanged();
}
bool QCatLedDeck::adjacent(int direction)
{
    if (!d->active || !d->active->repository || !d->registry)
        return false;
    const auto ps = d->active->repository->places(activeStorage());
    int index = -1;
    for (int i = 0; i < ps.size(); ++i)
        if (ps[i].toMap()["id"] == activePlace())
        {
            index = i;
            break;
        }
    if (index < 0)
        return false;
    for (int i = index + direction; i >= 0 && i < ps.size(); i += direction)
    {
        const auto p = ps[i].toMap();
        if (d->registry->validatePlace(p).isEmpty())
            return prepare(p, activeStorage(), p["id"].toString(), true, false,
                           d->active->repository);
    }
    return false;
}
bool QCatLedDeck::previous()
{
    return adjacent(-1);
}
bool QCatLedDeck::next()
{
    return adjacent(1);
}
bool QCatLedDeck::savePlace(const QString &s, const QString &p, const QString &n)
{
    return saveTo(d->repo, s, p, n);
}
bool QCatLedDeck::saveTo(QCatLedStorageRepository *r, const QString &s, const QString &p,
                         const QString &n)
{
    if (!r || !d->active)
        return false;
    auto c = d->active->place;
    c["name"] = n;
    c["empty"] = false;
    const bool ok = r->savePlace(s, p, c);
    d->error = ok ? QString{} : r->lastError();
    emit stateChanged();
    return ok;
}
void QCatLedDeck::layoutUpdated()
{
    ++d->token;
    const bool pending = bool(d->pending);
    const auto selected = pending ? d->pending : d->active;
    d->pending.reset();
    d->adapter->setLayout(d->layout ? d->layout->snapshot() : nullptr);
    if (selected && d->layout)
    {
        if (!prepare(selected->place, selected->storage, selected->id,
                     pending ? selected->activate : d->playing, pending ? selected->editing : true,
                     selected->repository))
        {
            d->playing = false;
            d->stopped = true;
        }
    }
    const auto schema = selected ? QCatLighting::ColorSchema(selected->place["schema"].toInt())
                                 : QCatLighting::RGB_CW_WW;
    if (d->layout)
        publish(QByteArray(d->layout->pointCount() * int(schema), '\0'), schema, d->token);
    else
    {
        QMutexLocker lock(&d->frameMutex);
        d->frame = {};
    }
    emit stateChanged();
}

void QCatLedDeck::publish(const QByteArray &data, QCatLighting::ColorSchema schema,
                          quint64 instance)
{
    if (!d->layout)
        return;
    const auto layout = d->layout->snapshot();
    QCatLightingFrame::Description info;
    info.projectId = layout->projectId;
    info.layoutRevision = layout->revision;
    info.pointCount = int(layout->positions.size());
    info.frameId = ++d->sequence;
    info.instanceId = instance;
    info.timestampUsec = qRound64(d->time * 1e6);
    info.schema = schema;
    info.stride = int(schema);
    QString error;
    auto frame = QCatLightingFrame::create(info, data, {}, &error);
    if (!frame.isValid())
    {
        d->error = error;
        emit stateChanged();
        return;
    }
    {
        QMutexLocker lock(&d->frameMutex);
        d->frame = frame;
    }
    d->adapter->submit(frame);
    emit frameReady();
}
void QCatLedDeck::tick()
{
    advanceClock();
    if (d->job)
    {
        auto job = d->job;
        bool done;
        {
            QMutexLocker lock(&job->mutex);
            done = job->done;
        }
        if (!done)
            return;
        d->job.reset();
        const bool selected = job->selection && d->pending == job->selection;
        if (d->layout && job->layout == d->layout->snapshot() &&
            (selected || (!job->selection && job->token == d->token && !d->pending)))
        {
            if (!job->error.isEmpty())
            {
                d->error = job->error;
                if (selected)
                    d->pending.reset();
                emit stateChanged();
            }
            else
            {
                if (selected)
                {
                    d->active = job->selection;
                    d->pending.reset();
                    if (!d->active->editing)
                        d->time = d->active->time;
                    d->stopped = d->active->keepStopped;
                    d->playing = d->active->activate && !d->stopped;
                    d->clock.restart();
                    d->error.clear();
                    emit stateChanged();
                }
                if (d->active)
                    publish(d->stopped ? QByteArray(job->bytes.size(), char(0)) : job->bytes,
                            QCatLighting::ColorSchema(d->active->place["schema"].toInt()),
                            d->active->token);
            }
        }
    }
    if (!d->layout || (!d->pending && (!d->active || d->stopped || !d->dirty)))
        return;
    const auto selection = d->pending ? d->pending : d->active;
    auto job = std::make_shared<DeckJob>();
    job->layout = d->layout->snapshot();
    job->selection = d->pending;
    job->token = d->token;
    const auto params = selection->parameters;
    const auto schema = QCatLighting::ColorSchema(selection->place["schema"].toInt());
    const double time = d->pending ? selection->time : d->time,
                 gain = selection->place.value("brightness", 1).toDouble() * d->brightness;
    const auto instance = selection->instance;
    d->job = job;
    d->dirty = false;
    QThreadPool::globalInstance()->start(
        [job, instance, params, schema, time, gain]
        {
            QByteArray bytes;
            QString error;
            try
            {
                bytes = instance->render(*job->layout, params, schema, time);
                if (bytes.size() != job->layout->positions.size() * int(schema))
                    error = QStringLiteral("Provider returned an invalid channel buffer");
                else if (gain == 0)
                    bytes.fill(char(0));
                else if (gain != 1)
                    for (auto &v : bytes)
                        v = char(qRound(uchar(v) * gain));
            }
            catch (const std::exception &e)
            {
                error = QString::fromUtf8(e.what());
            }
            catch (...)
            {
                error = QStringLiteral("Provider render failed");
            }
            QMutexLocker lock(&job->mutex);
            job->bytes = std::move(bytes);
            job->error = error;
            job->done = true;
        });
}
QVariantMap QCatLedDeck::configuration() const
{
    return {{"version", 1},
            {"place", workingPlace()},
            {"repositoryId", activeRepositoryId()},
            {"activeLocation", activeLocation()},
            {"activeStorage", activeStorage()},
            {"activePlace", activePlace()},
            {"speed", d->speed},
            {"brightness", d->brightness}};
}
bool QCatLedDeck::restoreConfiguration(const QVariantMap &c)
{
    if (c["version"].toInt() != 1 || !std::isfinite(c["speed"].toDouble()) ||
        std::abs(c["speed"].toDouble()) > 8 || !std::isfinite(c["brightness"].toDouble()) ||
        c["brightness"].toDouble() < 0 || c["brightness"].toDouble() > 1)
        return false;
    if (c["place"].toMap().isEmpty())
    {
        unload();
        setSpeed(c["speed"].toDouble());
        setBrightness(c["brightness"].toDouble());
        return true;
    }
    if (!prepare(c["place"].toMap(), c["activeStorage"].toString(), c["activePlace"].toString(),
                 false))
        return false;
    if (d->pending && c.contains("repositoryId"))
    {
        d->pending->repositoryId = c["repositoryId"].toString();
        d->pending->location = c["activeLocation"].toString();
        if (d->pending->repository &&
            d->pending->repository->repositoryId() != d->pending->repositoryId)
            d->pending->repository = nullptr;
    }
    setSpeed(c["speed"].toDouble());
    setBrightness(c["brightness"].toDouble());
    return true;
}

int QCatLedDeck::productionInterval() const
{
    return d->timer.interval();
}
void QCatLedDeck::setProductionInterval(int ms)
{
    if (ms < 4 || ms > 1000)
        return;
    d->timer.setInterval(ms);
    emit settingsChanged();
}

QString QCatLedDeck::activeLocation() const
{
    return d->active ? d->active->location : QString{};
}

QString QCatLedDeck::activeRepositoryId() const
{
    return d->active ? d->active->repositoryId : QString{};
}
