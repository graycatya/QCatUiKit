// SPDX-License-Identifier: Apache-2.0
#include "qcatledproject.h"
#include <QColor>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <cmath>
struct QCatLedProject::Impl
{
    QCatLedLayoutModel *layout;
    QCatLedEffectRegistry *registry;
    QCatLedStorageRepository *repo;
    QCatLedDeck *a, *b;
    QCatLedMixer *mixer;
    QCatLedOutput *output;
    QCatMemoryOutputBackend *memory;
    QVariantList preferences;
    QString error;
};
QCatLedProject::QCatLedProject(QObject *p) : QObject(p), d(new Impl)
{
    d->layout = new QCatLedLayoutModel(this);
    d->registry = new QCatLedEffectRegistry(this);
    d->repo = new QCatLedStorageRepository(this);
    d->a = new QCatLedDeck(this);
    d->b = new QCatLedDeck(this);
    for (auto *deck : {d->a, d->b})
    {
        deck->setLayout(d->layout);
        deck->setRegistry(d->registry);
        deck->setRepository(d->repo);
    }
    d->mixer = new QCatLedMixer(this);
    d->mixer->setLayout(d->layout);
    d->mixer->setDeckA(d->a);
    d->mixer->setDeckB(d->b);
    d->output = new QCatLedOutput(this);
    d->memory = new QCatMemoryOutputBackend(this);
    d->output->setMixer(d->mixer);
    d->output->setMemoryBackend(d->memory);
}
QCatLedProject::~QCatLedProject() = default;
QCatLedLayoutModel *QCatLedProject::layout() const
{
    return d->layout;
}
QCatLedEffectRegistry *QCatLedProject::registry() const
{
    return d->registry;
}
QCatLedStorageRepository *QCatLedProject::repository() const
{
    return d->repo;
}
QCatLedDeck *QCatLedProject::deckA() const
{
    return d->a;
}
QCatLedDeck *QCatLedProject::deckB() const
{
    return d->b;
}
QCatLedMixer *QCatLedProject::mixer() const
{
    return d->mixer;
}
QCatLedOutput *QCatLedProject::output() const
{
    return d->output;
}
QCatMemoryOutputBackend *QCatLedProject::memoryBackend() const
{
    return d->memory;
}
QVariantList QCatLedProject::previewPreferences() const
{
    return d->preferences;
}
void QCatLedProject::setPreviewPreferences(const QVariantList &v)
{
    d->preferences = v;
    emit preferencesChanged();
}
QString QCatLedProject::lastError() const
{
    return d->error;
}
QString QCatLedProject::exportJson() const
{
    return QString::fromUtf8(
        QJsonDocument::fromVariant(QVariantMap{{"version", 1},
                                               {"kind", "qcat-led-project"},
                                               {"layout", d->layout->configuration()},
                                               {"repository", d->repo->document()},
                                               {"deckA", d->a->configuration()},
                                               {"deckB", d->b->configuration()},
                                               {"mixer", d->mixer->configuration()},
                                               {"output", d->output->configuration()},
                                               {"previews", d->preferences}})
            .toJson());
}
bool QCatLedProject::importJson(const QString &json)
{
    auto fail = [this](const QString &e)
    {
        d->error = e;
        emit statusChanged();
        return false;
    };
    if (json.size() > 32 * 1024 * 1024)
        return fail(QStringLiteral("Project exceeds 32 MiB budget"));
    QJsonParseError parse;
    const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parse);
    const auto c = doc.object().toVariantMap();
    if (parse.error != QJsonParseError::NoError || c["version"].toInt() != 1 ||
        c["kind"] != "qcat-led-project")
        return fail(QStringLiteral("Invalid project JSON/version"));
    if (d->repo->readOnly())
        return fail(QStringLiteral("Project repository is read-only"));
    QString error = QCatLedStorageRepository::validateDocument(c["repository"].toMap());
    if (!error.isEmpty())
        return fail(error);
    auto layout = QCatLedLayoutModel::generate(c["layout"].toMap(), "validation", 1, &error);
    if (!layout)
        return fail(error);
    for (auto key : {"deckA", "deckB"})
    {
        const auto deck = c[key].toMap();
        if (deck["version"].toInt() != 1 || !std::isfinite(deck["speed"].toDouble()) ||
            std::abs(deck["speed"].toDouble()) > 8 ||
            !std::isfinite(deck["brightness"].toDouble()) || deck["brightness"].toDouble() < 0 ||
            deck["brightness"].toDouble() > 1)
            return fail(QStringLiteral("Invalid Deck controls"));
        const auto place = deck["place"].toMap();
        if (place.isEmpty())
            continue;
        if (place.value("configVersion", 1).toInt() != 1)
            return fail(QStringLiteral("Invalid Place version"));
        QVariantMap params;
        auto effect = d->registry->prepare(place, layout, &params, &error);
        if (!effect)
            return fail(error);
    }
    QCatLedMixer checkMixer;
    if (!checkMixer.restoreConfiguration(c["mixer"].toMap()))
        return fail(QStringLiteral("Invalid Mixer configuration"));
    const auto output = c["output"].toMap();
    if (output["version"].toInt() != 1 || output["interval"].toInt() < 4 ||
        output["interval"].toInt() > 10000)
        return fail(QStringLiteral("Invalid Output configuration"));
    error = QCatPatchMapper::validate(output["profiles"].toList(), int(layout->positions.size()));
    if (!error.isEmpty())
        return fail(error);
    const auto preferences = c["previews"].toList();
    if (preferences.size() > 64)
        return fail(QStringLiteral("Too many preview instances"));
    for (const auto &value : preferences)
    {
        const auto pref = value.toMap();
        if (pref["version"].toInt() != 1 || !QColor(pref["backgroundColor"].toString()).isValid())
            return fail(QStringLiteral("Invalid preview version/background"));
        const QMap<QString, QPair<double, double>> ranges{
            {"viewMode", {0, 1}},         {"pointShape", {0, 3}},      {"sizeMode", {0, 1}},
            {"pointSize", {1, 128}},      {"sceneSize", {.0001, 1e6}}, {"spacingX", {.1, 10}},
            {"spacingY", {.1, 10}},       {"spacingZ", {.1, 10}},      {"gapX", {0, .95}},
            {"gapY", {0, .95}},           {"sliceAxis", {0, 3}},       {"maximumFps", {1, 240}},
            {"previewBrightness", {0, 1}}};
        for (auto it = ranges.begin(); it != ranges.end(); ++it)
        {
            bool ok;
            const double v = pref.value(it.key()).toDouble(&ok);
            if (!ok || !std::isfinite(v) || v < it.value().first || v > it.value().second)
                return fail(QStringLiteral("Preview %1 is invalid").arg(it.key()));
        }
        const auto camera = pref["camera"].toMap();
        for (const auto &key : QStringList{"yaw", "pitch", "zoom", "panX", "panY"})
        {
            bool ok;
            const double v = camera.value(key).toDouble(&ok);
            if (!ok || !std::isfinite(v))
                return fail(QStringLiteral("Invalid preview camera field: %1").arg(key));
        }
        if (std::abs(camera["pitch"].toDouble()) > 89 || camera["zoom"].toDouble() < .2 ||
            camera["zoom"].toDouble() > 100 || std::abs(camera["panX"].toDouble()) > 100 ||
            std::abs(camera["panY"].toDouble()) > 100)
            return fail(QStringLiteral("Preview camera range is invalid"));
    }
    // All persistent data is validated before the live project is changed.
    d->output->cancel();
    d->a->unload();
    d->b->unload();
    d->repo->replaceDocument(c["repository"].toMap());
    d->layout->applyNow(c["layout"].toMap());
    d->a->restoreConfiguration(c["deckA"].toMap());
    d->b->restoreConfiguration(c["deckB"].toMap());
    d->mixer->restoreConfiguration(c["mixer"].toMap());
    d->output->restoreConfiguration(output);
    setPreviewPreferences(c["previews"].toList());
    d->error.clear();
    emit statusChanged();
    return true;
}
bool QCatLedProject::saveFile(const QUrl &url)
{
    QSaveFile f(url.toLocalFile());
    const auto bytes = exportJson().toUtf8();
    if (!url.isLocalFile() || !f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() ||
        !f.commit())
    {
        d->error = QStringLiteral("Cannot save project: %1").arg(f.errorString());
        emit statusChanged();
        return false;
    }
    d->error.clear();
    emit statusChanged();
    return true;
}
bool QCatLedProject::loadFile(const QUrl &url)
{
    QFile f(url.toLocalFile());
    if (!url.isLocalFile() || !f.open(QIODevice::ReadOnly) || f.size() > 32 * 1024 * 1024)
    {
        d->error = QStringLiteral("Cannot read project file or file exceeds budget");
        emit statusChanged();
        return false;
    }
    return importJson(QString::fromUtf8(f.readAll()));
}
