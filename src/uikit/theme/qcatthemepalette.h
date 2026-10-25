// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QColor>
#include <QObject>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ThemePalette : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ColorScheme colorScheme READ colorScheme WRITE setColorScheme NOTIFY
                   colorSchemeChanged FINAL)
    Q_PROPERTY(Density density READ density WRITE setDensity NOTIFY densityChanged FINAL)
    Q_PROPERTY(QColor primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY
                   primaryColorChanged FINAL)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged FINAL)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged FINAL)
    Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY
                   reducedMotionChanged FINAL)
    Q_PROPERTY(
        QVariantMap overrides READ overrides WRITE setOverrides NOTIFY overridesChanged FINAL)
    Q_PROPERTY(QVariantMap tokens READ tokens NOTIFY tokensChanged FINAL)
  public:
    enum ColorScheme
    {
        Light = 0,
        Dark = 1
    };
    Q_ENUM(ColorScheme)
    enum Density
    {
        Desktop = 0,
        Touch = 1,
        Mobile = 2
    };
    Q_ENUM(Density)
    explicit ThemePalette(QObject *parent = nullptr);
    ~ThemePalette() override;
    ColorScheme colorScheme() const;
    void setColorScheme(ColorScheme value);
    Density density() const;
    void setDensity(Density value);
    QColor primaryColor() const;
    void setPrimaryColor(const QColor &value);
    int radius() const;
    void setRadius(int value);
    int fontSize() const;
    void setFontSize(int value);
    bool reducedMotion() const;
    void setReducedMotion(bool value);
    QVariantMap overrides() const;
    void setOverrides(const QVariantMap &value);
    QVariantMap tokens() const;
  signals:
    void colorSchemeChanged();
    void densityChanged();
    void primaryColorChanged();
    void radiusChanged();
    void fontSizeChanged();
    void reducedMotionChanged();
    void overridesChanged();
    void tokensChanged();

  private:
    Q_DISABLE_COPY_MOVE(ThemePalette)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
