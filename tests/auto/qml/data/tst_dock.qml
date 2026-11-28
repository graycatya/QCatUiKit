// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "DockWorkspace"
    when: windowShown
    visible: true
    width: 800
    height: 600
    Component {
        id: sample
        CatDockWorkspace {
            width: 800
            height: 600
            CatDockPanel {
                dockId: "a"
                title: "Editor"
                property alias text: input.text
                CatTextField {
                    id: input
                    text: "retained"
                }
            }
            CatDockPanel {
                dockId: "b"
                title: "Log"
            }
        }
    }
    Component {
        id: rtlSample
        CatDockWorkspace {
            rtl: true
            width: 800
            height: 600
            CatDockPanel {
                dockId: "a"
                title: "Editor"
            }
            CatDockPanel {
                dockId: "b"
                title: "Log"
            }
        }
    }
    Component {
        id: customDockIcon
        Text {
            objectName: "customDockScrollIcon"
            text: ">"
        }
    }
    function test_contentAndRestore() {
        const w = createTemporaryObject(sample, test);
        verify(w !== null);
        tryCompare(w.controller, "panelIds", ["a", "b"]);
        const a = w.registry.a;
        a.text = "user edit";
        verify(w.controller.dock("a", "b", DockController.Left));
        compare(w.geometry.groups.length, 2);
        verify(a.visible);
        const saved = w.controller.saveLayout();
        verify(w.controller.closePanel("a"));
        verify(!a.visible);
        verify(w.controller.openPanel("a"));
        compare(a.text, "user edit");
        verify(w.controller.restoreLayout(saved));
        compare(w.registry.a, a);
        compare(a.text, "user edit");
        verify(w.controller.dock("a", "b", DockController.Center));
        compare(w.geometry.groups.length, 1);
        verify(a.visible);
    }
    function test_dragCancelAndCommit() {
        const w = createTemporaryObject(sample, test);
        const saved = w.controller.saveLayout();
        w.beginDrag("a");
        w.updateDrag(5, 300);
        verify(w.dropTarget !== null);
        w.cancelDrag();
        compare(w.controller.saveLayout(), saved);
        w.beginDrag("a");
        w.updateDrag(5, 300);
        w.finishDrag();
        compare(w.geometry.groups.length, 2);
        w.layoutLocked = true;
        w.beginDrag("a");
        verify(!w.dragging);
    }
    function test_directionGuides_data() {
        return [{ tag: "center", x: 400, y: 300, position: DockController.Center },
                { tag: "left", x: 364, y: 300, position: DockController.Left },
                { tag: "right", x: 436, y: 300, position: DockController.Right },
                { tag: "top", x: 400, y: 264, position: DockController.Top },
                { tag: "bottom", x: 400, y: 336, position: DockController.Bottom }];
    }
    function test_directionGuides(data) {
        const w = createTemporaryObject(sample, test);
        w.beginDrag("a");
        w.updateDrag(data.x, data.y);
        compare(w.dropTarget.position, data.position);
        const guide = findChild(w, "dockDropOverlay");
        verify(guide.visible);
        compare(guide.position, data.position);
        w.finishDrag();
        compare(w.geometry.groups.length, data.position === DockController.Center ? 1 : 2);
        if (data.position === DockController.Left)
            verify(w.registry.a.x < w.registry.b.x);
        if (data.position === DockController.Right)
            verify(w.registry.a.x > w.registry.b.x);
        if (data.position === DockController.Top)
            verify(w.registry.a.y < w.registry.b.y);
        if (data.position === DockController.Bottom)
            verify(w.registry.a.y > w.registry.b.y);
        verify(!guide.visible);
    }
    function test_pointerDragAndEscape() {
        const w = createTemporaryObject(sample, test);
        verify(waitForRendering(w));
        mousePress(w, 20, w.headerHeight / 2);
        mouseMove(w, 10, 300, 30);
        verify(w.dragging);
        keyClick(Qt.Key_Escape);
        verify(!w.dragging);
        mouseRelease(w, 10, 300);
        compare(w.geometry.groups.length, 1);
        mousePress(w, 20, w.headerHeight / 2);
        mouseMove(w, 10, 300, 30);
        mouseRelease(w, 10, 300);
        compare(w.geometry.groups.length, 2);
        const splitter = w.geometry.handles[0];
        const before = w.registry.a.width;
        mousePress(w, splitter.x + 4, 100);
        mouseMove(w, splitter.x + 65, 100, 30);
        mouseRelease(w, splitter.x + 65, 100);
        verify(w.registry.a.width > before);
    }
    function test_tabBottomHitTargets() {
        const w = createTemporaryObject(sample, test);
        verify(waitForRendering(w));
        const tab = findChild(w, "dockTab_a");
        const close = findChild(w, "dockTabClose_a");
        verify(tab !== null);
        verify(close !== null);
        w.controller.activatePanel("b");
        mouseMove(tab, 4, tab.height - 2);
        mouseClick(tab, 4, tab.height - 2);
        compare(w.geometry.groups[0].active, "a");

        let closeCount = 0;
        w.registry.a.closeRequested.connect(() => ++closeCount);
        mouseMove(close, close.width / 2, close.height - 2);
        verify(close.hovered);
        mouseClick(close, close.width / 2, close.height - 2);
        compare(closeCount, 1);
        compare(w.geometry.groups[0].active, "a");
    }
    function test_tabReorderReturnsFromDockPreview() {
        const w = createTemporaryObject(sample, test);
        verify(waitForRendering(w));
        const a = findChild(w, "dockTab_a");
        const b = findChild(w, "dockTab_b");
        verify(a !== null);
        verify(b !== null);
        tryVerify(() => b.mapToItem(w, 0, 0).x >= a.width);
        const start = a.mapToItem(w, 10, a.height / 2);
        const endX = b.mapToItem(w, b.width, 0).x + 80;
        const previousX = b.mapToItem(w, 0, 0).x;
        const original = w.controller.saveLayout();

        mousePress(w, start.x, start.y);
        mouseMove(w, endX, start.y);
        compare(w.dropTarget, null);
        compare(w.controller.saveLayout(), original);
        tryVerify(() => b.mapToItem(w, 0, 0).x < previousX);
        mouseMove(w, w.width / 2, w.height / 2);
        verify(w.dropTarget !== null);
        mouseMove(w, endX, start.y);
        compare(w.dropTarget, null);
        keyClick(Qt.Key_Escape);
        mouseRelease(w, endX, start.y);
        compare(w.controller.saveLayout(), original);
        tryVerify(() => b.mapToItem(w, 0, 0).x === previousX);

        mousePress(w, start.x, start.y);
        mouseMove(w, w.width / 2, w.height / 2);
        verify(w.dropTarget !== null);
        mouseMove(w, endX, start.y);
        compare(w.dropTarget, null);
        mouseRelease(w, endX, start.y);
        compare(Array.from(w.geometry.groups[0].tabs), ["b", "a"]);
        verify(!w.dragging);
    }
    function test_overflowDragScrollsWhilePointerIsStill() {
        const w = createTemporaryObject(sample, test, { width: 220 });
        w.registry.a.title = "Editor with a long title";
        w.registry.b.title = "Activity log with a long title";
        verify(waitForRendering(w));
        const a = findChild(w, "dockTab_a");
        const b = findChild(w, "dockTab_b");
        tryVerify(() => findChild(w, "dockTabsScrollRight") !== null);
        const left = findChild(w, "dockTabsScrollLeft");
        const right = findChild(w, "dockTabsScrollRight");
        verify(!left.enabled);
        verify(right.enabled);
        const original = w.controller.saveLayout();
        const originalX = a.mapToItem(w, 0, 0).x;
        compare(left.contentItem.color, w.tokens.textDisabled);
        mouseMove(right);
        tryCompare(right.background, "color", w.tokens.surfaceHover);
        mousePress(right);
        compare(right.background.color, w.tokens.primary);
        compare(right.contentItem.color, "#ffffff");
        mouseRelease(right);
        verify(a.mapToItem(w, 0, 0).x < originalX);
        verify(left.enabled);
        mouseMove(left);
        tryCompare(left.background, "color", w.tokens.surfaceHover);
        mouseClick(left);
        compare(a.mapToItem(w, 0, 0).x, originalX);
        compare(w.controller.saveLayout(), original);

        const start = a.mapToItem(w, 10, a.height / 2);
        const edge = right.mapToItem(w, right.width / 2, right.height / 2);
        const before = b.mapToItem(w, 0, 0).x;
        mousePress(w, start.x, start.y);
        mouseMove(w, edge.x, edge.y);
        compare(w.dropTarget, null);
        tryVerify(() => b.mapToItem(w, 0, 0).x < before);
        tryVerify(() => !w.reorderStrip.scrollDirection);
        compare(w.controller.saveLayout(), original);
        mouseMove(w, w.width / 2, w.height / 2);
        verify(w.dropTarget !== null);
        mouseMove(w, edge.x, edge.y);
        compare(w.dropTarget, null);
        mouseRelease(w, edge.x, edge.y);
        compare(Array.from(w.geometry.groups[0].tabs), ["b", "a"]);
    }
    function test_scrollIconOverride() {
        const w = createTemporaryObject(sample, test, { width: 220 });
        w.registry.a.title = "Editor with a long title";
        w.registry.b.title = "Activity log with a long title";
        verify(waitForRendering(w));
        tryVerify(() => findChild(w, "dockTabsScrollRight") !== null);
        const right = findChild(w, "dockTabsScrollRight");
        const glyph = findChild(right, "dockScrollRightGlyph");
        verify(glyph !== null);
        verify(glyph.visible);
        w.tabScrollLeftIcon = customDockIcon;
        w.tabScrollRightIcon = customDockIcon;
        wait(10);
        verify(!glyph.visible);
        verify(findChild(right, "customDockScrollIcon") !== null);
        verify(right.enabled);
    }
    function test_rtlTabStrip() {
        const w = createTemporaryObject(rtlSample, test, { width: 220, height: 600 });
        w.registry.a.title = "Editor with a long title";
        w.registry.b.title = "Activity log with a long title";
        verify(waitForRendering(w));
        tryVerify(() => findChild(w, "dockTabsScrollRight") !== null);
        const a = findChild(w, "dockTab_a");
        const b = findChild(w, "dockTab_b");
        verify(w.__rtl, "workspace must report RTL");
        verify(a.mapToItem(w, 0, 0).x > b.mapToItem(w, 0, 0).x,
               "RTL tabs must run from right to left");
        const closeA = findChild(w, "dockTabClose_a");
        verify(closeA.x < 28, "RTL close target must sit on the left edge");
        verify(a.leftPadding >= closeA.width + 4,
               "RTL tab text must start after the left close target");
    }
    function test_minimums() {
        const w = createTemporaryObject(sample, test);
        w.controller.dock("a", "b", DockController.Left);
        w.width = 100;
        verify(w.sizeConstrained);
        verify(w.registry.a.width >= w.registry.a.minimumDockWidth);
        verify(w.registry.b.width >= w.registry.b.minimumDockWidth);
        w.registry.a.minimumDockWidth = 240;
        verify(w.minimumDockWidth >= 368);
    }
    function test_splitterMouseExitAndKeyboardFocus() {
        const w = createTemporaryObject(sample, test);
        verify(w.controller.dock("a", "b", DockController.Left));
        verify(waitForRendering(w));
        const handle = findChild(w, "dockSplitter_0");
        verify(handle !== null);
        mouseMove(handle, handle.width / 2, 100);
        compare(handle.background.color, Theme.tokens.primary);
        mouseClick(handle, handle.width / 2, 100);
        verify(handle.activeFocus);
        mouseMove(w, 20, 100);
        verify(!handle.keyboardCue);
        tryCompare(handle.background, "color", Theme.tokens.background);
        w.forceActiveFocus();
        handle.forceActiveFocus(Qt.TabFocusReason);
        verify(handle.keyboardCue);
        compare(handle.background.color, Theme.tokens.primary);
        const before = w.registry.a.width;
        keyClick(Qt.Key_Right);
        verify(w.registry.a.width > before);
        mouseClick(handle, handle.width / 2, 100);
        mouseMove(w, 30, 140);
        verify(handle.pointerFocus);
        tryCompare(handle.background, "color", Theme.tokens.background);
    }
}
