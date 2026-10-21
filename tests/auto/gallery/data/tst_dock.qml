// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Window
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "DockGalleryViewport"
    when: windowShown
    visible: true
    width: 360
    height: 800
    Component {
        id: mainFactory
        Main { width: 1200; height: 900; initialPage: "dock-workspace" }
    }
    Component {
        id: factory
        GalleryDockPreview {
            width: 360
            height: 800
        }
    }
    function init() {
        // Offscreen cannot propagate native minimum-size hints after group changes.
        failOnWarning(/^(?!This plugin does not support propagateSizeHints\(\)$).*/);
    }
    function test_floatGroupAndReturn() {
        const page = createTemporaryObject(factory, test, {
            width: 800,
            height: 900
        });
        verify(waitForRendering(page));
        const workspace = findChild(page, "demoDockWorkspace");
        tryVerify(() => workspace.geometry.groups.length === 4);
        const original = workspace.registry.editor;
        mouseClick(findChild(page, "floatDockGroup"));
        tryVerify(() => original.Window.window !== workspace.Window.window);
        compare(workspace.controller.floatingWindows[0].tabs.length, 2);
        const win = original.Window.window;
        win.close();
        verify(win.visible); // Editor is explicitly non-closable.
        mouseClick(findChild(page, "returnDockWindows"));
        tryCompare(original.Window, "window", workspace.Window.window);
        compare(workspace.registry.editor, original);
        tryVerify(() => !Qt.isQtObject(win));
    }
    function test_titleBarFloatAction() {
        const page = createTemporaryObject(factory, test, { width: 800, height: 900 });
        verify(waitForRendering(page));
        const w = findChild(page, "demoDockWorkspace");
        tryVerify(() => w.geometry.groups.length === 4);
        const button = findChild(w, "dockFloat_editor");
        verify(button !== null);
        mouseClick(button);
        tryVerify(() => w.registry.editor.Window.window !== w.Window.window);
        mouseClick(findChild(page, "returnDockWindows"));
        tryCompare(w.registry.editor.Window, "window", w.Window.window);
    }
    function test_narrow() {
        const page = createTemporaryObject(factory, test);
        verify(waitForRendering(page));
        const workspace = findChild(page, "demoDockWorkspace");
        verify(workspace !== null);
        tryVerify(() => workspace.geometry.groups.length === 4);
        verify(workspace.width >= workspace.minimumDockWidth);
        verify(!workspace.sizeConstrained);
        workspace.registry.log.closeRequested();
        verify(!workspace.registry.log.visible);
        verify(workspace.controller.openPanel("log"));
        verify(workspace.registry.log.visible);
        verify(workspace.controller.restoreLayout(page.initialLayout));
        compare(workspace.geometry.groups.length, 4);
    }
    function test_dragTabOutToFloat_data() {
        return [{ tag: "tab", title: false }, { tag: "title-bar", title: true }];
    }
    function test_dragTabOutToFloat(data) {
        const page = createTemporaryObject(factory, test, { width: 800, height: 900 });
        verify(waitForRendering(page));
        const workspace = findChild(page, "demoDockWorkspace");
        tryVerify(() => workspace.geometry.groups.length === 4);
        const group = workspace.geometry.groups.find(g => g.tabs.indexOf("editor") >= 0);
        const startX = group.x + 20;
        const startY = group.y + (data.title ? 0 : workspace.tokens.controlHeight) + workspace.tokens.controlHeight / 2;
        mousePress(workspace, startX, startY);
        for (let y = startY - 15; y > -60; y -= 15)
            mouseMove(workspace, startX, y, 10);
        verify(workspace.dragging, "Gallery ScrollView must not steal the tab drag");
        mouseRelease(workspace, startX, -60);
        tryVerify(() => workspace.controller.floatingWindowForPanel("editor") !== "");
        const panel = workspace.registry.editor;
        tryVerify(() => panel.Window.window !== workspace.Window.window);
        verify(workspace.controller.dockBack(workspace.controller.floatingWindowForPanel("editor")));
        tryCompare(panel.Window, "window", workspace.Window.window);
    }
    function test_mainPageDragOut() {
        const gallery = createTemporaryObject(mainFactory, test);
        verify(waitForRendering(gallery.contentItem));
        const workspace = findChild(gallery.contentItem, "demoDockWorkspace");
        verify(workspace !== null);
        tryVerify(() => workspace.geometry.groups.length === 4);
        const scroll = findChild(gallery.contentItem, "galleryContentScroll").contentItem;
        const group = workspace.geometry.groups.find(g => g.tabs.indexOf("editor") >= 0);
        const x = group.x + 20, y = group.y + workspace.headerHeight / 2;
        const before = scroll.contentY;
        mousePress(workspace, x, y);
        for (let targetY = y - 15; targetY > -60; targetY -= 15)
            mouseMove(workspace, x, targetY, 10);
        verify(workspace.dragging, "Outer Gallery scrolling must not steal the drag");
        compare(scroll.contentY, before);
        mouseRelease(workspace, x, -60);
        tryVerify(() => workspace.controller.floatingWindowForPanel("editor") !== "");
        gallery.close();
    }
    function test_mainPageMergeIntoFloating_data() {
        return [{ tag: "main-source", floatingSource: false },
                { tag: "floating-source", floatingSource: true }];
    }
    function test_floatPropertiesBackToWorkspaceEdge_data() {
        return [{ tag: "right", position: DockController.Right, nx: 1, ny: 0.5 },
                { tag: "left", position: DockController.Left, nx: 0, ny: 0.5 },
                { tag: "top", position: DockController.Top, nx: 0.5, ny: 0 },
                { tag: "bottom", position: DockController.Bottom, nx: 0.5, ny: 1 }];
    }
    function test_floatPropertiesBackToWorkspaceEdge(data) {
        const gallery = createTemporaryObject(mainFactory, test);
        verify(waitForRendering(gallery.contentItem));
        const w = findChild(gallery.contentItem, "demoDockWorkspace");
        tryVerify(() => w.geometry.groups.length === 4);
        const panel = w.registry.properties;
        verify(w.floatPanel("properties"));
        tryVerify(() => panel.Window.window !== w.Window.window);
        const host = panel.Window.window;
        host.x = w.mapToGlobal(w.width + 100, 0).x;
        const source = findChild(host.contentItem, "dockFloatingTabDrag_properties");
        verify(source !== null);
        const global = w.mapToGlobal(Math.max(5, Math.min(w.width - 5, w.width * data.nx)),
                                     Math.max(5, Math.min(w.height - 5, w.height * data.ny)));
        const point = source.mapFromGlobal(global.x, global.y);
        mousePress(source, 20, source.height / 2);
        mouseMove(source, point.x, point.y, 30);
        verify(w.dropTarget !== null);
        verify(w.dropTarget.workspaceEdge);
        compare(w.dropTarget.position, data.position);
        const overlay = findChild(w, "dockDropOverlay");
        verify(overlay.outer);
        compare(overlay.height, w.height);
        keyClick(Qt.Key_Escape);
        mouseRelease(source, point.x, point.y);
        verify(w.controller.floatingWindowForPanel("properties") !== "");
        mousePress(source, 20, source.height / 2);
        mouseMove(source, point.x, point.y, 30);
        mouseRelease(source, point.x, point.y);
        tryCompare(panel.Window, "window", w.Window.window);
        const group = w.geometry.groups.find(g => g.tabs.indexOf("properties") >= 0);
        if (data.position === DockController.Right || data.position === DockController.Left) {
            compare(group.y, 0);
            compare(group.height, w.height);
            if (data.position === DockController.Right) {
                compare(group.x + group.width, w.width);
                const log = w.geometry.groups.find(g => g.tabs.indexOf("log") >= 0);
                verify(log.x + log.width <= group.x);
            } else {
                compare(group.x, 0);
            }
        } else {
            compare(group.x, 0);
            compare(group.width, w.width);
            compare(data.position === DockController.Top ? group.y : group.y + group.height,
                    data.position === DockController.Top ? 0 : w.height);
        }
        const saved = w.controller.saveLayout();
        verify(w.controller.restoreLayout(saved));
        compare(w.registry.properties, panel);
        gallery.close();
    }
    function test_dockBackPreservesPropertiesFullHeight() {
        const page = createTemporaryObject(factory, test, { width: 800, height: 900 });
        verify(waitForRendering(page));
        const w = findChild(page, "demoDockWorkspace");
        tryVerify(() => w.geometry.groups.length === 4);
        const original = w.geometry.groups.find(g => g.tabs.indexOf("properties") >= 0);
        verify(w.floatPanel("properties"));
        verify(w.controller.dockBack(w.controller.floatingWindowForPanel("properties")));
        const restored = w.geometry.groups.find(g => g.tabs.indexOf("properties") >= 0);
        compare(restored.height, original.height);
        compare(restored.width, original.width);
        compare(restored.x, original.x);
        compare(restored.y, original.y);
    }
    function test_mainPageMergeIntoFloating(data) {
        const gallery = createTemporaryObject(mainFactory, test);
        verify(waitForRendering(gallery.contentItem));
        const w = findChild(gallery.contentItem, "demoDockWorkspace");
        tryVerify(() => w.geometry.groups.length === 4);
        verify(w.floatPanel("project"));
        tryVerify(() => w.registry.project.Window.window !== w.Window.window);
        const destination = w.registry.project.Window.window;
        destination.width = 320;
        destination.height = 240;
        destination.x = gallery.x + gallery.width + 30;
        destination.y = gallery.y + 150;
        let source = w;
        let group = w.geometry.groups.find(g => g.tabs.indexOf("editor") >= 0);
        let sx = group.x + 20, sy = group.y + w.headerHeight / 2;
        if (data.floatingSource) {
            verify(w.floatPanel("editor"));
            tryVerify(() => w.registry.editor.Window.window !== w.Window.window);
            const host = w.registry.editor.Window.window;
            host.width = 320;
            host.height = 240;
            host.x = gallery.x + 30;
            host.y = gallery.y + 150;
            source = findChild(host.contentItem, "dockFloatingTabDrag_editor");
            verify(source !== null);
            sx = 20;
            sy = source.height / 2;
        }
        const scroll = findChild(gallery.contentItem, "galleryContentScroll").contentItem;
        const before = scroll.contentY;
        const global = destination.contentItem.mapToGlobal(160, 120);
        const end = source.mapFromGlobal(global.x, global.y);
        mousePress(source, sx, sy);
        mouseMove(source, end.x, end.y, 30);
        verify(w.dropTarget !== null);
        mouseRelease(source, end.x, end.y);
        compare(w.controller.floatingWindowForPanel("editor"), w.controller.floatingWindowForPanel("project"));
        tryCompare(w.registry.editor.Window, "window", destination);
        compare(scroll.contentY, before);
        gallery.close();
    }
}
