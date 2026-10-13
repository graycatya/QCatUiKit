// SPDX-License-Identifier: Apache-2.0
#include "qcatledoutput.h"
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QSet>
#include <QThreadPool>
#include <QTimer>
#include <cmath>

namespace
{
QStringList channels(int schema)
{
    return schema == 3   ? QStringList{"R", "G", "B"}
           : schema == 4 ? QStringList{"R", "G", "B", "W"}
                         : QStringList{"R", "G", "B", "CW", "WW"};
}
bool integer(const QVariantMap &c, const char *key, int min, int max)
{
    bool ok;
    const double value = c.value(key).toDouble(&ok);
    return ok && std::isfinite(value) && std::floor(value) == value && value >= min && value <= max;
}
class MemoryConsumer final : public QCatOutputBackend
{
  public:
    mutable QMutex mutex;
    QCatDeviceFramePtr latest;
    quint64 count = 0;
    bool submit(QCatDeviceFramePtr f, const std::shared_ptr<std::atomic_bool> &cancelled,
                QString *) override
    {
        QMutexLocker lock(&mutex);
        if (cancelled->load())
            return false;
        latest = std::move(f);
        ++count;
        return true;
    }
    void cancel() override {}
};
struct OutputJob
{
    QMutex mutex;
    bool done = false;
    QString error;
};
} // namespace
QVariantMap QCatFixtureProfile::configuration() const
{
    return {{"id", id},
            {"deviceId", deviceId},
            {"portId", portId},
            {"schema", int(schema)},
            {"channelOrder", channelOrder},
            {"whiteKelvin", whiteKelvin},
            {"coldKelvin", coldKelvin},
            {"warmKelvin", warmKelvin},
            {"conversion", int(conversion)},
            {"firstPoint", firstPoint},
            {"pointCount", pointCount},
            {"startChannel", startChannel}};
}
bool QCatFixtureProfile::parse(const QVariantMap &c, QCatFixtureProfile *p, QString *error)
{
    auto fail = [&](const QString &e)
    {
        if (error)
            *error = e;
        return false;
    };
    if (c["id"].toString().isEmpty() || c["deviceId"].toString().isEmpty() ||
        c["portId"].toString().isEmpty())
        return fail(QStringLiteral("Profile requires id, deviceId and portId"));
    if (!integer(c, "schema", 3, 5) ||
        !integer(c, "firstPoint", 0, QCatLedLayoutModel::MaximumPoints) ||
        !integer(c, "pointCount", 0, QCatLedLayoutModel::MaximumPoints) ||
        !integer(c, "startChannel", 1, 65536) || !integer(c, "conversion", 0, 2))
        return fail(
            QStringLiteral("Profile schema, point range, address or conversion is invalid"));
    const auto order = c["channelOrder"].toStringList();
    const auto semantic = channels(c["schema"].toInt());
    if (order.size() != semantic.size() || QSet<QString>(order.begin(), order.end()) !=
                                               QSet<QString>(semantic.begin(), semantic.end()))
        return fail(QStringLiteral("channelOrder must contain each semantic channel exactly once"));
    for (auto key : {"whiteKelvin", "coldKelvin", "warmKelvin"})
    {
        bool ok;
        double v = c.value(key).toDouble(&ok);
        if (!ok || !std::isfinite(v) || v < 1000 || v > 20000)
            return fail(QStringLiteral("%1: require 1,000…20,000 K").arg(key));
    }
    if (c["coldKelvin"].toDouble() <= c["warmKelvin"].toDouble())
        return fail(QStringLiteral("coldKelvin must exceed warmKelvin"));
    p->id = c["id"].toString();
    p->deviceId = c["deviceId"].toString();
    p->portId = c["portId"].toString();
    p->schema = QCatLighting::ColorSchema(c["schema"].toInt());
    p->channelOrder = order;
    p->firstPoint = c["firstPoint"].toInt();
    p->pointCount = c["pointCount"].toInt();
    p->startChannel = c["startChannel"].toInt();
    p->conversion = QCatLighting::Conversion(c["conversion"].toInt());
    p->whiteKelvin = c["whiteKelvin"].toDouble();
    p->coldKelvin = c["coldKelvin"].toDouble();
    p->warmKelvin = c["warmKelvin"].toDouble();
    if (error)
        error->clear();
    return true;
}
QString QCatPatchMapper::validate(const QVariantList &list, int count)
{
    if (list.size() > 256)
        return QStringLiteral("Profile count exceeds 256");
    QSet<QString> ids;
    QHash<QPair<QString, QString>, QVector<QPair<int, int>>> ranges;
    for (const auto &v : list)
    {
        QCatFixtureProfile p;
        QString error;
        if (!QCatFixtureProfile::parse(v.toMap(), &p, &error))
            return error;
        if (ids.contains(p.id))
            return QStringLiteral("Duplicate Profile ID");
        ids.insert(p.id);
        if (qint64(p.firstPoint) + p.pointCount > count)
            return QStringLiteral("Profile point range exceeds layout");
        const qint64 end = qint64(p.startChannel) + qint64(p.pointCount) * int(p.schema);
        if (end > 65537)
            return QStringLiteral("Port address exceeds 65,536-channel budget");
        auto &port = ranges[{p.deviceId, p.portId}];
        for (const auto &r : port)
            if (p.pointCount && p.startChannel < r.second && end > r.first)
                return QStringLiteral("Overlapping device channel addresses");
        if (p.pointCount)
            port.append({p.startChannel, int(end)});
    }
    if (ranges.size() > 64)
        return QStringLiteral("Port count exceeds 64");
    return {};
}
QCatDeviceFramePtr QCatPatchMapper::map(const QCatLightingFrame &f, const QCatLedLayoutPtr &l,
                                        const QVariantList &profiles, quint64 revision,
                                        QString *error)
{
    auto fail = [&](const QString &e) -> QCatDeviceFramePtr
    {
        if (error)
            *error = e;
        return {};
    };
    if (!revision || !f.matches(l))
        return fail(QStringLiteral("Output frame/layout/mapping version mismatch"));
    const auto e = validate(profiles, f.description().pointCount);
    if (!e.isEmpty())
        return fail(e);
    auto result = std::make_shared<QCatDeviceChannelFrame>();
    result->projectId = f.description().projectId;
    result->layoutRevision = f.description().layoutRevision;
    result->mappingRevision = revision;
    result->frameId = f.description().frameId;
    result->timestampUsec = f.description().timestampUsec;
    const auto bytes = f.opaqueChannels();
    QHash<QPair<QString, QString>, int> portIndex;
    for (const auto &v : profiles)
    {
        QCatFixtureProfile p;
        QCatFixtureProfile::parse(v.toMap(), &p, nullptr);
        QString message;
        const auto source = bytes.mid(qsizetype(p.firstPoint) * f.description().stride,
                                      qsizetype(p.pointCount) * f.description().stride);
        const auto converted =
            QCatLighting::convert(source, f.description().schema, p.schema, p.conversion, &message,
                                  p.whiteKelvin, p.coldKelvin, p.warmKelvin);
        if (!message.isEmpty())
            return fail(message);
        const QPair<QString, QString> key{p.deviceId, p.portId};
        if (!portIndex.contains(key))
        {
            portIndex[key] = int(result->ports.size());
            result->ports.append({p.deviceId, p.portId, 1, {}});
        }
        auto &out = result->ports[portIndex[key]].channels;
        const int end = p.startChannel - 1 + p.pointCount * int(p.schema);
        if (out.size() < end)
            out.append(QByteArray(end - out.size(), '\0'));
        const auto semantic = channels(p.schema);
        for (int i = 0; i < p.pointCount; ++i)
            for (int c = 0; c < int(p.schema); ++c)
                out[p.startChannel - 1 + i * int(p.schema) + c] =
                    converted[i * int(p.schema) + semantic.indexOf(p.channelOrder[c])];
    }
    if (error)
        error->clear();
    return result;
}
QCatOutputBackend::~QCatOutputBackend() = default;
struct QCatMemoryOutputBackend::Impl
{
    std::shared_ptr<MemoryConsumer> backend = std::make_shared<MemoryConsumer>();
    QTimer timer;
    quint64 lastCount = 0;
};
QCatMemoryOutputBackend::QCatMemoryOutputBackend(QObject *p) : QObject(p), d(new Impl)
{
    d->timer.setInterval(250);
    connect(&d->timer, &QTimer::timeout, this,
            [this]
            {
                const auto count = receivedFrames();
                if (count != d->lastCount)
                {
                    d->lastCount = count;
                    emit diagnosticsChanged();
                }
            });
    d->timer.start();
}
QCatMemoryOutputBackend::~QCatMemoryOutputBackend() = default;
std::shared_ptr<QCatOutputBackend> QCatMemoryOutputBackend::consumer() const
{
    return d->backend;
}
QCatDeviceFramePtr QCatMemoryOutputBackend::latestFrame() const
{
    QMutexLocker lock(&d->backend->mutex);
    return d->backend->latest;
}
quint64 QCatMemoryOutputBackend::receivedFrames() const
{
    QMutexLocker lock(&d->backend->mutex);
    return d->backend->count;
}
quint64 QCatMemoryOutputBackend::frameId() const
{
    auto f = latestFrame();
    return f ? f->frameId : 0;
}
quint64 QCatMemoryOutputBackend::mappingRevision() const
{
    auto f = latestFrame();
    return f ? f->mappingRevision : 0;
}
QVariantList QCatMemoryOutputBackend::ports() const
{
    QVariantList out;
    auto frame = latestFrame();
    if (frame)
        for (const auto &p : frame->ports)
        {
            QVariantList values;
            for (auto v : p.channels.left(32))
                values << int(uchar(v));
            out << QVariantMap{{"deviceId", p.deviceId},
                               {"portId", p.portId},
                               {"startChannel", p.startChannel},
                               {"channelCount", p.channels.size()},
                               {"values", values}};
        }
    return out;
}
struct QCatLedOutput::Impl
{
    QPointer<QCatLedLayoutModel> observedLayout;
    QMetaObject::Connection layoutConnection;
    QPointer<QCatLedMixer> mixer;
    QPointer<QCatMemoryOutputBackend> memory;
    std::shared_ptr<QCatOutputBackend> backend;
    QVariantList profiles;
    bool enabled = true, repeat = true;
    QTimer timer;
    QString error;
    quint64 revision = 1, lastFrame = 0, layoutRevision = 0;
    QString projectId;
    std::shared_ptr<std::atomic_bool> cancelled = std::make_shared<std::atomic_bool>(false);
    std::shared_ptr<OutputJob> job;
};
QCatLedOutput::QCatLedOutput(QObject *p) : QObject(p), d(new Impl)
{
    d->timer.setInterval(25);
    connect(&d->timer, &QTimer::timeout, this, &QCatLedOutput::tick);
    d->timer.start();
}
QCatLedOutput::~QCatLedOutput()
{
    cancel();
}
QCatLedMixer *QCatLedOutput::mixer() const
{
    return d->mixer;
}
void QCatLedOutput::setMixer(QCatLedMixer *m)
{
    cancel();
    if (d->mixer)
        disconnect(d->mixer, nullptr, this, nullptr);
    disconnect(d->layoutConnection);
    d->observedLayout = nullptr;
    d->mixer = m;
    d->layoutRevision = 0;
    auto observe = [this]
    {
        auto *layout = d->mixer ? d->mixer->layout() : nullptr;
        if (layout == d->observedLayout)
            return;
        disconnect(d->layoutConnection);
        d->observedLayout = layout;
        cancel();
        if (layout)
            d->layoutConnection = connect(layout, &QCatLedLayoutModel::layoutChanged, this,
                                          [this]
                                          {
                                              cancel();
                                              ++d->revision;
                                              if (d->observedLayout)
                                              {
                                                  auto l = d->observedLayout->snapshot();
                                                  d->layoutRevision = l->revision;
                                                  d->projectId = l->projectId;
                                              }
                                          });
    };
    if (m)
        connect(m, &QCatLedMixer::settingsChanged, this, observe);
    observe();
    emit settingsChanged();
}

