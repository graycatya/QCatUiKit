// SPDX-License-Identifier: Apache-2.0
#include <QAbstractItemModelTester>
#include <QCatUiKit/ChatModel>
#include <QCatUiKit/TableModel>
#include <QCatUiKit/ThemePalette>
#include <QtTest>
using namespace QCatUiKit;
class ModelTests : public QObject
{
    Q_OBJECT
  private slots:
    void chatContract()
    {
        ChatModel model;
        QAbstractItemModelTester checker(&model,
                                         QAbstractItemModelTester::FailureReportingMode::QtTest);
        QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
        QSignalSpy removed(&model, &QAbstractItemModel::rowsRemoved);
        QSignalSpy count(&model, &ChatModel::countChanged);
        QVERIFY(!model.remove(-1));
        QVERIFY(!model.remove(0));
        QVERIFY(model.get(100).isEmpty());
        QVERIFY(!model.append({{"type", 99}}));
        QVERIFY(model.append({{"messageId", "one"}, {"content", "Hello"}}));
        QVERIFY(model.append({{"messageId", "two"}, {"content", "World"}}));
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.rowCount(model.index(0)), 0);
        QVERIFY(model.update(1, {{"content", "Updated"}, {"status", ChatModel::Sent}}));
        QCOMPARE(changed.size(), 1);
        QCOMPARE(model.data(model.index(1), ChatModel::ContentRole).toString(), "Updated");
        QVERIFY(!model.update(1, {{"status", 9}}));
        QCOMPARE(model.get(1).value("status").toInt(), int(ChatModel::Sent));
        QVERIFY(model.remove(0));
        QCOMPARE(removed.at(0).at(1).toInt(), 0);
        QCOMPARE(removed.at(0).at(2).toInt(), 0);
        QCOMPARE(model.get(0).value("messageId").toString(), "two");
        model.clear();
        QCOMPARE(model.count(), 0);
        QCOMPARE(count.size(), 4);
        QVERIFY(!model.data(model.index(0)).isValid());
    }
    void tableContract()
    {
        TableModel model;
        QAbstractItemModelTester checker(&model,
                                         QAbstractItemModelTester::FailureReportingMode::QtTest);
        QVERIFY(!model.append({{"a", 1}}));
        model.setColumns({"z", "a"});
        model.setHeaders({"Last", "First"});
        model.append({{"a", 1}, {"z", 2}});
        QCOMPARE(model.data(model.index(0, 0)).toInt(), 2);
        QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), "Last");
        QVERIFY(!model.setCell(-1, 0, 4));
        QVERIFY(!model.setCell(0, 2, 4));
        QVERIFY(model.setCell(0, 1, 4));
        QCOMPARE(model.get(0).value("a").toInt(), 4);
        QSignalSpy changes(&model, &QAbstractItemModel::dataChanged);
        QVERIFY(model.update(0, {{"z", 5}}));
        QCOMPARE(changes.size(), 1);
        model.setColumns({"z", "z"});
        QCOMPARE(model.columns(), QStringList({"z", "a"}));
        model.setColumns({"a", "z"});
        QCOMPARE(model.data(model.index(0, 0)).toInt(), 4);
        QCOMPARE(model.columnCount(model.index(0, 0)), 0);
        QVERIFY(!model.remove(1));
        QVERIFY(model.remove(0));
        QCOMPARE(model.count(), 0);
        model.append({{"a", 3}});
        model.clear();
        QCOMPARE(model.rowCount(), 0);
    }
    void themeContract()
    {
        ThemePalette theme;
        QSignalSpy changed(&theme, &ThemePalette::tokensChanged);
        QCOMPARE(theme.tokens().value("controlHeight").toInt(), 32);
        theme.setDensity(ThemePalette::Touch);
        QCOMPARE(theme.tokens().value("controlHeight").toInt(), 48);
        theme.setDensity(ThemePalette::Mobile);
        QCOMPARE(theme.tokens().value("controlHeight").toInt(), 44);
        theme.setColorScheme(ThemePalette::Dark);
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1f1f1f"));
        theme.setPrimaryColor(QColor("#ffffff"));
        QCOMPARE(theme.tokens().value("onPrimary").value<QColor>(), QColor("#000000"));
        theme.setReducedMotion(true);
        QCOMPARE(theme.tokens().value("motionDuration").toInt(), 0);
        theme.setOverrides({{"surface", QString("#123456")}, {"unknown", 5}, {"radius", -1}});
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#123456"));
        QVERIFY(!theme.tokens().contains("unknown"));
        QCOMPARE(theme.tokens().value("radius").toInt(), 6);
        const int before = changed.size();
        theme.setDensity(ThemePalette::Mobile);
        QCOMPARE(changed.size(), before);
        theme.setRadius(-1);
        QCOMPARE(theme.radius(), 0);
        theme.setOverrides({});
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1f1f1f"));
    }
    void reducedMotionOverridesAndSignals()
    {
        ThemePalette theme;
        theme.tokens(); // Prime a cached snapshot before changing a seed.
        bool observedFreshTokens = false;
        connect(&theme, &ThemePalette::densityChanged, &theme,
                [&] { observedFreshTokens = theme.tokens().value("controlHeight").toInt() == 48; });
        theme.setDensity(ThemePalette::Touch);
        QVERIFY(observedFreshTokens);
        theme.setOverrides({{"motionDuration", 500}});
        theme.setReducedMotion(true);
        QCOMPARE(theme.tokens().value("motionDuration").toInt(), 0);
        theme.setReducedMotion(false);
        QCOMPARE(theme.tokens().value("motionDuration").toInt(), 500);
    }
    void foreignIndexes()
    {
        ChatModel first, second;
        first.append({});
        second.append({});
        QVERIFY(!first.data(second.index(0), ChatModel::ContentRole).isValid());
        TableModel a, b;
        a.setColumns({"x"});
        b.setColumns({"x"});
        a.append({});
        b.append({});
        QVERIFY(!a.setData(b.index(0, 0), 1));
        QVERIFY(!a.data(b.index(0, 0)).isValid());
    }
};
QTEST_GUILESS_MAIN(ModelTests)
#include "tst_models.moc"
