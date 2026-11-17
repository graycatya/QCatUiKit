// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Window
import QtTest
import QCatUiKit
import "qrc:/qt/qml/QCatUiKit/qml" as DockInternal

TestCase {
    id: test
    name: "DockWayland"
    when: windowShown
    width: 800
    height: 600
    visible: true
    Component {
        id: dragAreaFactory
        Item {
            id: harness
            width: 400
            height: 300
            property bool nativeWaylandDrag: true
            property bool nativeToplevelDrag: true
            property int reorderIndex: 2
            property bool layoutLocked: false
            property bool dragging: false
            property var registry: ({})
            property int nativeStarts: 0
            property int localUpdates: 0
            property int cancellations: 0
            property alias area: dragArea
            property alias strip: strip
            function beginWaylandDrag(id, source) { nativeStarts++; }
            function updateWaylandLocalDrag(source, x, y) { localUpdates++; }
            function cancelDrag() { cancellations++; }
            Item {
                id: strip
                property string floatingWindowId: ""
                property bool inside: false
                function preview(point) { return inside; }
            }
            DockInternal.DockDragArea {
                id: dragArea
                workspace: harness
                panelId: "a"
                focusItem: harness
                tabStrip: strip
            }
        }
    }
    function test_leaveStripStartsNativeDragInsideWindow() {
        const h = createTemporaryObject(dragAreaFactory, test);
        h.area.updateDragPosition(80, 80, false);
        compare(h.nativeStarts, 1);
        compare(h.localUpdates, 0);
        verify(h.area.nativeHandled);
        compare(h.reorderIndex, -1);
    }
    function test_insideStripKeepsReordering() {
        const h = createTemporaryObject(dragAreaFactory, test);
        h.strip.inside = true;
        h.area.updateDragPosition(80, 80, false);
        compare(h.nativeStarts, 0);
        compare(h.reorderIndex, 2);
    }
    function test_noToplevelProtocolKeepsSystemMoveFallback() {
        const h = createTemporaryObject(dragAreaFactory, test);
        h.nativeToplevelDrag = false;
        h.area.updateDragPosition(80, 80, false);
        compare(h.nativeStarts, 0);
        compare(h.localUpdates, 1);
        h.strip.floatingWindowId = "floating";
        h.area.updateDragPosition(80, 80, false);
        compare(h.nativeStarts, 1);
    }
    function test_releaseDoesNotStartNativeDrag() {
        const h = createTemporaryObject(dragAreaFactory, test);
        h.area.updateDragPosition(80, 80, true);
        compare(h.nativeStarts, 0);
        compare(h.cancellations, 1);
    }
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
    function test_localPreviewEscapesWorkspaceClip() {
        const w = createTemporaryObject(factory, test);
        const preview = findChild(w.Window.window.contentItem, "dockLocalDragPreview");
        verify(preview !== null);
        verify(preview.parent !== w);
        w.dragPoint = Qt.point(w.width - 10, 100);
        const expected = w.mapToItem(preview.parent, w.dragPoint.x, w.dragPoint.y);
        compare(preview.x, expected.x + 20);
        compare(preview.y, expected.y + 24);
        w.destroy();
        wait(0);
        compare(findChild(test.Window.window.contentItem, "dockLocalDragPreview"), null);
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
