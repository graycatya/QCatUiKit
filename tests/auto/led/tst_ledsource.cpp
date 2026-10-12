// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatledsource.h>
#include <QThread>
#include <QtTest>
#include <limits>

class LedSourceTests : public QObject
{
    Q_OBJECT
  private slots:
    void validationAndLatestFrame()
    {
        QCatLedSource source;
        QSignalSpy changes(&source, &QCatLedSource::dataChanged);
        QCOMPARE(source.pointCount(), 0);
        const auto layout = source.setLayout({{0, 0, 0}, {0, 0, 0}});
        QVERIFY(layout > 0);
        QCOMPARE(source.pointCount(), 2);
        QVERIFY(!source.submitFrame(QByteArray(4, '\xff'), 1, layout));
        QVERIFY(source.lastError().contains("mismatch"));
        QVERIFY(source.submitFrame(QByteArray(8, '\xff'), 1, layout));
        QVERIFY(source.lastError().isEmpty());
        QVERIFY(!source.submitFrame(QByteArray(8, '\xff'), 1, layout));
        QVERIFY(source.lastError().contains("frameId"));
        const auto replacement = source.setLayout({{1, 2, 3}, {4, 5, 6}});
        QVERIFY(replacement != layout);
        QVERIFY(!source.submitFrame(QByteArray(8, '\xff'), 2, layout));
        QCOMPARE(source.frameId(), quint64(0));
        QVERIFY(source.submitFrame(QByteArray(8, '\xff'), 1, replacement));
        // GUI event loop is intentionally blocked while producer publishes 10,000 frames.
        auto *worker = QThread::create(
            [&]
            {
                for (quint64 id = 2; id <= 10001; ++id)
                    source.submitFrame(QByteArray(8, char(id)), id, replacement);
            });
        worker->start();
        QVERIFY(worker->wait(10000));
        delete worker;
        QCOMPARE(source.frameId(), quint64(10001));
        QCOMPARE(changes.size(), 0);
        QTRY_COMPARE(changes.size(), 1);
        QCOMPARE(source.submittedFrames(), quint64(10002));
        QCOMPARE(source.setLayout({{std::numeric_limits<float>::quiet_NaN(), 0, 0}}), quint64(0));
        QCOMPARE(source.pointCount(), 2);
        QVERIFY(!source.lastError().isEmpty());
        const auto empty = source.setLayout({});
        QVERIFY(empty > 0);
        QVERIFY(source.submitFrame({}, 1, empty));
        QCOMPARE(source.pointCount(), 0);
    }
};
QTEST_GUILESS_MAIN(LedSourceTests)
#include "tst_ledsource.moc"
