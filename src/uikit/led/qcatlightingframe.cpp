// SPDX-License-Identifier: Apache-2.0
#include "qcatlightingframe.h"
#include <cmath>

bool QCatLighting::validSchema(int s)
{
    return s == RGB || s == RGBW || s == RGB_CW_WW;
}
QVector3D QCatLighting::whiteReference(double kelvin)
{
    const float t = float(qBound(0.0, (kelvin - 2700) / 3800, 1.0));
    return QVector3D(1, .55f, .2f) * (1 - t) + QVector3D(.75f, .85f, 1) * t;
}
QByteArray QCatLighting::convert(const QByteArray &v, ColorSchema from, ColorSchema to,
                                 Conversion policy, QString *error, double wk, double ck, double hk)
{
    if (error)
        error->clear();
    auto fail = [&](const QString &e)
    {
        if (error)
            *error = e;
        return QByteArray{};
    };
    if (!validSchema(from) || !validSchema(to) || v.size() % int(from))
        return fail(QStringLiteral("ColorSchema/stride mismatch"));
    if (from == to)
        return v;
    if (policy == Strict)
        return fail(QStringLiteral("ColorSchema conversion requires an explicit policy"));
    if (from != RGB && policy != ApproximateWhite)
        return fail(
            QStringLiteral("Independent white channels require ApproximateWhite conversion"));
    const qsizetype count = v.size() / int(from);
    QByteArray out(count * int(to), Qt::Uninitialized);
    const auto white = whiteReference(from == RGBW ? wk : ck);
    const auto warm = whiteReference(hk);
    const double cold = qBound(0.0, (wk - hk) / qMax(1.0, ck - hk), 1.0);
    const auto *s = reinterpret_cast<const uchar *>(v.constData());
    auto *t = reinterpret_cast<uchar *>(out.data());
    for (qsizetype i = 0; i < count; ++i, s += int(from), t += int(to))
    {
        for (int c = 0; c < 3; ++c)
            t[c] = s[c];
        for (int c = 3; c < int(to); ++c)
            t[c] = 0;
        if (from == RGB)
            continue;
        if (to == RGB)
        {
            QVector3D rgb(s[0], s[1], s[2]);
            rgb += white * s[3];
            if (from == RGB_CW_WW)
                rgb += warm * s[4];
            for (int c = 0; c < 3; ++c)
                t[c] = uchar(qBound(0, qRound(rgb[c]), 255));
        }
        else if (to == RGBW)
            t[3] = uchar(qMin(255, int(s[3]) + int(s[4])));
        else
        {
            t[3] = uchar(qRound(s[3] * cold));
            t[4] = uchar(s[3] - t[3]);
        }
    }
    return out;
}
struct QCatLightingFrame::Data
{
    Description info;
    QByteArray channels, opacity;
};
QCatLightingFrame::QCatLightingFrame() = default;
QCatLightingFrame QCatLightingFrame::create(const Description &info, const QByteArray &channels,
                                            const QByteArray &opacity, QString *error)
{
    QString e;
    if (info.projectId.isEmpty() || !info.layoutRevision || !info.frameId)
        e = QStringLiteral("Frame: require project/layout/frame identity");
    else if (!QCatLighting::validSchema(info.schema) ||
             info.encoding != QCatLighting::ChannelValues8 || info.stride != int(info.schema))
        e = QStringLiteral("Frame: invalid schema, encoding or stride");
    else if (info.pointCount < 0 || info.pointCount > QCatLedLayoutModel::MaximumPoints ||
             channels.size() != qint64(info.pointCount) * info.stride)
        e = QStringLiteral("Frame: point count/buffer length exceeds or mismatches budget");
    else if (!opacity.isEmpty() && opacity.size() != info.pointCount)
        e = QStringLiteral("Frame: opacity length mismatch");
    if (error)
        *error = e;
    QCatLightingFrame f;
    if (e.isEmpty())
    {
        auto data = std::make_shared<Data>();
        data->info = info;
        data->info.projectId = QString(info.projectId.constData(), info.projectId.size());
        data->channels = QByteArray(channels.constData(), channels.size());
        data->opacity = QByteArray(opacity.constData(), opacity.size());
        f.d = data;
    }
    return f;
}
QCatLightingFrame QCatLightingFrame::fromRgba8(Description info, const QByteArray &rgba,
                                               QString *error)
{
    if (info.pointCount < 0 || rgba.size() != qint64(info.pointCount) * 4)
    {
        if (error)
            *error = QStringLiteral("RGBA8 length mismatch");
        return {};
    }
    info.schema = QCatLighting::RGB;
    info.stride = 3;
    QByteArray rgb(info.pointCount * 3, Qt::Uninitialized),
        alpha(info.pointCount, Qt::Uninitialized);
    for (int i = 0; i < info.pointCount; ++i)
    {
        for (int c = 0; c < 3; ++c)
            rgb[i * 3 + c] = rgba[i * 4 + c];
        alpha[i] = rgba[i * 4 + 3];
    }
    return create(info, rgb, alpha, error);
}
bool QCatLightingFrame::isValid() const
{
    return bool(d);
}
const QCatLightingFrame::Description &QCatLightingFrame::description() const
{
    static const Description empty;
    return d ? d->info : empty;
}
const QByteArray &QCatLightingFrame::channels() const
{
    static const QByteArray empty;
    return d ? d->channels : empty;
}
const QByteArray &QCatLightingFrame::opacity() const
{
    static const QByteArray empty;
    return d ? d->opacity : empty;
}
QByteArray QCatLightingFrame::opaqueChannels() const
{
    if (!d || d->opacity.isEmpty())
        return channels();
    auto bytes = channels();
    for (int i = 0; i < d->info.pointCount; ++i)
        for (int c = 0; c < d->info.stride; ++c)
            bytes[i * d->info.stride + c] =
                char((uchar(bytes[i * d->info.stride + c]) * uchar(d->opacity[i]) + 127) / 255);
    return bytes;
}
bool QCatLightingFrame::matches(const QCatLedLayoutPtr &l) const
{
    return d && l && d->info.projectId == l->projectId && d->info.layoutRevision == l->revision &&
           d->info.pointCount == l->positions.size();
}
struct QCatPreviewColorAdapter::Impl
{
    QCatLedSource *source;
    QCatLedLayoutPtr layout;
    quint64 localId = 0, sequence = 0;
    QString error;
};
QCatPreviewColorAdapter::QCatPreviewColorAdapter(QObject *p) : QObject(p), d(new Impl)
{
    d->source = new QCatLedSource(this);
}
QCatPreviewColorAdapter::~QCatPreviewColorAdapter() = default;
QCatLedSource *QCatPreviewColorAdapter::source() const
{
    return d->source;
}
QString QCatPreviewColorAdapter::lastError() const
{
    return d->error;
}
void QCatPreviewColorAdapter::setLayout(QCatLedLayoutPtr l)
{
    if (l == d->layout)
        return;
    d->layout = std::move(l);
    d->sequence = 0;
    const auto c = d->layout ? d->layout->configuration : QVariantMap{};
    d->localId = d->source->setLayout(
        d->layout ? d->layout->positions : QVector<QCatLedSource::Position>{}, c);
}
QByteArray QCatPreviewColorAdapter::toRgba8(const QCatLightingFrame &f, double wk, double ck,
                                            double hk)
{
    if (!f.isValid())
        return {};
    const auto schema = f.description().schema;
    const auto white = QCatLighting::whiteReference(schema == QCatLighting::RGBW ? wk : ck);
    const auto warm = QCatLighting::whiteReference(hk);
    const auto *channels = reinterpret_cast<const uchar *>(f.channels().constData());
    const auto *alpha = f.opacity().isEmpty() ? nullptr : f.opacity().constData();
    QByteArray out(f.description().pointCount * 4, Qt::Uninitialized);
    auto *rgba = reinterpret_cast<uchar *>(out.data());
    // Write screen RGBA directly: no temporary RGB frame or per-point detach checks.
    for (int i = 0; i < f.description().pointCount; ++i, channels += int(schema), rgba += 4)
    {
        if (schema == QCatLighting::RGB)
        {
            for (int c = 0; c < 3; ++c)
                rgba[c] = channels[c];
        }
        else
        {
            QVector3D rgb(channels[0], channels[1], channels[2]);
            rgb += white * channels[3];
            if (schema == QCatLighting::RGB_CW_WW)
                rgb += warm * channels[4];
            for (int c = 0; c < 3; ++c)
                rgba[c] = uchar(qBound(0, qRound(rgb[c]), 255));
        }
        rgba[3] = alpha ? uchar(alpha[i]) : 255;
    }
    return out;
}
bool QCatPreviewColorAdapter::submit(const QCatLightingFrame &f)
{
    if (!f.matches(d->layout))
    {
        d->error = QStringLiteral("Preview frame/layout identity mismatch");
        emit statusChanged();
        return false;
    }
    const bool ok = d->source->submitFrame(toRgba8(f), ++d->sequence, d->localId);
    d->error = ok ? QString{} : d->source->lastError();
    emit statusChanged();
    return ok;
}
