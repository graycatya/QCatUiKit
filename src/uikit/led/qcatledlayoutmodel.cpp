// SPDX-License-Identifier: Apache-2.0
#include "qcatledlayoutmodel.h"
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>
#include <QTimer>
#include <QUuid>
#include <cmath>

namespace
{
QVariantMap normalized(const QVariantMap &input)
{
    auto c = QCatLedLayoutModel::defaults();
    for (auto it = input.begin(); it != input.end(); ++it)
        c[it.key()] = it.value();
    if (c["type"] == "Matrix2D")
        c["countZ"] = 1;
    return c;
}
bool number(const QVariantMap &c, const char *key, double low, double high, bool integer = false)
{
    bool ok = false;
    const double v = c.value(QLatin1String(key)).toDouble(&ok);
    return ok && std::isfinite(v) && v >= low && v <= high && (!integer || std::floor(v) == v);
}
struct LayoutJob
{
    QMutex mutex;
    bool done = false;
    QCatLedLayoutPtr result;
    QString error;
};
} // namespace
qsizetype QCatLedLayoutData::index(int x, int y, int z, int nx, int ny)
{
    return x + qsizetype(nx) * (y + qsizetype(ny) * z);
}
QVariantMap QCatLedLayoutModel::defaults()
{
    return {{"version", 1},      {"type", "Matrix2D"},  {"countX", 64},
            {"countY", 32},      {"countZ", 1},         {"spacingX", 1.0},
            {"spacingY", 1.0},   {"spacingZ", 1.0},     {"count", 128},
            {"radius", 16.0},    {"startAngle", 0.0},   {"clockwise", false},
            {"radialLayers", 8}, {"angularPoints", 32}, {"positions", QVariantList{}}};
}
QString QCatLedLayoutModel::validate(const QVariantMap &input)
{
    const auto c = normalized(input);
    if (c["version"].toInt() != 1)
        return QStringLiteral("version: unsupported layout format");
    const auto type = c["type"].toString();
    if (type == "Matrix2D" || type == "Matrix3D")
    {
        qint64 n = 1;
        for (auto key : {"countX", "countY", "countZ"})
        {
            if (!number(c, key, 0, MaximumPoints, true))
                return QStringLiteral("%1: require integer 0…1,000,000").arg(key);
            n *= c[key].toLongLong();
            if (n > MaximumPoints)
                return QStringLiteral("count: matrix exceeds 1,000,000-point memory budget");
        }
        for (auto key : {"spacingX", "spacingY", "spacingZ"})
            if (!number(c, key, 0.0001, 1000000))
                return QStringLiteral("%1: require finite spacing 0.0001…1,000,000").arg(key);
    }
    else if (type == "Ring" || type == "Disk" || type == "SphereSurface")
    {
        if (!number(c, "radius", 0.0001, 1000000))
            return QStringLiteral("radius: require finite value 0.0001…1,000,000");
        if (!number(c, "startAngle", -360000, 360000))
            return QStringLiteral("startAngle: invalid degrees");
        if (type == "Disk")
        {
            if (!number(c, "radialLayers", 0, MaximumPoints, true) ||
                !number(c, "angularPoints", 1, MaximumPoints, true))
                return QStringLiteral("radialLayers/angularPoints: invalid integer count");
            if (1 + c["radialLayers"].toLongLong() * c["angularPoints"].toLongLong() >
                MaximumPoints)
                return QStringLiteral("count: disk exceeds memory budget");
        }
        else if (!number(c, "count", 0, MaximumPoints, true))
            return QStringLiteral("count: invalid point count");
    }
    else if (type == "Custom")
    {
        const auto points = c["positions"].toList();
        if (points.size() > MaximumPoints)
            return QStringLiteral("positions: exceeds memory budget");
        for (const auto &v : points)
        {
            const auto p = v.toList();
            if (p.size() != 3)
                return QStringLiteral("positions: require XYZ triples");
            for (const auto &a : p)
            {
                bool ok;
                const double x = a.toDouble(&ok);
                if (!ok || !std::isfinite(x) || std::abs(x) > 1e12)
                    return QStringLiteral("positions: invalid XYZ");
            }
        }
    }
    else
        return QStringLiteral("type: unknown layout type");
    return {};
}
QCatLedLayoutPtr QCatLedLayoutModel::generate(const QVariantMap &input, const QString &id,
                                              quint64 revision, QString *error)
{
    const auto message = validate(input);
    if (error)
        *error = message;
    if (!message.isEmpty() || id.isEmpty() || !revision)
        return {};
    auto result = std::make_shared<QCatLedLayoutData>();
    result->configuration = normalized(input);
    result->projectId = id;
    result->revision = revision;
    const auto &c = result->configuration;
    const auto type = c["type"].toString();
    auto &p = result->positions;
    if (type.startsWith("Matrix"))
    {
        const int nx = c["countX"].toInt(), ny = c["countY"].toInt(), nz = c["countZ"].toInt();
        result->countX = nx;
        result->countY = ny;
        result->countZ = nz;
        p.reserve(qsizetype(nx) * ny * nz);
        if (!nx || !ny || !nz)
            return result;
        for (int z = 0; z < nz; ++z)
            for (int y = 0; y < ny; ++y)
                for (int x = 0; x < nx; ++x)
                    p.append({float((x - (nx - 1) * .5) * c["spacingX"].toDouble()),
                              float(((ny - 1) * .5 - y) * c["spacingY"].toDouble()),
                              float((z - (nz - 1) * .5) * c["spacingZ"].toDouble())});
    }
    else if (type == "Custom")
    {
        for (const auto &v : c["positions"].toList())
        {
            const auto a = v.toList();
            p.append({a[0].toFloat(), a[1].toFloat(), a[2].toFloat()});
        }
    }
    else
    {
        const double radius = c["radius"].toDouble(),
                     start = c["startAngle"].toDouble() * M_PI / 180;
        const double sign = c["clockwise"].toBool() ? -1 : 1;
        if (type == "SphereSurface")
        {
            int n = c["count"].toInt();
            p.reserve(n);
            for (int i = 0; i < n; ++i)
            {
                const double z = 1 - 2 * (i + .5) / qMax(1, n), r = std::sqrt(qMax(0.0, 1 - z * z)),
                             a = i * M_PI * (3 - std::sqrt(5.0));
                p.append({float(radius * r * std::cos(a)), float(radius * r * std::sin(a)),
                          float(radius * z)});
            }
        }
        else
        {
            const bool disk = type == "Disk";
            const int layers = disk ? c["radialLayers"].toInt() : 1;
            const int n = c[disk ? "angularPoints" : "count"].toInt();
            p.reserve(disk ? 1 + layers * n : n);
            if (disk)
                p.append({0, 0, 0});
            for (int l = 1; l <= layers; ++l)
                for (int i = 0; i < n; ++i)
                {
                    const double r = radius * l / layers, a = start + sign * 2 * M_PI * i / n;
                    p.append({float(r * std::cos(a)), float(r * std::sin(a)), 0});
                }
        }
    }
    return result;
}
struct QCatLedLayoutModel::Impl
{
    QCatLedLayoutPtr layout;
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces), error;
    QVariantMap pending;
    quint64 request = 0, runningRequest = 0;
    std::shared_ptr<LayoutJob> job;
    QTimer timer;
};
QCatLedLayoutModel::QCatLedLayoutModel(QObject *parent) : QObject(parent), d(new Impl)
{
    d->layout = generate(defaults(), d->id, 1);
    d->timer.setInterval(8);
    connect(&d->timer, &QTimer::timeout, this,
            [this]
            {
                if (!d->job)
                    return;
                QCatLedLayoutPtr result;
                QString error;
                {
                    QMutexLocker lock(&d->job->mutex);
                    if (!d->job->done)
                        return;
                    result = d->job->result;
                    error = d->job->error;
                }
                d->job.reset();
                if (d->runningRequest == d->request)
                {
                    if (result)
                    {
                        d->layout = result;
                        emit layoutChanged();
                    }
                    d->error = error;
                    d->timer.stop();
                    emit statusChanged();
                }
                else
                    launch();
            });
}
QCatLedLayoutModel::~QCatLedLayoutModel() = default;
QVariantMap QCatLedLayoutModel::configuration() const
{
    return d->layout->configuration;
}
int QCatLedLayoutModel::pointCount() const
{
    return int(d->layout->positions.size());
}
quint64 QCatLedLayoutModel::revision() const
{
    return d->layout->revision;
}
bool QCatLedLayoutModel::busy() const
{
    return bool(d->job);
}
QString QCatLedLayoutModel::lastError() const
{
    return d->error;
}
QCatLedLayoutPtr QCatLedLayoutModel::snapshot() const
{
    return d->layout;
}
QVariantMap QCatLedLayoutModel::defaultConfiguration() const
{
    return defaults();
}
void QCatLedLayoutModel::apply(const QVariantMap &c)
{
    const auto error = validate(c);
    if (!error.isEmpty())
    {
        d->error = error;
        emit statusChanged();
        return;
    }
    d->pending = c;
    ++d->request;
    d->error.clear();
    if (!d->job)
        launch();
    emit statusChanged();
}
void QCatLedLayoutModel::launch()
{
    d->job = std::make_shared<LayoutJob>();
    d->runningRequest = d->request;
    const auto job = d->job;
    const auto config = d->pending;
    const auto id = d->id;
    const auto rev = revision() + 1;
    QThreadPool::globalInstance()->start(
        [job, config, id, rev]
        {
            QString error;
            auto result = generate(config, id, rev, &error);
            QMutexLocker lock(&job->mutex);
            job->result = std::move(result);
            job->error = error;
            job->done = true;
        });
    d->timer.start();
}
bool QCatLedLayoutModel::applyNow(const QVariantMap &c)
{
    QString error;
    auto result = generate(c, d->id, revision() + 1, &error);
    if (!result)
    {
        d->error = error;
        emit statusChanged();
        return false;
    }
    ++d->request;
    d->job.reset();
    d->timer.stop();
    d->layout = std::move(result);
    d->error.clear();
    emit layoutChanged();
    emit statusChanged();
    return true;
}

QString QCatLedLayoutModel::validateConfiguration(const QVariantMap &c) const
{
    return validate(c);
}
