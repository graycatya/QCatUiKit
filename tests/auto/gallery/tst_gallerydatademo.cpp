// SPDX-License-Identifier: Apache-2.0
#include "gallerydatademo.h"
#include <QtTest>

using namespace QCatUiKit;
class GalleryDataDemoTests : public QObject
{
    Q_OBJECT
  private slots:
    void retainedCapacity_data()
    {
        QTest::addColumn<int>("mode");
        QTest::newRow("table-100000") << 0;
        QTest::newRow("conversation-10000") << 1;
        QTest::newRow("composer-history-10000") << 2;
    }
    void retainedCapacity()
    {
        QFETCH(int, mode);
        GalleryDataDemo demo;
        demo.setMode(mode);
        demo.configureLargeData();
        demo.clearData();
        const int limit = mode == 0 ? 100000 : 10000;
        QCOMPARE(demo.capacity(), limit);
        int iterations = 0;
        while (demo.count() < limit && ++iterations <= limit)
            demo.addBatch();
        QCOMPARE(demo.count(), limit);
        const auto first = mode == 0 ? demo.tableModel()->get(0) : demo.chatModel()->get(0);
        demo.addBatch();
        QCOMPARE(demo.count(), limit);
        if (mode == 0)
        {
            QCOMPARE(demo.tableModel()->get(0).value("id"), first.value("id"));
            QVERIFY(demo.changed() > 0);
        }
        else
        {
            QVERIFY(demo.chatModel()->get(0).value("messageId") != first.value("messageId"));
            QVERIFY(demo.removed() > 0);
        }
        demo.removeBatch();
        QVERIFY(demo.count() < limit);
        QCOMPARE(demo.count(), limit - demo.lastBatchCount());
        demo.setCapacity(1);
        QCOMPARE(demo.capacity(), demo.count());
        demo.clearData();
        demo.setCapacity(3);
        demo.addBatch();
        QCOMPARE(demo.count(), 3);
        demo.resetData();
        QCOMPARE(demo.count(), mode == 0 ? 3 : mode == 1 ? 1 : 0);
        QCOMPARE(demo.added(), 0);
    }
    void editingAndMessageTypes()
    {
        GalleryDataDemo demo;
        demo.select(1);
        QVERIFY(demo.editCell(1, "Edited name"));
        QCOMPARE(demo.tableModel()->get(1).value("name").toString(), "Edited name");
        QVERIFY(!demo.editCell(0, "New ID"));
        demo.removeSelected();
        QCOMPARE(demo.count(), 2);
        demo.setMode(1);
        for (int type = 0; type < 4; ++type)
        {
            demo.addMessage(true, type);
            QCOMPARE(demo.selectedRecord().value("type").toInt(), type);
            QCOMPARE(demo.selectedRecord().value("sender").toString(), "peer");
        }
        QVERIFY(demo.setMessageStatus(ChatModel::Failed));
        QCOMPARE(demo.selectedRecord().value("status").toInt(), int(ChatModel::Failed));
        QVERIFY(!demo.setMessageStatus(99));
        demo.recordSubmission("  My message  ");
        QCOMPARE(demo.selectedRecord().value("content").toString(), "My message");
        QCOMPARE(demo.selectedRecord().value("sender").toString(), "me");
        const int count = demo.count();
        demo.recordSubmission(" \t ");
        QCOMPARE(demo.count(), count);
    }
    void continuousModes_data()
    {
        QTest::addColumn<int>("mode");
        QTest::addColumn<int>("workload");
        for (int mode = 0; mode < 3; ++mode)
            for (int workload = 0; workload < (mode == 2 ? 1 : 3); ++workload)
                QTest::newRow(qPrintable(QString("%1-%2").arg(mode).arg(workload)))
                    << mode << workload;
    }
    void continuousModes()
    {
        QFETCH(int, mode);
        QFETCH(int, workload);
        GalleryDataDemo demo;
        demo.setMode(mode);
        demo.setCapacity(30);
        demo.setBatchSize(5);
        demo.setInterval(16);
        demo.setWorkload(workload);
        connect(&demo, &GalleryDataDemo::submitRequested, &demo,
                &GalleryDataDemo::recordSubmission);
        demo.startStress();
        demo.setCapacity(100);
        QCOMPARE(demo.capacity(), 30);
        QTRY_VERIFY_WITH_TIMEOUT(demo.batches() >= 8, 2000);
        demo.stopStress();
        QVERIFY(demo.count() <= 30);
        QVERIFY(demo.maximumBatchMs() > 0);
        QVERIFY(demo.elapsedSeconds() > 0);
        QVERIFY(demo.operationsPerSecond() > 0);
        if (mode == 2)
        {
            QVERIFY(demo.added() > 0);
            QVERIFY(demo.rejected() > 0);
        }
        else if (workload == 1)
        {
            QVERIFY(demo.changed() > 0);
            QVERIFY(demo.removed() > 0);
        }
        else if (workload == 2)
            QVERIFY(demo.changed() > 0);
        const auto batches = demo.batches();
        const int count = demo.count();
        QTest::qWait(50);
        QCOMPARE(demo.batches(), batches);
        QCOMPARE(demo.count(), count);
        demo.startStress();
        QCOMPARE(demo.batches(), 0);
        demo.resetData();
        QVERIFY(!demo.running());
    }
};
QTEST_GUILESS_MAIN(GalleryDataDemoTests)
#include "tst_gallerydatademo.moc"
