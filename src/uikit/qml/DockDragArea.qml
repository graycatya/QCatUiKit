// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick

MouseArea {
    id: area
    required property var workspace
    required property string panelId
    required property Item focusItem
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
    onPositionChanged: mouse => {
        if (!pressed || !(pressedButtons & Qt.LeftButton))
            return;
        if (nativeHandled)
            return;
        if (!moved && Math.hypot(mouse.x - pressPoint.x, mouse.y - pressPoint.y) >= workspace.dragStyleHints.startDragDistance) {
            moved = true;
            if (workspace.nativeWaylandDrag) {
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
        }
        if (workspace.dragging) {
            if (workspace.nativeWaylandDrag) {
                workspace.updateWaylandLocalDrag(area, mouse.x, mouse.y);
                return;
            }
            const point = mapToGlobal(mouse.x, mouse.y);
            workspace.updateGlobalDrag(point.x, point.y);
        }
    }
    onReleased: mouse => {
        if (mouse.button !== Qt.LeftButton)
            return;
        if (nativeHandled)
            return;
        if (moved) {
            if (workspace.nativeWaylandDrag) {
                workspace.updateWaylandLocalDrag(area, mouse.x, mouse.y);
                workspace.finishDrag();
                return;
            }
            const point = mapToGlobal(mouse.x, mouse.y);
            workspace.updateGlobalDrag(point.x, point.y);
            workspace.finishDrag();
        } else {
            workspace.controller.activatePanel(panelId);
        }
    }
    // QDrag takes over the pointer grab; that cancellation is not user cancel.
    onCanceled: if (!nativeHandled)
        workspace.cancelDrag()
}
