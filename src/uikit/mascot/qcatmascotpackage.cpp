// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotpackage_p.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <cmath>

namespace QCatUiKit::MascotPrivate
{
namespace
{
struct Reader
{
    QString root;
    bool resource = false;
    QString error;
    QMap<QString, QSize> images;
    qint64 decodedBytes = 0;

    bool fail(const QString &where, const QString &message)
    {
        if (error.isEmpty())
            error = where + ": " + message;
        return false;
    }
    bool number(const QJsonValue &v, double *out, const QString &where, double low, double high,
                bool integer = false)
    {
        if (!v.isDouble() || !std::isfinite(v.toDouble()) || v.toDouble() < low ||
            v.toDouble() > high || (integer && std::floor(v.toDouble()) != v.toDouble()))
            return fail(where, "invalid numeric value");
        *out = v.toDouble();
        return true;
    }
    bool pair(const QJsonValue &v, QPointF *out, const QString &where, double low, double high,
              bool integer = false)
    {
        const auto a = v.toArray();
        double x, y;
        if (!v.isArray() || a.size() != 2)
            return fail(where, "expected two coordinates");
        if (!number(a[0], &x, where, low, high, integer) ||
            !number(a[1], &y, where, low, high, integer))
            return false;
        *out = {x, y};
        return true;
    }
    QString path(const QJsonValue &v, const QString &where)
    {
        const QString rel = v.toString();
        const QUrl url(rel);
        if (!v.isString() || rel.isEmpty() || QDir::isAbsolutePath(rel) || rel.contains('\\') ||
            !url.scheme().isEmpty() || url.hasQuery() || url.hasFragment() ||
            rel.split('/').contains(".."))
        {
            fail(where, "expected a relative package resource path");
            return {};
        }
        const QString candidate = QDir::cleanPath(root + '/' + rel);
        const QString resolved = resource ? candidate : QFileInfo(candidate).canonicalFilePath();
        const QString boundary = root.endsWith('/') ? root : root + '/';
        if (resolved.isEmpty() || !resolved.startsWith(boundary) || !QFileInfo(resolved).isFile())
        {
            fail(where, "resource missing or outside package boundary: " + rel);
            return {};
        }
        return resolved;
    }
    QUrl url(const QString &path) const
    {
        return resource ? QUrl("qrc" + path) : QUrl::fromLocalFile(path);
    }
    bool image(const QString &path, QSize *size, const QString &where)
    {
        if (images.contains(path))
        {
            *size = images.value(path);
            return true;
        }
        QImageReader reader(path);
        const QSize s = reader.size();
        if (s.isEmpty() || s.width() > 4096 || s.height() > 4096)
            return fail(where, "image dimensions invalid or exceed 4096");
        const qint64 bytes = qint64(s.width()) * s.height() * 4;
        if (decodedBytes + bytes > 128 * 1024 * 1024)
            return fail(where, "package exceeds 128 MiB decoded image budget");
        if (reader.read().isNull())
            return fail(where, "image decode failed: " + reader.errorString());
        decodedBytes += bytes;
        images.insert(path, s);
        *size = s;
        return true;
    }
    bool regions(const QJsonValue &v, QVariantList *out, const QString &where)
    {
        if (v.isUndefined())
            return true;
        if (!v.isArray() || v.toArray().size() > 128)
            return fail(where, "invalid region list");
        QSet<QString> ids;
        for (const auto &entry : v.toArray())
        {
            const auto r = entry.toObject();
            const QString id = r.value("id").toString();
            if (id.isEmpty() || ids.contains(id))
                return fail(where, "missing or duplicate region id");
            ids.insert(id);
            for (const auto &flag : {"clickable", "draggable"})
                if (r.contains(flag) && !r.value(flag).isBool())
                    return fail(where, "invalid region flag");
            const QString shape = r.value("shape").toString();
            if (shape == "rect" || shape == "ellipse")
            {
                const auto rect = r.value("rect").toArray();
                if (rect.size() != 4)
                    return fail(where, "expected rectangle");
                double n;
                for (int i = 0; i < 4; ++i)
                    if (!number(rect[i], &n, where, i < 2 ? -65536 : 0.000001, 65536))
                        return false;
            }
            else if (shape == "polygon")
            {
                const auto points = r.value("points").toArray();
                if (points.size() < 3 || points.size() > 256)
                    return fail(where, "invalid polygon");
                QPointF pt;
                for (const auto &point : points)
                    if (!pair(point, &pt, where, -65536, 65536))
                        return false;
            }
            else
                return fail(where, "unknown region shape");
            out->append(r.toVariantMap());
        }
        return true;
    }
};

QJsonObject adaptSprite(const QJsonObject &old)
{
    QJsonObject result = old;
    result.insert("renderer", "sprite");
    result.insert("logicalSize", old.value("frameSize"));
    const auto size = old.value("frameSize").toArray();
    const auto anchor = old.value("anchor").toArray();
    if (size.size() == 2 && anchor.size() == 2 && anchor[0].isDouble() && anchor[1].isDouble())
        result.insert("footAnchor", QJsonArray{size[0].toDouble() * anchor[0].toDouble(),
                                               size[1].toDouble() * anchor[1].toDouble()});
    result.insert("bubbleAnchor", QJsonArray{size.size() == 2 ? size[0].toDouble() / 2 : 0, 0});
    result.insert("defaultFacing", "right");
    result.insert("allowMirror", false);
    result.insert("defaultAction", old.value("stateMap").toObject().value("Idle").toString("idle"));
    QJsonObject actions;
    const auto animations = old.value("animations").toObject();
    for (auto it = animations.begin(); it != animations.end(); ++it)
    {
        auto sprite = it.value().toObject();
        sprite.insert("source", old.value("asset"));
        sprite.insert("frameSize", old.value("frameSize"));
        actions.insert(it.key(), QJsonObject{{"loops", sprite.value("loop").toBool() ? -1 : 1},
                                             {"right", sprite}});
    }
    result.insert("actions", actions);
    return result;
}
} // namespace

bool loadPackage(const QUrl &source, Package *package, QString *error,
                 const std::function<bool()> &cancelled)
{
    Reader r;
    auto done = [&](const QString &message)
    {
        *error = message;
        return false;
    };
    if (cancelled && cancelled())
        return done("package load cancelled");
    if (source.hasQuery() || source.hasFragment() ||
        (!source.isLocalFile() && source.scheme() != "qrc"))
        return done("packageSource: only local file and qrc URLs are supported");
    r.resource = source.scheme() == "qrc";
    if (r.resource && !source.host().isEmpty())
        return done("packageSource: qrc authority is not allowed");
    QString manifest = r.resource ? ':' + source.path() : source.toLocalFile();
    if (QFileInfo(manifest).isDir())
    {
        const QString base = manifest;
        manifest = base + "/mascot.json";
        if (!QFileInfo::exists(manifest))
            manifest = base + "/manifest.json";
    }
    if (!r.resource)
        manifest = QFileInfo(manifest).canonicalFilePath();
    r.root = r.resource ? QDir::cleanPath(QFileInfo(manifest).path())
                        : QFileInfo(manifest).absolutePath();
    if (manifest.isEmpty())
        return done("packageSource: manifest does not exist");
    QFile file(manifest);
    if (!file.open(QIODevice::ReadOnly))
        return done("packageSource: cannot open manifest");
    if (file.size() > 1024 * 1024)
        return done("manifest: exceeds 1 MiB");
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return done("manifest: invalid JSON object: " + parseError.errorString());
    auto obj = document.object();
    if (obj.value("schemaVersion") != QJsonValue(1))
        return done("schemaVersion: unsupported version");
    if (!obj.contains("renderer") && obj.value("type") == "sprite")
    {
        QPointF anchor;
        if (!r.pair(obj.value("anchor"), &anchor, "anchor", 0, 1))
            return done(r.error);
        if (!obj.value("animations").isObject())
            return done("animations: expected object");
        for (const auto &entry : obj.value("animations").toObject())
            if (!entry.isObject() || !entry.toObject().value("loop").isBool())
                return done("animations: each loop must be boolean");
        obj = adaptSprite(obj);
    }
    Package parsed;
    parsed.id = obj.value("id").toString();
    parsed.renderer = obj.value("renderer").toString();
    if (parsed.id.isEmpty())
        return done("id: expected nonempty string");
    if (parsed.renderer != "sprite" && parsed.renderer != "lottie")
        return done("renderer: unsupported renderer");
    QPointF size;
    if (!r.pair(obj.value("logicalSize"), &size, "logicalSize", 1, 4096, true) ||
        !r.pair(obj.value("footAnchor"), &parsed.footAnchor, "footAnchor", 0, 4096) ||
        !r.pair(obj.value("bubbleAnchor"), &parsed.bubbleAnchor, "bubbleAnchor", 0, 4096))
        return done(r.error);
    parsed.logicalSize = QSize(int(size.x()), int(size.y()));
    for (const auto &anchor : {parsed.footAnchor, parsed.bubbleAnchor})
        if (anchor.x() > size.x() || anchor.y() > size.y())
            return done("anchor: outside logicalSize");
    parsed.defaultAction = obj.value("defaultAction").toString();
    parsed.defaultFacing = obj.value("defaultFacing").toString("right");
    if (parsed.defaultFacing != "right" && parsed.defaultFacing != "left")
        return done("defaultFacing: expected left or right");
    if (obj.contains("allowMirror") && !obj.value("allowMirror").isBool())
        return done("allowMirror: expected boolean");
    parsed.allowMirror = obj.value("allowMirror").toBool();
    QVariantList regions;
    if (!r.regions(obj.value("regions"), &regions, "regions"))
        return done(r.error);
    if (obj.contains("fallbackImage"))
    {
        const QString path = r.path(obj.value("fallbackImage"), "fallbackImage");
        QSize s;
        if (path.isEmpty() || !r.image(path, &s, "fallbackImage"))
            return done(r.error);
        parsed.fallbackImage = r.url(path);
    }
    const auto actions = obj.value("actions").toObject();
    if (actions.isEmpty() || actions.size() > 256)
        return done("actions: expected 1–256 actions");
    for (auto it = actions.begin(); it != actions.end(); ++it)
    {
        if (cancelled && cancelled())
            return done("package load cancelled");
        const QString where = "actions." + it.key();
        if (it.key().isEmpty() || !it.value().isObject())
            return done(where + ": invalid action");
        Action action;
        const auto a = it.value().toObject();
        double loops;
        if (!r.number(a.value("loops"), &loops, where + ".loops", -1, 2147483647, true) ||
            loops == 0)
            return done(r.error.isEmpty() ? where + ": loops must be -1 or positive" : r.error);
        action.loops = int(loops);
        action.regions = regions;
        if (a.contains("regions"))
        {
            action.regions.clear();
            if (!r.regions(a.value("regions"), &action.regions, where + ".regions"))
                return done(r.error);
        }
        for (const auto &direction : {QString("left"), QString("right")})
        {
            if (!a.contains(direction))
                continue;
            if (cancelled && cancelled())
                return done("package load cancelled");
            const auto entry = a.value(direction).toObject();
            const QString path = r.path(entry.value("source"), where + '.' + direction + ".source");
            if (path.isEmpty())
                return done(r.error);
            Sprite sprite;
            sprite.source = r.url(path);
            if (parsed.renderer == "sprite")
            {
                QPointF frame;
                double start = 0, count, fps;
                if (!r.pair(entry.value("frameSize"), &frame, where + ".frameSize", 1, 4096,
                            true) ||
                    !r.number(entry.value("frameCount"), &count, where + ".frameCount", 1, 4096,
                              true) ||
                    !r.number(entry.value("fps"), &fps, where + ".fps", 0.01, 240) ||
                    (entry.contains("startFrame") &&
                     !r.number(entry.value("startFrame"), &start, where + ".startFrame", 0,
                               16777215, true)) ||
                    !r.image(path, &sprite.atlasSize, where))
                    return done(r.error);
                sprite.frameSize = QSize(int(frame.x()), int(frame.y()));
                sprite.startFrame = int(start);
                sprite.frameCount = int(count);
                sprite.fps = fps;
                if (sprite.atlasSize.width() % sprite.frameSize.width() ||
                    sprite.atlasSize.height() % sprite.frameSize.height() ||
                    qint64(sprite.startFrame) + sprite.frameCount >
                        qint64(sprite.atlasSize.width() / sprite.frameSize.width()) *
                            (sprite.atlasSize.height() / sprite.frameSize.height()))
                    return done(where + ": frame range or grid dimensions exceed atlas");
            }
            else
            {
                for (const auto &field : {"frameSize", "startFrame", "frameCount", "fps"})
                    if (entry.contains(field))
                        return done(where + ": sprite fields in lottie package");
                QFile lottie(path);
                if (!lottie.open(QIODevice::ReadOnly) || lottie.size() > 64 * 1024 * 1024)
                    return done(where + ": invalid or oversized Lottie resource");
                QByteArray bytes;
                while (!lottie.atEnd())
                {
                    if (cancelled && cancelled())
                        return done("package load cancelled");
                    const auto chunk = lottie.read(1024 * 1024);
                    if (chunk.isEmpty() && lottie.error() != QFileDevice::NoError)
                        return done(where + ": cannot read Lottie resource");
                    bytes += chunk;
                    if (bytes.size() > 64 * 1024 * 1024)
                        return done(where + ": oversized Lottie resource");
                }
                const auto animation = QJsonDocument::fromJson(bytes);
                if (cancelled && cancelled())
                    return done("package load cancelled");
                if (!animation.isObject())
                    return done(where + ": invalid Lottie JSON object");
                for (const auto &asset : animation.object().value("assets").toArray())
                {
                    const auto external = asset.toObject();
                    if (!external.contains("p"))
                        continue;
                    // Embedded and external images need an explicit playback contract.
                    // Do not pass unchecked paths or network URLs to the renderer.
                    const auto prefix = external.value("u");
                    if ((!prefix.isUndefined() && !prefix.isString()) ||
                        !external.value("p").isString() || external.value("e").toInt() == 1)
                        return done(where + ": unsupported embedded or malformed Lottie asset");
                    const QString imagePath = r.path(
                        prefix.toString() + external.value("p").toString(), where + ".assets");
                    QSize imageSize;
                    if (imagePath.isEmpty() || !r.image(imagePath, &imageSize, where + ".assets"))
                        return done(r.error);
                }
                for (const auto &font :
                     animation.object().value("fonts").toObject().value("list").toArray())
                {
                    const auto entry = font.toObject();
                    if (entry.contains("fPath") &&
                        r.path(entry.value("fPath"), where + ".fonts").isEmpty())
                        return done(r.error);
                }
            }
            action.directions.insert(direction, sprite);
        }
        if (action.directions.isEmpty())
            return done(where + ": missing direction resource");
        if (!parsed.allowMirror && action.directions.size() != 2 &&
            !action.directions.contains(parsed.defaultFacing))
            return done(where + ": default facing is unavailable and mirroring disabled");
        parsed.actions.insert(it.key(), action);
    }
    if (!parsed.actions.contains(parsed.defaultAction))
        return done("defaultAction: missing action");
    *package = parsed;
    error->clear();
    return true;
}
} // namespace QCatUiKit::MascotPrivate
