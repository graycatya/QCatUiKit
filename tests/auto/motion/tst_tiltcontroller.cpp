// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/TiltController>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <cmath>
#include <limits>
using namespace QCatUiKit;

class TiltTest : public QObject
{
    Q_OBJECT
    static QQuaternion pose(float right, float forward = 0)
    {
        return QQuaternion::fromAxisAndAngle(0, 1, 0, right) *
               QQuaternion::fromAxisAndAngle(1, 0, 0, -forward);
    }
    static void prepare(TiltController &c, QQuaternion zero = {})
    {
        c.setCalibrationDurationMs(100);
        c.setFilterTimeConstantMs(0);
        c.setExpo(0);
        c.setEnabled(true);
        QVERIFY(c.submitOrientation(zero, 0));
        QVERIFY(c.calibrate());
        for (int i = 1; i <= 5; ++i)
            QVERIFY(c.submitOrientation(i % 2 ? -zero : zero, i * 20000));
        QCOMPARE(c.state(), TiltController::Ready);
        QVERIFY(c.calibrated());
        QCOMPARE(c.rollInput(), 0);
        QCOMPARE(c.pitchInput(), 0);
    }
  private slots:
    void explicitEnableAndEngagement()
    {
        TiltController c;
        QCOMPARE(c.state(), TiltController::Disabled);
        QVERIFY(!c.submitOrientation({}, 0));
        QVERIFY(!c.calibrate());
        QVERIFY(!c.engage());
        prepare(c);
        QVERIFY(!c.engage());
        c.setControlAllowed(true);
        QVERIFY(!c.outputValid());
        QVERIFY(c.engage());
        QVERIFY(c.outputValid());
        c.setControlAllowed(false);
        QCOMPARE(c.state(), TiltController::Ready);
        QVERIFY(!c.outputValid());
        c.setControlAllowed(true);
        QVERIFY(!c.outputValid());
        QVERIFY(c.engage());
        c.setEnabled(false);
        QCOMPARE(c.state(), TiltController::Disabled);
        QVERIFY(!c.calibrated());
        QVERIFY(!c.sampleFresh());
        QVERIFY(!c.outputValid());
        c.setEnabled(true);
        QVERIFY(!c.engage());
    }
    void mappingDirectionsDeadzoneAndSaturation()
    {
        TiltController c;
        prepare(c);
        QVERIFY(c.submitOrientation(pose(1), 120000));
        QCOMPARE(c.rollInput(), 0);
        QVERIFY(c.submitOrientation(pose(13.5), 140000));
        QVERIFY(std::abs(c.rollInput() - .5) < 1e-6);
        QVERIFY(c.submitOrientation(pose(-40), 160000));
        QCOMPARE(c.rollInput(), -1);
        QVERIFY(c.submitOrientation(pose(0, 25), 180000));
        QVERIFY(std::abs(c.pitchInput() - 1) < 1e-6);
        QVERIFY(std::abs(c.rollInput()) < 1e-6);
        QVERIFY(c.submitOrientation(pose(0, -13.5), 200000));
        QVERIFY(std::abs(c.pitchInput() + .5) < 1e-6);
    }
    void calibrationAtArbitraryOrientationAndQuaternionSign()
    {
        TiltController c;
        const auto zero = QQuaternion::fromAxisAndAngle(0, 0, 1, 179) * pose(30, -35);
        prepare(c, zero);
        QVERIFY(c.submitOrientation(-(zero * pose(25)), 120000));
        QVERIFY(std::abs(c.rollInput() - 1) < 1e-6);
        QVERIFY(std::abs(c.pitchInput()) < 1e-6);
    }
    void yawDoesNotProduceTilt()
    {
        TiltController c;
        prepare(c);
        QVERIFY(c.submitOrientation(QQuaternion::fromAxisAndAngle(0, 0, 1, 179), 120000));
        QCOMPARE(c.rollInput(), 0);
        QCOMPARE(c.pitchInput(), 0);
        QVERIFY(c.submitOrientation(QQuaternion::fromAxisAndAngle(0, 0, 1, -179), 140000));
        QCOMPARE(c.rollInput(), 0);
        QCOMPARE(c.pitchInput(), 0);
    }
    void physicalForwardDirection()
    {
        TiltController c;
        prepare(c);
        // A negative right-handed X rotation lowers the phone's +Y (top) edge.
        QVERIFY(c.submitOrientation(QQuaternion::fromAxisAndAngle(1, 0, 0, -25), 120000));
        QVERIFY(std::abs(c.pitchInput() - 1) < 1e-6);
        QCOMPARE(c.rollInput(), 0);
    }
    void inputNormalization()
    {
        TiltController c;
        prepare(c);
        QVERIFY(c.submitOrientation(pose(13.5) * 4, 120000));
        QVERIFY(std::abs(c.rollInput() - .5) < 1e-6);
    }
    void unstableCalibrationRestartsWindow()
    {
        TiltController c;
        c.setCalibrationDurationMs(100);
        c.setEnabled(true);
        QVERIFY(c.submitOrientation({}, 0));
        QVERIFY(c.calibrate());
        QVERIFY(c.submitOrientation({}, 60000));
        QVERIFY(c.submitOrientation(pose(10), 100000));
        QVERIFY(c.submitOrientation(pose(10), 160000));
        QCOMPARE(c.state(), TiltController::Calibrating);
        QVERIFY(c.submitOrientation(pose(10), 200000));
        QCOMPARE(c.state(), TiltController::Ready);
        QVERIFY(c.submitOrientation(pose(10), 220000));
        QCOMPARE(c.rollInput(), 0);
    }
    void filterUsesActualDt()
    {
        TiltController a, b;
        prepare(a);
        prepare(b);
        a.setDeadzoneDegrees(0);
        b.setDeadzoneDegrees(0);
        a.setFilterTimeConstantMs(100);
        b.setFilterTimeConstantMs(100);
        for (int i = 1; i <= 10; ++i)
            QVERIFY(a.submitOrientation(pose(20), 100000 + i * 10000));
        for (int i = 1; i <= 5; ++i)
            QVERIFY(b.submitOrientation(pose(20), 100000 + i * 20000));
        QVERIFY(std::abs(a.rollInput() - b.rollInput()) < 1e-6);
        QVERIFY(std::abs(a.rollInput() - .8 * (1 - std::exp(-1.0))) < 1e-6);
    }
    void curveAndInversion()
    {
        TiltController c;
        prepare(c);
        c.setExpo(1);
        c.setInvertRoll(true);
        c.setInvertPitch(true);
        QVERIFY(c.submitOrientation(pose(13.5), 120000));
        QVERIFY(std::abs(c.rollInput() + .125) < 1e-6);
        QVERIFY(c.submitOrientation(pose(0, 13.5), 140000));
        QVERIFY(std::abs(c.pitchInput() + .125) < 1e-6);
    }
    void invalidSamples_data()
    {
        QTest::addColumn<QQuaternion>("q");
        QTest::addColumn<qint64>("timestamp");
        QTest::newRow("zero") << QQuaternion(0, 0, 0, 0) << qint64(120000);
        QTest::newRow("nan") << QQuaternion(std::numeric_limits<float>::quiet_NaN(), 0, 0, 0)
                             << qint64(120000);
        QTest::newRow("infinite") << QQuaternion(1, std::numeric_limits<float>::infinity(), 0, 0)
                                  << qint64(120000);
        QTest::newRow("duplicate-time") << QQuaternion() << qint64(100000);
        QTest::newRow("backwards-time") << QQuaternion() << qint64(90000);
        QTest::newRow("negative-time") << QQuaternion() << qint64(-1);
        QTest::newRow("sample-gap") << QQuaternion() << qint64(400000);
        QTest::newRow("excessive-tilt") << pose(80) << qint64(120000);
    }
    void invalidSamples()
    {
        QFETCH(QQuaternion, q);
        QFETCH(qint64, timestamp);
        TiltController c;
        prepare(c);
        c.setControlAllowed(true);
        QVERIFY(c.engage());
        QSignalSpy fault(&c, &TiltController::faultOccurred);
        QSignalSpy samples(&c, &TiltController::controlSample);
        QVERIFY(!c.submitOrientation(q, timestamp));
        QCOMPARE(c.state(), TiltController::Fault);
        QVERIFY(!c.outputValid());
        QVERIFY(!c.sampleFresh());
        QVERIFY(!c.calibrated());
        QCOMPARE(c.rollInput(), 0);
        QCOMPARE(fault.count(), 1);
        QVERIFY(!samples.last()[2].toBool());
        QVERIFY(!c.submitOrientation({}, 500000));
        c.resetCalibration();
        QCOMPARE(c.state(), TiltController::Uncalibrated);
        QVERIFY(c.error().isEmpty());
        QVERIFY(c.submitOrientation({}, 0));
        QVERIFY(!c.engage());
    }
    void watchdogRevokesOutputWithoutNewSamples()
    {
        TiltController c;
        prepare(c);
        c.setSampleTimeoutMs(30);
        c.setControlAllowed(true);
        QVERIFY(c.engage());
        QTRY_COMPARE_WITH_TIMEOUT(c.state(), TiltController::Fault, 200);
        QVERIFY(!c.outputValid());
        QVERIFY(!c.sampleFresh());
    }
    void stalledEventLoopCannotEngageStaleInput()
    {
        TiltController c;
        prepare(c);
        c.setSampleTimeoutMs(20);
        c.setControlAllowed(true);
        QThread::msleep(30);
        QVERIFY(!c.engage());
        QCOMPARE(c.state(), TiltController::Fault);
    }
    void parameterValidationAndActiveChanges()
    {
        TiltController c;
        prepare(c);
        c.setControlAllowed(true);
        QVERIFY(c.engage());
        c.setDeadzoneDegrees(25);
        c.setFullScaleAngleDegrees(0);
        c.setExpo(2);
        c.setFilterTimeConstantMs(std::numeric_limits<double>::quiet_NaN());
        c.setSampleTimeoutMs(0);
        QCOMPARE(c.deadzoneDegrees(), 2);
        QCOMPARE(c.fullScaleAngleDegrees(), 25);
        QCOMPARE(c.expo(), 0);
        QCOMPARE(c.filterTimeConstantMs(), 0);
        QCOMPARE(c.sampleTimeoutMs(), 250);
        QVERIFY(c.outputValid());
        c.setFullScaleAngleDegrees(30);
        QVERIFY(!c.outputValid());
        QCOMPARE(c.state(), TiltController::Ready);
    }
    void resetCancelsCalibration()
    {
        TiltController c;
        c.setEnabled(true);
        QVERIFY(c.submitOrientation({}, 0));
        QVERIFY(c.calibrate());
        QSignalSpy result(&c, &TiltController::calibrationFinished);
        c.resetCalibration();
        QCOMPARE(result.count(), 1);
        QCOMPARE(result.first()[0].toBool(), false);
        QCOMPARE(c.state(), TiltController::Uncalibrated);
        QVERIFY(!c.sampleFresh());
    }
};
QTEST_GUILESS_MAIN(TiltTest)
#include "tst_tiltcontroller.moc"
