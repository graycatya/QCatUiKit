// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "DrawerContracts"
    when: windowShown
    width: 800
    height: 700
    visible: true
    Item {
        id: nestedParent
        x: 120
        y: 80
        width: 300
        height: 250
    }
    Component {
        id: paletteFactory
        ThemePalette {}
    }
    Component {
        id: drawerFactory
        CatDrawer {
            width: 160
            height: 180
            contentItem: CatLabel {
                text: "Drawer content"
            }
        }
    }
    function init() {
        failOnWarning(/.*/);
    }
    function test_openClose_data() {
        const rows = [];
        for (const edge of [Qt.LeftEdge, Qt.RightEdge, Qt.TopEdge, Qt.BottomEdge])
            for (const reducedMotion of [false, true])
                rows.push({
                    tag: edge + "_" + reducedMotion,
                    edge: edge,
                    reducedMotion: reducedMotion
                });
        return rows;
    }
    function test_openClose(data) {
        const palette = createTemporaryObject(paletteFactory, test, {
            reducedMotion: data.reducedMotion
        });
        const drawer = createTemporaryObject(drawerFactory, nestedParent, {
            edge: data.edge,
            theme: palette
        });
        verify(drawer !== null);
        verify(drawer.parent !== nestedParent, "Drawer must use the window overlay");
        for (let turn = 0; turn < 2; ++turn) {
            drawer.open();
            tryCompare(drawer, "opened", true);
            compare(drawer.position, 1);
            verify(drawer.background.visible);
            const origin = drawer.background.mapToItem(drawer.parent, 0, 0);
            const expectedX = data.edge === Qt.RightEdge ? drawer.parent.width - drawer.width : 0;
            const expectedY = data.edge === Qt.BottomEdge ? drawer.parent.height - drawer.height : 0;
            verify(Math.abs(origin.x - expectedX) < 1, "Drawer must reach the requested window edge");
            verify(Math.abs(origin.y - expectedY) < 1, "Drawer must reach the requested window edge");
            drawer.close();
            tryCompare(drawer, "visible", false);
            compare(drawer.position, 0);
        }
    }
}