QCatMemoryOutputBackend *QCatLedOutput::memoryBackend() const
{
    return d->memory;
}
void QCatLedOutput::setMemoryBackend(QCatMemoryOutputBackend *b)
{
    d->memory = b;
    setBackend(b ? b->consumer() : nullptr);
    emit settingsChanged();
}
void QCatLedOutput::setBackend(std::shared_ptr<QCatOutputBackend> b)
{
    cancel();
    d->backend = std::move(b);
}
QVariantList QCatLedOutput::profiles() const
{
    return d->profiles;
}
bool QCatLedOutput::applyProfiles(const QVariantList &profiles)
{
    const int count = d->mixer && d->mixer->layout() ? d->mixer->layout()->pointCount() : 0;
    const auto e = QCatPatchMapper::validate(profiles, count);
    if (!e.isEmpty())
    {
        d->error = e;
        emit statusChanged();
        return false;
    }
    cancel();
    d->profiles = profiles;
    ++d->revision;
    d->error.clear();
    emit settingsChanged();
    emit statusChanged();
    return true;
}
bool QCatLedOutput::enabled() const
{
    return d->enabled;
}
void QCatLedOutput::setEnabled(bool v)
{
    if (!v)
        cancel();
    d->enabled = v;
    emit settingsChanged();
}
int QCatLedOutput::interval() const
{
    return d->timer.interval();
}
void QCatLedOutput::setInterval(int v)
{
    if (v < 4 || v > 10000)
        return;
    d->timer.setInterval(v);
    emit settingsChanged();
}
bool QCatLedOutput::repeatFrames() const
{
    return d->repeat;
}
void QCatLedOutput::setRepeatFrames(bool v)
{
    d->repeat = v;
    emit settingsChanged();
}
QString QCatLedOutput::lastError() const
{
    return d->error;
}
void QCatLedOutput::cancel()
{
    d->cancelled->store(true);
    if (d->backend)
        d->backend->cancel();
    d->cancelled = std::make_shared<std::atomic_bool>(false);
    d->lastFrame = 0;
}
void QCatLedOutput::tick()
{
    if (d->job)
    {
        bool done;
        QString error;
        {
            QMutexLocker lock(&d->job->mutex);
            done = d->job->done;
            error = d->job->error;
        }
        if (!done)
            return;
        d->job.reset();
        d->error = error;
        emit statusChanged();
    }
    if (!d->enabled || !d->backend || !d->mixer || !d->mixer->layout())
        return;
    auto layout = d->mixer->layout()->snapshot();
    if (layout->revision != d->layoutRevision || layout->projectId != d->projectId)
    {
        cancel();
        ++d->revision;
        d->layoutRevision = layout->revision;
        d->projectId = layout->projectId;
    }
    const auto frame = d->mixer->latestFrame();
    if (!frame.matches(layout))
        return;
    if (!d->repeat && frame.description().frameId == d->lastFrame)
        return;
    auto job = std::make_shared<OutputJob>();
    const auto backend = d->backend;
    const auto profiles = d->profiles;
    const auto revision = d->revision;
    const auto cancelled = d->cancelled;
    d->job = job;
    d->lastFrame = frame.description().frameId;
    QThreadPool::globalInstance()->start(
        [job, backend, frame, layout, profiles, revision, cancelled]
        {
            QString error;
            if (!cancelled->load())
            {
                auto mapped = QCatPatchMapper::map(frame, layout, profiles, revision, &error);
                if (mapped && !cancelled->load())
                    backend->submit(std::move(mapped), cancelled, &error);
            }
            QMutexLocker lock(&job->mutex);
            job->error = error;
            job->done = true;
        });
}
QVariantMap QCatLedOutput::configuration() const
{
    return {{"version", 1},
            {"profiles", d->profiles},
            {"interval", interval()},
            {"repeatFrames", d->repeat}};
}
bool QCatLedOutput::restoreConfiguration(const QVariantMap &c)
{
    if (c["version"].toInt() != 1 || !integer(c, "interval", 4, 10000))
        return false;
    if (!applyProfiles(c["profiles"].toList()))
        return false;
    setInterval(c["interval"].toInt());
    setRepeatFrames(c["repeatFrames"].toBool());
    return true;
}
