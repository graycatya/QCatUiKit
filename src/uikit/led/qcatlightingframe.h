// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledlayoutmodel.h"
#include <QVector3D>

class QCATUIKIT_EXPORT QCatLighting : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Lighting)
    QML_UNCREATABLE("Lighting exposes color schema and conversion enums")
  public:
    enum ColorSchema
    {
        RGB = 3,
        RGBW = 4,
        RGB_CW_WW = 5
    };
    Q_ENUM(ColorSchema)
    enum Conversion
    {
        Strict = 0,
        AddWhiteZero = 1,
        ApproximateWhite = 2
    };
    Q_ENUM(Conversion)
    enum Encoding
    {
        ChannelValues8 = 0
    };
    Q_ENUM(Encoding)
    static bool validSchema(int schema);
    static QVector3D whiteReference(double kelvin);
    static QByteArray convert(const QByteArray &values, ColorSchema from, ColorSchema to,
                              Conversion policy, QString *error = nullptr,
                              double whiteKelvin = 4000, double coldKelvin = 6500,
                              double warmKelvin = 2700);
};

// Immutable value: constructor copies external storage; readers only receive const data.
class QCATUIKIT_EXPORT QCatLightingFrame
{
  public:
    struct Description
    {
        QString projectId;
        quint64 layoutRevision = 0, frameId = 0, instanceId = 0;
        qint64 timestampUsec = 0;
        int pointCount = 0, stride = 3;
        QCatLighting::ColorSchema schema = QCatLighting::RGB;
        QCatLighting::Encoding encoding = QCatLighting::ChannelValues8;
    };
    QCatLightingFrame();
    static QCatLightingFrame create(const Description &description, const QByteArray &channels,
                                    const QByteArray &opacity = {}, QString *error = nullptr);
    static QCatLightingFrame fromRgba8(Description description, const QByteArray &rgba,
                                       QString *error = nullptr);
    bool isValid() const;
    const Description &description() const;
    const QByteArray &channels() const;
    const QByteArray &opacity() const;
    QByteArray opaqueChannels() const; // composite against black; never interpret alpha as white
    bool matches(const QCatLedLayoutPtr &layout) const;

  private:
    struct Data;
    std::shared_ptr<const Data> d;
};
Q_DECLARE_METATYPE(QCatLightingFrame)

class QCATUIKIT_EXPORT QCatPreviewColorAdapter : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PreviewColorAdapter)
    Q_PROPERTY(QCatLedSource *source READ source CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
  public:
    explicit QCatPreviewColorAdapter(QObject *parent = nullptr);
    ~QCatPreviewColorAdapter() override;
    QCatLedSource *source() const;
    QString lastError() const;
    void setLayout(QCatLedLayoutPtr layout);
    bool submit(const QCatLightingFrame &frame);
    static QByteArray toRgba8(const QCatLightingFrame &frame, double whiteKelvin = 4000,
                              double coldKelvin = 6500, double warmKelvin = 2700);
  Q_SIGNALS:
    void statusChanged();

  private:
    struct Impl;
    const std::unique_ptr<Impl> d;
};
