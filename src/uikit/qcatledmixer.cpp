// SPDX-License-Identifier: Apache-2.0
#include "qcatledmixer.h"
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QTimer>
#include <cmath>
struct QCatLedMixer::Impl
{
    QPointer<QCatLedDeck> a, b;
    QPointer<QCatLedLayoutModel> layout;
    QCatPreviewColorAdapter *adapter;
    double crossfader = .5, master = 1;
    bool blackout = false, freeze = false, queued = false;
    int schema = 5, conversion = 0;
    QString error;
    quint64 sequence = 0;
    QCatLightingFrame mixed, inputA, inputB;
    mutable QMutex mutex;
    QCatLightingFrame output;
};
QCatLedMixer::QCatLedMixer(QObject *p) : QObject(p), d(new Impl)
{
    d->adapter = new QCatPreviewColorAdapter(this);
}
QCatLedMixer::~QCatLedMixer() = default;
QCatLedDeck *QCatLedMixer::deckA() const
{
    return d->a;
}
QCatLedDeck *QCatLedMixer::deckB() const
{
    return d->b;
}
void QCatLedMixer::setDeckA(QCatLedDeck *a)
{
    if (a == d->a)
        return;
    if (d->a)
        disconnect(d->a, nullptr, this, nullptr);
    d->a = a;
    if (a)
    {
        connect(a, &QCatLedDeck::frameReady, this, &QCatLedMixer::schedule);
        connect(a, &QObject::destroyed, this, &QCatLedMixer::schedule);
    }
    schedule();
    emit settingsChanged();
}
void QCatLedMixer::setDeckB(QCatLedDeck *b)
{
    if (b == d->b)
        return;
    if (d->b)
        disconnect(d->b, nullptr, this, nullptr);
    d->b = b;
    if (b)
    {
        connect(b, &QCatLedDeck::frameReady, this, &QCatLedMixer::schedule);
        connect(b, &QObject::destroyed, this, &QCatLedMixer::schedule);
    }
    schedule();
    emit settingsChanged();
}
QCatLedLayoutModel *QCatLedMixer::layout() const
{
    return d->layout;
}
void QCatLedMixer::setLayout(QCatLedLayoutModel *l)
{
    if (l == d->layout)
        return;
    if (d->layout)
        disconnect(d->layout, nullptr, this, nullptr);
    d->layout = l;
    auto changed = [this]
    {
        d->mixed = {};
        d->freeze = false;
        d->adapter->setLayout(d->layout ? d->layout->snapshot() : nullptr);
        schedule();
        emit settingsChanged();
    };
    if (l)
    {
        connect(l, &QCatLedLayoutModel::layoutChanged, this, changed);
        connect(l, &QObject::destroyed, this, changed);
    }
    changed();
}
QCatLedSource *QCatLedMixer::source() const
{
    return d->adapter->source();
}
double QCatLedMixer::crossfader() const
{
    return d->crossfader;
}
double QCatLedMixer::master() const
{
    return d->master;
}
bool QCatLedMixer::blackout() const
{
    return d->blackout;
}
bool QCatLedMixer::freeze() const
{
    return d->freeze;
}
int QCatLedMixer::workingSchema() const
{
    return d->schema;
}
int QCatLedMixer::conversion() const
{
    return d->conversion;
}
QString QCatLedMixer::lastError() const
{
    return d->error;
}
void QCatLedMixer::setCrossfader(double v)
{
    if (!std::isfinite(v) || v < 0 || v > 1)
        return;
    d->crossfader = v;
    remix();
    emit settingsChanged();
}
void QCatLedMixer::setMaster(double v)
{
    if (!std::isfinite(v) || v < 0 || v > 1)
        return;
    d->master = v;
    remix();
    emit settingsChanged();
}
void QCatLedMixer::setBlackout(bool v)
{
    d->blackout = v;
    remix();
    emit settingsChanged();
}
void QCatLedMixer::setFreeze(bool v)
{
    d->freeze = v;
    remix();
    emit settingsChanged();
}
void QCatLedMixer::setWorkingSchema(int v)
{
    if (!QCatLighting::validSchema(v) || v == d->schema)
        return;
    d->schema = v;
    d->freeze = false;
    d->mixed = {};
    remix();
    emit settingsChanged();
}
void QCatLedMixer::setConversion(int v)
{
    if (v < 0 || v > 2)
        return;
    d->conversion = v;
    remix();
    emit settingsChanged();
}
QCatLightingFrame QCatLedMixer::latestFrame() const
{
    QMutexLocker lock(&d->mutex);
    return d->output;
}
void QCatLedMixer::schedule()
{
    if (d->queued)
        return;
    d->queued = true;
    QTimer::singleShot(0, this,
                       [this]
                       {
                           d->queued = false;
                           remix();
                       });
}
void QCatLedMixer::remix()
{
    mixFrames(d->a ? d->a->latestFrame() : d->inputA, d->b ? d->b->latestFrame() : d->inputB);
}
bool QCatLedMixer::mixFrames(const QCatLightingFrame &a, const QCatLightingFrame &b)
{
    d->inputA = a;
    d->inputB = b;
    auto fail = [this](const QString &e)
    {
        d->error = e;
        emit statusChanged();
        return false;
    };
    if (!d->layout)
        return fail(QStringLiteral("Mixer requires a project layout"));
    const auto layout = d->layout->snapshot();
    if (!d->freeze || !d->mixed.matches(layout))
    {
        if (!a.matches(layout) || !b.matches(layout))
            return fail(QStringLiteral("A/B project layout identity or revision mismatch"));
        QString error;
        const auto schema = QCatLighting::ColorSchema(d->schema);
        const auto policy = QCatLighting::Conversion(d->conversion);
        auto av = QCatLighting::convert(a.opaqueChannels(), a.description().schema, schema, policy,
                                        &error);
        if (!error.isEmpty())
            return fail(error);
        auto bv = QCatLighting::convert(b.opaqueChannels(), b.description().schema, schema, policy,
                                        &error);
        if (!error.isEmpty())
            return fail(error);
        QByteArray mixed;
        if (d->crossfader == 0 || d->crossfader == 1)
            mixed = d->crossfader == 0 ? av : bv;
        else
        {
            mixed.resize(av.size());
            auto *dst = reinterpret_cast<uchar *>(mixed.data());
            const auto *ap = reinterpret_cast<const uchar *>(av.constData());
            const auto *bp = reinterpret_cast<const uchar *>(bv.constData());
            const double weightB = d->crossfader, weightA = 1 - weightB;
            for (qsizetype i = 0; i < av.size(); ++i)
                dst[i] = uchar(qRound(ap[i] * weightA + bp[i] * weightB));
        }
        auto info = a.description();
        info.schema = schema;
        info.stride = d->schema;
        info.frameId = d->sequence + 1;
        info.instanceId = 0;
        d->mixed = QCatLightingFrame::create(info, mixed);
    }
    auto values = d->mixed.channels();
    const double gain = d->blackout ? 0 : d->master;
    if (gain == 0)
        values.fill(char(0));
    else if (gain != 1)
        for (auto &v : values)
            v = char(qRound(uchar(v) * gain));
    auto info = d->mixed.description();
    info.frameId = ++d->sequence;
    auto output = QCatLightingFrame::create(info, values);
    {
        QMutexLocker lock(&d->mutex);
        d->output = output;
    }
    d->error.clear();
    d->adapter->submit(output);
    emit statusChanged();
    emit frameReady();
    return true;
}
QVariantMap QCatLedMixer::configuration() const
{
    return {{"version", 1},
            {"crossfader", d->crossfader},
            {"master", d->master},
            {"workingSchema", d->schema},
            {"conversion", d->conversion}};
}
bool QCatLedMixer::restoreConfiguration(const QVariantMap &c)
{
    const double cross = c["crossfader"].toDouble(), master = c["master"].toDouble();
    if (c["version"].toInt() != 1 || !std::isfinite(cross) || cross < 0 || cross > 1 ||
        !std::isfinite(master) || master < 0 || master > 1 ||
        !QCatLighting::validSchema(c["workingSchema"].toInt()) || c["conversion"].toInt() < 0 ||
        c["conversion"].toInt() > 2)
        return false;
    d->crossfader = cross;
    d->master = master;
    d->schema = c["workingSchema"].toInt();
    d->conversion = c["conversion"].toInt();
    d->blackout = d->freeze = false;
    d->mixed = {};
    remix();
    emit settingsChanged();
    return true;
}

QVariantList QCatLedMixer::channelSample() const
{
    QVariantList out;
    const auto f = latestFrame();
    for (auto v : f.channels().left(f.description().stride))
        out << int(uchar(v));
    return out;
}
