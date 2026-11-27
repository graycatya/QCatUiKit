// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import QCatUiKit

Item {
    id: strip
    required property var workspace
    required property var record
    property string floatingWindowId: ""
    readonly property var tabs: record ? record.tabs : []
    readonly property var tokens: workspace.tokens
    readonly property bool reordering: workspace.reorderStrip === strip && workspace.reorderIndex >= 0
    readonly property int draggedIndex: reordering ? tabs.indexOf(workspace.dragId) : -1
    readonly property Item draggedTab: draggedIndex >= 0 ? cells.itemAt(draggedIndex) : null
    readonly property bool animate: !(workspace.theme || Theme).reducedMotion
    height: tokens.controlHeight
    readonly property bool overflow: row.width > width
    readonly property real arrowWidth: overflow ? Math.min(height, width / 4) : 0
    readonly property real scrollLimit: Math.max(0, row.width - viewport.width)
    readonly property real edgeWidth: Math.min(viewport.width / 3, height / 2)
    property point dragPosition
    readonly property int scrollDirection: {
        if (!reordering || !overflow)
            return 0;
        if (dragPosition.x < arrowWidth + edgeWidth && viewport.contentX > 0)
            return -1;
        if (dragPosition.x >= width - arrowWidth - edgeWidth && viewport.contentX < scrollLimit)
            return 1;
        return 0;
    }

    function scrollBy(delta) {
        const next = Math.max(0, Math.min(scrollLimit, viewport.contentX + delta));
        if (next === viewport.contentX)
            return false;
        viewport.contentX = next;
        return true;
    }

    Timer {
        interval: 16
        repeat: true
        running: strip.scrollDirection !== 0
        onTriggered: {
            strip.scrollBy(strip.scrollDirection * strip.height / 8);
            strip.preview(strip.dragPosition);
        }
    }

    Repeater {
        model: strip.overflow ? [-1, 1] : []
        delegate: CatButton {
            id: scrollButton
            required property int modelData
            objectName: modelData < 0 ? "dockTabsScrollLeft" : "dockTabsScrollRight"
            x: modelData < 0 ? 0 : strip.width - width
            width: strip.arrowWidth
            height: strip.height
            padding: 0
            theme: strip.workspace.theme
            text: modelData < 0 ? qsTr("Scroll tabs left") : qsTr("Scroll tabs right")
            enabled: !strip.workspace.dragging && (modelData < 0 ? viewport.contentX > 0 :
                                                                  viewport.contentX < strip.scrollLimit)
            HoverHandler {
                enabled: scrollButton.enabled && scrollButton.hoverEnabled
                cursorShape: Qt.PointingHandCursor
            }
            background: Rectangle {
                radius: strip.tokens.radius
                color: scrollButton.down ? strip.tokens.primary : scrollButton.hovered
                                          ? strip.tokens.surfaceHover : "transparent"
                border.width: scrollButton.visualFocus ? 1 : 0
                border.color: strip.tokens.focusRing
            }
            contentItem: Item {
                id: arrow
                readonly property color color: !scrollButton.enabled ? strip.tokens.textDisabled :
                                                scrollButton.down ? "white" : strip.tokens.text
                Shape {
                    id: chevron
                    anchors.centerIn: parent
                    width: Math.min(5, arrow.width / 2)
                    height: Math.min(8, arrow.height / 2)
                    ShapePath {
                        strokeColor: arrow.color
                        strokeWidth: 1.5
                        capStyle: ShapePath.RoundCap
                        joinStyle: ShapePath.RoundJoin
                        fillColor: "transparent"
                        startX: scrollButton.modelData < 0 ? chevron.width : 0
                        startY: 0
                        PathLine {
                            x: scrollButton.modelData < 0 ? 0 : chevron.width
                            y: chevron.height / 2
                        }
                        PathLine { x: scrollButton.modelData < 0 ? chevron.width : 0; y: chevron.height }
                    }
                }
            }
            onClicked: {
                viewport.cancelFlick();
                strip.scrollBy(modelData * Math.max(strip.height, viewport.width / 2));
            }
        }
    }

    function preview(point) {
        const margin = workspace.dragStyleHints.startDragDistance;
        if (point.x < -margin || point.x >= width + margin
                || point.y < -margin || point.y >= height + margin)
            return false;
        const source = tabs.indexOf(workspace.dragId);
        const cell = cells.itemAt(source);
        if (!cell)
            return false;
        dragPosition = point;
        const pointerLeft = point.x - viewport.x - workspace.reorderGrabX;
        const visibleLeft = overflow ? Math.max(0, Math.min(viewport.width - cell.width, pointerLeft)) : pointerLeft;
        const lastLeft = row.width - cell.width;
        const visualLeft = Math.max(0, Math.min(lastLeft, viewport.contentX + visibleLeft));
        // A tab wider than the viewport can still reach the last insertion slot.
        const atEnd = overflow && viewport.contentX >= scrollLimit && point.x >= width - arrowWidth - edgeWidth;
        const left = atEnd ? lastLeft : visualLeft;
        const offset = left - cell.x;
        // Compare the leading edge of the moving tab with each neighbor's center.
        // Using the travel edge in each direction also works with mixed tab widths.
        let target = source;
        if (offset < 0) {
            while (target > 0) {
                const neighbor = cells.itemAt(target - 1);
                if (left > neighbor.x + neighbor.width / 2)
                    break;
                --target;
            }
        } else {
            while (target + 1 < tabs.length) {
                const neighbor = cells.itemAt(target + 1);
                if (left + cell.width < neighbor.x + neighbor.width / 2)
                    break;
                ++target;
            }
        }
        workspace.reorderOffset = visualLeft - cell.x;
        workspace.reorderIndex = target;
        workspace.dropTarget = null;
        workspace.floatDropPoint = null;
        workspace.hasDragPoint = false;
        return true;
    }

    Flickable {
        id: viewport
        x: strip.arrowWidth
        width: strip.width - 2 * x
        height: strip.height
        contentWidth: row.width
        contentHeight: height
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        WheelHandler {
            enabled: !strip.workspace.dragging
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: event => {
                const delta = event.pixelDelta.x || event.pixelDelta.y
                              || (event.angleDelta.x || event.angleDelta.y) / 120 * strip.tokens.controlHeight;
                event.accepted = strip.scrollBy(-delta);
            }
        }

        Row {
            id: row
            height: strip.height
            Repeater {
                id: cells
                model: strip.tabs.length
                delegate: Item {
                    id: cell
                    required property int index
                    readonly property string panelId: strip.tabs[index] || ""
                    readonly property CatDockPanel panel: strip.workspace.registry[panelId] || null
                    readonly property bool selected: strip.record.active === panelId
                    readonly property bool lifted: strip.draggedTab === cell
                    readonly property real siblingOffset: {
                        const dragged = strip.draggedTab;
                        if (!dragged || lifted)
                            return 0;
                        const target = strip.workspace.reorderIndex;
                        if (strip.draggedIndex < index && index <= target)
                            return -dragged.width;
                        if (target <= index && index < strip.draggedIndex)
                            return dragged.width;
                        return 0;
                    }
                    width: tabButton.implicitWidth + (closeButton.visible ? closeButton.width : 0)
                    height: row.height
                    z: lifted ? 1 : 0
                    opacity: !lifted && strip.workspace.dragId === panelId ? 0.45 : 1

                    transform: [
                        Translate { x: cell.lifted ? strip.workspace.reorderOffset : 0 },
                        Translate {
                            x: cell.siblingOffset
                            Behavior on x {
                                enabled: strip.animate && strip.reordering
                                NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
                            }
                        }
                    ]

                    Rectangle {
                        anchors.fill: parent
                        topLeftRadius: 6
                        topRightRadius: 6
                        color: cell.selected ? strip.tokens.surface :
                               cell.lifted || tabDrag.containsMouse || closeButton.hovered ? strip.tokens.surfaceHover :
                               "transparent"
                        border.width: tabButton.visualFocus ? 1 : 0
                        border.color: strip.tokens.focusRing
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 2
                            visible: cell.selected
                            color: strip.tokens.primary
                        }
                    }

                    DockTabButton {
                        id: tabButton
                        objectName: "dockTab_" + cell.panelId
                        theme: strip.workspace.theme
                        width: implicitWidth
                        height: parent.height
                        text: cell.panel ? cell.panel.title : cell.panelId
                        checkable: true
                        checked: cell.selected
                        leftPadding: 12
                        rightPadding: closeButton.visible ? 4 : 12
                        background: null
                        onClicked: strip.workspace.controller.activatePanel(cell.panelId)
                        Keys.onLeftPressed: {
                            if (!strip.workspace.layoutLocked && cell.panel && cell.panel.movable)
                                strip.workspace.controller.moveTab(cell.panelId, Math.max(0, cell.index - 1));
                        }
                        Keys.onRightPressed: {
                            if (!strip.workspace.layoutLocked && cell.panel && cell.panel.movable)
                                strip.workspace.controller.moveTab(cell.panelId, Math.min(strip.tabs.length - 1, cell.index + 1));
                        }
                        Keys.onMenuPressed: if (!strip.floatingWindowId)
                            tabMenu.popup()
                        CatMenu {
                            id: tabMenu
                            theme: strip.workspace.theme
                            CatMenuItem {
                                theme: strip.workspace.theme
                                text: qsTr("Float panel")
                                enabled: !strip.workspace.layoutLocked && cell.panel && cell.panel.movable && cell.panel.floatable
                                onTriggered: strip.workspace.floatPanel(cell.panelId)
                            }
                            CatMenuItem {
                                theme: strip.workspace.theme
                                text: qsTr("Float tab group")
                                enabled: !strip.workspace.layoutLocked && strip.tabs.every(id => {
                                    const panel = strip.workspace.registry[id];
                                    return panel && panel.movable && panel.floatable;
                                })
                                onTriggered: strip.workspace.floatPanel(cell.panelId, true)
                            }
                        }
                        DockDragArea {
                            id: tabDrag
                            objectName: strip.floatingWindowId ? "dockFloatingTabDrag_" + cell.panelId : ""
                            workspace: strip.workspace
                            panelId: cell.panelId
                            focusItem: tabButton
                            contextMenu: strip.floatingWindowId ? null : tabMenu
                            tabStrip: strip
                            hoverEnabled: strip.tokens.hoverEnabled
                        }
                    }
                    DockTabButton {
                        id: closeButton
                        objectName: "dockTabClose_" + cell.panelId
                        theme: strip.workspace.theme
                        width: 28
                        height: parent.height
                        anchors.right: parent.right
                        padding: 0
                        tool: true
                        symbol: 2
                        visible: !!cell.panel && cell.panel.closable
                        text: qsTr("Close %1").arg(cell.panel ? cell.panel.title : "")
                        onClicked: if (cell.panel)
                            cell.panel.closeRequested()
                    }
                }
            }
        }
    }
}
