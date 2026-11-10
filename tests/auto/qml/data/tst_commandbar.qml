// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "CommandBar"
    when: windowShown
    width: 480
    height: 240
    visible: true
    B.Action {
        id: openAction
        text: "Open"
    }
    B.Action {
        id: exportAction
        text: "Export"
    }
    Component {
        id: barFactory
        CatCommandBar {
            width: 360
            primaryActions: [openAction]
            secondaryActions: [exportAction]
        }
    }
    Component {
        id: primaryOnlyFactory
        CatCommandBar {
            width: 360
            primaryActions: [openAction]
        }
    }
    function init() {
        failOnWarning(/.*/);
    }
    function test_overflowGlyphKeepsRoom() {
        const bar = createTemporaryObject(barFactory, test);
        verify(bar);
        const more = findChild(bar.contentItem, "commandBarOverflow");
        verify(more, "secondary actions must produce an overflow button");
        compare(more.text, "⋯");
        // 旧实现把右内边距和箭头区叠加，可用宽度为负，字形被裁成 0 宽。
        verify(more.availableWidth > 0, "overflow glyph must not be squeezed out");
        verify(more.width >= bar.tokens.controlHeight);
    }
    function test_overflowHiddenWithoutSecondaryActions() {
        const bar = createTemporaryObject(primaryOnlyFactory, test);
        verify(bar);
        const more = findChild(bar.contentItem, "commandBarOverflow");
        verify(more);
        verify(!more.visible);
    }
    function test_overflowOpensMenu() {
        const bar = createTemporaryObject(barFactory, test);
        verify(bar);
        const more = findChild(bar.contentItem, "commandBarOverflow");
        verify(more);
        const menu = findChild(more, "commandBarOverflowMenu");
        verify(menu);
        verify(!menu.opened);
        mouseClick(more);
        tryCompare(menu, "opened", true);
        compare(menu.count, 1);
    }
}
