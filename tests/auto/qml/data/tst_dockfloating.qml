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
    }
    function cleanup() {
        Theme.reducedMotion = false;
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
