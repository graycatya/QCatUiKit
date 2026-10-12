// SPDX-License-Identifier: Apache-2.0
#include "galleryleddemo.h"
#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include <cmath>
#include <mutex>

struct GalleryLedDemo::Impl
{
    QCatLedSource a, b, output;
    std::mutex mutex;
    int count = 4096, interval = 16;
    bool spatial = false, running = true, stop = false;
    double mix = 0.5;
    quint64 revision = 1;
    QThread *worker = nullptr;

    void generate()
    {
        int layoutCount = -1;
        bool layoutSpatial = false;
        quint64 frameId = 0, lastRevision = 0;
        quint64 aId = 0, bId = 0, outputId = 0;
        QVector<QCatLedSource::Position> positions;
        QByteArray colorsA, colorsB, colorsOutput;
        QElapsedTimer clock;
        clock.start();
        QEventLoop loop;
        QTimer timer;
        timer.setTimerType(Qt::PreciseTimer);
        timer.setInterval(16);
        QObject::connect(
            &timer, &QTimer::timeout, &loop,
            [&]
            {
                int newCount, ms;
                bool newSpatial, animate;
                double blend;
                quint64 currentRevision;
                {
                    std::lock_guard lock(mutex);
                    if (stop)
                    {
                        loop.quit();
                        return;
                    }
                    newCount = count;
                    newSpatial = spatial;
                    animate = running;
                    ms = interval;
                    blend = mix;
                    currentRevision = revision;
                }
                if (timer.interval() != ms)
                    timer.setInterval(ms);
                if (!animate && currentRevision == lastRevision)
                    return;
                if (layoutCount != newCount || layoutSpatial != newSpatial)
                {
                    layoutCount = newCount;
                    layoutSpatial = newSpatial;
                    positions.resize(newCount);
                    const int side = qMax(1, int(std::ceil(std::sqrt(newCount))));
                    for (int i = 0; i < newCount; ++i)
                    {
                        if (newSpatial)
                        {
                            const double y = 1.0 - 2.0 * (i + 0.5) / qMax(1, newCount);
                            const double r = std::sqrt(qMax(0.0, 1.0 - y * y));
                            const double angle = i * 2.399963229728653;
                            positions[i] = {float(r * std::cos(angle)), float(y),
                                            float(r * std::sin(angle))};
                        }
                        else
                            positions[i] = {float(i % side), -float(i / side), 0};
                    }
                    aId = a.setLayout(positions);
                    bId = b.setLayout(positions);
                    outputId = output.setLayout(positions);
                    colorsA.resize(newCount * 4);
                    colorsB.resize(newCount * 4);
                    colorsOutput.resize(newCount * 4);
                }
                const double t = clock.elapsed() / 1000.0;
                const double inverseScale = 1.0 / qMax(1.0, std::sqrt(newCount));
                auto *ca = reinterpret_cast<unsigned char *>(colorsA.data());
                auto *cb = reinterpret_cast<unsigned char *>(colorsB.data());
                auto *co = reinterpret_cast<unsigned char *>(colorsOutput.data());
                for (int i = 0; i < newCount; ++i)
                {
                    const auto &p = positions[i];
                    const double x = newSpatial ? p.x : p.x * inverseScale;
                    const double y = newSpatial ? p.y : p.y * inverseScale;
                    const double phase = (x - y) * 4.0 + t;
                    // An off-center emitter gives spatial ripples on a sphere too;
                    // distance from the sphere's center would be constant for all LEDs.
                    const double waveX = x - 0.35;
                    const double waveY = y + 0.25;
                    const double waveZ = p.z - 0.15;
                    const double wave =
                        0.5 +
                        0.5 *
                            std::sin(std::sqrt(waveX * waveX + waveY * waveY + waveZ * waveZ) * 16 -
                                     t * 3);
                    for (int c = 0; c < 3; ++c)
                    {
                        ca[i * 4 + c] = static_cast<unsigned char>(
                            127.5 + 127.5 * std::sin(phase + c * 2.094395102393195));
                        const double channel = c == 0 ? 0.2 : c == 1 ? 0.65 : 1.0;
                        cb[i * 4 + c] = static_cast<unsigned char>(255 * wave * channel);
                        co[i * 4 + c] = static_cast<unsigned char>(
                            std::lround(ca[i * 4 + c] * (1 - blend) + cb[i * 4 + c] * blend));
                    }
                    ca[i * 4 + 3] = cb[i * 4 + 3] = co[i * 4 + 3] = 255;
                }
                ++frameId;
                a.submitFrame(colorsA, frameId, aId);
                b.submitFrame(colorsB, frameId, bId);
                output.submitFrame(colorsOutput, frameId, outputId);
                lastRevision = currentRevision;
            });
        timer.start();
        loop.exec();
    }
};

GalleryLedDemo::GalleryLedDemo(QObject *parent) : QObject(parent), d(new Impl)
{
    d->worker = QThread::create([this] { d->generate(); });
    d->worker->setObjectName(QStringLiteral("LED frame producer"));
    d->worker->start();
}
GalleryLedDemo::~GalleryLedDemo()
{
    {
        std::lock_guard lock(d->mutex);
        d->stop = true;
    }
    d->worker->quit();
    d->worker->wait();
    delete d->worker;
}
QCatLedSource *GalleryLedDemo::deckA() const
{
    return &d->a;
}
QCatLedSource *GalleryLedDemo::deckB() const
{
    return &d->b;
}
QCatLedSource *GalleryLedDemo::output() const
{
    return &d->output;
}
int GalleryLedDemo::pointCount() const
{
    std::lock_guard lock(d->mutex);
    return d->count;
}
bool GalleryLedDemo::spatial() const
{
    std::lock_guard lock(d->mutex);
    return d->spatial;
}
qreal GalleryLedDemo::mix() const
{
    std::lock_guard lock(d->mutex);
    return d->mix;
}
bool GalleryLedDemo::running() const
{
    std::lock_guard lock(d->mutex);
    return d->running;
}
int GalleryLedDemo::interval() const
{
    std::lock_guard lock(d->mutex);
    return d->interval;
}
void GalleryLedDemo::setPointCount(int count)
{
    count = qBound(0, count, 100000);
    {
        std::lock_guard lock(d->mutex);
        if (d->count == count)
            return;
        d->count = count;
        ++d->revision;
    }
    emit settingsChanged();
}
void GalleryLedDemo::setSpatial(bool spatial)
{
    {
        std::lock_guard lock(d->mutex);
        if (d->spatial == spatial)
            return;
        d->spatial = spatial;
        ++d->revision;
    }
    emit settingsChanged();
}
void GalleryLedDemo::setMix(qreal mix)
{
    if (!std::isfinite(mix))
        return;
    mix = qBound(0.0, mix, 1.0);
    {
        std::lock_guard lock(d->mutex);
        if (d->mix == mix)
            return;
        d->mix = mix;
        ++d->revision;
    }
    emit settingsChanged();
}
void GalleryLedDemo::setRunning(bool running)
{
    {
        std::lock_guard lock(d->mutex);
        if (d->running == running)
            return;
        d->running = running;
        ++d->revision;
    }
    emit settingsChanged();
}
void GalleryLedDemo::setInterval(int ms)
{
    ms = qBound(4, ms, 1000);
    {
        std::lock_guard lock(d->mutex);
        if (d->interval == ms)
            return;
        d->interval = ms;
        ++d->revision;
    }
    emit settingsChanged();
}
