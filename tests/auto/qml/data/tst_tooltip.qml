// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "ToolTipPosition"
    when: windowShown
    width: 640
    height: 480
    visible: true

    CatButton {
        id: button
        x: 100
        y: 100
        width: 240
        height: 60
        text: "Hover"
        CatToolTip {
            id: tip
            visible: button.hovered
            delay: 0
            timeout: -1
            text: "Pointer context"
        }
    }

    function init() { failOnWarning(/.*/); }
    function cleanup() {
        mouseMove(test, 0, 0);
        button.x = 100;
        button.y = 100;
        tip.delay = 0;
        tip.timeout = -1;
    }
    function test_fixedPositionAndClickDismissal() {
        mouseMove(button, 30, 20);
        tryCompare(tip, "opened", true);
        compare(tip.x, 42);
        compare(tip.y, 36);
        mouseMove(button, 90, 30);
        compare(tip.x, 42);
        compare(tip.y, 36);
        mousePress(button, 90, 30);
        verify(button.down);
        tryCompare(tip, "visible", false);
        mouseRelease(button, 90, 30);
        mouseMove(button, 100, 30);
        wait(50);
        compare(tip.visible, false);
        mouseMove(test, 0, 0);
        tryCompare(tip, "visible", false);
        mouseMove(button, 40, 20);
        tryCompare(tip, "opened", true);
    }
    function test_movementRestartsDelay() {
        tip.delay = 200;
        mouseMove(button, 30, 20);
        wait(130);
        compare(tip.visible, false);
        mouseMove(button, 90, 30);
        wait(130);
        compare(tip.visible, false);
        tryCompare(tip, "opened", true);
        compare(tip.x, 102);
    }
    function test_clickCancelsPendingShow() {
        tip.delay = 100;
        mouseMove(button, 30, 20);
        mouseClick(button, 30, 20);
        wait(150);
        compare(tip.visible, false);
    }
    function test_escapeAndLeave() {
        mouseMove(button, 30, 20);
        tryCompare(tip, "opened", true);
        keyClick(Qt.Key_Escape);
        tryCompare(tip, "visible", false);
        mouseMove(button, 60, 20);
        wait(50);
        compare(tip.visible, false);
        mouseMove(test, 0, 0);
        mouseMove(button, 60, 20);
        tryCompare(tip, "opened", true);
        mouseMove(test, 0, 0);
        tryCompare(tip, "visible", false);
    }
    function test_doesNotOpenWithoutRequest() {
        tip.visible = false;
        mouseMove(button, 30, 20);
        wait(50);
        compare(tip.visible, false);
        tip.visible = Qt.binding(function() { return button.hovered; });
    }
    function test_timeoutDoesNotReopenOnMovement() {
        tip.timeout = 100;
        mouseMove(button, 30, 20);
        tryCompare(tip, "opened", true);
        tryCompare(tip, "visible", false);
        mouseMove(button, 60, 20);
        wait(50);
        compare(tip.visible, false);
    }
    function test_bottomEdge() {
        button.y = 410;
        mouseMove(button, 50, 30);
        tryCompare(tip, "opened", true);
        verify(tip.y + tip.height < 30);
    }
    function test_rightEdge() {
        button.x = 390;
        mouseMove(button, 230, 30);
        tryCompare(tip, "opened", true);
        const bounds = tip.contentItem.mapToItem(test, 0, 0);
        verify(bounds.x >= 0);
        verify(bounds.x + tip.contentItem.width <= test.width);
    }
}
