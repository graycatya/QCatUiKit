// SPDX-License-Identifier: Apache-2.0
#include "qcatledeffectregistry.h"
#include <QColor>
#include <QHash>
#include <cmath>

QCatLedEffectInstance::~QCatLedEffectInstance() = default;
QCatLedEffectProvider::~QCatLedEffectProvider() = default;
namespace
{
QVariantMap parameter(const QString &id, const QString &name, double value, double low, double high)
{
    return {{"id", id}, {"name", name}, {"default", value}, {"min", low}, {"max", high}};
}
class BasicInstance final : public QCatLedEffectInstance
{
    QString effect;

  public:
    explicit BasicInstance(QString id) : effect(std::move(id)) {}
    QByteArray render(const QCatLedLayoutData &layout, const QVariantMap &p,
                      QCatLighting::ColorSchema schema, double t) override
    {
        const int stride = int(schema);
        const qsizetype count = layout.positions.size();
        QByteArray data(count * stride, Qt::Uninitialized);
        auto *out = data.data();
        const auto *positions = layout.positions.constData();
        // Resolve immutable frame parameters once, outside the high-frequency point loop.
        const float frequency = p.value("frequency", 1).toFloat();
        const double red = p.value("R", 0).toDouble(), green = p.value("G", 0).toDouble(),
                     blue = p.value("B", 0).toDouble();
        const double white = p.value(schema == QCatLighting::RGBW ? "W" : "CW", 0).toDouble(),
                     warm = p.value("WW", 0).toDouble();
        const float saturation = float(p.value("saturation", 1).toDouble());
        const int channel = qBound(0, p.value("channel", 0).toInt(), stride - 1);
        const double level = p.value("level", 1).toDouble();
        const bool rainbow = effect == "rainbow", ripple = effect == "ripple",
                   chase = effect == "chase";
        for (qsizetype i = 0; i < count; ++i, out += stride)
        {
            const auto &pos = positions[i];
            double r = red, g = green, b = blue;
            if (rainbow)
            {
                double hue =
                    std::fmod(i / double(qMax(qsizetype(1), count)) + t * .1 * frequency, 1.0);
                if (hue < 0)
                    hue += 1;
                const auto color = QColor::fromHsvF(float(hue), saturation, 1);
                r = color.redF();
                g = color.greenF();
                b = color.blueF();
            }
            else if (ripple)
            {
                const double wave =
                    .5 +
                    .5 * std::sin(std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z) * .35 -
                                  frequency * t * 3);
                r *= wave;
                g *= wave;
                b *= wave;
            }
            double v[5] = {r, g, b, white, warm};
            if (chase)
            {
                const double phase = std::fmod(i * .1 - t * frequency, 1.0);
                const double wave = .5 + .5 * std::cos(2 * M_PI * phase);
                v[channel] = wave * level;
            }
            for (int c = 0; c < stride; ++c)
                out[c] = char(qRound(qBound(0.0, v[c], 1.0) * 255));
        }
        return data;
    }
};
class BasicProvider final : public QCatLedEffectProvider
{
    bool waves;

