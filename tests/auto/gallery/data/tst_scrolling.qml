// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryScrolling"
    when: windowShown
    property var gallery
    Component { id: factory; Main { width: 1200; height: 900; initialPage: "table" } }

    function init() {
        failOnWarning(/.*/);
        Theme.reducedMotion = true;
        gallery = createTemporaryObject(factory, test);
        verify(waitForRendering(gallery.contentItem));
    }
    function cleanup() {
        gallery.close();
        Theme.density = ThemePalette.Desktop;
        Theme.reducedMotion = false;
    }
    function find(name) {
        const object = findChild(gallery.contentItem, name) || findChild(gallery, name);
        verify(object !== null, name);
        return object;
    }
    function reset(view) {
        view.cancelFlick();
        view.contentY = view.originY;
        wait(20);
    }
    function reveal(item) {
        const scroll = find("galleryContentScroll");
        const at = item.mapToItem(scroll, 0, 0);
        scroll.contentItem.contentY = Math.max(0, at.y + scroll.contentItem.contentY - 40);
        verify(waitForPolish(gallery.contentItem));
        return scroll.contentItem;
    }
    function test_pageDragAndWheel_data() {
        return [{tag: "desktop", width: 1200, density: ThemePalette.Desktop},
                {tag: "touch", width: 1200, density: ThemePalette.Touch},
                {tag: "mobile", width: 420, density: ThemePalette.Mobile}];
    }
    function test_pageDragAndWheel(data) {
        Theme.density = data.density;
        gallery.width = data.width;
        gallery.update();
        verify(waitForRendering(gallery.contentItem));
        const scroll = find("galleryContentScroll");
        const view = scroll.contentItem;
        verify(view.contentHeight > view.height);
        mouseDrag(scroll, 6, 450, 0, -180, Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentY > 20, "Page margins must allow mouse dragging");
        reset(view);
        mouseWheel(scroll, 6, 250, 0, -120);
        tryVerify(() => view.contentY > 20);
    }
    function test_navigationDragAndWheel_data() {
        return [{tag: "sidebar", width: 1200}, {tag: "drawer", width: 420}];
    }
    function test_navigationDragAndWheel(data) {
        gallery.width = data.width;
        gallery.height = 600;
        verify(waitForRendering(gallery.contentItem));
        let scroll;
        let drawer;
        if (data.width === 420) {
            mouseClick(find("openNavigationButton"));
            drawer = find("galleryNavigationDrawer");
            tryCompare(drawer, "opened", true);
            scroll = drawer.contentItem;
        } else {
            scroll = find("galleryNavigationScroll");
        }
        const view = scroll.contentItem;
        verify(view.contentHeight > view.height);
        mouseDrag(scroll, 80, 280, 0, -170, Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentY > 20);
        compare(gallery.componentId, "table", "Dragging a navigation item must not activate it");
        if (drawer) compare(drawer.opened, true);
        reset(view);
        mouseWheel(scroll, 80, 250, 0, -120);
        tryVerify(() => view.contentY > 20);
    }
    function test_nestedDataView_data() {
        return [{tag: "table", page: "table", name: "demoTableView"},
                {tag: "chat", page: "chat", name: "demoChatView"}];
    }
    function test_nestedDataView(data) {
        gallery.showComponent(data.page);
        // The initial table page may already be static, with no pending frame.
        gallery.update();
        verify(waitForRendering(gallery.contentItem));
        const demo = find("dataPreview").dataController;
        demo.batchSize = 100;
        demo.addBatch();
        const control = find(data.name);
        const view = data.page === "table" ? control.view : control;
        if (data.page === "chat") view.followTail = false;
        reset(view);
        const outer = reveal(control);
        const outerY = outer.contentY;
        mouseDrag(view, 80, view.height - 40, 0, -160, Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentY > view.originY + 20);
        fuzzyCompare(outer.contentY, outerY, 1, "An inner drag must not move the page");
        reset(view);
        mouseWheel(view, 80, 120, 0, -120);
        tryVerify(() => view.contentY > view.originY + 20);
        fuzzyCompare(outer.contentY, outerY, 1);
        if (data.page === "table") {
            reset(view);
            mouseClick(view, 80, Theme.tokens.controlHeight * 1.5);
            compare(demo.selectedIndex, 1);
            mousePress(view, 80, view.height - 40);
            wait(400);
            for (let step = 1; step <= 8; step++)
                mouseMove(view, 80, view.height - 40 - step * 20, 30, Qt.LeftButton);
            mouseRelease(view, 80, view.height - 200);
            verify(view.contentY > view.originY + 20, "A held press must still start dragging");
            fuzzyCompare(outer.contentY, outerY, 1);
            reset(view);
            const beforeWidth = view.columnWidth(0);
            mouseMove(view, beforeWidth, Theme.tokens.controlHeight / 2);
            // TableView resolves the resize edge during frame-synchronous hover delivery.
            gallery.update();
            verify(waitForRendering(gallery.contentItem));
            mouseDrag(view, beforeWidth, Theme.tokens.controlHeight / 2, 60, 0,
                      Qt.LeftButton, Qt.NoModifier, 20);
            verify(view.columnWidth(0) > beforeWidth + 20, "Column resizing must retain its grab");
        }
    }
    function test_chartKeepsMouseGrab() {
        gallery.showComponent("chart");
        verify(waitForRendering(gallery.contentItem));
        const chart = find("demoChart");
        const outer = reveal(chart);
        const outerY = outer.contentY;
        const plot = chart.plotRect();
        const start = Qt.point(plot.x + plot.width / 2, plot.y + plot.height / 2);
        const value = chart.mapToValue(start);
        mousePress(chart, start.x, start.y);
        try {
            for (let step = 1; step <= 5; step++) {
                mouseMove(chart, start.x - step * 15, start.y - step * 12, 20, Qt.LeftButton);
                fuzzyCompare(outer.contentY, outerY, 1);
                const at = chart.mapToPosition(value);
                fuzzyCompare(at.x, start.x - step * 15, 1);
                fuzzyCompare(at.y, start.y - step * 12, 1);
            }
        } finally {
            mouseRelease(chart, start.x - 75, start.y - 60);
        }
    }
}
