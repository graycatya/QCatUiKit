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
        id: factory
        GalleryDockPreview {
            width: 360
            height: 800
        }
    }
    function init() {
        failOnWarning(/.*/);
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
}
