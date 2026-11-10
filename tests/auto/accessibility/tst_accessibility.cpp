// SPDX-License-Identifier: Apache-2.0
#include <QAccessible>
#include <QAccessibleActionInterface>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QtTest>

namespace {

// The installed module must not shadow the build output; keep the build QML
// directory first in the import path.
QStringList importPathsWithBuildFirst(QQmlEngine *engine)
{
    QStringList paths = engine->importPathList();
    paths.prepend(QStringLiteral(QCAT_QML_IMPORT_PATH));
    return paths;
}

QAccessibleInterface *findByName(QAccessibleInterface *root, const QString &name)
{
    if (!root)
        return nullptr;
    if (root->text(QAccessible::Name) == name)
        return root;
    for (int i = 0; i < root->childCount(); ++i) {
        if (QAccessibleInterface *match = findByName(root->child(i), name))
            return match;
    }
    return nullptr;
}

bool platformSupportsInterfaceTest()
{
    // Native platform allowlist, not a probe for an active OS bridge. The local
    // offscreen setup crashed during interface queries, so skip before querying.
    const QString platform = QGuiApplication::platformName();
    return platform == QLatin1String("windows") || platform == QLatin1String("cocoa")
        || platform == QLatin1String("xcb") || platform == QLatin1String("wayland");
}

} // namespace

class TestAccessibility : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void overflowButtonIsReachable();
    void overflowMenuOpensThroughAccessibleAction();

  private:
    QAccessibleInterface *overflowButton();

    QQuickView m_view;
    QQmlComponent *m_component = nullptr;
    QQuickItem *m_bar = nullptr;
};

void TestAccessibility::initTestCase()
{
    if (!platformSupportsInterfaceTest())
        QSKIP(qPrintable(QStringLiteral("QCAT_ACCESSIBILITY_UNSUPPORTED_PLATFORM: '%1'; "
                                        "run on a real desktop session")
                             .arg(QGuiApplication::platformName())));

    // Activate Qt's local interfaces; this does not establish an OS bridge.
    QAccessible::setActive(true);
    m_view.engine()->setImportPathList(importPathsWithBuildFirst(m_view.engine()));
    m_view.resize(480, 240);
    m_view.setResizeMode(QQuickView::SizeRootObjectToView);

    m_component = new QQmlComponent(m_view.engine(), this);
    m_component->setData(R"QML(
import QtQuick
import QtQuick.Controls.Basic as B
import QCatUiKit
Item {
    width: 480
    height: 240
    B.Action { id: openAction; text: "Open" }
    B.Action { id: exportAction; text: "Export" }
    CatCommandBar {
        objectName: "bar"
        width: 360
        primaryActions: [openAction]
        secondaryActions: [exportAction]
    }
}
)QML",
                          QUrl(QStringLiteral("qrc:/tst_accessibility/scene.qml")));
    if (m_component->isError()) {
        for (const QQmlError &error : m_component->errors())
            qWarning().noquote() << error.toString();
        QFAIL("accessibility scene failed to load");
    }

    QObject *item = m_component->create();
    QVERIFY(item);
    m_view.setContent(QUrl(QStringLiteral("qrc:/tst_accessibility/scene.qml")), m_component, item);
    m_view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&m_view));
    m_bar = m_view.rootObject();
    QVERIFY(m_bar);
}

QAccessibleInterface *TestAccessibility::overflowButton()
{
    // Start at the window: the plain Item scene root is not an accessible
    // control and need not expose the same children as the window interface.
    QAccessibleInterface *window = QAccessible::queryAccessibleInterface(&m_view);
    if (!window)
        return nullptr;
    return findByName(window, QStringLiteral("More commands"));
}

void TestAccessibility::overflowButtonIsReachable()
{
    QAccessibleInterface *button = overflowButton();
    QVERIFY2(button, "the overflow button is not exposed by its accessible name");
    QCOMPARE(button->role(), QAccessible::Button);

    QAccessibleActionInterface *actions = button->actionInterface();
    QVERIFY(actions);
    QVERIFY(actions->actionNames().contains(QAccessibleActionInterface::pressAction()));
}

void TestAccessibility::overflowMenuOpensThroughAccessibleAction()
{
    QAccessibleInterface *button = overflowButton();
    QVERIFY(button);
    QAccessibleActionInterface *actions = button->actionInterface();
    QVERIFY(actions);

    QObject *menu = m_bar->findChild<QObject *>(QStringLiteral("commandBarOverflowMenu"));
    QVERIFY(menu);
    QVERIFY(!menu->property("opened").toBool());

    actions->doAction(QAccessibleActionInterface::pressAction());
    QTRY_VERIFY_WITH_TIMEOUT(menu->property("opened").toBool(), 3000);
}

QTEST_MAIN(TestAccessibility)
#include "tst_accessibility.moc"
