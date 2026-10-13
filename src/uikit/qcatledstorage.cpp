// SPDX-License-Identifier: Apache-2.0
#include "qcatledstorage.h"
#include <QBuffer>
#include <QCache>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QSaveFile>
#include <QSet>
#include <QThreadPool>
#include <QTimer>
#include <QUuid>

namespace
{
struct ThumbnailJob
{
    QMutex mutex;
    bool done = false;
    quint64 generation = 0;
    QString key, result;
};
QString uid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
QVariantMap emptyPlace(const QString &id = QString())
{
    return {{"id", id.isEmpty() ? uid() : id},
            {"name", QString()},
            {"empty", true},
            {"configVersion", 1}};
}
int findId(const QVariantList &list, const QString &id)
{
    for (qsizetype i = 0; i < list.size(); ++i)
        if (list[i].toMap()["id"] == id)
            return int(i);
    return -1;
}
} // namespace
struct QCatLedStorageRepository::Impl
{
    QVariantMap doc{{"version", 1}, {"repositoryId", uid()}, {"storages", QVariantList{}}};
    bool readOnly = false;
    QString error;
    QCache<QString, QString> thumbnails{128};
    QVector<std::shared_ptr<ThumbnailJob>> jobs;
    QTimer thumbnailTimer;
    quint64 thumbnailRevision = 0, generation = 0;
};
QCatLedStorageRepository::QCatLedStorageRepository(QObject *p) : QObject(p), d(new Impl)
{
    d->thumbnailTimer.setInterval(50);
    connect(&d->thumbnailTimer, &QTimer::timeout, this,
            [this]
            {
                bool changed = false;
                for (int i = int(d->jobs.size()) - 1; i >= 0; --i)
                {
                    auto job = d->jobs[i];
                    QMutexLocker lock(&job->mutex);
                    if (!job->done)
                        continue;
                    if (job->generation == d->generation)
                        d->thumbnails.insert(job->key, new QString(job->result));
                    d->jobs.removeAt(i);
                    changed = true;
                }
                if (changed)
                {
                    ++d->thumbnailRevision;
                    emit thumbnailsChanged();
                }
                if (d->jobs.isEmpty())
                    d->thumbnailTimer.stop();
            });
}
quint64 QCatLedStorageRepository::thumbnailRevision() const
{
    return d->thumbnailRevision;
}
QCatLedStorageRepository::~QCatLedStorageRepository() = default;
QString QCatLedStorageRepository::repositoryId() const
{
    return d->doc["repositoryId"].toString();
}
QVariantList QCatLedStorageRepository::storages() const
{
    auto s = d->doc["storages"].toList();
    for (auto &v : s)
    {
        auto m = v.toMap();
        m.remove("places");
        v = m;
    }
    return s;
}
bool QCatLedStorageRepository::readOnly() const
{
    return d->readOnly;
}
void QCatLedStorageRepository::setReadOnly(bool v)
{
    if (v != d->readOnly)
    {
        d->readOnly = v;
        emit changed();
    }
}
QString QCatLedStorageRepository::lastError() const
{
    return d->error;
}
QVariantMap QCatLedStorageRepository::document() const
{
    return d->doc;
}
bool QCatLedStorageRepository::fail(const QString &e)
{
    d->error = e;
    emit statusChanged();
    return false;
}
QString QCatLedStorageRepository::validateDocument(const QVariantMap &doc)
{
    if (doc["version"].toInt() != 1 || doc["repositoryId"].toString().isEmpty())
        return QStringLiteral("Repository version/identity is invalid");
    if (doc["storages"].metaType().id() != QMetaType::QVariantList)
        return QStringLiteral("storages: require an array");
    const auto storages = doc["storages"].toList();
    if (storages.size() > 128)
        return QStringLiteral("Storage count exceeds 128");
    QSet<QString> ids;
    qsizetype total = 0;
    for (const auto &v : storages)
    {
        const auto s = v.toMap();
        const auto id = s["id"].toString();
        if (id.isEmpty() || ids.contains(id))
            return QStringLiteral("Duplicate/missing Storage ID");
        ids.insert(id);
        if (s["places"].metaType().id() != QMetaType::QVariantList)
            return QStringLiteral("places: require an array");
        const auto ps = s["places"].toList();
        total += ps.size();
        if (ps.size() > 4096 || total > 65536)
            return QStringLiteral("Place count exceeds memory budget");
        QSet<QString> places;
        for (const auto &pv : ps)
        {
            const auto p = pv.toMap();
            const auto pid = p["id"].toString();
            if (pid.isEmpty() || places.contains(pid))
                return QStringLiteral("Duplicate/missing Place ID");
            places.insert(pid);
            if (!p.value("empty", false).toBool() &&
                (!p.contains("parameters") ||
                 p["parameters"].metaType().id() != QMetaType::QVariantMap))
                return QStringLiteral("Place parameters must be an object");
        }
    }
    return {};
}
bool QCatLedStorageRepository::persist(const QVariantMap &, QString *)
{
    return true;
}
bool QCatLedStorageRepository::commit(QVariantMap c)
{
    if (d->readOnly)
        return fail(QStringLiteral("Repository is read-only; save to a writable repository"));
    auto e = validateDocument(c);
    if (!e.isEmpty())
        return fail(e);
    if (!persist(c, &e))
        return fail(e);
    d->doc = std::move(c);
    d->error.clear();
    d->thumbnails.clear();
    ++d->generation;
    ++d->thumbnailRevision;
    emit thumbnailsChanged();
    emit changed();
    emit statusChanged();
    return true;
}
bool QCatLedStorageRepository::replaceDocument(const QVariantMap &c)
{
    return commit(c);
}
QString QCatLedStorageRepository::createStorage(const QString &name, int count)
{
    if (count < 0 || count > 4096)
    {
        fail(QStringLiteral("slots: require 0…4096"));
        return {};
    }
    auto c = d->doc;
    auto ss = c["storages"].toList();
    QVariantList ps;
    for (int i = 0; i < count; ++i)
        ps << emptyPlace();
    const auto id = uid();
    ss << QVariantMap{{"id", id}, {"name", name}, {"places", ps}};
    c["storages"] = ss;
    return commit(c) ? id : QString{};
}
QVariantList QCatLedStorageRepository::places(const QString &s) const
{
    for (const auto &v : d->doc["storages"].toList())
    {
        auto m = v.toMap();
        if (m["id"] == s)
            return m["places"].toList();
    }
    return {};
}
QVariantMap QCatLedStorageRepository::place(const QString &s, const QString &p) const
{
    for (const auto &v : places(s))
    {
        auto m = v.toMap();
        if (m["id"] == p)
            return m;
    }
    return {};
}
bool QCatLedStorageRepository::savePlace(const QString &sid, const QString &pid,
                                         const QVariantMap &config)
{
    auto c = d->doc;
    auto ss = c["storages"].toList();
    const int i = findId(ss, sid);
    if (i < 0)
        return fail(QStringLiteral("Storage not found"));
    auto s = ss[i].toMap();
    auto ps = s["places"].toList();
    const int j = findId(ps, pid);
    if (j < 0)
        return fail(QStringLiteral("Place not found"));
    auto p = config;
    p["id"] = pid;
    if (!p.contains("configVersion"))
        p["configVersion"] = 1;
    ps[j] = p;
    s["places"] = ps;
    ss[i] = s;
    c["storages"] = ss;
    return commit(c);
}
bool QCatLedStorageRepository::clearPlace(const QString &s, const QString &p)
{
    return savePlace(s, p, emptyPlace(p));
}
bool QCatLedStorageRepository::renamePlace(const QString &s, const QString &p, const QString &name)
{
    auto c = place(s, p);
    if (c.isEmpty())
        return fail(QStringLiteral("Place not found"));
    c["name"] = name;
    return savePlace(s, p, c);
}
bool QCatLedStorageRepository::copyPlace(const QString &s, const QString &p, const QString &ts,
                                         const QString &tp)
{
    auto c = place(s, p);
    if (c.isEmpty())
        return fail(QStringLiteral("Place not found"));
    return savePlace(ts, tp, c);
}
QString QCatLedStorageRepository::exportJson(const QString &s, const QString &p) const
{
    QVariantMap root{{"version", 1}};
    if (s.isEmpty())
    {
        root["kind"] = "repository";
        root["data"] = d->doc;
    }
    else if (p.isEmpty())
    {
        const auto ss = d->doc["storages"].toList();
        int i = findId(ss, s);
        if (i < 0)
            return {};
        root["kind"] = "storage";
        root["data"] = ss[i];
    }
    else
    {
        auto config = place(s, p);
        if (config.isEmpty())
            return {};
        root["kind"] = "place";
        root["data"] = config;
    }
    return QString::fromUtf8(QJsonDocument::fromVariant(root).toJson(QJsonDocument::Indented));
}
bool QCatLedStorageRepository::importJson(const QString &json, const QString &target,
                                          const QString &slot)
{
    if (json.size() > 16 * 1024 * 1024)
        return fail(QStringLiteral("JSON exceeds 16 MiB budget"));
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(json.toUtf8(), &error);
    const auto root = document.object().toVariantMap();
    if (error.error != QJsonParseError::NoError || root["version"].toInt() != 1)
        return fail(QStringLiteral("Invalid JSON or unsupported format version"));
    const auto kind = root["kind"].toString();
    const auto data = root["data"].toMap();
    if (kind == "place")
        return savePlace(target, slot, data);
    QVariantList incoming;
    if (kind == "storage")
        incoming << data;
    else if (kind == "repository")
    {
        const auto e = validateDocument(data);
        if (!e.isEmpty())
            return fail(e);
        incoming = data["storages"].toList();
    }
    else
        return fail(QStringLiteral("Unknown import kind"));
    auto c = d->doc;
    auto ss = c["storages"].toList();
    for (const auto &v : incoming)
    {
        auto s = v.toMap();
        if (findId(ss, s["id"].toString()) >= 0)
        {
            s["id"] = uid();
            auto ps = s["places"].toList();
            for (auto &pv : ps)
            {
                auto p = pv.toMap();
                p["id"] = uid();
                pv = p;
            }
            s["places"] = ps;
        }
        ss << s;
    }
    c["storages"] = ss;
    return commit(c);
}
bool QCatLedStorageRepository::saveFile(const QUrl &url, const QString &s, const QString &p)
{
    if (!url.isLocalFile())
        return fail(QStringLiteral("Choose a local file URL"));
    QSaveFile f(url.toLocalFile());
    const auto bytes = exportJson(s, p).toUtf8();
    if (bytes.isEmpty() || !f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() ||
        !f.commit())
        return fail(f.errorString());
    d->error.clear();
    emit statusChanged();
    return true;
}
bool QCatLedStorageRepository::loadFile(const QUrl &url, const QString &s, const QString &p)
{
    if (!url.isLocalFile())
        return fail(QStringLiteral("Choose a local file URL"));
    QFile f(url.toLocalFile());
    if (!f.open(QIODevice::ReadOnly) || f.size() > 16 * 1024 * 1024)
        return fail(QStringLiteral("Cannot read file or file exceeds 16 MiB"));
    return importJson(QString::fromUtf8(f.readAll()), s, p);
}
QString QCatLedStorageRepository::thumbnail(const QString &s, const QString &p,
                                            QCatLedEffectRegistry *registry)
{
    if (!registry)
        return {};
    const auto key = s + "/" + p;
    if (auto *cached = d->thumbnails.object(key))
        return *cached;
    const auto config = place(s, p);
    const auto reference = config.value("thumbnail").toString();
    if (!reference.isEmpty())
        return reference;
    if (d->jobs.size() >= 2)
        return {};
    for (const auto &job : d->jobs)
        if (job->key == key && job->generation == d->generation)
            return {};
    auto c = QCatLedLayoutModel::defaults();
    c["countX"] = 32;
    c["countY"] = 16;
    auto layout = QCatLedLayoutModel::generate(c, "thumbnail", 1);
    QVariantMap params;
    QString error;
    auto effect = registry->prepare(config, layout, &params, &error);
    if (!effect)
    {
        d->thumbnails.insert(key, new QString);
        return {};
    }
    auto job = std::make_shared<ThumbnailJob>();
    job->key = key;
    job->generation = d->generation;
    d->jobs.append(job);
    d->thumbnailTimer.start();
    QThreadPool::globalInstance()->start(
        [job, layout, params, config, effect]
        {
            QString result;
            try
            {
                const auto schema = QCatLighting::ColorSchema(config["schema"].toInt());
                auto bytes = effect->render(*layout, params, schema, 0);
                if (bytes.size() == 512 * int(schema))
                {
                    for (auto &v : bytes)
                        v = char(qRound(uchar(v) * config.value("brightness", 1).toDouble()));
                    QCatLightingFrame::Description info;
                    info.projectId = "thumbnail";
                    info.layoutRevision = 1;
                    info.frameId = 1;
                    info.pointCount = 512;
                    info.schema = schema;
                    info.stride = int(schema);
                    auto rgba =
                        QCatPreviewColorAdapter::toRgba8(QCatLightingFrame::create(info, bytes));
                    QImage image(reinterpret_cast<const uchar *>(rgba.constData()), 32, 16,
                                 QImage::Format_RGBA8888);
                    QByteArray png;
                    QBuffer buffer(&png);
                    buffer.open(QIODevice::WriteOnly);
                    image.save(&buffer, "PNG");
                    result = "data:image/png;base64," + QString::fromLatin1(png.toBase64());
                }
            }
            catch (...)
            { /* A failed thumbnail must never replace live Deck state. */
            }
            QMutexLocker lock(&job->mutex);
            job->result = result;
            job->done = true;
        });
    return {};
}

