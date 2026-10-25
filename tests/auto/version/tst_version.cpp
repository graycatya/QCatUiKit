// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/qcatuikitversion.h>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QtTest>
#include <memory>

class VersionTests : public QObject
{
    Q_OBJECT
  private slots:
    void runtimeIdentity()
    {
        QCOMPARE(QCatUiKit::version().toString(), QStringLiteral(EXPECTED_BASE_VERSION));
        QCOMPARE(QCatUiKit::versionString(), QStringLiteral(EXPECTED_VERSION_STRING));
        QCOMPARE(QCatUiKit::abiVersion(), EXPECTED_ABI_VERSION);
        QCOMPARE(QCatUiKit::isExperimentalRelease(), bool(EXPECTED_EXPERIMENTAL));
        QCOMPARE(QCatUiKit::isStableRelease(), !bool(EXPECTED_EXPERIMENTAL));
        QCOMPARE(QCatUiKit::releaseChannel(), EXPECTED_EXPERIMENTAL
                                                  ? QCatUiKit::ReleaseChannel::Experimental
                                                  : QCatUiKit::ReleaseChannel::Stable);
    }

    void qmlIdentity_data()
    {
        QTest::addColumn<QString>("infoType");
        QTest::newRow("module-info") << QStringLiteral("QCatUiKit");
        QTest::newRow("qualified-module-info") << QStringLiteral("UiKit.QCatUiKit");
    }
    void qmlIdentity()
    {
        QFETCH(QString, infoType);
        // Qualified/unqualified access and separate engines must expose the same identity.
        for (int iteration = 0; iteration < 2; ++iteration)
        {
            QQmlEngine engine;
            QQmlComponent component(&engine);
            component.setData(QStringLiteral(R"(
                import QtQml
                import QCatUiKit 1.0
                import QCatUiKit 1.0 as UiKit
                QtObject {
                    property QtObject info: %1
                    property string version: %1.version
                    property string channel: %1.releaseChannel
                    property bool stable: %1.isStable
                    property bool experimental: %1.isExperimental
                    property int major: %1.versionMajor
                    property int minor: %1.versionMinor
                    property int patch: %1.versionPatch
                    property int abi: %1.abiVersion
                }
            )").arg(infoType).toUtf8(),
                              QUrl(QStringLiteral("qrc:/version-test.qml")));
            std::unique_ptr<QObject> object(component.create());
            QVERIFY2(object, qPrintable(component.errorString()));
            QObject *info = object->property("info").value<QObject *>();
            QVERIFY(info);
            for (const char *property : {"version", "versionMajor", "versionMinor", "versionPatch",
                                         "abiVersion", "releaseChannel", "isStable", "isExperimental"})
            {
                const QQmlProperty infoProperty(info, QString::fromLatin1(property), &engine);
                QVERIFY2(infoProperty.isValid(), property);
                QVERIFY2(!infoProperty.isWritable(), property);
            }
            QCOMPARE(object->property("version").toString(),
                     QStringLiteral(EXPECTED_VERSION_STRING));
            QCOMPARE(object->property("channel").toString(),
                     QStringLiteral(EXPECTED_RELEASE_CHANNEL));
            QCOMPARE(object->property("stable").toBool(), !bool(EXPECTED_EXPERIMENTAL));
            QCOMPARE(object->property("experimental").toBool(), bool(EXPECTED_EXPERIMENTAL));
            QCOMPARE(object->property("major").toInt(), QCatUiKit::version().majorVersion());
            QCOMPARE(object->property("minor").toInt(), QCatUiKit::version().minorVersion());
            QCOMPARE(object->property("patch").toInt(), QCatUiKit::version().microVersion());
            QCOMPARE(object->property("abi").toInt(), EXPECTED_ABI_VERSION);
        }
    }
};
QTEST_MAIN(VersionTests)
#include "tst_version.moc"
