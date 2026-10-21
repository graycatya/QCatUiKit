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
    void workspaceEdgeDock_data()
    {
        QTest::addColumn<int>("position");
        QTest::newRow("left") << int(DockController::Left);
        QTest::newRow("right") << int(DockController::Right);
        QTest::newRow("top") << int(DockController::Top);
        QTest::newRow("bottom") << int(DockController::Bottom);
    }
    void workspaceEdgeDock()
    {
        QFETCH(int, position);
        DockController c;
        for (const auto &id : {"project", "editor", "properties", "log"})
            QVERIFY(c.registerPanel(id));
        QVERIFY(c.dock("project", "editor", DockController::Left));
        QVERIFY(c.dock("properties", "editor", DockController::Right));
        QVERIFY(c.dock("log", "editor", DockController::Bottom));
        QVERIFY(c.floatPanel("properties"));
        const auto remaining = c.layout();
        QSignalSpy changed(&c, &DockController::layoutChanged);
        QVERIFY(c.dockToWorkspace("properties", DockController::Position(position)));
        QCOMPARE(changed.size(), 1);
        QVERIFY(c.floatingWindows().isEmpty());
        const auto root = c.layout();
        const bool before = position == DockController::Left || position == DockController::Top;
        QCOMPARE(root.value("orientation").toString(),
                 position == DockController::Left || position == DockController::Right
                     ? "horizontal"
                     : "vertical");
        QCOMPARE(root.value(before ? "first" : "second").toMap().value("tabs").toStringList(),
                 QStringList{"properties"});
        QCOMPARE(root.value(before ? "second" : "first").toMap(), remaining);
        const auto saved = c.saveLayout();
        QVERIFY(c.restoreLayout(saved));
        QCOMPARE(c.saveLayout(), saved);
        QVERIFY(!c.dockToWorkspace("unknown", DockController::Right));
        QVERIFY(!c.dockToWorkspace("properties", DockController::Center));
        QCOMPARE(c.saveLayout(), saved);
        // Moving an existing main-workspace panel must collapse its old branch.
        QVERIFY(c.dockToWorkspace("editor", DockController::Top));
        QVERIFY(c.restoreLayout(c.saveLayout()));
        QCOMPARE(c.panelIds().size(), 4);
    }
    void returnBesideSplitSubtree()
    {
        DockController c;
        for (const auto &id : {"editor", "properties", "log"})
            QVERIFY(c.registerPanel(id));
        QVERIFY(c.dock("properties", "editor", DockController::Right));
        QVERIFY(c.dock("log", "editor", DockController::Bottom));
        QVERIFY(c.setSplitRatio(c.layout().value("id").toString(), 0.7));
        const auto neighbor = c.layout().value("first").toMap();
        QVERIFY(c.floatPanel("properties"));
        QCOMPARE(c.floatingWindows().first().toMap().value("returnNode").toString(),
                 neighbor.value("id").toString());
        const auto floating = c.saveLayout();
        QVERIFY(c.restoreLayout(floating));
        QVERIFY(c.dockBack(c.floatingWindowForPanel("properties")));
        QCOMPARE(c.layout().value("first").toMap(), neighbor);
        QCOMPARE(c.layout().value("ratio").toDouble(), 0.7);
        QVERIFY(c.floatPanel("properties"));
        // If the anchored subtree disappears, retain the surviving panel fallback.
        QVERIFY(c.closePanel("log"));
        QVERIFY(c.dockBack(c.floatingWindowForPanel("properties")));
        QCOMPARE(c.layout().value("first").toMap().value("tabs").toStringList(),
                 QStringList{"editor"});
        QVERIFY(c.floatPanel("properties"));
        auto doc = QJsonDocument::fromJson(c.saveLayout().toUtf8()).object();
        auto windows = doc.value("floating").toArray();
        auto entry = windows.first().toObject();
        entry.insert("returnNode", 123);
        windows[0] = entry;
        doc.insert("floating", windows);
        const auto before = c.saveLayout();
        QVERIFY(!c.restoreLayout(QString::fromUtf8(QJsonDocument(doc).toJson())));
        QCOMPARE(c.saveLayout(), before);
        entry.remove("returnNode");
        windows[0] = entry;
        doc.insert("floating", windows);
        QVERIFY(c.restoreLayout(QString::fromUtf8(QJsonDocument(doc).toJson())));
        QVERIFY(c.dockBack(c.floatingWindowForPanel("properties")));
    }
    void workspaceEdgeEmptyAndInvalidRatio()
    {
        DockController c;
        c.registerPanel("a");
        c.registerPanel("b");
        QVERIFY(c.floatPanel("a", true));
        QVERIFY(c.dockToWorkspace("a", DockController::Right));
        const auto solo = c.saveLayout();
        QVERIFY(c.dockToWorkspace("a", DockController::Top));
        QCOMPARE(c.saveLayout(), solo);
        QVERIFY(!c.dockToWorkspace("b", DockController::Right, 0));
        QVERIFY(!c.dockToWorkspace("b", DockController::Right, 1));
        QCOMPARE(c.saveLayout(), solo);
        QVERIFY(c.dockToWorkspace("b", DockController::Right, 0.7));
        QCOMPARE(c.layout().value("ratio").toDouble(), 0.7);
        QVERIFY(c.floatingWindows().isEmpty());
        QVERIFY(c.closePanel("a"));
        QVERIFY(c.closePanel("b"));
        QVERIFY(c.dockToWorkspace("b", DockController::Left));
        QCOMPARE(c.layout().value("tabs").toStringList(), QStringList{"b"});
    }
    void mergeAcrossFloatingWindows()
    {
        DockController c;
        for (const auto &id : {"a", "b", "c", "d"})
            QVERIFY(c.registerPanel(id));
        QVERIFY(c.floatPanel("b"));
        const auto destination = c.floatingWindowForPanel("b");
        QVERIFY(c.setFloatingGeometry(destination, 100, 100, 600, 400));
        const auto before = c.saveLayout();
        QVERIFY(!c.dock("a", "b", DockController::Left));
        QCOMPARE(c.saveLayout(), before);
        QVERIFY(c.dock("a", "b"));
        QCOMPARE(c.floatingWindowForPanel("a"), destination);
        QVERIFY(c.floatPanel("c"));
        const auto source = c.floatingWindowForPanel("c");
        QVERIFY(c.dock("c", "b"));
        QCOMPARE(c.floatingWindows().size(), 1);
        QVERIFY(source != destination);
        const auto record = c.floatingWindows().first().toMap();
        QCOMPARE(record.value("tabs").toStringList(), (QStringList{"b", "a", "c"}));
        QCOMPARE(record.value("active").toString(), "c");
        QCOMPARE(record.value("geometry").toMap().value("width").toDouble(), 600.0);
        QVERIFY(c.dock("a", "b")); // Same-host application operation preserves uniqueness.
        QCOMPARE(c.floatingWindows().first().toMap().value("tabs").toStringList(),
                 (QStringList{"b", "c", "a"}));
        const auto saved = c.saveLayout();
        QVERIFY(c.restoreLayout(saved));
        QCOMPARE(c.saveLayout(), saved);
        QVERIFY(c.dockBack(destination));
        QVERIFY(c.floatingWindows().isEmpty());
        QCOMPARE(c.layout().value("tabs").toStringList().size(), 4);
    }
    void returnSinglePanelToEmptyWorkspace()
    {
        DockController c;
        c.registerPanel("a");
        c.registerPanel("b");
        QVERIFY(c.floatPanel("a", true));
        QVERIFY(c.layout().isEmpty());
        const auto saved = c.saveLayout();
        QVERIFY(!c.dock("unknown", ""));
        QVERIFY(!c.dock("a", "", DockController::Left));
        QCOMPARE(c.saveLayout(), saved);
        QVERIFY(c.dock("a", ""));
        QCOMPARE(c.layout().value("tabs").toStringList(), QStringList{"a"});
        QVERIFY(!c.floatingWindowForPanel("b").isEmpty());
        QVERIFY(c.dock("b", "a"));
        QVERIFY(c.floatingWindows().isEmpty());
    }
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
