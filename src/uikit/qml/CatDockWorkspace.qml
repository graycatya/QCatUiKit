// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Window
import QCatUiKit
import "DockGeometry.js" as Geometry

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var dragStyleHints: Qt.styleHints
    readonly property var tokens: (theme || Theme).tokens
    readonly property DockController controller: dockState
    default property alias panels: panelLayer.data
    readonly property var minimumSize: Geometry.measure(dockState.layout, registry, headerHeight, 8)
    readonly property real minimumDockWidth: minimumSize.width
    readonly property real minimumDockHeight: minimumSize.height
    readonly property bool sizeConstrained: width < minimumDockWidth || height < minimumDockHeight
    readonly property bool dragging: dragId.length > 0
    property bool layoutLocked: false
    readonly property real headerHeight: tokens.controlHeight
    readonly property var geometry: Geometry.calculate(dockState.layout, registry, width, height, headerHeight, 8)
    property var registry: Object.create(null)
    property var floatingHosts: Object.create(null)
    property var focusItems: Object.create(null)
    property bool updatingWindows: false
    property string dragId: ""
    property var dropTarget: null
    property bool ready: false
    implicitWidth: 720
    implicitHeight: 480
    clip: true

    function synchronize() {
        if (!ready)
            return;
        const next = Object.create(null);
        for (const id of Object.keys(registry))
            if (registry[id])
                next[id] = registry[id];
        for (const child of panelLayer.children) {
            if (child instanceof CatDockPanel) {
                if (!child.dockId || (next[child.dockId] && next[child.dockId] !== child)) {
                    console.warn("Dock panel IDs must be nonempty and unique");
                    child.visible = false;
                    continue;
                }
                next[child.dockId] = child;
                child._dockWorkspace = control;
            }
        }
        registry = next;
        for (const id of dockState.panelIds)
            if (!next[id])
                dockState.unregisterPanel(id);
        for (const id of Object.keys(next))
            if (dockState.panelIds.indexOf(id) < 0)
                dockState.registerPanel(id);
        placePanels();
    }
    function panelDestroyed(id) {
        if (!ready)
            return;
        const next = Object.assign(Object.create(null), registry);
        delete next[id];
        delete focusItems[id];
        registry = next;
        if (dockState.panelIds.indexOf(id) >= 0)
            dockState.unregisterPanel(id);
    }
    function movePanel(panel, destination) {
        if (panel.parent === destination)
            return;
        const oldWindow = panel.Window.window;
        const focused = oldWindow ? oldWindow.activeFocusItem : null;
        let ancestor = focused;
        while (ancestor && ancestor !== panel)
            ancestor = ancestor.parent;
        if (ancestor === panel)
            focusItems[panel.dockId] = focused;
        panel.parent = destination;
        Qt.callLater(restorePanelFocus, panel.dockId);
    }
    function restorePanelFocus(id) {
        const panel = registry[id], focused = focusItems[id];
        if (!ready || !panel || !panel.visible || !focused || !panel.Window.window)
            return;
        panel.Window.window.requestActivate();
        focused.forceActiveFocus();
    }
    function placePanels() {
        const placements = Object.create(null);
        for (const group of geometry.groups)
            placements[group.active] = group;
        for (const id of Object.keys(registry)) {
            const panel = registry[id];
            if (!panel || dockState.floatingWindowForPanel(id))
                continue;
            movePanel(panel, panelLayer);
            const group = placements[id];
            if (group) {
                panel.x = group.x;
                panel.y = group.y + headerHeight;
                panel.width = group.width;
                panel.height = Math.max(0, group.height - headerHeight);
            }
            panel.visible = !!group;
        }
        if (dragId && (!registry[dragId] || !registry[dragId].movable))
            cancelDrag();
    }
    function placeFloatingPanels(record, host) {
        if (!ready)
            return;
        for (const id of record.tabs) {
            const panel = registry[id];
            if (!panel)
                continue;
            movePanel(panel, host);
            panel.x = 0;
            panel.y = 0;
            panel.width = host.width;
            panel.height = host.height;
            panel.visible = id === record.active;
        }
    }
    function updateWindows() {
        if (!ready || updatingWindows)
            return;
        updatingWindows = true;
        placePanels();
        const present = Object.create(null);
        for (const record of dockState.floatingWindows) {
            present[record.id] = true;
            let host = floatingHosts[record.id];
            if (!host) {
                host = floatingFactory.createObject(control, {
                    workspace: control
                });
                floatingHosts[record.id] = host;
            }
            host.applyRecord(record);
            placeFloatingPanels(record, host.panelHost);
        }
        for (const id of Object.keys(floatingHosts)) {
            if (present[id])
                continue;
            const host = floatingHosts[id];
            for (const key of Object.keys(registry)) {
                const panel = registry[key];
                if (panel && panel.parent === host.panelHost)
                    movePanel(panel, panelLayer);
            }
            host.destroy();
            delete floatingHosts[id];
        }
        updatingWindows = false;
    }
    function floatPanel(id, wholeGroup = false) {
        const panel = registry[id];
        if (!panel || layoutLocked || !panel.movable || !panel.floatable)
            return false;
        if (wholeGroup) {
            for (const group of geometry.groups)
                if (group.tabs.indexOf(id) >= 0) {
                    for (const member of group.tabs) {
                        if (!registry[member] || !registry[member].movable || !registry[member].floatable)
                            return false;
                    }
                }
        }
        const point = panel.mapToGlobal(20, 20);
        const w = Math.max(320, panel.width), h = Math.max(240, panel.height + headerHeight);
        if (!dockState.floatPanel(id, wholeGroup))
            return false;
        dockState.setFloatingGeometry(dockState.floatingWindowForPanel(id), point.x, point.y, w, h);
        dockState.constrainFloatingWindows();
        return true;
    }
    function requestCloseWindow(id) {
        const record = dockState.floatingWindows.find(window => window.id === id);
        if (!record)
            return;
        for (const key of record.tabs)
            if (registry[key] && !registry[key].closable)
                return;
        for (const key of record.tabs)
            if (registry[key])
                registry[key].closeRequested();
    }
    function beginDrag(id) {
        if (layoutLocked || !registry[id] || !registry[id].movable)
            return;
        dragId = id;
        dropTarget = null;
        forceActiveFocus();
    }
    function updateDrag(x, y) {
        dropTarget = null;
        if (!dragId || x < 0 || y < 0 || x >= width || y >= height)
            return;
        for (const g of geometry.groups) {
            if (x < g.x || y < g.y || x >= g.x + g.width || y >= g.y + g.height)
                continue;
            const nx = (x - g.x) / g.width, ny = (y - g.y) / g.height;
            let edge = DockController.Center;
            if (nx < 0.23)
                edge = DockController.Left;
            else if (nx > 0.77)
                edge = DockController.Right;
            else if (ny < 0.23)
                edge = DockController.Top;
            else if (ny > 0.77)
                edge = DockController.Bottom;
            const target = g.tabs.find(id => id !== dragId);
            if (!target)
                return;
            dropTarget = {
                target: target,
                position: edge,
                x: g.x,
                y: g.y,
                width: g.width,
                height: g.height
            };
            return;
        }
    }
    function finishDrag() {
        const id = dragId, target = dropTarget;
        cancelDrag();
        if (id && target && !layoutLocked)
            dockState.dock(id, target.target, target.position);
    }
    function cancelDrag() {
        dragId = "";
        dropTarget = null;
    }
    onGeometryChanged: placePanels()
    onLayoutLockedChanged: if (layoutLocked)
        cancelDrag()
    Keys.onEscapePressed: event => {
        if (dragging) {
            cancelDrag();
            event.accepted = true;
        }
    }
    Component.onCompleted: {
        ready = true;
        synchronize();
    }
    Component.onDestruction: {
        ready = false;
        for (const id of Object.keys(registry)) {
            if (registry[id]) {
                registry[id]._dockWorkspace = null;
                registry[id].parent = panelLayer;
            }
        }
        for (const id of Object.keys(floatingHosts))
            floatingHosts[id].destroy();
    }
    Component {
        id: floatingFactory
        DockFloatingWindow {}
    }
    DockController {
        id: dockState
        onLayoutChanged: Qt.callLater(control.updateWindows)
    }
    Item {
        id: panelLayer
        anchors.fill: parent
        onChildrenChanged: Qt.callLater(control.synchronize)
    }
    Repeater {
        model: control.geometry.groups.length
        delegate: Rectangle {
            id: groupView
            required property int index
            readonly property var group: control.geometry.groups[index]
            x: group ? group.x : 0
            y: group ? group.y : 0
            width: group ? group.width : 0
            height: control.headerHeight
            color: control.tokens.surfaceHover
            clip: true
            B.ScrollView {
                anchors.fill: parent
                contentWidth: tabRow.width
                contentHeight: height
                B.ScrollBar.vertical.policy: B.ScrollBar.AlwaysOff
                B.ScrollBar.horizontal.policy: B.ScrollBar.AsNeeded
                Row {
                    id: tabRow
                    height: groupView.height
                    Repeater {
                        model: groupView.group ? groupView.group.tabs.length : 0
                        delegate: Row {
                            id: tabCell
                            required property int index
                            readonly property string panelId: groupView.group.tabs[index] || ""
                            readonly property CatDockPanel panel: control.registry[panelId] || null
                            height: groupView.height
                            CatButton {
                                id: tabButton
                                theme: control.theme
                                height: parent.height
                                text: tabCell.panel ? tabCell.panel.title : tabCell.panelId
                                checkable: true
                                checked: groupView.group.active === tabCell.panelId
                                Accessible.name: text
                                onClicked: dockState.activatePanel(tabCell.panelId)
                                Keys.onLeftPressed: if (!control.layoutLocked)
                                    dockState.moveTab(tabCell.panelId, Math.max(0, tabCell.index - 1))
                                Keys.onRightPressed: if (!control.layoutLocked)
                                    dockState.moveTab(tabCell.panelId, Math.min(groupView.group.tabs.length - 1, tabCell.index + 1))
                                CatMenu {
                                    id: tabMenu
                                    theme: control.theme
                                    CatMenuItem {
                                        theme: control.theme
                                        text: qsTr("Float panel")
                                        enabled: !control.layoutLocked && tabCell.panel && tabCell.panel.movable && tabCell.panel.floatable
                                        onTriggered: control.floatPanel(tabCell.panelId)
                                    }
                                    CatMenuItem {
                                        theme: control.theme
                                        text: qsTr("Float tab group")
                                        enabled: !control.layoutLocked && groupView.group.tabs.every(id => control.registry[id] && control.registry[id].movable && control.registry[id].floatable)
                                        onTriggered: control.floatPanel(tabCell.panelId, true)
                                    }
                                }
                                Keys.onMenuPressed: tabMenu.popup()
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    property point pressPoint
                                    property bool moved: false
                                    onPressed: mouse => {
                                        if (mouse.button === Qt.RightButton) {
                                            tabMenu.popup();
                                            return;
                                        }
                                        pressPoint = Qt.point(mouse.x, mouse.y);
                                        moved = false;
                                        tabButton.forceActiveFocus();
                                    }
                                    onPositionChanged: mouse => {
                                        if (!pressed || !(pressedButtons & Qt.LeftButton))
                                            return;
                                        if (!moved && Math.hypot(mouse.x - pressPoint.x, mouse.y - pressPoint.y) >= control.dragStyleHints.startDragDistance) {
                                            moved = true;
                                            control.beginDrag(tabCell.panelId);
                                        }
                                        if (control.dragging) {
                                            const p = mapToItem(control, mouse.x, mouse.y);
                                            control.updateDrag(p.x, p.y);
                                        }
                                    }
                                    onReleased: mouse => {
                                        if (mouse.button !== Qt.LeftButton)
                                            return;
                                        if (moved)
                                            control.finishDrag();
                                        else
                                            dockState.activatePanel(tabCell.panelId);
                                    }
                                    onCanceled: control.cancelDrag()
                                }
                            }
                            CatButton {
                                theme: control.theme
                                height: parent.height
                                implicitWidth: control.tokens.controlHeight
                                padding: 0
                                visible: tabCell.panel ? tabCell.panel.closable : false
                                text: "×"
                                Accessible.name: qsTr("Close %1").arg(tabCell.panel ? tabCell.panel.title : "")
                                onClicked: if (tabCell.panel)
                                    tabCell.panel.closeRequested()
                            }
                        }
                    }
                }
            }
        }
    }
    Repeater {
        model: control.geometry.handles.length
        delegate: Rectangle {
            id: handle
            required property int index
            readonly property var splitData: control.geometry.handles[index]
            x: splitData ? splitData.x : 0
            y: splitData ? splitData.y : 0
            width: splitData ? splitData.width : 0
            height: splitData ? splitData.height : 0
            color: resizeArea.pressed ? control.tokens.primary : control.tokens.border
            activeFocusOnTab: !control.layoutLocked
            Accessible.role: Accessible.Slider
            Accessible.name: qsTr("Resize dock panels")
            function adjust(delta) {
                if (!control.layoutLocked)
                    dockState.setSplitRatio(splitData.id, Math.max(splitData.minRatio, Math.min(splitData.maxRatio, splitData.ratio + delta)));
            }
            Keys.onLeftPressed: adjust(-0.05)
            Keys.onRightPressed: adjust(0.05)
            Keys.onUpPressed: adjust(-0.05)
            Keys.onDownPressed: adjust(0.05)
            MouseArea {
                id: resizeArea
                anchors.fill: parent
                enabled: !control.layoutLocked
                cursorShape: handle.splitData && handle.splitData.horizontal ? Qt.SplitHCursor : Qt.SplitVCursor
                onPressed: handle.forceActiveFocus()
                onPositionChanged: mouse => {
                    if (!pressed || !handle.splitData || handle.splitData.available <= 0)
                        return;
                    const p = mapToItem(control, mouse.x, mouse.y);
                    const value = ((handle.splitData.horizontal ? p.x : p.y) - handle.splitData.origin) / handle.splitData.available;
                    const ratio = Math.max(handle.splitData.minRatio, Math.min(handle.splitData.maxRatio, value));
                    if (ratio > 0 && ratio < 1)
                        dockState.setSplitRatio(handle.splitData.id, ratio);
                }
            }
        }
    }
    Rectangle {
        readonly property var target: control.dropTarget
        visible: control.dragging && target !== null
        x: target ? target.x + (target.position === DockController.Right ? target.width / 2 : 0) : 0
        y: target ? target.y + (target.position === DockController.Bottom ? target.height / 2 : 0) : 0
        width: target ? target.width / (target.position === DockController.Left || target.position === DockController.Right ? 2 : 1) : 0
        height: target ? target.height / (target.position === DockController.Top || target.position === DockController.Bottom ? 2 : 1) : 0
        color: control.tokens.primary
        opacity: 0.3
        border.width: 2
        border.color: control.tokens.text
    }
}
