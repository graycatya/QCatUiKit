// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryDensity"
    when: windowShown
    width: 400
    height: 200
    visible: true
    Component {
        id: buttonFactory
        CatButton {
            implicitWidth: 36
            padding: 6
            contentItem: GalleryIcon { name: "menu" }
        }
    }
    function init() { failOnWarning(/.*/); Theme.reducedMotion = true; }
    function cleanup() { Theme.density = ThemePalette.Desktop; Theme.reducedMotion = false; }
    function test_buttonIconAlignmentAcrossDensity_data() {
        return [{tag: "menu", name: "menu"}, {tag: "moon", name: "moon"},
                {tag: "sun", name: "sun"}];
    }
    function test_buttonIconAlignmentAcrossDensity(data) {
        const button = createTemporaryObject(buttonFactory, test);
        const icon = button.contentItem;
        icon.name = data.name;
        const shape = icon.children[0];
        for (const density of [0, 1, 2, 0, 2, 1, 0]) {
            Theme.density = density;
            verify(waitForRendering(button));
            const center = shape.mapToItem(button, shape.width / 2, shape.height / 2);
            fuzzyCompare(center.x, button.width / 2, 0.5);
            fuzzyCompare(center.y, button.height / 2, 0.5,
                         "Button icon must remain centered at density " + density);
            const topLeft = shape.mapToItem(icon, 0, 0);
            const bottomRight = shape.mapToItem(icon, shape.width, shape.height);
            verify(topLeft.x >= -0.5 && topLeft.y >= -0.5);
            verify(bottomRight.x <= icon.width + 0.5 && bottomRight.y <= icon.height + 0.5);
            fuzzyCompare(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, 0.5);
        }
    }
}
