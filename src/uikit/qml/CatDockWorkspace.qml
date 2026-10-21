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
    readonly property real headerHeight: tokens.controlHeight * 2
    readonly property var geometry: Geometry.calculate(dockState.layout, registry, width, height, headerHeight, 8)
    property var registry: Object.create(null)
    property var floatingHosts: Object.create(null)
    property var floatingOrder: []
    property var focusItems: Object.create(null)
    property bool updatingWindows: false
    property string dragId: ""
    property var dropTarget: null
    property var floatDropPoint: null
    property point dragPoint
    property point dragGlobalPoint
    property bool hasDragPoint: false
    property var dragPreviewHost: null
    property var dragImageResult: null
    property int dragGeneration: 0
    property Item headerFocusItem: null
    property string headerFocusId: ""
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
                floatingOrder = floatingOrder.concat([record.id]);
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
            floatingOrder = floatingOrder.filter(key => key !== id);
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
    function raiseFloatingHost(id) {
        if (floatingOrder.indexOf(id) >= 0)
            floatingOrder = floatingOrder.filter(key => key !== id).concat([id]);
    }
    function beginDrag(id, focusWorkspace = true) {
        if (layoutLocked || !registry[id] || !registry[id].movable)
            return;
        dragId = id;
        hasDragPoint = false;
        dragImageResult = null;
        const generation = ++dragGeneration;
        if (!dragPreviewHost)
            dragPreviewHost = dragPreviewFactory.createObject(control, { workspace: control });
        const panel = registry[id];
        if (panel.visible && panel.width > 0 && panel.height > 0) {
            const scale = Math.min(1, 480 / panel.width, 320 / panel.height);
            panel.grabToImage(result => {
                if (ready && dragId === id && dragGeneration === generation)
                    dragImageResult = result;
            }, Qt.size(Math.max(1, Math.round(panel.width * scale)), Math.max(1, Math.round(panel.height * scale))));
        }
        dropTarget = null;
        floatDropPoint = null;
        if (focusWorkspace)
            forceActiveFocus();
    }
    function updateGlobalDrag(x, y) {
        if (!dragId)
            return;
        dragGlobalPoint = Qt.point(x, y);
        hasDragPoint = true;
        dragPoint = mapFromGlobal(x, y);
        dropTarget = null;
        floatDropPoint = null;
        if (!visible || !enabled || !Window.window || !Window.window.visible || Window.window.visibility === Window.Minimized)
            return;
        // Check native hosts before the main workspace, in activation order.
        for (const key of floatingOrder.slice().reverse()) {
            const host = floatingHosts[key];
            if (host.visible && host.visibility !== Window.Minimized && x >= host.x && y >= host.y && x < host.x + host.width && y < host.y + host.height) {
                const target = host.record.tabs.find(id => id !== dragId);
                const panel = registry[dragId];
                if (dragId && target && panel && panel.floatable && dockState.floatingWindowForPanel(dragId) !== key)
                    dropTarget = { target: target, position: DockController.Center, windowId: key };
                return;
            }
        }
        const point = mapFromGlobal(x, y);
        updateDrag(point.x, point.y);
    }
    function updateDrag(x, y) {
        dropTarget = null;
        floatDropPoint = null;
        if (!dragId)
            return;
        if (x < 0 || y < 0 || x >= width || y >= height) {
            const panel = registry[dragId];
            if (panel && panel.floatable && !dockState.floatingWindowForPanel(dragId))
                floatDropPoint = mapToGlobal(x, y);
            return;
        }
        if (geometry.groups.length === 0) {
            dropTarget = { target: "", position: DockController.Center, x: 0, y: 0, width: width, height: height };
            return;
        }
        const band = Math.min(tokens.controlHeight + 16, Math.min(width, height) / 4);
        const edges = [ { position: DockController.Left, distance: x },
                        { position: DockController.Right, distance: width - x },
                        { position: DockController.Top, distance: y },
                        { position: DockController.Bottom, distance: height - y } ];
        const edge = edges.reduce((nearest, candidate) => candidate.distance < nearest.distance ? candidate : nearest);
        if (edge.distance < band && geometry.groups.some(group => group.tabs.some(id => id !== dragId))) {
            const panel = registry[dragId];
            const horizontal = edge.position === DockController.Left || edge.position === DockController.Right;
            const extent = panel ? horizontal ? panel.width : panel.height + headerHeight : 0;
            const fraction = extent > 0 ? Math.max(0.15, Math.min(0.5, extent / (horizontal ? width : height))) : 0.25;
            const before = edge.position === DockController.Left || edge.position === DockController.Top;
            dropTarget = { target: "", position: edge.position, workspaceEdge: true,
                           ratio: before ? fraction : 1 - fraction, x: 0, y: 0, width: width, height: height };
            return;
        }
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
            const guide = Geometry.guidePosition(x - g.x, y - g.y, g.width, g.height, tokens.controlHeight);
            if (guide >= 0)
                edge = guide;
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
        const id = dragId, target = dropTarget, floatPoint = floatDropPoint;
        const open = dockState.floatingWindowForPanel(id) !== "" || geometry.groups.some(group => group.tabs.indexOf(id) >= 0);
        cancelDrag();
        if (!open)
            return;
        if (id && target && !layoutLocked && registry[id] && registry[id].movable
                && (!target.windowId || (registry[id].floatable && dockState.floatingWindowForPanel(target.target) === target.windowId))) {
            if (target.workspaceEdge)
                dockState.dockToWorkspace(id, target.position, target.ratio);
            else
                dockState.dock(id, target.target, target.position);
        }
        else if (id && floatPoint && floatPanel(id)) {
            const windowId = dockState.floatingWindowForPanel(id);
            const record = dockState.floatingWindows.find(window => window.id === windowId);
            dockState.setFloatingGeometry(windowId, floatPoint.x - 40, floatPoint.y - headerHeight / 2,
                                          record.geometry.width, record.geometry.height);
            dockState.constrainFloatingWindows();
        }
    }
    function cancelDrag() {
        ++dragGeneration;
        dragId = "";
        hasDragPoint = false;
        dragImageResult = null;
        dropTarget = null;
        floatDropPoint = null;
    }
    function panelHasFocus(panel) {
        if (panel && headerFocusItem && headerFocusItem.activeFocus && headerFocusId === panel.dockId)
            return true;
        let item = Window.window ? Window.window.activeFocusItem : null;
        while (item) {
            if (item === panel)
                return true;
            item = item.parent;
        }
        return false;
    }
    onGeometryChanged: placePanels()
    onLayoutLockedChanged: if (layoutLocked)
        cancelDrag()
    Shortcut {
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        enabled: control.dragging
        onActivated: control.cancelDrag()
    }
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
        if (dragPreviewHost)
            dragPreviewHost.destroy();
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
    Component {
        id: dragPreviewFactory
        DockDragPreview {}
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
            required property int index
            readonly property var group: control.geometry.groups[index]
            x: group ? group.x : 0
            y: group ? group.y : 0
            width: group ? group.width : 0
            height: group ? group.height : 0
            color: "transparent"
            border.color: group && control.panelHasFocus(control.registry[group.active]) ? control.tokens.primary : control.tokens.border
        }
    }
    Repeater {
        model: control.geometry.groups.length
        delegate: Rectangle {
            id: groupView
            required property int index
            readonly property var group: control.geometry.groups[index]
            readonly property var activePanel: group ? control.registry[group.active] : null
            x: group ? group.x : 0
            y: group ? group.y : 0
            width: group ? group.width : 0
            height: control.headerHeight
            color: control.tokens.background
            clip: true
            Rectangle {
                id: titleBar
                width: parent.width
                height: control.tokens.controlHeight
                color: groupView.activePanel && control.panelHasFocus(groupView.activePanel) ? control.tokens.surfaceHover : control.tokens.surface
                Rectangle { width: parent.width; height: 1; color: control.tokens.border }
                Text {
                    id: groupTitle
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: titleTools.left
                    height: parent.height
                    text: groupView.activePanel ? groupView.activePanel.title : ""
                    font.pixelSize: control.tokens.fontSize
                    font.weight: Font.DemiBold
                    color: control.tokens.text
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    DockDragArea {
                        workspace: control
                        panelId: groupView.group ? groupView.group.active : ""
                        focusItem: groupTitle
                    }
                }
                Row {
                    id: titleTools
                    anchors.right: parent.right
                    height: parent.height
                    DockTabButton {
                        objectName: "dockFloat_" + (groupView.group ? groupView.group.active : "")
                        theme: control.theme
                        height: parent.height
                        tool: true
                        symbol: 1
                        text: qsTr("Float panel")
                        enabled: !control.layoutLocked && !!groupView.activePanel && groupView.activePanel.movable && groupView.activePanel.floatable
                        onClicked: control.floatPanel(groupView.group.active)
                    }
                    DockTabButton {
                        theme: control.theme
                        height: parent.height
                        tool: true
                        symbol: 2
                        text: qsTr("Close %1").arg(groupView.activePanel ? groupView.activePanel.title : "")
                        visible: groupView.activePanel ? groupView.activePanel.closable : false
                        onClicked: if (groupView.activePanel)
                            groupView.activePanel.closeRequested()
                    }
                }
            }
            B.ScrollView {
                anchors.top: titleBar.bottom
                width: parent.width
                height: control.tokens.controlHeight
                contentWidth: tabRow.width
                contentHeight: height
                B.ScrollBar.vertical.policy: B.ScrollBar.AlwaysOff
                B.ScrollBar.horizontal.policy: B.ScrollBar.AsNeeded
                Row {
                    id: tabRow
                    height: control.tokens.controlHeight
                    Repeater {
                        model: groupView.group ? groupView.group.tabs.length : 0
                        delegate: Row {
                            id: tabCell
                            required property int index
                            readonly property string panelId: groupView.group.tabs[index] || ""
                            readonly property CatDockPanel panel: control.registry[panelId] || null
                            opacity: control.dragging && control.dragId === panelId ? 0.45 : 1
                            height: control.tokens.controlHeight
                            DockTabButton {
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
                                DockDragArea {
                                    workspace: control
                                    panelId: tabCell.panelId
                                    focusItem: tabButton
                                    contextMenu: tabMenu
                                }
                            }
                            DockTabButton {
                                theme: control.theme
                                height: parent.height
                                implicitWidth: control.tokens.controlHeight
                                padding: 0
                                tool: true
                                symbol: 2
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
        delegate: B.Control {
            id: handle
            objectName: "dockSplitter_" + index
            required property int index
            property bool pointerFocus: false
            readonly property bool keyboardCue: activeFocus && !pointerFocus
            onActiveFocusChanged: if (!activeFocus)
                pointerFocus = false
            readonly property var splitData: control.geometry.handles[index]
            x: splitData ? splitData.x : 0
            y: splitData ? splitData.y : 0
            width: splitData ? splitData.width : 0
            height: splitData ? splitData.height : 0
            padding: 0
            hoverEnabled: true
            background: Rectangle {
                color: resizeArea.pressed || handle.hovered || handle.keyboardCue ? control.tokens.primary : control.tokens.background
            }
            activeFocusOnTab: !control.layoutLocked
            Accessible.role: Accessible.Slider
            Accessible.name: qsTr("Resize dock panels")
            Rectangle {
                anchors.centerIn: parent
                width: handle.splitData && handle.splitData.horizontal ? 2 : Math.min(24, parent.width)
                height: handle.splitData && handle.splitData.horizontal ? Math.min(24, parent.height) : 2
                radius: 1
                color: handle.keyboardCue || resizeArea.pressed || handle.hovered ? control.tokens.onPrimary : control.tokens.border
            }
            function adjust(delta) {
                pointerFocus = false;
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
                onPressed: {
                    handle.forceActiveFocus(Qt.MouseFocusReason);
                    // Refocusing an already focused control need not change Qt's focus reason.
                    handle.pointerFocus = true;
                }
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
    DockDropOverlay {
        objectName: "dockDropOverlay"
        readonly property var target: control.dropTarget && !control.dropTarget.windowId ? control.dropTarget : null
        visible: control.dragging && target !== null && !target.windowId
        x: target ? target.x : 0
        y: target ? target.y : 0
        width: target ? target.width : 0
        height: target ? target.height : 0
        position: target ? target.position : DockController.Center
        outer: target ? !!target.workspaceEdge : false
        ratio: target && target.workspaceEdge ? target.ratio : 0.5
        guidesVisible: !outer
        tokens: control.tokens
    }
    DockDropOverlay {
        objectName: "dockWorkspaceEdgeGuides"
        anchors.fill: parent
        visible: control.dragging && control.hasDragPoint && control.dragPoint.x >= 0 && control.dragPoint.y >= 0
                 && control.dragPoint.x < control.width && control.dragPoint.y < control.height
                 && control.geometry.groups.some(group => group.tabs.some(id => id !== control.dragId))
        tokens: control.tokens
        outer: true
        guidesOnly: true
        allOuterGuides: true
        position: control.dropTarget && control.dropTarget.workspaceEdge ? control.dropTarget.position : DockController.Center
    }
}
