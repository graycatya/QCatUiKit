// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window

MouseArea {
    id: area
    required property var workspace
    required property string panelId
    required property Item focusItem
    // DockTabStrip creates this component; avoid a cyclic QML type dependency.
    property var tabStrip: null
    property var contextMenu: null
    property point pressPoint
    property bool moved: false
    property bool windowMoveHandle: false
    property bool nativeHandled: false
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    preventStealing: true
    cursorShape: !workspace.layoutLocked && workspace.registry[panelId] && workspace.registry[panelId].movable
                 ? workspace.dragging ? Qt.ClosedHandCursor : Qt.OpenHandCursor : Qt.ArrowCursor
    onPressed: mouse => {
        moved = false;
        nativeHandled = false;
        if (mouse.button === Qt.RightButton) {
            if (contextMenu)
                contextMenu.popup();
            return;
        }
        pressPoint = Qt.point(mouse.x, mouse.y);
        workspace.headerFocusItem = focusItem;
        workspace.headerFocusId = panelId;
        focusItem.forceActiveFocus();
    }
    function updateDragPosition(x, y, releasing) {
        if (tabStrip && tabStrip.preview(mapToItem(tabStrip, x, y)))
            return;
        // Clearing the preview moves the source item. Capture the pointer first.
        if (!workspace.nativeWaylandDrag) {
            const point = mapToGlobal(x, y);
            workspace.reorderIndex = -1;
            workspace.updateGlobalDrag(point.x, point.y);
        } else if (tabStrip) {
            const windowItem = area.Window.window.contentItem;
            const point = mapToItem(windowItem, x, y);
            workspace.reorderIndex = -1;
            if (!workspace.nativeToplevelDrag && !tabStrip.floatingWindowId) {
                workspace.updateWaylandLocalDrag(windowItem, point.x, point.y);
            } else if (point.x < 0 || point.y < 0 || point.x >= windowItem.width || point.y >= windowItem.height) {
                if (!releasing) {
                    nativeHandled = true;
                    workspace.beginWaylandDrag(panelId, area);
                } else {
                    workspace.cancelDrag();
                }
            } else {
                workspace.updateWaylandDrag(tabStrip.floatingWindowId, point);
            }
        } else {
            workspace.updateWaylandLocalDrag(area, x, y);
        }
    }
    onPositionChanged: mouse => {
        if (!pressed || !(pressedButtons & Qt.LeftButton))
            return;
        if (nativeHandled)
            return;
        if (!moved && Math.hypot(mouse.x - pressPoint.x, mouse.y - pressPoint.y) >= workspace.dragStyleHints.startDragDistance) {
            moved = true;
            if (!tabStrip && workspace.nativeWaylandDrag) {
                if (windowMoveHandle && !workspace.nativeToplevelDrag) {
                    nativeHandled = workspace.moveWaylandWindow(area, panelId);
                    if (nativeHandled)
                        return;
                }
                if (workspace.nativeToplevelDrag || workspace.controller.floatingWindowForPanel(panelId)) {
                    nativeHandled = true;
                    workspace.beginWaylandDrag(panelId, area);
                    return;
                }
            }
            workspace.beginDrag(panelId, false);
            if (tabStrip && workspace.dragId === panelId) {
                workspace.reorderStrip = tabStrip;
                workspace.reorderGrabX = pressPoint.x;
            }
        }
        if (workspace.dragId === panelId)
            updateDragPosition(mouse.x, mouse.y, false);
    }
    onReleased: mouse => {
        if (mouse.button !== Qt.LeftButton || nativeHandled)
            return;
        if (!moved) {
            workspace.controller.activatePanel(panelId);
        } else if (workspace.dragId === panelId) {
            updateDragPosition(mouse.x, mouse.y, true);
            workspace.finishDrag();
        }
    }
    // QDrag takes over the pointer grab; that cancellation is not user cancel.
    onCanceled: if (!nativeHandled)
        workspace.cancelDrag()
}
