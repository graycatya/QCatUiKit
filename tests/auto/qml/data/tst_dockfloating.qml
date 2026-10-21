// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Window
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "DockFloating"
    when: windowShown
    visible: true
    width: 800
    height: 600
    property int originalDensity
    property int originalScheme
    Component {
        id: factory
        CatDockWorkspace {
            id: workspace
            width: 800
            height: 600
            CatDockPanel {
                dockId: "a"
                title: "Editor"
                property alias input: field
                property alias popup: popup
                property bool allowClose: false
                property int closeCount: 0
                onCloseRequested: {
                    closeCount++;
                    if (allowClose)
                        workspace.controller.closePanel(dockId);
                }
                CatTextField {
                    id: field
                    text: "retained"
                }
                CatPopup {
                    id: popup
                    width: 200
                    height: 100
                    CatLabel {
                        text: "Panel popup"
                    }
                }
            }
            CatDockPanel {
                dockId: "b"
                title: "Plot"
                property alias chart: chart
                onCloseRequested: workspace.controller.closePanel(dockId)
                CatChart {
                    id: chart
                    anchors.fill: parent
                    legendVisible: false
                    CatChartSeries {
                        color: "red"
                        Component.onCompleted: setData([
                            {
                                x: 0,
                                y: 0
                            },
                            {
                                x: 1,
                                y: 1
                            },
                            {
                                x: 2,
                                y: 0
                            }
                        ])
                    }
                }
            }
        }
    }
    function init() {
        // The offscreen platform cannot propagate native minimum-size hints.
        failOnWarning(/^(?!This plugin does not support propagateSizeHints\(\)$).*/);
        Theme.reducedMotion = true;
        originalDensity = Theme.density;
        originalScheme = Theme.colorScheme;
    }
    function cleanup() {
        Theme.reducedMotion = false;
        Theme.density = originalDensity;
        Theme.colorScheme = originalScheme;
    }
    function floatEditor(w, group) {
        verify(w.floatPanel("a", group));
        tryVerify(() => w.registry.a.Window.window !== w.Window.window);
        const win = w.registry.a.Window.window;
        tryCompare(win, "visible", true);
        return win;
    }
    function verifyChartPixels(chart) {
        chart.Window.window.update();
        verify(waitForRendering(chart));
        const image = grabImage(chart), plot = chart.plotRect();
        let colored = 0;
        for (let y = Math.ceil(plot.y); y < plot.y + plot.height; y += 3)
            for (let x = Math.ceil(plot.x); x < plot.x + plot.width; x += 3) {
                const c = image.pixel(x, y);
                if (c.r > c.g + 0.2 && c.r > c.b + 0.2)
                    colored++;
            }
        verify(colored > 0, "Chart must render its red series after a window move");
    }
    function test_contentFocusAndPopup() {
        const w = createTemporaryObject(factory, test);
        w.controller.activatePanel("a");
        const panel = w.registry.a, input = panel.input;
        verify(waitForRendering(panel));
        input.text = "user edit";
        input.forceActiveFocus();
        tryCompare(input, "activeFocus", true);
        const mainWindow = panel.Window.window;
        const win = floatEditor(w, false);
        tryCompare(input, "activeFocus", true);
        compare(input.text, "user edit");
        panel.popup.open();
        tryCompare(panel.popup, "opened", true);
        compare(panel.popup.contentItem.Window.window, win);
        panel.popup.close();
        const id = w.controller.floatingWindowForPanel("a");
        verify(w.controller.dockBack(id));
        tryCompare(panel.Window, "window", mainWindow);
        compare(panel.input, input);
        compare(input.text, "user edit");
        panel.popup.open();
        tryCompare(panel.popup, "opened", true);
        compare(panel.popup.contentItem.Window.window, mainWindow);
        panel.popup.close();
        tryVerify(() => !Qt.isQtObject(win));
    }
    function test_closeCanBeRejectedAndAccepted() {
        const w = createTemporaryObject(factory, test);
        const panel = w.registry.a;
        const win = floatEditor(w, false);
        win.close();
        compare(panel.closeCount, 1);
        verify(win.visible);
        compare(w.controller.floatingWindows.length, 1);
        panel.allowClose = true;
        win.close();
        tryCompare(w.controller.floatingWindows, "length", 0);
        tryVerify(() => !Qt.isQtObject(win));
        verify(!panel.visible);
        verify(w.controller.openPanel("a"));
        verify(panel.visible);
    }
    function test_groupPersistenceAndChart() {
        const w = createTemporaryObject(factory, test);
        const chart = w.registry.b.chart;
        const win = floatEditor(w, true);
        compare(w.controller.floatingWindows[0].tabs.length, 2);
        w.controller.activatePanel("b");
        tryCompare(w.registry.b, "visible", true);
        verifyChartPixels(chart);
        compare(chart.Window.window, win);
        const saved = w.controller.saveLayout();
        verify(w.controller.dockBack(w.controller.floatingWindowForPanel("b")));
        tryCompare(chart.Window, "window", w.Window.window);
        verifyChartPixels(chart);
        verify(w.controller.restoreLayout(saved));
        tryVerify(() => chart.Window.window !== w.Window.window);
        compare(w.registry.b.chart, chart);
        verifyChartPixels(chart);
    }
    function test_nativeGeometryIsSaved() {
        const w = createTemporaryObject(factory, test);
        const win = floatEditor(w, false);
        win.width += 40;
        win.height += 20;
        tryVerify(() => w.controller.floatingWindows[0].geometry.width === win.width && w.controller.floatingWindows[0].geometry.height === win.height);
        const saved = JSON.parse(w.controller.saveLayout());
        compare(saved.version, 2);
        compare(saved.floating[0].geometry.width, win.width);
        compare(saved.floating[0].geometry.height, win.height);
    }
    function test_pointerMergeIntoFloating_data() {
        const rows = [];
        for (let density = 0; density < 3; density++)
            for (const floatingSource of [false, true])
                rows.push({ tag: (floatingSource ? "floating" : "main") + "-density-" + density,
                            floatingSource: floatingSource, density: density });
        return rows;
    }
    function test_pointerMergeIntoFloating(data) {
        Theme.density = data.density;
        Theme.colorScheme = data.floatingSource ? ThemePalette.Dark : ThemePalette.Light;
        const w = createTemporaryObject(factory, test);
        const panel = w.registry.a;
        panel.input.text = "migrated edit";
        verify(w.floatPanel("b"));
        tryVerify(() => w.registry.b.Window.window !== w.Window.window);
        const destination = w.registry.b.Window.window;
        destination.width = 320;
        destination.height = 240;
        destination.x = w.mapToGlobal(450, 150).x;
        destination.y = w.mapToGlobal(450, 150).y;
        let source = w, sourceWindow = null;
        let sx = 20, sy = w.headerHeight / 2;
        if (data.floatingSource) {
            sourceWindow = floatEditor(w, false);
            sourceWindow.width = 320;
            sourceWindow.height = 240;
            sourceWindow.x = w.mapToGlobal(20, 150).x;
            sourceWindow.y = destination.y;
            source = findChild(sourceWindow.contentItem, "dockFloatingTabDrag_a");
            verify(source !== null);
            sy = source.height / 2;
        }
        destination.update();
        verify(waitForRendering(destination.contentItem));
        const global = destination.contentItem.mapToGlobal(160, 120);
        const point = source.mapFromGlobal(global.x, global.y);
        const id = w.controller.floatingWindowForPanel("b");
        mousePress(source, sx, sy);
        const focusOwner = source.Window.window;
        const focused = focusOwner.activeFocusItem;
        mouseMove(source, point.x, point.y, 30);
        verify(w.dropTarget !== null);
        compare(w.dropTarget.windowId, id);
        const ghost = w.dragPreviewHost;
        verify(ghost !== null);
        tryCompare(ghost, "visible", true);
        compare(focusOwner.activeFocusItem, focused);
        compare(ghost.x, Math.round(global.x + 20));
        compare(ghost.y, Math.round(global.y + 24));
        verify((ghost.flags & Qt.WindowTransparentForInput) !== 0);
        verify((ghost.flags & Qt.WindowDoesNotAcceptFocus) !== 0);
        tryVerify(() => w.dragImageResult !== null);
        const thumbnail = findChild(ghost.contentItem, "dockDragThumbnail");
        tryCompare(thumbnail, "status", Image.Ready);
        const preview = findChild(destination.contentItem, "dockFloatingDropPreview");
        verify(preview.visible);
        verify(findChild(destination.contentItem, "dockFloatingTabTarget").visible);
        destination.update();
        verify(waitForRendering(destination.contentItem));
        keyClick(Qt.Key_Escape);
        mouseRelease(source, point.x, point.y);
        verify(!preview.visible);
        tryCompare(ghost, "visible", false);
        compare(w.dragImageResult, null);
        verify(w.controller.floatingWindowForPanel("a") !== id);
        mousePress(source, sx, sy);
        mouseMove(source, point.x, point.y, 30);
        mouseRelease(source, point.x, point.y);
        compare(w.controller.floatingWindowForPanel("a"), id);
        tryCompare(panel.Window, "window", destination);
        compare(panel.input.text, "migrated edit");
        compare(w.controller.floatingWindows.length, 1);
        tryCompare(ghost, "visible", false);
        if (sourceWindow)
            tryVerify(() => !Qt.isQtObject(sourceWindow));
        const saved = w.controller.saveLayout();
        verify(w.controller.restoreLayout(saved));
        compare(w.controller.floatingWindowForPanel("a"), id);
        verify(w.controller.dockBack(id));
        tryCompare(panel.Window, "window", w.Window.window);
        compare(panel.input.text, "migrated edit");
    }
    function test_previewFollowsOutsideEveryWindow() {
        const w = createTemporaryObject(factory, test);
        const win = floatEditor(w, false);
        const source = findChild(win.contentItem, "dockFloatingTabDrag_a");
        verify(source !== null);
        win.update();
        verify(waitForRendering(win.contentItem));
        tryVerify(() => w.controller.floatingWindows[0].geometry.x === win.x);
        const saved = w.controller.saveLayout();
        const outside = w.mapToGlobal(w.width + 500, w.height + 100);
        const point = source.mapFromGlobal(outside.x, outside.y);
        mousePress(source, 20, source.height / 2);
        mouseMove(source, point.x, point.y, 30);
        verify(w.dragging);
        compare(w.dropTarget, null);
        const ghost = w.dragPreviewHost;
        tryCompare(ghost, "visible", true);
        compare(ghost.x, Math.round(outside.x + 20));
        mouseMove(source, point.x + 80, point.y + 60, 30);
        compare(ghost.x, Math.round(outside.x + 100));
        compare(ghost.y, Math.round(outside.y + 84));
        mouseRelease(source, point.x + 80, point.y + 60);
        tryCompare(ghost, "visible", false);
        compare(w.controller.saveLayout(), saved);
    }
    function test_destroyWorkspaceDuringPreviewCapture() {
        const w = createTemporaryObject(factory, test);
        verify(w.controller.activatePanel("a"));
        verify(waitForRendering(w.registry.a));
        w.beginDrag("a");
        const point = w.mapToGlobal(300, 300);
        w.updateGlobalDrag(point.x, point.y);
        const ghost = w.dragPreviewHost;
        verify(ghost.visible);
        w.destroy();
        tryVerify(() => !Qt.isQtObject(ghost));
    }
    function test_emptyMainAndFloatingTargetPolicies() {
        const w = createTemporaryObject(factory, test);
        const host = floatEditor(w, false);
        const point = host.contentItem.mapToGlobal(100, 100);
        w.registry.b.floatable = false;
        w.beginDrag("b");
        w.updateGlobalDrag(point.x, point.y);
        compare(w.dropTarget, null);
        w.cancelDrag();
        w.registry.b.floatable = true;
        w.beginDrag("b");
        w.updateGlobalDrag(point.x, point.y);
        verify(w.dropTarget !== null);
        w.layoutLocked = true;
        w.finishDrag();
        compare(w.controller.floatingWindowForPanel("b"), "");
        w.layoutLocked = false;
        w.beginDrag("b");
        w.updateGlobalDrag(point.x, point.y);
        verify(w.controller.dockBack(w.controller.floatingWindowForPanel("a")));
        w.finishDrag(); // A stale native-window target must not turn into a main-window dock.
        compare(w.controller.floatingWindows.length, 0);
        verify(w.floatPanel("a", true));
        tryVerify(() => w.registry.a.Window.window !== w.Window.window);
        const groupWindow = w.registry.a.Window.window;
        groupWindow.x = w.mapToGlobal(w.width + 50, 0).x;
        const main = w.mapToGlobal(400, 300);
        w.beginDrag("a", false);
        w.updateGlobalDrag(main.x, main.y);
        verify(w.dropTarget !== null);
        compare(w.dropTarget.target, "");
        w.finishDrag();
        tryCompare(w.registry.a.Window, "window", w.Window.window);
        compare(w.controller.floatingWindows.length, 1);
        verify(w.controller.floatingWindowForPanel("b") !== "");
    }
    function test_dragFloatingPanelToWorkspace_data() {
        return [
            { tag: "tabs", x: 400, y: 300, groups: 1 },
            { tag: "left", x: 5, y: 300, groups: 2 },
            { tag: "right", x: 795, y: 300, groups: 2 },
            { tag: "top", x: 400, y: 5, groups: 2 },
            { tag: "bottom", x: 400, y: 595, groups: 2 }
        ];
    }
    function test_pointerDragFromFloatingWindow() {
        const w = createTemporaryObject(factory, test);
        const win = floatEditor(w, false);
        win.x = w.mapToGlobal(w.width + 50, 0).x;
        verify(waitForRendering(win.contentItem));
        const area = findChild(win.contentItem, "dockFloatingTabDrag_a");
        verify(area !== null);
        const global = w.mapToGlobal(5, 300);
        const target = area.mapFromGlobal(global.x, global.y);
        mousePress(area, 10, area.height / 2);
        mouseMove(area, target.x, target.y, 30);
        verify(w.dragging);
        verify(w.dropTarget !== null);
        keyClick(Qt.Key_Escape);
        verify(!w.dragging);
        mouseRelease(area, target.x, target.y);
        compare(w.controller.floatingWindows.length, 1);
        mousePress(area, 10, area.height / 2);
        mouseMove(area, target.x, target.y, 30);
        mouseRelease(area, target.x, target.y);
        compare(w.geometry.groups.length, 2);
        tryCompare(w.registry.a.Window, "window", w.Window.window);
    }
    function test_pointerDragOutCreatesFloatingWindow() {
        const w = createTemporaryObject(factory, test);
        verify(waitForRendering(w));
        const panel = w.registry.a;
        panel.input.text = "dragged content";
        const saved = w.controller.saveLayout();
        mousePress(w, 20, w.headerHeight / 2);
        mouseMove(w, -60, 150, 30);
        verify(w.dragging);
        verify(w.floatDropPoint !== null);
        keyClick(Qt.Key_Escape);
        mouseRelease(w, -60, 150);
        compare(w.controller.saveLayout(), saved);
        panel.floatable = false;
        mousePress(w, 20, w.headerHeight / 2);
        mouseMove(w, -60, 150, 30);
        mouseRelease(w, -60, 150);
        compare(w.controller.floatingWindows.length, 0);
        panel.floatable = true;
        mousePress(w, 20, w.headerHeight / 2);
        mouseMove(w, -60, 150, 30);
        compare(w.controller.floatingWindows.length, 0);
        mouseRelease(w, -60, 150);
        compare(w.controller.floatingWindows.length, 1);
        tryVerify(() => panel.Window.window !== w.Window.window);
        compare(panel.input.text, "dragged content");
        compare(w.controller.floatingWindowForPanel("b"), "");
        verify(w.controller.dockBack(w.controller.floatingWindowForPanel("a")));
        tryCompare(panel.Window, "window", w.Window.window);
    }
    function test_dragPoliciesAndOutsideRelease() {
        const w = createTemporaryObject(factory, test);
        floatEditor(w, false);
        const saved = w.controller.saveLayout();
        w.layoutLocked = true;
        w.beginDrag("a", false);
        verify(!w.dragging);
        w.layoutLocked = false;
        w.registry.a.movable = false;
        w.beginDrag("a", false);
        verify(!w.dragging);
        w.registry.a.movable = true;
        w.beginDrag("a", false);
        w.updateDrag(5, 300);
        verify(w.dropTarget !== null);
        w.registry.a.movable = false;
        w.finishDrag();
        compare(w.controller.saveLayout(), saved);
        w.registry.a.movable = true;
        w.beginDrag("a", false);
        const outside = w.mapToGlobal(-20, -20);
        w.updateGlobalDrag(outside.x, outside.y);
        compare(w.dropTarget, null);
        w.finishDrag();
        compare(w.controller.saveLayout(), saved);
    }
    function test_dragFloatingPanelToWorkspace(data) {
        const w = createTemporaryObject(factory, test);
        const win = floatEditor(w, false);
        // Keep the native host clear of the target for deterministic hit testing.
        win.x = w.mapToGlobal(w.width + 50, 0).x;
        const point = w.mapToGlobal(data.x, data.y);
        const saved = w.controller.saveLayout();
        w.beginDrag("a", false);
        w.updateGlobalDrag(point.x, point.y);
        verify(w.dropTarget !== null);
        w.cancelDrag();
        compare(w.controller.saveLayout(), saved);
        w.beginDrag("a", false);
        w.updateGlobalDrag(win.x + 10, win.y + 10);
        compare(w.dropTarget, null);
        w.updateGlobalDrag(point.x, point.y);
        w.finishDrag();
        compare(w.geometry.groups.length, data.groups);
        compare(w.controller.floatingWindows.length, 0);
        tryCompare(w.registry.a.Window, "window", w.Window.window);
        compare(w.registry.a.input.text, "retained");
    }
    function test_hiddenWorkspaceAndOffscreenRestore() {
        const w = createTemporaryObject(factory, test);
        const win = floatEditor(w, false);
        w.visible = false;
        tryCompare(win, "visible", false);
        w.visible = true;
        tryCompare(win, "visible", true);
        const saved = JSON.parse(w.controller.saveLayout());
        saved.floating[0].geometry.x = 9000000;
        saved.floating[0].geometry.y = 9000000;
        verify(w.controller.restoreLayout(JSON.stringify(saved)));
        tryVerify(() => win.x !== 9000000 && win.y !== 9000000);
        const geometry = w.controller.floatingWindows[0].geometry;
        verify(geometry.x !== 9000000 && geometry.y !== 9000000);
        verify(win.width > 0 && win.height > 0);
    }
    function test_destroyWorkspaceClosesFloatingWindow() {
        const w = createTemporaryObject(factory, test);
        const win = floatEditor(w, false);
        w.destroy();
        tryVerify(() => !Qt.isQtObject(win));
    }
}
