// SPDX-License-Identifier: Apache-2.0
#include "qcatthemepalette.h"
#include <QtMath>
namespace QCatUiKit
{
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
    mutable QVariantMap cachedTokens;
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
QVariantMap ThemePalette::tokens() const
{
    if (!d->cachedTokens.isEmpty())
        return d->cachedTokens;
    const bool dark = d->scheme == Dark;
    const int height = d->density == Desktop ? 32 : (d->density == Touch ? 48 : 44);
    // WCAG relative luminance chooses a legible foreground for arbitrary brand seeds.
    auto linear = [](qreal c) { return c <= .04045 ? c / 12.92 : qPow((c + .055) / 1.055, 2.4); };
    const qreal luminance = .2126 * linear(d->primary.redF()) +
                            .7152 * linear(d->primary.greenF()) +
                            .0722 * linear(d->primary.blueF());
    QVariantMap result{{"primary", d->primary},
                       {"primaryHover", d->primary.lighter(115)},
                       {"primaryPressed", d->primary.darker(115)},
                       {"onPrimary", QColor(luminance > .179 ? "#000000" : "#ffffff")},
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
                       {"danger", QColor(dark ? "#ff7875" : "#cf1322")},
                       {"radius", d->radius},
                       {"fontSize", d->fontSize},
                       {"controlHeight", height},
                       {"spacing", d->density == Desktop ? 8 : 12},
                       {"padding", d->density == Desktop ? 12 : 16},
                       {"motionDuration", d->reducedMotion ? 0 : 160},
                       {"hoverEnabled", d->density == Desktop}};
    // Overrides are semantic tokens; reject unknown keys and incompatible values.
    for (auto it = d->overrides.cbegin(); it != d->overrides.cend(); ++it)
    {
        if (!result.contains(it.key()))
            continue;
        const QVariant original = result.value(it.key());
        if (original.metaType() == QMetaType::fromType<QColor>())
        {
            const QColor color = it.value().value<QColor>();
            if (color.isValid())
                result[it.key()] = color;
        }
        else if (original.metaType() == QMetaType::fromType<bool>())
        {
            if (it.value().metaType() == QMetaType::fromType<bool>())
                result[it.key()] = it.value();
        }
        else
        {
            bool ok = false;
            const int number = it.value().toInt(&ok);
            if (ok && number >= 0 && number <= 4096)
                result[it.key()] = number;
        }
    }
    if (d->reducedMotion)
        result["motionDuration"] = 0;
    d->cachedTokens = result;
    return result;
}
} // namespace QCatUiKit
