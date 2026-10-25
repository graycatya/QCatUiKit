// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryVector"
    when: windowShown
    width: 640; height: 640
    visible: true
    Component { id: factory; GalleryVectorPreview { width: 600; componentId: "vector-canvas" } }
    function init() { failOnWarning(/.*/); }
    function test_addShapeUsesTheme() {
        const preview = createTemporaryObject(factory, test);
        verify(preview !== null);
        tryVerify(() => preview.vector !== null);
        compare(preview.vector.shapes.length, 2);
        const add = findChild(preview, "vectorAddShape");
        verify(add !== null);
        mouseClick(add);
        compare(preview.vector.shapes.length, 3);
        compare(String(preview.vector.shapes[2].fill), String(Theme.tokens.primary));
        compare(preview.vector.shapes[2].type, "ellipse");
        verify(!preview.vector.appendShape({type: "path", commands: [["X", 1, 2]]}));
        compare(preview.vector.shapes.length, 3);
        preview.vector.clearShapes();
        compare(preview.vector.shapes.length, 0);
    }
}
