// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/Version.h>
#include <QQmlComponent>
#include <QQmlEngine>
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

    void qmlIdentity()
    {
        // Separate engines must each expose the same library identity.
        for (int iteration = 0; iteration < 2; ++iteration)
        {
            QQmlEngine engine;
            QQmlComponent component(&engine);
            component.setData(R"(
                import QtQml
                import QCatUiKit 1.0
                QtObject {
                    property string version: LibraryInfo.version
                    property string channel: LibraryInfo.releaseChannel
                    property bool stable: LibraryInfo.isStable
                    property bool experimental: LibraryInfo.isExperimental
                    property int major: LibraryInfo.versionMajor
                    property int minor: LibraryInfo.versionMinor
                    property int patch: LibraryInfo.versionPatch
                    property int abi: LibraryInfo.abiVersion
                }
            )",
                              QUrl(QStringLiteral("qrc:/version-test.qml")));
            std::unique_ptr<QObject> object(component.create());
            QVERIFY2(object, qPrintable(component.errorString()));
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