void QCatLedStorageRepository::populateExamples()
{
    if (!storages().isEmpty() || readOnly())
        return;
    const auto base = createStorage(tr("Basic effects"), 12),
               whites = createStorage(tr("White and space"), 12);
    auto add = [&](const QString &s, int index, const QString &name, const QString &provider,
                   const QString &effect, int schema, QVariantMap params)
    {
        auto ps = places(s);
        savePlace(s, ps[index].toMap()["id"].toString(),
                  {{"name", name},
                   {"description", name},
                   {"providerId", provider},
                   {"effectId", effect},
                   {"effectVersion", 1},
                   {"schema", schema},
                   {"parameters", params},
                   {"brightness", 1.0},
                   {"speed", 1.0},
                   {"conversion", int(QCatLighting::ApproximateWhite)},
                   {"thumbnail", QString()},
                   {"empty", false}});
    };
    add(base, 0, tr("Prism"), "qcat.waves", "rainbow", 5, {{"saturation", 1}});
    add(base, 1, tr("Pastel rainbow"), "qcat.waves", "rainbow", 5,
        {{"saturation", .35}, {"frequency", .5}});
    add(base, 2, tr("Ocean ripple"), "qcat.waves", "ripple", 5, {{"R", .05}, {"G", .4}, {"B", 1}});
    add(base, 3, tr("Amber"), "qcat.channels", "solid", 5, {{"R", 1}, {"G", .3}, {"B", 0}});
    add(whites, 0, tr("Cold white"), "qcat.channels", "solid", 5, {{"B", 0}, {"CW", 1}, {"WW", 0}});
    add(whites, 1, tr("Warm white"), "qcat.channels", "solid", 5, {{"B", 0}, {"CW", 0}, {"WW", 1}});
    add(whites, 2, tr("White chase"), "qcat.channels", "chase", 5,
        {{"B", 0}, {"channel", 4}, {"level", 1}});
    add(whites, 3, tr("Single white"), "qcat.channels", "solid", 4, {{"B", 0}, {"W", 1}});
}
struct QCatLedPlaceModel::Impl
{
    QPointer<QCatLedStorageRepository> repo;
    QPointer<QCatLedEffectRegistry> registry;
    QString storage;
    QVariantList rows;
};
QCatLedPlaceModel::QCatLedPlaceModel(QObject *p) : QAbstractListModel(p), d(new Impl) {}
QCatLedPlaceModel::~QCatLedPlaceModel() = default;
int QCatLedPlaceModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : int(d->rows.size());
}
QHash<int, QByteArray> QCatLedPlaceModel::roleNames() const
{
    return {{Qt::UserRole + 1, "placeId"},
            {Qt::UserRole + 2, "name"},
            {Qt::UserRole + 3, "empty"},
            {Qt::UserRole + 4, "error"},
            {Qt::UserRole + 5, "number"}};
}
QVariant QCatLedPlaceModel::data(const QModelIndex &i, int role) const
{
    const auto p = get(i.row());
    if (!i.isValid() || p.isEmpty())
        return {};
    switch (role)
    {
    case Qt::UserRole + 1:
        return p["id"];
    case Qt::UserRole + 2:
        return p["name"];
    case Qt::UserRole + 3:
        return p.value("empty", false);
    case Qt::UserRole + 4:
        return d->registry ? d->registry->validatePlace(p) : QStringLiteral("No Registry");
    case Qt::UserRole + 5:
        return i.row() + 1;
    default:
        return {};
    }
}
QVariantMap QCatLedPlaceModel::get(int i) const
{
    return i >= 0 && i < d->rows.size() ? d->rows[i].toMap() : QVariantMap{};
}
QCatLedStorageRepository *QCatLedPlaceModel::repository() const
{
    return d->repo;
}
QCatLedEffectRegistry *QCatLedPlaceModel::registry() const
{
    return d->registry;
}
QString QCatLedPlaceModel::storageId() const
{
    return d->storage;
}
void QCatLedPlaceModel::refresh()
{
    beginResetModel();
    d->rows = d->repo ? d->repo->places(d->storage) : QVariantList{};
    endResetModel();
}
void QCatLedPlaceModel::setRepository(QCatLedStorageRepository *r)
{
    if (r == d->repo)
        return;
    if (d->repo)
        disconnect(d->repo, nullptr, this, nullptr);
    d->repo = r;
    if (r)
    {
        connect(r, &QCatLedStorageRepository::changed, this, &QCatLedPlaceModel::refresh);
        connect(r, &QObject::destroyed, this, &QCatLedPlaceModel::refresh);
    }
    refresh();
    emit settingsChanged();
}
void QCatLedPlaceModel::setRegistry(QCatLedEffectRegistry *r)
{
    if (r == d->registry)
        return;
    if (d->registry)
        disconnect(d->registry, nullptr, this, nullptr);
    d->registry = r;
    if (r)
    {
        connect(r, &QCatLedEffectRegistry::effectsChanged, this, &QCatLedPlaceModel::refresh);
        connect(r, &QObject::destroyed, this, &QCatLedPlaceModel::refresh);
    }
    refresh();
    emit settingsChanged();
}
void QCatLedPlaceModel::setStorageId(const QString &id)
{
    if (id == d->storage)
        return;
    d->storage = id;
    refresh();
    emit settingsChanged();
}
