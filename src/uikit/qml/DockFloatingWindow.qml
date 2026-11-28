// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QCatUiKit
import "DockGeometry.js" as Geometry

Window {
    id: floating
    required property var workspace
    property var record: ({
            tabs: [],
            active: ""
        })
    property bool applying: false
    property bool initialized: false
    readonly property alias panelHost: body
    readonly property var minimumSize: Geometry.measure(record, workspace.registry, workspace.headerHeight, 8)
    title: workspace.registry[record.active] ? workspace.registry[record.active].title : ""
    color: workspace.tokens.surface
    minimumWidth: Math.ceil(minimumSize.width)
    minimumHeight: Math.ceil(minimumSize.height)
    transientParent: workspace.Window.window
    flags: Qt.Window
    visible: initialized && workspace.ready && workspace.visible && transientParent !== null && transientParent.visible
    onActiveChanged: if (active && initialized)
        workspace.raiseFloatingHost(record.id)
    function applyRecord(value) {
        applying = true;
        // Unrelated layout changes must not overwrite pending native move/resize events.
        if (!workspace.nativeWaylandDrag && (!initialized || value.geometry.x !== record.geometry.x))
            x = value.geometry.x;
        if (!workspace.nativeWaylandDrag && (!initialized || value.geometry.y !== record.geometry.y))
            y = value.geometry.y;
        if (!initialized || value.geometry.width !== record.geometry.width)
            width = value.geometry.width;
        if (!initialized || value.geometry.height !== record.geometry.height)
            height = value.geometry.height;
        record = value;
        initialized = true;
        applying = false;
        Qt.callLater(saveGeometry);
    }
    function saveGeometry() {
        if (initialized && !applying && workspace.ready)
            workspace.controller.setFloatingGeometry(record.id, x, y, width, height);
    }
    onXChanged: if (!applying)
        Qt.callLater(saveGeometry)
    onYChanged: if (!applying)
        Qt.callLater(saveGeometry)
    onWidthChanged: if (!applying)
        Qt.callLater(saveGeometry)
    onHeightChanged: if (!applying)
        Qt.callLater(saveGeometry)
    onClosing: close => {
        close.accepted = false;
        workspace.requestCloseWindow(record.id);
    }
    Rectangle {
        id: bar
        width: parent.width
        height: floating.workspace.headerHeight
        color: floating.workspace.tokens.background
        Rectangle {
            width: parent.width
            height: floating.workspace.tokens.controlHeight
            color: floating.workspace.tokens.surface
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: backButton.left
                height: parent.height
                text: floating.title
                font.pixelSize: floating.workspace.tokens.fontSize
                font.weight: Font.DemiBold
                color: floating.workspace.tokens.text
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                DockDragArea {
                    windowMoveHandle: true
                    workspace: floating.workspace
                    panelId: floating.record.active
                    focusItem: parent
                }
            }
            DockTabButton {
                id: backButton
                theme: floating.workspace.theme
                anchors.right: parent.right
                height: parent.height
                text: qsTr("Dock back")
                tool: true
                symbol: 3
                enabled: !floating.workspace.layoutLocked
                onClicked: floating.workspace.controller.dockBack(floating.record.id)
            }
        }
        DockTabStrip {
            y: floating.workspace.tokens.controlHeight
            width: parent.width
            workspace: floating.workspace
            record: floating.record
            floatingWindowId: floating.record.id || ""
            leftScrollIcon: floating.workspace.tabScrollLeftIcon
            rightScrollIcon: floating.workspace.tabScrollRightIcon
        }
    }
    Item {
        id: body
        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        onWidthChanged: if (floating.initialized)
            floating.workspace.placeFloatingPanels(floating.record, body)
        onHeightChanged: if (floating.initialized)
            floating.workspace.placeFloatingPanels(floating.record, body)
    }
    DockDropOverlay {
        objectName: "dockFloatingDropPreview"
        anchors.fill: parent
        visible: floating.workspace.dragging && floating.workspace.dropTarget !== null
                 && floating.workspace.dropTarget.windowId === floating.record.id
        tokens: floating.workspace.tokens
        tabsOnly: true
    }
    Rectangle {
        objectName: "dockFloatingTabTarget"
        y: floating.workspace.tokens.controlHeight
        width: parent.width
        height: floating.workspace.tokens.controlHeight
        visible: floating.workspace.dragging && floating.workspace.dropTarget !== null
                 && floating.workspace.dropTarget.windowId === floating.record.id
        color: "transparent"
        border.width: 2
        border.color: floating.workspace.tokens.primary
    }
}
