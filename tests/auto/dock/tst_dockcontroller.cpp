// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/DockController>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>
using namespace QCatUiKit;
class TestDock : public QObject
{
    Q_OBJECT
  private slots:
    void floatingRoundTrip()
    {
        DockController c;
        for (const auto &id : {"a", "b", "c"})
            QVERIFY(c.registerPanel(id));
        QVERIFY(c.floatPanel("b"));
        QCOMPARE(c.layout().value("tabs").toStringList(), (QStringList{"a", "c"}));
        const auto id = c.floatingWindowForPanel("b");
        QVERIFY(!id.isEmpty());
        QVERIFY(c.setFloatingGeometry(id, -100, 200, 600, 400));
        const auto saved = c.saveLayout();
        QVERIFY(c.restoreLayout(saved));
        QCOMPARE(c.saveLayout(), saved);
        QVERIFY(c.dockBack(id));
        QCOMPARE(c.layout().value("tabs").toStringList(), (QStringList{"a", "b", "c"}));
        QVERIFY(c.floatingWindows().isEmpty());
        QVERIFY(c.dock("b", "a", DockController::Left));
        QVERIFY(c.setSplitRatio(c.layout().value("id").toString(), 0.3));
        QVERIFY(c.floatPanel("b"));
        QVERIFY(c.dockBack(c.floatingWindowForPanel("b")));
        QCOMPARE(c.layout().value("ratio").toDouble(), 0.3);
        QCOMPARE(c.layout().value("first").toMap().value("tabs").toStringList(), QStringList{"b"});
    }
    void groupFloatingAndMissingReturnTarget()
    {
        DockController c;
        c.registerPanel("a");
        c.registerPanel("b");
        c.registerPanel("c");
        c.dock("c", "a", DockController::Right);
        QVERIFY(c.floatPanel("a", true));
        const auto id = c.floatingWindowForPanel("a");
        QCOMPARE(c.floatingWindowForPanel("b"), id);
        QVERIFY(c.activatePanel("a"));
        QVERIFY(c.moveTab("b", 0));
        QVERIFY(c.closePanel("c"));
        QVERIFY(c.dockBack(id));
        QCOMPARE(c.layout().value("tabs").toStringList(), (QStringList{"b", "a"}));
        QVERIFY(c.floatPanel("b", true));
        QVERIFY(c.closePanel("a"));
        QVERIFY(c.unregisterPanel("b"));
        QVERIFY(c.floatingWindows().isEmpty());
        QVERIFY(c.layout().isEmpty());
    }
    void floatingRestoreRejectsDuplicatesAtomically()
    {
        DockController c;
        c.registerPanel("a");
        c.registerPanel("b");
        c.floatPanel("b");
        const auto saved = c.saveLayout();
        auto doc = QJsonDocument::fromJson(saved.toUtf8()).object();
        auto entries = doc.value("floating").toArray();
        auto entry = entries.first().toObject();
        entry.insert("tabs", QJsonArray{"a"});
        entry.insert("active", "a");
        entries[0] = entry;
        doc.insert("floating", entries);
        QVERIFY(!c.restoreLayout(QString::fromUtf8(QJsonDocument(doc).toJson())));
        QCOMPARE(c.saveLayout(), saved);
        QVERIFY(!c.setFloatingGeometry(c.floatingWindowForPanel("b"), 0, 0, -1, 200));
        QCOMPARE(c.saveLayout(), saved);
        doc = QJsonDocument::fromJson(saved.toUtf8()).object();
        doc.insert("version", 1);
        doc.remove("floating");
        QVERIFY(c.restoreLayout(QString::fromUtf8(QJsonDocument(doc).toJson())));
        QVERIFY(c.floatingWindows().isEmpty());
        QVERIFY(c.openPanel("b"));
    }
    void operations()
    {
        DockController c;
        QVERIFY(c.registerPanel("editor"));
        QVERIFY(c.registerPanel("log"));
        QVERIFY(c.registerPanel("tree"));
        QVERIFY(!c.registerPanel("log"));
        QVERIFY(c.dock("tree", "editor", DockController::Left));
        QVERIFY(c.dock("log", "editor", DockController::Bottom));
        QCOMPARE(c.layout().value("kind").toString(), "split");
        const auto snapshot = c.saveLayout();
        QVERIFY(!c.dock("log", "log", DockController::Left));
        QCOMPARE(c.saveLayout(), snapshot);
        QVERIFY(c.closePanel("log"));
        QVERIFY(c.closePanel("tree"));
        QCOMPARE(c.layout().value("kind").toString(), "tabs");
        QVERIFY(c.restoreLayout(snapshot));
        QCOMPARE(c.saveLayout(), snapshot);
        QVERIFY(c.dock("tree", "editor"));
        QVERIFY(c.dock("log", "editor"));
        QCOMPARE(c.layout().value("tabs").toStringList().size(), 3);
        QVERIFY(c.moveTab("log", 0));
        QCOMPARE(c.layout().value("tabs").toStringList().first(), "log");
        QVERIFY(c.unregisterPanel("log"));
        QVERIFY(!c.openPanel("log"));
        QVERIFY(c.closePanel("tree"));
        QVERIFY(c.closePanel("editor"));
        QVERIFY(c.layout().isEmpty());
        QVERIFY(c.openPanel("editor"));
    }
    void malformedRestoreIsAtomic()
    {
        DockController c;
        c.registerPanel("a");
        c.registerPanel("b");
        const auto saved = c.saveLayout();
        QSignalSpy changed(&c, &DockController::layoutChanged);
        for (const auto &text :
             {QString("bad"), QString("{}"), QString("{\"version\":2,\"root\":{}}"),
              QString("{\"version\":1,\"root\":{\"id\":\"x\",\"kind\":\"tabs\",\"tabs\":[\"a\","
                      "\"a\"],\"active\":\"a\"}}")})
        {
            QVERIFY(!c.restoreLayout(text));
            QCOMPARE(c.saveLayout(), saved);
        }
        QCOMPARE(changed.count(), 0);
    }
    void missingPanelsAndClosedPanels()
    {
        DockController c;
        c.registerPanel("a");
        c.registerPanel("b");
        c.dock("b", "a", DockController::Right);
        const auto saved = c.saveLayout();
        c.unregisterPanel("b");
        c.registerPanel("new");
        QVERIFY(c.restoreLayout(saved));
        QCOMPARE(c.layout().value("tabs").toStringList(), QStringList{"a"});
        QVERIFY(c.openPanel("new"));
        QVERIFY(c.restoreLayout("{\"version\":1,\"root\":{}}"));
        QVERIFY(c.layout().isEmpty());
    }
};
QTEST_GUILESS_MAIN(TestDock)
#include "tst_dockcontroller.moc"
