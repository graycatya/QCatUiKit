// SPDX-License-Identifier: Apache-2.0
#include "qcatthemepalette.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QSet>
#include <QStringList>
#include <QtMath>
namespace QCatUiKit
{
Q_LOGGING_CATEGORY(lcTheme, "qcatuikit.theme")

namespace
{
const QSet<QString> &colorTokenNames()
{
    static const QSet<QString> names{"primary",       "primaryHover", "primaryPressed",
                                     "onPrimary",     "background",   "surface",
                                     "surfaceHover",  "text",         "textSecondary",
                                     "textDisabled",  "border",       "disabled",
                                     "success",       "warning",      "danger"};
    return names;
}

// WCAG relative luminance chooses a legible foreground for arbitrary brand seeds.
QColor contrastForeground(const QColor &primary)
{
    auto linear = [](qreal c) { return c <= .04045 ? c / 12.92 : qPow((c + .055) / 1.055, 2.4); };
    const qreal luminance = .2126 * linear(primary.redF()) +
                            .7152 * linear(primary.greenF()) +
                            .0722 * linear(primary.blueF());
    return QColor(luminance > .179 ? "#000000" : "#ffffff");
}

QVariantMap builtinColors(bool dark, const QColor &primary)
{
    return QVariantMap{{"primary", primary},
                       {"primaryHover", primary.lighter(115)},
                       {"primaryPressed", primary.darker(115)},
                       {"onPrimary", contrastForeground(primary)},
                       {"background", QColor(dark ? "#141414" : "#f5f5f5")},
                       {"surface", QColor(dark ? "#1f1f1f" : "#ffffff")},
                       {"surfaceHover", QColor(dark ? "#303030" : "#f0f5ff")},
                       {"text", QColor(dark ? "#f0f0f0" : "#1f1f1f")},
                       {"textSecondary", QColor(dark ? "#bfbfbf" : "#595959")},
                       {"textDisabled", QColor(dark ? "#707070" : "#8c8c8c")},
                       {"border", QColor(dark ? "#595959" : "#d9d9d9")},
                       {"disabled", QColor(dark ? "#262626" : "#f0f0f0")},
                       {"success", QColor(dark ? "#73d13d" : "#389e0d")},
                       {"warning", QColor(dark ? "#ffc53d" : "#ad6800")},
                       {"danger", QColor(dark ? "#ff7875" : "#cf1322")}};
}

bool numericTokenRange(const QString &key, int *minimum, int *maximum)
{
    if (key == "radius")
    {
        *minimum = 0;
        *maximum = 32;
        return true;
    }
    if (key == "fontSize")
    {
        *minimum = 10;
        *maximum = 32;
        return true;
    }
    if (key == "controlHeight" || key == "spacing" || key == "padding"
        || key == "motionDuration")
    {
        *minimum = 0;
        *maximum = 4096;
        return true;
    }
    return false;
}
} // namespace

class ThemePalette::Impl
{
  public:
    ColorScheme scheme = Light;
    Density density = Desktop;
    QColor primary = QColor("#1677ff");
    int radius = 6;
    int fontSize = 14;
    bool reducedMotion = false;
    QVariantMap overrides;
    QVariantMap colorPalette;
    mutable QVariantMap cachedTokens;
    mutable QStringList reportedWarnings;
};
ThemePalette::ThemePalette(QObject *parent) : QObject(parent), d(std::make_unique<Impl>()) {}
ThemePalette::~ThemePalette() = default;
ThemePalette::ColorScheme ThemePalette::colorScheme() const
{
    return d->scheme;
}
void ThemePalette::setColorScheme(ColorScheme value)
{
    if ((value != Light && value != Dark) || d->scheme == value)
        return;
    d->scheme = value;
    d->cachedTokens.clear();
    emit colorSchemeChanged();
    emit tokensChanged();
}
ThemePalette::Density ThemePalette::density() const
{
    return d->density;
}
void ThemePalette::setDensity(Density value)
{
    if (value < Desktop || value > Mobile || d->density == value)
        return;
    d->density = value;
    d->cachedTokens.clear();
    emit densityChanged();
    emit tokensChanged();
}
QColor ThemePalette::primaryColor() const
{
    return d->primary;
}
void ThemePalette::setPrimaryColor(const QColor &value)
{
    if (!value.isValid() || d->primary == value)
        return;
    d->primary = value;
    d->cachedTokens.clear();
    emit primaryColorChanged();
    emit tokensChanged();
}
int ThemePalette::radius() const
{
    return d->radius;
}
void ThemePalette::setRadius(int value)
{
    value = qBound(0, value, 32);
    if (d->radius == value)
        return;
    d->radius = value;
    d->cachedTokens.clear();
    emit radiusChanged();
    emit tokensChanged();
}
int ThemePalette::fontSize() const
{
    return d->fontSize;
}
void ThemePalette::setFontSize(int value)
{
    value = qBound(10, value, 32);
    if (d->fontSize == value)
        return;
    d->fontSize = value;
    d->cachedTokens.clear();
    emit fontSizeChanged();
    emit tokensChanged();
}
bool ThemePalette::reducedMotion() const
{
    return d->reducedMotion;
}
void ThemePalette::setReducedMotion(bool value)
{
    if (d->reducedMotion == value)
        return;
    d->reducedMotion = value;
    d->cachedTokens.clear();
    emit reducedMotionChanged();
    emit tokensChanged();
}
QVariantMap ThemePalette::overrides() const
{
    return d->overrides;
}
void ThemePalette::setOverrides(const QVariantMap &value)
{
    if (d->overrides == value)
        return;
    d->overrides = value;
    d->cachedTokens.clear();
    emit overridesChanged();
    emit tokensChanged();
}
QVariantMap ThemePalette::colorPalette() const
{
    return d->colorPalette;
}
void ThemePalette::setColorPalette(const QVariantMap &value)
{
    if (d->colorPalette == value)
        return;
    d->colorPalette = value;
    d->cachedTokens.clear();
    emit colorPaletteChanged();
    emit tokensChanged();
}
QVariantMap ThemePalette::tokens() const
{
    if (!d->cachedTokens.isEmpty())
        return d->cachedTokens;

    const bool dark = d->scheme == Dark;
    const int height = d->density == Desktop ? 32 : (d->density == Touch ? 48 : 44);

    QVariantMap result = builtinColors(dark, d->primary);
    result.insert("radius", d->radius);
    result.insert("fontSize", d->fontSize);
    result.insert("controlHeight", height);
    result.insert("spacing", d->density == Desktop ? 8 : 12);
    result.insert("padding", d->density == Desktop ? 12 : 16);
    result.insert("motionDuration", d->reducedMotion ? 0 : 160);
    result.insert("hoverEnabled", d->density == Desktop);

    // Caller supplied values win over derived colors. Unknown override keys are kept so an
    // application can expose its own design tokens through the same map.
    QSet<QString> explicitTokens;
    QStringList warnings;

    for (auto it = d->colorPalette.cbegin(); it != d->colorPalette.cend(); ++it)
    {
        if (!colorTokenNames().contains(it.key()))
        {
            warnings.append(
                QStringLiteral("colorPalette ignores non-color token '%1'").arg(it.key()));
            continue;
        }
        const QColor color = it.value().value<QColor>();
        if (!color.isValid())
        {
            warnings.append(
                QStringLiteral("colorPalette ignores invalid color for '%1'").arg(it.key()));
            continue;
        }
        result[it.key()] = color;
        explicitTokens.insert(it.key());
    }

    for (auto it = d->overrides.cbegin(); it != d->overrides.cend(); ++it)
    {
        if (!result.contains(it.key()))
        {
            result[it.key()] = it.value();
            warnings.append(QStringLiteral("overrides keeps unknown token '%1'").arg(it.key()));
            continue;
        }
        const QVariant original = result.value(it.key());
        if (original.metaType() == QMetaType::fromType<QColor>())
        {
            const QColor color = it.value().value<QColor>();
            if (!color.isValid())
            {
                warnings.append(
                    QStringLiteral("overrides ignores invalid color for '%1'").arg(it.key()));
                continue;
            }
            result[it.key()] = color;
            explicitTokens.insert(it.key());
        }
        else if (original.metaType() == QMetaType::fromType<bool>())
        {
            if (it.value().metaType() != QMetaType::fromType<bool>())
            {
                warnings.append(
                    QStringLiteral("overrides ignores non-boolean value for '%1'").arg(it.key()));
                continue;
            }
            result[it.key()] = it.value();
            explicitTokens.insert(it.key());
        }
        else
        {
            bool ok = false;
            const int number = it.value().toInt(&ok);
            if (!ok)
            {
                warnings.append(
                    QStringLiteral("overrides ignores non-numeric value for '%1'").arg(it.key()));
                continue;
            }
            int minimum = 0;
            int maximum = 4096;
            numericTokenRange(it.key(), &minimum, &maximum);
            if (number < minimum || number > maximum)
            {
                warnings.append(QStringLiteral("overrides clamps '%1' to %2..%3")
                                    .arg(it.key())
                                    .arg(minimum)
                                    .arg(maximum));
            }
            result[it.key()] = qBound(minimum, number, maximum);
            explicitTokens.insert(it.key());
        }
    }

    // Derived colors follow the effective brand color, including palette and override input.
    const QColor primary = result.value("primary").value<QColor>();
    if (!explicitTokens.contains("onPrimary"))
        result["onPrimary"] = contrastForeground(primary);
    if (!explicitTokens.contains("primaryHover"))
        result["primaryHover"] = primary.lighter(115);
    if (!explicitTokens.contains("primaryPressed"))
        result["primaryPressed"] = primary.darker(115);

    if (d->reducedMotion)
        result["motionDuration"] = 0;

    d->cachedTokens = result;
    if (warnings != d->reportedWarnings)
    {
        d->reportedWarnings = warnings;
        for (const QString &warning : warnings)
            qCWarning(lcTheme).noquote() << warning;
    }
    return result;
}
} // namespace QCatUiKit
