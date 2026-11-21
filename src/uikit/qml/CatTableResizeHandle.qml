// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QCatUiKit

Rectangle {
    id: handle
    objectName: "catTableResizeHandle"
    property var tableView: null
    property int visualColumn: -1
    readonly property bool rtl: tableView && tableView.LayoutMirroring.enabled
    property real startWidth: 0
    property real startSceneX: 0
    width: tableView && tableView.resizableColumns ? 6 : 0
    visible: width > 0
    z: 2
    x: rtl ? 0 : (parent ? parent.width - width : 0)
    anchors.top: parent ? parent.top : undefined
    anchors.bottom: parent ? parent.bottom : undefined
    color: handleHover.hovered || resizeDrag.active ? Theme.tokens.primary : "transparent"

    HoverHandler {
        id: handleHover
        enabled: handle.tableView && handle.tableView.resizableColumns
        cursorShape: Qt.SplitHCursor
    }

    DragHandler {
        id: resizeDrag
        enabled: handle.tableView && handle.tableView.resizableColumns
        target: null
        xAxis.enabled: true
        yAxis.enabled: false
        grabPermissions: PointerHandler.CanTakeOverFromAnything
        onActiveChanged: {
            if (!active || !handle.tableView)
                return;
            handle.startWidth = handle.tableView.columnWidth(handle.visualColumn);
            handle.startSceneX = centroid.scenePosition.x;
        }
        onTranslationChanged: {
            if (!active || !handle.tableView)
                return;
            const dx = handle.rtl
                    ? centroid.scenePosition.x - handle.startSceneX
                    : translation.x;
            const resizeDelta = handle.rtl ? -dx : dx;
            handle.tableView.setColumnWidth(handle.visualColumn, handle.startWidth + resizeDelta);
        }
    }
}
