// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatledproject.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QTemporaryDir>
#include <QThread>
#include <QtTest>
#include <cmath>
#include <limits>

namespace
{
QVariantMap solid(int schema, QVariantMap channels)
{
    return {{"configVersion", 1},
            {"name", "Test"},
            {"description", "Test preset"},
            {"providerId", "qcat.channels"},
            {"effectId", "solid"},
            {"effectVersion", 1},
            {"schema", schema},
            {"parameters", channels},
            {"brightness", 1.0},
            {"speed", 1.0},
            {"conversion", 2},
            {"empty", false}};
}
QCatLightingFrame frame(QCatLedLayoutPtr l, int schema, QByteArray bytes, quint64 id = 1)
{
    QCatLightingFrame::Description d;
    d.projectId = l->projectId;
    d.layoutRevision = l->revision;
    d.frameId = id;
    d.pointCount = int(l->positions.size());
    d.schema = QCatLighting::ColorSchema(schema);
    d.stride = schema;
    return QCatLightingFrame::create(d, bytes);
}
QVariantMap onePoint()
{
    auto c = QCatLedLayoutModel::defaults();
    c["countX"] = 1;
    c["countY"] = 1;
    return c;
}
QVariantList profiles(int count = 1)
{
    QVariantList out;
    for (int s = 3; s <= 5; ++s)
    {
        QCatFixtureProfile p;
        p.id = QString::number(s);
        p.deviceId = "memory";
        p.portId = p.id;
        p.schema = QCatLighting::ColorSchema(s);
        p.pointCount = count;
        p.startChannel = s;
        p.channelOrder = s == 3   ? QStringList{"G", "B", "R"}
                         : s == 4 ? QStringList{"W", "B", "G", "R"}
                                  : QStringList{"WW", "CW", "B", "G", "R"};
        p.conversion = QCatLighting::ApproximateWhite;
        out << p.configuration();
    }
    return out;
}
class SlowBackend : public QCatOutputBackend
{
  public:
    QMutex mutex;
    QCatDeviceFramePtr accepted;
    std::atomic_int attempts{0};
    bool submit(QCatDeviceFramePtr frame, const std::shared_ptr<std::atomic_bool> &cancelled,
                QString *) override
    {
        ++attempts;
        for (int i = 0; i < 40; ++i)
        {
            QThread::msleep(5);
            if (cancelled->load())
                return false;
        }
        QMutexLocker lock(&mutex);
        if (cancelled->load())
            return false;
        accepted = std::move(frame);
        return true;
    }
    void cancel() override {}
    QCatDeviceFramePtr latest()
    {
        QMutexLocker lock(&mutex);
        return accepted;
    }
};
class DelayedInstance : public QCatLedEffectInstance
{
  public:
    QByteArray render(const QCatLedLayoutData &l, const QVariantMap &p, QCatLighting::ColorSchema s,
                      double) override
    {
        QThread::msleep(80);
        return QByteArray(l.positions.size() * int(s), char(p["value"].toInt()));
    }
};
class DelayedProvider : public QCatLedEffectProvider
{
  public:
    QString id() const override
    {
        return "test.delay";
    }
    QVariantList effects() const override
    {
        return {QVariantMap{
            {"id", "delay"},
            {"version", 1},
            {"name", "Delayed"},
            {"schemas", QVariantList{3, 4, 5}},
            {"reverse", false},
            {"parameters",
             QVariantList{QVariantMap{
                 {"id", "value"}, {"name", "value"}, {"default", 0}, {"min", 0}, {"max", 255}}}}}};
    }
    std::unique_ptr<QCatLedEffectInstance> create(const QString &, int, const QCatLedLayoutData &,
                                                  QCatLighting::ColorSchema, const QVariantMap &,
                                                  QString *) const override
    {
        return std::make_unique<DelayedInstance>();
    }
};
} // namespace
class LedPipelineTests : public QObject
{
    Q_OBJECT
  private slots:
    void layoutMappingAndValidation()
    {
        QCatLedLayoutModel m;
        QCOMPARE(m.pointCount(), 2048);
        auto l = m.snapshot();
        QCOMPARE(l->positions[0].x, -31.5f);
        QCOMPARE(l->positions[0].y, 15.5f);
        QCOMPARE(l->positions[64].y, 14.5f);
        QCOMPARE(QCatLedLayoutData::index(3, 2, 1, 16, 16), qsizetype(291));
        auto c = m.configuration();
        c["type"] = "Matrix3D";
        c["countX"] = 16;
        c["countY"] = 16;
        c["countZ"] = 8;
        QVERIFY(m.applyNow(c));
        QCOMPARE(m.pointCount(), 2048);
        l = m.snapshot();
        QCOMPARE(l->positions[256].z, -2.5f);
        const auto revision = m.revision();
        c["countX"] = 1000000;
        c["countY"] = 1000000;
        QVERIFY(!m.applyNow(c));
        QCOMPARE(m.revision(), revision);
        c = onePoint();
        c["spacingX"] = std::numeric_limits<double>::infinity();
        QVERIFY(!m.applyNow(c));
        c = onePoint();
        c["countX"] = 0;
        QVERIFY(m.applyNow(c));
        QCOMPARE(m.pointCount(), 0);
        c = onePoint();
        QVERIFY(m.applyNow(c));
        QCOMPARE(m.pointCount(), 1);
        c["type"] = "Ring";
        c["count"] = 8;
        c["radius"] = 2;
        QVERIFY(m.applyNow(c));
        l = m.snapshot();
        QCOMPARE(l->positions[0].x, 2.0f);
        QVERIFY(l->positions.last().y != l->positions.first().y);
        c["type"] = "Disk";
        c["radialLayers"] = 3;
        c["angularPoints"] = 8;
        QVERIFY(m.applyNow(c));
        QCOMPARE(m.pointCount(), 25);
        int centers = 0;
        for (const auto &p : m.snapshot()->positions)
            centers += p.x == 0 && p.y == 0;
        QCOMPARE(centers, 1);
        c["type"] = "Custom";
        c["positions"] = QVariantList{QVariantList{1, 2, 3}};
        QVERIFY(m.applyNow(c));
        QCOMPARE(m.snapshot()->countX, 0);
    }
    void coalescedLayoutRequests()
    {
        QCatLedLayoutModel m;
        auto big = m.configuration();
        big["countX"] = 1000;
        big["countY"] = 1000;
        m.apply(big);
        auto last = onePoint();
        last["countX"] = 7;
        m.apply(last);
        QTRY_VERIFY_WITH_TIMEOUT(!m.busy(), 10000);
        QCOMPARE(m.pointCount(), 7);
    }
    void frameSemanticsAndPreview()
    {
        QCatLedLayoutModel m;
        QVERIFY(m.applyNow(onePoint()));
        auto l = m.snapshot();
        QCatLightingFrame::Description info;
        info.projectId = l->projectId;
        info.layoutRevision = l->revision;
        info.pointCount = 1;
        info.frameId = 1;
        const auto black = QCatLightingFrame::fromRgba8(info, QByteArray::fromHex("000000ff"));
        QVERIFY(black.isValid());
        QCOMPARE(black.channels().toHex(), QByteArray("000000"));
        QCOMPARE(black.opacity().toHex(), QByteArray("ff"));
        const auto white = frame(l, 4, QByteArray::fromHex("000000ff"));
        QVERIFY(white.isValid());
        QVERIFY(QCatPreviewColorAdapter::toRgba8(white) != QCatPreviewColorAdapter::toRgba8(black));
        const auto cold = frame(l, 5, QByteArray::fromHex("000000ff00")),
                   warm = frame(l, 5, QByteArray::fromHex("00000000ff"));
        const auto cp = QCatPreviewColorAdapter::toRgba8(cold),
                   wp = QCatPreviewColorAdapter::toRgba8(warm);
        QVERIFY(uchar(cp[2]) > uchar(wp[2]));
        QVERIFY(uchar(wp[0]) > uchar(cp[0]));
        info.schema = QCatLighting::RGBW;
        info.stride = 3;
        QString error;
        QVERIFY(!QCatLightingFrame::create(info, QByteArray(4, 0), {}, &error).isValid());
        QVERIFY(!error.isEmpty());
        QByteArray owned = QByteArray::fromHex("01020304");
        auto immutable = frame(l, 4, owned);
        owned[3] = 0;
        QCOMPARE(uchar(immutable.channels()[3]), 4);
        QChar projectName[] = {QChar('t'), QChar('e'), QChar('s'), QChar('t')};
        auto rawDescription = white.description();
        rawDescription.projectId = QString::fromRawData(projectName, 4);
        const auto rawIdentity = QCatLightingFrame::create(rawDescription, white.channels());
        projectName[0] = QChar('x');
        QCOMPARE(rawIdentity.description().projectId, QString("test"));
        QCatPreviewColorAdapter adapter;
        adapter.setLayout(l);
        QVERIFY(adapter.submit(cold));
        const auto sourceLayout = adapter.source()->layoutId();
        QVERIFY(m.applyNow(onePoint()));
        adapter.setLayout(m.snapshot());
        QVERIFY(!adapter.submit(cold));
        QVERIFY(adapter.source()->layoutId() != sourceLayout);
    }
    void previewConversionPreservesChannelsAndOpacity()
    {
        QCatLightingFrame::Description info;
        info.projectId = "preview-conversion";
        info.layoutRevision = info.frameId = 1;
        info.pointCount = 256;
        QByteArray opacity(info.pointCount, Qt::Uninitialized);
        for (int i = 0; i < info.pointCount; ++i)
            opacity[i] = char(i);
        for (int schema : {3, 4, 5})
        {
            info.schema = QCatLighting::ColorSchema(schema);
            info.stride = schema;
            QByteArray channels(info.pointCount * schema, Qt::Uninitialized);
            for (qsizetype i = 0; i < channels.size(); ++i)
                channels[i] = char((i * 37) % 256);
            const auto frame = QCatLightingFrame::create(info, channels, opacity);
            for (double whiteKelvin : {2700., 4000., 6500.})
            {
                const auto reference = QCatLighting::convert(
                    channels, info.schema, QCatLighting::RGB, QCatLighting::ApproximateWhite,
                    nullptr, whiteKelvin, 6000, 3000);
                const auto rgba = QCatPreviewColorAdapter::toRgba8(frame, whiteKelvin, 6000, 3000);
                QCOMPARE(rgba.size(), info.pointCount * 4);
                for (int i = 0; i < info.pointCount; ++i)
                {
                    QCOMPARE(rgba.mid(i * 4, 3), reference.mid(i * 3, 3));
                    QCOMPARE(rgba[i * 4 + 3], opacity[i]);
                }
            }
            QCOMPARE(frame.channels(), channels);
            QCOMPARE(frame.opacity(), opacity);
        }
    }
    void mixWhiteMasterFreezeAndIdentity()
    {
        QCatLedLayoutModel m;
        QVERIFY(m.applyNow(onePoint()));
        QCatLedMixer mixer;
        mixer.setLayout(&m);
        auto a = frame(m.snapshot(), 5, QByteArray::fromHex("000000ff00")),
             b = frame(m.snapshot(), 5, QByteArray::fromHex("00000000ff"));
        QVERIFY(mixer.mixFrames(a, b));
        QCOMPARE(mixer.latestFrame().channels().toHex(), QByteArray("0000008080"));
        mixer.setCrossfader(0);
        QCOMPARE(mixer.latestFrame().channels(), a.channels());
        mixer.setCrossfader(1);
        QCOMPARE(mixer.latestFrame().channels(), b.channels());
        mixer.setCrossfader(.5);
        mixer.setFreeze(true);
        mixer.setCrossfader(0);
        QCOMPARE(mixer.latestFrame().channels().toHex(), QByteArray("0000008080"));
        mixer.setMaster(.5);
        QCOMPARE(mixer.latestFrame().channels().toHex(), QByteArray("0000004040"));
        mixer.setBlackout(true);
        QCOMPARE(mixer.latestFrame().channels(), QByteArray(5, 0));
        mixer.setBlackout(false);
        QCOMPARE(mixer.latestFrame().channels().toHex(), QByteArray("0000004040"));
        mixer.setMaster(0);
        QCOMPARE(mixer.latestFrame().channels(), QByteArray(5, 0));
        mixer.setMaster(1);
        mixer.setFreeze(false);
        const auto valid = mixer.latestFrame().channels();
        auto other = QCatLedLayoutModel::generate(onePoint(), "other-project", m.revision());
        QVERIFY(!mixer.mixFrames(frame(other, 5, QByteArray(5, 1)), b));
        QCOMPARE(mixer.latestFrame().channels(), valid);
        QVERIFY(!mixer.mixFrames(frame(m.snapshot(), 3, QByteArray::fromHex("ff0000")), b));
        mixer.setConversion(QCatLighting::AddWhiteZero);
        QVERIFY(mixer.mixFrames(frame(m.snapshot(), 3, QByteArray::fromHex("ff0000")), b));
    }
    void deckClockStorageIsolationAndSave()
    {
        QCatLedProject project;
        QVERIFY(project.layout()->applyNow(onePoint()));
        auto *repo = project.repository();
        repo->populateExamples();
        const auto s = repo->storages()[0].toMap()["id"].toString(),
                   other = repo->storages()[1].toMap()["id"].toString(),
                   p = repo->places(s)[0].toMap()["id"].toString();
        auto *a = project.deckA(), *b = project.deckB();
        QVERIFY(a->activatePlace(s, p));
        QVERIFY(b->loadPlace(s, p));
        QTRY_VERIFY(!a->loading() && !b->loading());
        QTRY_VERIFY(a->effectTime() > .05);
        QCOMPARE(b->effectTime(), 0.0);
        QVERIFY(!b->playing());
        a->pause();
        const double held = a->effectTime();
        QTest::qWait(130);
        QCOMPARE(a->effectTime(), held);
        project.mixer()->setCrossfader(.7);
        QCOMPARE(a->effectTime(), held);
        a->play();
        QTest::qWait(70);
        a->pause();
        QVERIFY(a->effectTime() - held < .11);
        QVERIFY(a->effectTime() > held);
        a->setSpeed(0);
        a->play();
        const double t = a->effectTime();
        QTest::qWait(80);
        QCOMPARE(a->effectTime(), t);
        a->pause();
        a->browseStorage(other);
        QCOMPARE(a->activeStorage(), s);
        QCOMPARE(a->activePlace(), p);
        QVERIFY(!a->loadPlace(s, repo->places(s)[9].toMap()["id"].toString()));
        QCOMPARE(a->activePlace(), p);
        QVERIFY(a->setParameter("saturation", .25));
        QTRY_VERIFY(!a->loading());
        QCOMPARE(a->workingPlace()["parameters"].toMap()["saturation"].toDouble(), .25);
        QCOMPARE(b->workingPlace()["parameters"].toMap()["saturation"].toDouble(), 1.0);
        QCOMPARE(repo->place(s, p)["parameters"].toMap()["saturation"].toDouble(), 1.0);
        QVERIFY(a->savePlace(s, p, "Edited"));
        QCOMPARE(repo->place(s, p)["name"].toString(), QString("Edited"));
        QCOMPARE(b->workingPlace()["name"].toString(), QString("Prism"));
        const double before = a->effectTime();
        QTRY_VERIFY(!repo->thumbnail(s, p, project.registry()).isEmpty());
        QCOMPARE(a->effectTime(), before);
        a->stop();
        QCOMPARE(a->effectTime(), 0.0);
        QCOMPARE(a->latestFrame().channels(), QByteArray(5, 0));
        QCOMPARE(a->activePlace(), p);
        a->setSpeed(1);
        a->restart();
        QTRY_VERIFY(a->effectTime() > 0);
        a->pause();
        QVERIFY(a->next());
        QTRY_VERIFY(!a->loading());
        QCOMPARE(a->activeStorage(), s);
        QVERIFY(repo->clearPlace(s, p));
        QVERIFY(!b->workingPlace().isEmpty());
        repo->setReadOnly(true);
        QVERIFY(!a->savePlace(s, p, "No"));
    }
    void brightnessLayersAndWhiteAnimation()
    {
        QCatLedProject p;
        QVERIFY(p.layout()->applyNow(onePoint()));
        auto preset = solid(5, {{"R", 1}, {"G", 1}, {"B", 1}, {"CW", 1}, {"WW", 1}});
        preset["brightness"] = .5;
        p.deckA()->setBrightness(.5);
        p.deckB()->setBrightness(.5);
        QVERIFY(p.deckA()->loadConfiguration(preset));
        QVERIFY(p.deckB()->loadConfiguration(preset));
        QTRY_VERIFY(!p.deckA()->loading() && !p.deckB()->loading());
        QCOMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, char(64)));
        p.mixer()->setMaster(.5);
        QTRY_COMPARE(p.mixer()->latestFrame().channels(), QByteArray(5, char(32)));
        auto chase = solid(5, {{"B", 0}, {"channel", 4}, {"level", 1}, {"frequency", 2}});
        chase["effectId"] = "chase";
        QVERIFY(p.deckA()->loadConfiguration(chase, true));
        QTRY_VERIFY(!p.deckA()->loading());
        const auto initial = p.deckA()->latestFrame().channels();
        QTRY_VERIFY(p.deckA()->latestFrame().channels() != initial);
        QCOMPARE(p.deckA()->latestFrame().channels().left(4), QByteArray(4, 0));
    }
    void latestSelectionWinsAndFailureKeepsPlayback()
    {
        QCatLedProject p;
        QVERIFY(p.layout()->applyNow(onePoint()));
        QVERIFY(p.registry()->registerProvider(std::make_shared<DelayedProvider>()));
        auto preset = solid(5, {{"value", 11}});
        preset["providerId"] = "test.delay";
        preset["effectId"] = "delay";
        QVERIFY(p.deckA()->loadConfiguration(preset));
        preset["parameters"] = QVariantMap{{"value", 222}};
        QVERIFY(p.deckA()->loadConfiguration(preset));
        QTRY_VERIFY_WITH_TIMEOUT(!p.deckA()->loading(), 2000);
        QCOMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, char(222)));
        preset["providerId"] = "missing";
        QVERIFY(!p.deckA()->loadConfiguration(preset));
        QCOMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, char(222)));
        preset["providerId"] = "test.delay";
        preset["effectVersion"] = 2;
        QVERIFY(!p.deckA()->loadConfiguration(preset));
    }
    void layoutChangePreservesPendingSelectionAndStoppedDeck()
    {
        QCatLedProject p;
        QVERIFY(p.registry()->registerProvider(std::make_shared<DelayedProvider>()));
        auto preset = solid(5, {{"value", 123}});
        preset["providerId"] = "test.delay";
        preset["effectId"] = "delay";
        QVERIFY(p.deckA()->loadConfiguration(preset, true));
        QVERIFY(p.layout()->applyNow(onePoint()));
        QTRY_VERIFY_WITH_TIMEOUT(!p.deckA()->loading(), 3000);
        QCOMPARE(p.deckA()->workingPlace()["effectId"].toString(), QString("delay"));
        QVERIFY(p.deckA()->playing());
        QCOMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, char(123)));
        QCOMPARE(p.deckA()->latestFrame().description().layoutRevision, p.layout()->revision());
        p.deckA()->stop();
        auto c = onePoint();
        c["spacingY"] = 2;
        QVERIFY(p.layout()->applyNow(c));
        QTRY_VERIFY_WITH_TIMEOUT(!p.deckA()->loading(), 3000);
        QVERIFY(!p.deckA()->playing());
        QCOMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, 0));
        QVERIFY(p.deckA()->setParameter("value", 55));
        QTRY_VERIFY(!p.deckA()->loading());
        QCOMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, 0));
        p.deckA()->play();
        QTRY_COMPARE(p.deckA()->latestFrame().channels(), QByteArray(5, char(55)));
    }
    void repositoryJsonRoundTripAndConflicts()
    {
        QCatLedStorageRepository repo;
        repo.populateExamples();
        auto s = repo.storages()[1].toMap()["id"].toString(),
             p = repo.places(s)[1].toMap()["id"].toString();
        const auto original = repo.place(s, p);
        QCatLedStorageRepository copy;
        QVERIFY(copy.importJson(repo.exportJson()));
        QCOMPARE(copy.place(s, p), original);
        QVERIFY(copy.importJson(repo.exportJson(s)));
        QCOMPARE(copy.storages().size(), 3);
        QVERIFY(copy.storages().last().toMap()["id"] != s);
        auto target = copy.places(s)[8].toMap()["id"].toString();
        QVERIFY(copy.importJson(repo.exportJson(s, p), s, target));
        QCOMPARE(copy.place(s, target)["schema"], original["schema"]);
        QCOMPARE(copy.place(s, target)["parameters"], original["parameters"]);
        const auto before = copy.document();
        QVERIFY(!copy.importJson("{bad"));
        QCOMPARE(copy.document(), before);
        QTemporaryDir dir;
        auto url = QUrl::fromLocalFile(dir.filePath("presets.json"));
        QVERIFY(repo.saveFile(url));
        QCatLedStorageRepository local;
        QVERIFY(local.loadFile(url));
        QCOMPARE(local.place(s, p), original);
        repo.setReadOnly(true);
        QVERIFY(!repo.clearPlace(s, p));
        QCOMPARE(repo.place(s, p), original);
    }
    void patchOrderingConversionAndHeadlessOutput()
    {
        QCatLedProject p;
        QVERIFY(p.layout()->applyNow(onePoint()));
        const auto l = p.layout()->snapshot();
        const auto f = frame(l, 5, QByteArray::fromHex("0102034080"));
        QString error;
        const auto mapped = QCatPatchMapper::map(f, l, profiles(), 7, &error);
        QVERIFY2(mapped, qPrintable(error));
        QCOMPARE(mapped->ports.size(), 3);
        QCOMPARE(mapped->ports[2].channels.toHex(), QByteArray("000000008040030201"));
        QCOMPARE(mapped->ports[1].channels.toHex(), QByteArray("000000c0030201"));
        QCOMPARE(mapped->mappingRevision, quint64(7));
        auto invalid = profiles();
        auto first = invalid[0].toMap();
        first["pointCount"] = 2;
        invalid[0] = first;
        QVERIFY(!QCatPatchMapper::map(f, l, invalid, 8, &error));
        QString e;
        QVERIFY(QCatLighting::convert(f.channels(), QCatLighting::RGB_CW_WW, QCatLighting::RGB,
                                      QCatLighting::Strict, &e)
                    .isEmpty());
        QVERIFY(!e.isEmpty());
        QVERIFY(p.output()->applyProfiles(profiles()));
        QVERIFY(p.deckA()->loadConfiguration(solid(5, {{"B", 0}, {"CW", 1}, {"WW", 0}})));
        QVERIFY(p.deckB()->loadConfiguration(solid(5, {{"B", 0}, {"CW", 0}, {"WW", 1}})));
        QTRY_VERIFY(!p.deckA()->loading() && !p.deckB()->loading());
        QTRY_VERIFY(p.memoryBackend()->receivedFrames() > 1);
        QTRY_COMPARE(p.memoryBackend()->latestFrame()->ports[2].channels.mid(4).toHex(),
                     QByteArray("8080000000"));
        auto count = p.memoryBackend()->receivedFrames();
        QTRY_VERIFY(p.memoryBackend()->receivedFrames() > count);
        p.mixer()->setBlackout(true);
        QTRY_COMPARE(p.memoryBackend()->latestFrame()->ports[2].channels.mid(4), QByteArray(5, 0));
    }
    void slowOutputCancelsOldMappingAndLayout()
    {
        QCatLedProject p;
        QVERIFY(p.layout()->applyNow(onePoint()));
        QVERIFY(p.deckA()->loadConfiguration(solid(5, {{"B", 0}, {"CW", 1}})));
        QVERIFY(p.deckB()->loadConfiguration(solid(5, {{"B", 0}, {"WW", 1}})));
        QTRY_VERIFY(!p.deckA()->loading() && !p.deckB()->loading());
        auto backend = std::make_shared<SlowBackend>();
        p.output()->setBackend(backend);
        QVERIFY(p.output()->applyProfiles(profiles()));
        QTRY_VERIFY(backend->attempts.load() > 0);
        auto patch = profiles();
        auto group = patch[2].toMap();
        group["startChannel"] = 20;
        patch[2] = group;
        QVERIFY(p.output()->applyProfiles(patch));
        auto config = onePoint();
        config["spacingX"] = 2;
        QVERIFY(p.layout()->applyNow(config));
        const auto revision = p.layout()->revision();
        QTRY_VERIFY_WITH_TIMEOUT(backend->latest() != nullptr, 3000);
        QCOMPARE(backend->latest()->layoutRevision, revision);
        QCOMPARE(backend->latest()->ports[2].channels.size(), 24);
        QVERIFY(backend->attempts.load() >= 2);
    }
    void projectRollbackAndRestore()
    {
        QCatLedProject p;
        QVERIFY(p.layout()->applyNow(onePoint()));
        p.repository()->populateExamples();
        QVERIFY(p.deckA()->loadConfiguration(solid(4, {{"B", 0}, {"W", .7}})));
        QVERIFY(p.deckB()->loadConfiguration(solid(5, {{"B", 0}, {"CW", .2}, {"WW", .8}})));
        QTRY_VERIFY(!p.deckA()->loading() && !p.deckB()->loading());
        p.mixer()->setConversion(2);
        p.mixer()->setMaster(.8);
        QVERIFY(p.output()->applyProfiles(profiles()));
        const auto saved = p.exportJson();
        auto doc = QJsonDocument::fromJson(saved.toUtf8()).object();
        auto layout = doc["layout"].toObject();
        layout["countX"] = -1;
        doc["layout"] = layout;
        QVERIFY(!p.importJson(QString::fromUtf8(QJsonDocument(doc).toJson())));
        QCOMPARE(p.exportJson(), saved);
        p.mixer()->setBlackout(true);
        p.mixer()->setFreeze(true);
        QVERIFY(p.importJson(saved));
        QTRY_VERIFY(!p.deckA()->loading() && !p.deckB()->loading());
        QVERIFY(!p.mixer()->blackout());
        QVERIFY(!p.mixer()->freeze());
        QVERIFY(!p.deckA()->playing());
        QCOMPARE(p.deckA()->effectTime(), 0.0);
        QCOMPARE(p.deckA()->latestFrame().description().schema, QCatLighting::RGBW);
        QCOMPARE(uchar(p.deckA()->latestFrame().channels()[3]), 179);
    }
};
QTEST_GUILESS_MAIN(LedPipelineTests)
#include "tst_ledpipeline.moc"
