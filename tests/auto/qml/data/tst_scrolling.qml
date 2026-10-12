// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls.Basic as B
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "Scrolling"
    when: windowShown
    width: 800
    height: 700
    visible: true
    property var objects: []

    function create(source) {
        const object = Qt.createQmlObject('import QtQuick; import QCatUiKit; ' + source, test);
        objects.push(object);
        verify(waitForRendering(object));
        return object;
    }
    function init() {
        failOnWarning(/.*/);
        Theme.reducedMotion = true;
    }
    function cleanup() {
        for (const object of objects) object.destroy();
        objects = [];
        wait(10);
        Theme.density = ThemePalette.Desktop;
        Theme.reducedMotion = false;
    }
    function reset(view) {
        view.cancelFlick();
        view.contentX = view.originX;
        view.contentY = view.originY;
        wait(20);
    }
    function test_mouseDragAndWheel_data() {
        const cases = [
            {tag: "scroll", source: 'CatScrollView { width: 320; height: 240; contentWidth: availableWidth; contentHeight: 1500; Rectangle { width: 320; height: 1500; color: "lightgrey" } }', wrapped: true},
            {tag: "flow", source: 'CatFlowView { width: 320; height: 240; model: 100 }', wrapped: true},
            {tag: "list", source: 'CatListView { width: 320; height: 240; model: 100 }'},
            {tag: "grid", source: 'CatGridView { width: 320; height: 240; model: 100 }'},
            {tag: "tree", source: 'CatTreeView { width: 320; height: 240; model: TreeModel { items: Array.from({length: 100}, (_, i) => ({text: "Row " + i})) } }'},
            {tag: "table", source: 'CatTableView { width: 320; height: 240; model: TableModel { columns: ["name"]; Component.onCompleted: { for (let i = 0; i < 100; i++) append({name: "Row " + i}); } } }', table: true},
            {tag: "chat", source: 'CatChatView { width: 320; height: 240; followTail: false; model: ChatModel { Component.onCompleted: { for (let i = 0; i < 100; i++) append({sender: "peer", username: "Peer", content: "Message " + i}); } } }'}
        ];
        const rows = [];
        for (const row of cases)
            for (const density of [0, 1, 2])
                rows.push(Object.assign({}, row, {tag: row.tag + "_" + density, density: density}));
        return rows;
    }
    function test_mouseDragAndWheel(data) {
        Theme.density = data.density;
        const control = create(data.source);
        const view = data.wrapped ? control.contentItem : data.table ? control.view : control;
        verify(view.contentHeight > view.height);
        mouseDrag(view, 100, view.height - 30, 0, -(view.height - 60), Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentY > view.originY + 20, "Dragging must scroll " + data.tag);
        reset(view);
        mouseWheel(view, 100, 120, 0, -120);
        tryVerify(() => view.contentY > view.originY + 20, 1000, "Wheel must scroll " + data.tag);
    }
    function test_clickAndDragFromButton() {
        const view = create('CatFlowView { width: 320; height: 240; model: 100 }');
        mouseClick(view, 60, 30);
        compare(view.currentIndex, 0);
        view.currentIndex = -1;
        mouseDrag(view, 60, 180, 0, -130, Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentItem.contentY > 20);
        compare(view.currentIndex, -1, "A drag must not activate a button on release");
    }
    function test_textSelectionAndScrollBar() {
        const view = create('CatScrollView { width: 320; height: 240; contentWidth: availableWidth; contentHeight: 1500; Item { width: 320; height: 1500; CatTextField { objectName: "editor"; width: 260; text: "Select these words with the mouse" } } }');
        const editor = findChild(view, "editor");
        mouseDrag(editor, 25, editor.height / 2, 180, 0, Qt.LeftButton, Qt.NoModifier, 20);
        verify(editor.selectedText.length > 0);
        compare(view.contentItem.contentY, 0);
        const bar = view.B.ScrollBar.vertical;
        mouseDrag(bar, bar.width / 2, 8, 0, 100, Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentItem.contentY > 20, "The scroll bar must remain draggable");
        verify(editor.selectedText.length > 0);
    }
    function test_scrollBarPolicy() {
        const view = create('CatScrollView { width: 320; height: 240; contentWidth: availableWidth; contentHeight: 1500; Item { width: 320; height: 1500 } }');
        const horizontal = view.B.ScrollBar.horizontal;
        const vertical = view.B.ScrollBar.vertical;
        compare(horizontal.visible, false, "A fitted direction needs no scroll bar");
        compare(vertical.visible, true);
        horizontal.policy = B.ScrollBar.AlwaysOn;
        compare(horizontal.visible, true);
        vertical.policy = B.ScrollBar.AlwaysOff;
        compare(vertical.visible, false);
        vertical.policy = B.ScrollBar.AsNeeded;
        compare(vertical.visible, true);
        view.contentHeight = view.availableHeight;
        tryCompare(vertical, "visible", false);
    }
    function test_multilineTextSelection() {
        const view = create('CatScrollView { width: 320; height: 240; contentWidth: availableWidth; contentHeight: 1500; Item { width: 320; height: 1500; CatTextArea { objectName: "editor"; width: 260; height: 180; text: "First line\\nSecond line\\nThird line\\nFourth line\\nFifth line" } } }');
        const editor = findChild(view, "editor");
        mouseDrag(editor, 25, 20, 100, 70, Qt.LeftButton, Qt.NoModifier, 20);
        verify(editor.selectedText.includes("\n"), "Vertical dragging must select multiple text lines");
        compare(view.contentItem.contentY, 0);
    }
    function test_touchScrollAfterMouse() {
        const view = create('CatScrollView { width: 320; height: 240; contentWidth: availableWidth; contentHeight: 1500; Item { width: 320; height: 1500 } }');
        mouseWheel(view, 100, 100, 0, -120);
        tryVerify(() => view.contentItem.contentY > 20);
        reset(view.contentItem);
        const touch = touchEvent(view);
        touch.press(0, view, 100, 200).commit();
        for (let y = 180; y >= 40; y -= 20) {
            touch.move(0, view, 100, y).commit();
            wait(20);
        }
        touch.release(0, view, 100, 40).commit();
        verify(view.contentItem.contentY > 20, "Touch dragging must still scroll");
    }
    function test_horizontalDragAndWheel() {
        const view = create('CatScrollView { width: 320; height: 240; contentWidth: 1500; contentHeight: availableHeight; Item { width: 1500; height: 240 } }');
        mouseDrag(view, 260, 100, -180, 0, Qt.LeftButton, Qt.NoModifier, 20);
        verify(view.contentItem.contentX > 20);
        reset(view.contentItem);
        mouseWheel(view, 100, 100, -120, 0);
        tryVerify(() => view.contentItem.contentX > 20);
        view.wheelEnabled = false;
        reset(view.contentItem);
        mouseWheel(view, 100, 100, -120, 0);
        wait(100);
        compare(view.contentItem.contentX, 0);
    }
    function test_nestedViewKeepsGesture() {
        const outer = create('CatScrollView { width: 420; height: 360; contentWidth: availableWidth; contentHeight: 1500; Item { width: 420; height: 1500; CatListView { objectName: "inner"; x: 20; y: 20; width: 280; height: 240; model: 100 } } }');
        const inner = findChild(outer, "inner");
        mouseDrag(inner, 100, 190, 0, -140, Qt.LeftButton, Qt.NoModifier, 20);
        verify(inner.contentY > 20);
        compare(outer.contentItem.contentY, 0);
        inner.cancelFlick();
        mouseDrag(outer, 350, 300, 0, -150, Qt.LeftButton, Qt.NoModifier, 20);
        verify(outer.contentItem.contentY > 20);
    }
    function test_nestedScrollViewKeepsGesture() {
        const outer = create('CatScrollView { width: 420; height: 360; contentWidth: availableWidth; contentHeight: 1500; Item { width: 420; height: 1500; CatScrollView { objectName: "inner"; x: 20; y: 20; width: 280; height: 240; contentWidth: availableWidth; contentHeight: 1500; Rectangle { width: 280; height: 1500; color: "lightgrey" } } } }');
        const inner = findChild(outer, "inner");
        mouseDrag(inner, 100, 190, 0, -140, Qt.LeftButton, Qt.NoModifier, 20);
        verify(inner.contentItem.contentY > 20);
        compare(outer.contentItem.contentY, 0);
        reset(inner.contentItem);
        mouseWheel(inner, 100, 100, 0, -120);
        tryVerify(() => inner.contentItem.contentY > 20);
        compare(outer.contentItem.contentY, 0);
    }
    function test_multiSelectPopup() {
        const picker = create('CatMultiSelectComboBox { width: 280; model: Array.from({length: 100}, (_, i) => "Option " + i) }');
        mouseClick(picker);
        let popup = null;
        for (const resource of picker.resources)
            if (resource instanceof CatPopup) popup = resource;
        verify(popup !== null);
        tryCompare(popup, "opened", true);
        let scroll = null;
        for (const child of popup.contentItem.children)
            if (child instanceof CatScrollView) scroll = child;
        verify(scroll !== null);
        mouseDrag(scroll, 80, 200, 0, -140, Qt.LeftButton, Qt.NoModifier, 20);
        verify(scroll.contentItem.contentY > 20);
        compare(picker.selectedIndexes.length, 0, "Dragging options must not toggle a checkbox");
        reset(scroll.contentItem);
        mouseWheel(scroll, 80, 100, 0, -120);
        tryVerify(() => scroll.contentItem.contentY > 20);
        reset(scroll.contentItem);
        mouseClick(scroll, 40, Theme.tokens.controlHeight / 2);
        compare(picker.selectedIndexes.length, 1);
        compare(picker.selectedIndexes[0], 0);
        popup.close();
    }
}
