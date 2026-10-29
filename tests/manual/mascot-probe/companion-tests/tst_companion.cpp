// SPDX-License-Identifier: Apache-2.0
#include "mascotcompanion.h"
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusReply>
#include <QJsonArray>
#include <QProcess>
#include <QSignalSpy>
#include <QTest>
#include <signal.h>
namespace
{
const QString tokenA = "01234567-89ab-cdef-0123-456789abcdef";
const QString tokenB = "aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa";
} // namespace
class CompanionTest final : public QObject
{
    Q_OBJECT
    QProcess server;
    void launch(const QString &mode = {})
    {
        server.start(GJS_EXECUTABLE, {"-m", FIXTURE_PATH,
                                      QString::number(QCoreApplication::applicationPid()), mode});
        QVERIFY(server.waitForStarted());
    }
    void stop()
    {
        server.terminate();
        if (!server.waitForFinished(2000))
        {
            server.kill();
            server.waitForFinished(2000);
        }
    }
  private slots:
    void cleanup()
    {
        if (server.state() != QProcess::NotRunning)
            stop();
    }
    void missingAndRecovery()
    {
        MascotCompanion client(tokenA);
        QSignalSpy finished(&client, &MascotCompanion::completed);
        client.start();
        client.call("snapshot");
        QTRY_COMPARE(finished.size(), 1);
        QVERIFY(!finished[0][1].toBool());
        QVERIFY(!client.ready());
        launch();
        QTRY_VERIFY_WITH_TIMEOUT(client.ready(), 5000);
        client.call("snapshot");
        QTRY_COMPARE(finished.size(), 2);
        QVERIFY(finished[1][1].toBool());
    }
    void protocolMismatch()
    {
        launch("incompatible");
        MascotCompanion client(tokenA);
        client.start();
        QTRY_COMPARE(client.status().value("state").toString(), QString("Incompatible"));
        QVERIFY(!client.ready());
    }
    void rejectsServiceOutsideShell()
    {
        launch("untrusted");
        MascotCompanion client(tokenA);
        client.start();
        QTRY_COMPARE(client.status().value("state").toString(), QString("Untrusted"));
        QVERIFY(!client.ready());
    }
    void rejectsMalformedNativeEvidence()
    {
        launch("malformed");
        MascotCompanion client(tokenA);
        client.start();
        QTRY_VERIFY(client.ready());
        QSignalSpy done(&client, &MascotCompanion::completed);
        client.call("snapshot");
        QTRY_COMPARE(done.size(), 1);
        QVERIFY(!done[0][1].toBool());
        QCOMPARE(done[0][2].toString(), QString("InvalidNativeWindowReply"));
    }
    void instanceIsolationAndCallerIdentity()
    {
        launch();
        MascotCompanion first(tokenA), second(tokenB);
        first.start();
        second.start();
        QTRY_VERIFY(first.ready() && second.ready());
        QSignalSpy one(&first, &MascotCompanion::completed),
            two(&second, &MascotCompanion::completed);
        first.call("move", {{"x", 120}, {"y", 220}});
        QTRY_COMPARE(one.size(), 1);
        QVERIFY2(one[0][1].toBool(), qPrintable(one[0][2].toString()));
        second.call("snapshot");
        QTRY_COMPARE(two.size(), 1);
        const auto result = qvariant_cast<QJsonObject>(two[0][3]);
        QCOMPARE(result.value("rect").toArray(), (QJsonArray{10, 20, 420, 320}));
        first.call("move", {{"x", 1.5}, {"y", 2}});
        QTRY_COMPARE(one.size(), 2);
        QVERIFY(!one[1][1].toBool());
        QProcess foreign;
        foreign.start(QCoreApplication::applicationFilePath(), {"--foreign-call"});
        QVERIFY(foreign.waitForStarted());
        QVERIFY(foreign.waitForFinished(5000));
        QCOMPARE(foreign.exitCode(),
                 0); // The real service rejects the other PID despite its token.
    }
    void disconnectCancelsOnceAndReconnectDoesNotReplay()
    {
        launch("slow");
        MascotCompanion client(tokenA);
        client.start();
        QTRY_VERIFY(client.ready());
        QSignalSpy done(&client, &MascotCompanion::completed);
        client.call("move", {{"x", 500}, {"y", 600}});
        stop();
        QTRY_COMPARE(done.size(), 1);
        QVERIFY(!done[0][1].toBool());
        launch();
        QTRY_VERIFY(client.ready());
        QCOMPARE(done.size(), 1);
        client.call("snapshot");
        QTRY_COMPARE(done.size(), 2);
        const auto result = qvariant_cast<QJsonObject>(done[1][3]);
        QCOMPARE(result.value("rect").toArray(), (QJsonArray{10, 20, 420, 320}));
    }
    void disableRestoresTopmostAndEnableReconnects()
    {
        launch();
        MascotCompanion client(tokenA);
        client.start();
        QTRY_VERIFY(client.ready());
        QSignalSpy done(&client, &MascotCompanion::completed);
        client.call("above", {{"enabled", true}});
        QTRY_COMPARE(done.size(), 1);
        QVERIFY(done[0][1].toBool());
        QCOMPARE(::kill(pid_t(server.processId()), SIGUSR1), 0);
        QTRY_VERIFY(!client.ready());
        QCOMPARE(::kill(pid_t(server.processId()), SIGUSR2), 0);
        QTRY_VERIFY(client.ready());
        client.call("snapshot");
        QTRY_COMPARE(done.size(), 2);
        QVERIFY(!qvariant_cast<QJsonObject>(done[1][3]).value("above").toBool());
    }
};
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (app.arguments().contains("--foreign-call"))
    {
        auto message = QDBusMessage::createMethodCall(
            "org.qcat.MascotProbe1", "/org/qcat/MascotProbe1", "org.qcat.MascotProbe1", "Call");
        message << uint(1) << tokenA << QString("snapshot") << QString("{}");
        const auto reply = QDBusConnection::sessionBus().call(message, QDBus::Block, 2000);
        return reply.type() == QDBusMessage::ErrorMessage &&
                       reply.errorName().endsWith("WindowNotFound")
                   ? 0
                   : 1;
    }
    CompanionTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "tst_companion.moc"
