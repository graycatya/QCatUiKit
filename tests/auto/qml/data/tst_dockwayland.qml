// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Window
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "DockWayland"
    when: windowShown
    width: 800
    height: 600
    visible: true
    Component {
        id: factory
        CatDockWorkspace {
            x: 40
            y: 30
            width: 600
            height: 420
            CatDockPanel {
                dockId: "a"
                title: "Editor"
                property int retained: 42
                Rectangle { anchors.fill: parent; color: "red" }
            }
            CatDockPanel {
                dockId: "b"
                title: "Plot"
                Rectangle { anchors.fill: parent; color: "blue" }
            }
        }
    }
    function requireWayland() {
        if (Qt.platform.pluginName.startsWith("wayland"))
            return true;
        skip("Wayland QPA integration only; other platforms retain their existing Dock tests");
        return false;
    }
    function init() {
        failOnWarning(/.*/);
    }
    function floatWindow(w, id) {
        verify(w.floatPanel(id));
        w.updateWindows();
        tryVerify(() => w.registry[id].Window.window !== w.Window.window);
        return w.registry[id].Window.window;
    }
    function test_mainWindowLocalCoordinates() {
        if (!requireWayland())
            return;
        const w = createTemporaryObject(factory, test);
        const win = floatWindow(w, "a");
        w.dragId = "a";
        // Input is in the receiving window, not the workspace or desktop.
        const point = w.mapToItem(w.Window.window.contentItem, 5, 200);
        w.updateWaylandDrag("", point);
        verify(w.dropTarget !== null);
        verify(w.dropTarget.workspaceEdge);
        compare(w.dropTarget.position, DockController.Left);
        compare(w.dragPoint, Qt.point(5, 200));
        w.nativeDragPanel = "a";
        w.nativeDropTarget = w.dropTarget;
        w.finishWaylandDrag(true);
        compare(w.controller.floatingWindows.length, 0);
        tryCompare(w.registry.a.Window, "window", w.Window.window);
        compare(w.registry.a.retained, 42);
        tryVerify(() => !Qt.isQtObject(win));
    }
    function test_cancelRestoresDetachedPanel() {
        if (!requireWayland())
            return;
        const w = createTemporaryObject(factory, test);
        const saved = w.controller.saveLayout();
        const win = floatWindow(w, "a");
        w.nativeDragLayout = saved;
        w.nativeDragPanel = "a";
        w.dragId = "a";
        w.finishWaylandDrag(false);
        compare(w.controller.saveLayout(), saved);
        tryCompare(w.registry.a.Window, "window", w.Window.window);
        compare(w.registry.a.retained, 42);
        tryVerify(() => !Qt.isQtObject(win));
    }
    function test_floatingWindowLocalCoordinatesAndPermissions() {
        if (!requireWayland())
            return;
        const w = createTemporaryObject(factory, test);
        const source = floatWindow(w, "a");
        const destination = floatWindow(w, "b");
        const sourceId = w.controller.floatingWindowForPanel("a");
        const destinationId = w.controller.floatingWindowForPanel("b");
        w.dragId = "a";
        w.updateWaylandDrag(sourceId, Qt.point(80, 60));
        compare(w.dropTarget, null);
        w.registry.a.floatable = false;
        w.updateWaylandDrag(destinationId, Qt.point(80, 60));
        compare(w.dropTarget, null);
        w.registry.a.floatable = true;
        w.updateWaylandDrag(destinationId, Qt.point(80, 60));
        verify(w.dropTarget !== null);
        compare(w.dropTarget.windowId, destinationId);
        w.nativeDragPanel = "a";
        w.nativeDropTarget = w.dropTarget;
        w.finishWaylandDrag(true);
        tryCompare(w.registry.a.Window, "window", destination);
        compare(w.controller.floatingWindows.length, 1);
        compare(w.registry.a.retained, 42);
        tryVerify(() => !Qt.isQtObject(source));
    }
    function test_systemMoveRespectsPermissions() {
        if (!requireWayland())
            return;
        const w = createTemporaryObject(factory, test);
        w.layoutLocked = true;
        verify(!w.moveWaylandWindow(w, "a"));
        w.layoutLocked = false;
        w.registry.a.movable = false;
        verify(!w.moveWaylandWindow(w, "a"));
        verify(!w.moveWaylandWindow(w, "missing"));
    }
}
