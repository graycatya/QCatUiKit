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
}