  public:
    explicit BasicProvider(bool w) : waves(w) {}
    QString id() const override
    {
        return waves ? QStringLiteral("qcat.waves") : QStringLiteral("qcat.channels");
    }
    QVariantList effects() const override
    {
        QVariantList out;
        const QStringList ids =
            waves ? QStringList{"rainbow", "ripple"} : QStringList{"solid", "chase"};
        for (const auto &id : ids)
        {
            QVariantList params;
            if (id != "rainbow")
                for (const auto &channel : QStringList{"R", "G", "B"})
                    params << parameter(channel, channel, channel == "B" ? 1 : 0, 0, 1);
            for (const auto &channel : QStringList{"W", "CW", "WW"})
                params << parameter(channel, channel, 0, 0, 1);
            if (id != "solid")
                params << parameter("frequency", QObject::tr("Frequency"), 1, 0, 10);
            if (id == "rainbow")
                params << parameter("saturation", QObject::tr("Saturation"), 1, 0, 1);
            if (id == "chase")
            {
                params << parameter("channel", QObject::tr("Animated channel (0-based)"), 0, 0, 4);
                params << parameter("level", QObject::tr("Animation level"), 1, 0, 1);
            }
            const auto name = id == "rainbow"  ? QObject::tr("Rainbow")
                              : id == "ripple" ? QObject::tr("Ripple")
                              : id == "solid"  ? QObject::tr("Independent channels")
                                               : QObject::tr("Channel chase");
            out << QVariantMap{{"id", id},        {"version", 1},
                               {"name", name},    {"schemas", QVariantList{3, 4, 5}},
                               {"reverse", true}, {"parameters", params}};
        }
        return out;
    }
    std::unique_ptr<QCatLedEffectInstance>
    create(const QString &id, int version, const QCatLedLayoutData &,
           QCatLighting::ColorSchema schema, const QVariantMap &p, QString *error) const override
    {
        if (version != 1)
        {
            *error = QStringLiteral("Unsupported effect version");
            return {};
        }
        if (id == "chase" &&
            (p.value("channel").toInt() >= int(schema) ||
             std::floor(p.value("channel").toDouble()) != p.value("channel").toDouble()))
        {
            *error = QStringLiteral("channel: outside selected ColorSchema");
            return {};
        }
        return std::make_unique<BasicInstance>(id);
    }
};
} // namespace
struct QCatLedEffectRegistry::Impl
{
    QHash<QString, std::shared_ptr<const QCatLedEffectProvider>> providers;
};
QCatLedEffectRegistry::QCatLedEffectRegistry(QObject *p) : QObject(p), d(new Impl)
{
    registerProvider(std::make_shared<BasicProvider>(true));
    registerProvider(std::make_shared<BasicProvider>(false));
}
QCatLedEffectRegistry::~QCatLedEffectRegistry() = default;
bool QCatLedEffectRegistry::registerProvider(std::shared_ptr<const QCatLedEffectProvider> p)
{
    if (!p || p->id().isEmpty() || d->providers.contains(p->id()))
        return false;
    d->providers.insert(p->id(), std::move(p));
    emit effectsChanged();
    return true;
}
QVariantList QCatLedEffectRegistry::effects() const
{
    QVariantList out;
    auto ids = d->providers.keys();
    ids.sort();
    for (const auto &id : ids)
        for (const auto &v : d->providers[id]->effects())
        {
            auto m = v.toMap();
            m["providerId"] = id;
            out << m;
        }
    return out;
}
QVariantMap QCatLedEffectRegistry::effect(const QString &p, const QString &e) const
{
    for (const auto &v : effects())
    {
        const auto m = v.toMap();
        if (m["providerId"] == p && m["id"] == e)
            return m;
    }
    return {};
}
QString QCatLedEffectRegistry::validatePlace(const QVariantMap &p) const
{
    if (p.value("configVersion", 1).toInt() != 1)
        return QStringLiteral("Unsupported Place configuration version");
    if (p.value("empty", false).toBool())
        return QStringLiteral("Empty Storage Place");
    const auto e = effect(p["providerId"].toString(), p["effectId"].toString());
    if (e.isEmpty())
        return QStringLiteral("Missing Provider or effect: %1 / %2")
            .arg(p["providerId"].toString(), p["effectId"].toString());
    if (e["version"].toInt() != p.value("effectVersion", 1).toInt())
        return QStringLiteral("Incompatible effect version");
    if (!e["schemas"].toList().contains(p["schema"]))
        return QStringLiteral("Provider does not support ColorSchema");
    for (auto key : {"brightness", "speed"})
    {
        bool ok;
        double value = p.value(key, 1).toDouble(&ok);
        if (!ok || !std::isfinite(value) ||
            value < (QString::fromLatin1(key) == "brightness" ? 0 : -8) ||
            value > (QString::fromLatin1(key) == "brightness" ? 1 : 8))
            return QStringLiteral("%1: invalid preset value").arg(key);
    }
    if (p.value("speed", 1).toDouble() < 0 && !e.value("reverse", false).toBool())
        return QStringLiteral("Provider does not support reverse playback");
    const auto params = p["parameters"].toMap();
    for (const auto &v : e["parameters"].toList())
    {
        const auto def = v.toMap();
        const auto id = def["id"].toString();
        bool ok;
        const double value = params.value(id, def["default"]).toDouble(&ok);
        if (!ok || !std::isfinite(value) || value < def["min"].toDouble() ||
            value > def["max"].toDouble())
            return QStringLiteral("%1: outside [%2, %3]")
                .arg(id, def["min"].toString(), def["max"].toString());
    }
    return {};
}
std::shared_ptr<QCatLedEffectInstance>
QCatLedEffectRegistry::prepare(const QVariantMap &p, const QCatLedLayoutPtr &layout,
                               QVariantMap *params, QString *error) const
{
    *error = validatePlace(p);
    if (!error->isEmpty())
        return {};
    if (!layout)
    {
        *error = QStringLiteral("Layout is missing");
        return {};
    }
    const auto e = effect(p["providerId"].toString(), p["effectId"].toString());
    *params = p["parameters"].toMap();
    for (const auto &v : e["parameters"].toList())
    {
        auto m = v.toMap();
        if (!params->contains(m["id"].toString()))
            params->insert(m["id"].toString(), m["default"]);
    }
    return d->providers[p["providerId"].toString()]->create(
        p["effectId"].toString(), p.value("effectVersion", 1).toInt(), *layout,
        QCatLighting::ColorSchema(p["schema"].toInt()), *params, error);
}
