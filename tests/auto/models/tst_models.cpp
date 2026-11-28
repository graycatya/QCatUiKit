// SPDX-License-Identifier: Apache-2.0
#include <QAbstractItemModelTester>
#include <QCatUiKit/ChatModel>
#include <QCatUiKit/ColumnReverseProxyModel>
#include <QCatUiKit/TableModel>
#include <QCatUiKit/ThemePalette>
#include <QtTest>
#include <cmath>
using namespace QCatUiKit;

class MutableTableModel : public QAbstractTableModel
{
  public:
    int rowCount(const QModelIndex &parent = {}) const override
    {
        return parent.isValid() ? 0 : 1;
    }
    int columnCount(const QModelIndex &parent = {}) const override
    {
        return parent.isValid() ? 0 : int(values.size());
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || role != Qt::DisplayRole || index.column() >= values.size())
            return {};
        return values.at(index.column());
    }
    void insertColumn(int at)
    {
        beginInsertColumns({}, at, at);
        values.insert(at, 100 + at);
        endInsertColumns();
    }
    void removeColumn(int at)
    {
        beginRemoveColumns({}, at, at);
        values.removeAt(at);
        endRemoveColumns();
    }

  private:
    QList<int> values{0, 10, 20};
};

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
    void reverseProxyContract()
    {
        TableModel source;
        source.setColumns({"a", "b", "c"});
        source.setHeaders({"A", "B", "C"});
        QVERIFY(source.append({{"a", 1}, {"b", 2}, {"c", 3}}));

        ColumnReverseProxyModel proxy;
        proxy.setSourceModel(&source);
        QAbstractItemModelTester checker(&proxy,
                                         QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.columnCount(), 3);
        QCOMPARE(proxy.data(proxy.index(0, 0)).toInt(), 3);
        QCOMPARE(proxy.data(proxy.index(0, 2)).toInt(), 1);
        QCOMPARE(proxy.headerData(0, Qt::Horizontal).toString(), "C");
        QCOMPARE(proxy.headerData(2, Qt::Horizontal).toString(), "A");
        QCOMPARE(proxy.mapToSource(proxy.index(0, 0)), source.index(0, 2));
        QCOMPARE(proxy.mapFromSource(source.index(0, 0)), proxy.index(0, 2));
        QCOMPARE(proxy.mapToSource(QModelIndex()), QModelIndex());
        QCOMPARE(proxy.mapFromSource(QModelIndex()), QModelIndex());

        QVERIFY(proxy.setData(proxy.index(0, 0), 30));
        QCOMPARE(source.data(source.index(0, 2)).toInt(), 30);
        QVERIFY(proxy.flags(proxy.index(0, 0)).testFlag(Qt::ItemIsEditable));
        QVERIFY(!proxy.setData(proxy.index(0, 0), 1, Qt::DisplayRole));

        source.setColumns({"a", "b", "c", "d"});
        QCOMPARE(proxy.columnCount(), 4);
        QCOMPARE(proxy.mapToSource(proxy.index(0, 0)), source.index(0, 3));

        proxy.setSourceModel(nullptr);
        QCOMPARE(proxy.rowCount(), 0);
        QCOMPARE(proxy.columnCount(), 0);
        QVERIFY(!proxy.mapToSource(proxy.index(0, 0)).isValid());

        MutableTableModel mutableSource;
        ColumnReverseProxyModel columnProxy;
        columnProxy.setSourceModel(&mutableSource);
        QAbstractItemModelTester columnChecker(
            &columnProxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(columnProxy.data(columnProxy.index(0, 2)).toInt(), 0);
        mutableSource.insertColumn(1);
        QCOMPARE(columnProxy.columnCount(), 4);
        QCOMPARE(columnProxy.data(columnProxy.index(0, 2)).toInt(), 101);
        mutableSource.removeColumn(2);
        QCOMPARE(columnProxy.columnCount(), 3);
        QCOMPARE(columnProxy.data(columnProxy.index(0, 1)).toInt(), 101);
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
        // Unknown tokens are kept for the application; known numeric tokens clamp to range.
        QCOMPARE(theme.tokens().value("unknown").toInt(), 5);
        QCOMPARE(theme.tokens().value("radius").toInt(), 0);
        const int before = changed.size();
        theme.setDensity(ThemePalette::Mobile);
        QCOMPARE(changed.size(), before);
        theme.setRadius(-1);
        QCOMPARE(theme.radius(), 0);
        theme.setOverrides({});
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1f1f1f"));
    }
    static qreal relativeLuminance(const QColor &color)
    {
        const auto channel = [](qreal value) {
            return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * channel(color.redF()) + 0.7152 * channel(color.greenF()) +
               0.0722 * channel(color.blueF());
    }
    void stateTokens()
    {
        ThemePalette theme;
        const QVariantMap light = theme.tokens();
        QCOMPARE(light.value("surfaceHover").value<QColor>(), QColor("#e9f2ff"));
        QCOMPARE(light.value("surfaceSelected").value<QColor>(), QColor("#dbe8ff"));
        QCOMPARE(light.value("surfacePressed").value<QColor>(), QColor("#c6dcff"));
        QCOMPARE(light.value("focusRing").value<QColor>(), QColor("#1677ff"));
        QCOMPARE(light.value("disabledOpacity").toInt(), 60);

        // Each state stays distinguishable from the surface; Light darkens in order.
        const qreal lightBase = relativeLuminance(light.value("surface").value<QColor>());
        const qreal lightHover = relativeLuminance(light.value("surfaceHover").value<QColor>());
        const qreal lightSelected = relativeLuminance(light.value("surfaceSelected").value<QColor>());
        const qreal lightPressed = relativeLuminance(light.value("surfacePressed").value<QColor>());
        QVERIFY(lightBase - lightHover >= 0.015);
        QVERIFY(lightBase - lightSelected >= 0.015);
        QVERIFY(lightBase - lightPressed >= 0.015);
        QVERIFY(lightHover > lightSelected);
        QVERIFY(lightSelected > lightPressed);

        theme.setColorScheme(ThemePalette::Dark);
        const QVariantMap dark = theme.tokens();
        QCOMPARE(dark.value("surfaceHover").value<QColor>(), QColor("#333333"));
        QCOMPARE(dark.value("surfaceSelected").value<QColor>(), QColor("#2b3a52"));
        QCOMPARE(dark.value("surfacePressed").value<QColor>(), QColor("#3f5470"));

        // Dark lifts each state above the surface and brightens in order.
        const qreal darkBase = relativeLuminance(dark.value("surface").value<QColor>());
        const qreal darkHover = relativeLuminance(dark.value("surfaceHover").value<QColor>());
        const qreal darkSelected = relativeLuminance(dark.value("surfaceSelected").value<QColor>());
        const qreal darkPressed = relativeLuminance(dark.value("surfacePressed").value<QColor>());
        QVERIFY(darkHover - darkBase >= 0.015);
        QVERIFY(darkSelected - darkBase >= 0.015);
        QVERIFY(darkPressed - darkBase >= 0.015);
        QVERIFY(darkHover < darkSelected);
        QVERIFY(darkSelected < darkPressed);

        // focusRing follows the effective primary unless explicitly supplied.
        theme.setPrimaryColor(QColor("#13c2c2"));
        QCOMPARE(theme.tokens().value("focusRing").value<QColor>(), QColor("#13c2c2"));
        theme.setColorPalette({{"primary", QString("#fa8c16")}});
        QCOMPARE(theme.tokens().value("focusRing").value<QColor>(), QColor("#fa8c16"));
        theme.setColorPalette({{"primary", QString("#fa8c16")}, {"focusRing", QString("#722ed1")}});
        QCOMPARE(theme.tokens().value("focusRing").value<QColor>(), QColor("#722ed1"));

        // Palette supplies state colors; missing state tokens keep built-in values.
        theme.setColorPalette({{"surfaceSelected", QString("#101418")},
                               {"surfacePressed", QString("#1b2126")}});
        QCOMPARE(theme.tokens().value("surfaceSelected").value<QColor>(), QColor("#101418"));
        QCOMPARE(theme.tokens().value("surfacePressed").value<QColor>(), QColor("#1b2126"));
        QCOMPARE(theme.tokens().value("surfaceHover").value<QColor>(), QColor("#333333"));

        // disabledOpacity clamps to 0..100 and restores with an empty override map.
        theme.setOverrides({{"disabledOpacity", 150}});
        QCOMPARE(theme.tokens().value("disabledOpacity").toInt(), 100);
        theme.setOverrides({{"disabledOpacity", -5}});
        QCOMPARE(theme.tokens().value("disabledOpacity").toInt(), 0);
        theme.setOverrides({});
        QCOMPARE(theme.tokens().value("disabledOpacity").toInt(), 60);
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
    void customColorPalette()
    {
        ThemePalette theme;
        theme.setColorPalette({{"background", QString("#101418")},
                               {"surface", QString("#1b2126")},
                               {"text", QString("#e6edf3")},
                               {"border", QString("#30363d")}});
        QCOMPARE(theme.tokens().value("background").value<QColor>(), QColor("#101418"));
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1b2126"));
        QCOMPARE(theme.tokens().value("text").value<QColor>(), QColor("#e6edf3"));
        QCOMPARE(theme.tokens().value("border").value<QColor>(), QColor("#30363d"));
        // Colors not present in the palette keep the built-in value for the active scheme.
        QCOMPARE(theme.tokens().value("primary").value<QColor>(), QColor("#1677ff"));
        QCOMPARE(theme.tokens().value("success").value<QColor>(), QColor("#389e0d"));
        // A palette changes colors only, not dimensions or motion.
        QCOMPARE(theme.tokens().value("controlHeight").toInt(), 32);
        QCOMPARE(theme.tokens().value("radius").toInt(), 6);
        QCOMPARE(theme.tokens().value("motionDuration").toInt(), 160);

        theme.setColorScheme(ThemePalette::Dark);
        QCOMPARE(theme.tokens().value("background").value<QColor>(), QColor("#101418"));
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1b2126"));

        // Non-color keys and invalid colors fall back instead of hiding other tokens.
        theme.setColorPalette({{"radius", 20}, {"surface", QString("not-a-color")}});
        QCOMPARE(theme.tokens().value("radius").toInt(), 6);
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1f1f1f"));

        theme.setColorPalette({});
        QCOMPARE(theme.tokens().value("surface").value<QColor>(), QColor("#1f1f1f"));
        QCOMPARE(theme.tokens().value("background").value<QColor>(), QColor("#141414"));
    }
    void customPaletteIsolation()
    {
        ThemePalette first;
        ThemePalette second;
        first.setColorPalette({{"surface", QString("#111111")}});
        second.setColorPalette({{"surface", QString("#222222")}});
        QCOMPARE(first.tokens().value("surface").value<QColor>(), QColor("#111111"));
        QCOMPARE(second.tokens().value("surface").value<QColor>(), QColor("#222222"));
        first.setColorScheme(ThemePalette::Dark);
        QCOMPARE(first.tokens().value("surface").value<QColor>(), QColor("#111111"));
        QCOMPARE(second.tokens().value("surface").value<QColor>(), QColor("#222222"));
        second.setColorPalette({});
        QCOMPARE(second.tokens().value("surface").value<QColor>(), QColor("#ffffff"));
        QCOMPARE(first.tokens().value("surface").value<QColor>(), QColor("#111111"));
    }
    void derivedColorsFollowBrand()
    {
        ThemePalette theme;
        theme.setOverrides({{"primary", QString("#002b8c")}});
        const QVariantMap overridden = theme.tokens();
        QCOMPARE(overridden.value("primary").value<QColor>(), QColor("#002b8c"));
        QCOMPARE(overridden.value("onPrimary").value<QColor>(), QColor("#ffffff"));
        QVERIFY(overridden.value("primaryHover").value<QColor>() != QColor("#002b8c"));
        QVERIFY(overridden.value("primaryPressed").value<QColor>() != QColor("#002b8c"));

        theme.setOverrides({{"primary", QString("#002b8c")},
                            {"onPrimary", QString("#112233")},
                            {"primaryHover", QString("#445566")}});
        const QVariantMap explicitValues = theme.tokens();
        QCOMPARE(explicitValues.value("onPrimary").value<QColor>(), QColor("#112233"));
        QCOMPARE(explicitValues.value("primaryHover").value<QColor>(), QColor("#445566"));
        QVERIFY(explicitValues.value("primaryPressed").value<QColor>() != QColor("#002b8c"));

        theme.setOverrides({});
        theme.setColorPalette({{"primary", QString("#ffd400")}});
        QCOMPARE(theme.tokens().value("onPrimary").value<QColor>(), QColor("#000000"));
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
