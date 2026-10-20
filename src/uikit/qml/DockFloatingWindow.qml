// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic as B
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
    function applyRecord(value) {
        applying = true;
        record = value;
        x = value.geometry.x;
        y = value.geometry.y;
        width = value.geometry.width;
        height = value.geometry.height;
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
        color: floating.workspace.tokens.surfaceHover
        CatButton {
            id: backButton
            theme: floating.workspace.theme
            anchors.right: parent.right
            height: parent.height
            text: qsTr("Dock back")
            enabled: !floating.workspace.layoutLocked
            onClicked: floating.workspace.controller.dockBack(floating.record.id)
        }
        B.ScrollView {
            anchors.left: parent.left
            anchors.right: backButton.left
            height: parent.height
            contentWidth: tabs.width
            contentHeight: height
            B.ScrollBar.vertical.policy: B.ScrollBar.AlwaysOff
            Row {
                id: tabs
                height: bar.height
                Repeater {
                    model: floating.record.tabs.length
                    delegate: Row {
                        id: tab
                        required property int index
                        readonly property string panelId: floating.record.tabs[index] || ""
                        readonly property var panel: floating.workspace.registry[panelId]
                        height: bar.height
                        CatButton {
                            theme: floating.workspace.theme
                            height: parent.height
                            text: tab.panel ? tab.panel.title : tab.panelId
                            checkable: true
                            checked: floating.record.active === tab.panelId
                            onClicked: floating.workspace.controller.activatePanel(tab.panelId)
                            Keys.onLeftPressed: if (!floating.workspace.layoutLocked)
                                floating.workspace.controller.moveTab(tab.panelId, Math.max(0, tab.index - 1))
                            Keys.onRightPressed: if (!floating.workspace.layoutLocked)
                                floating.workspace.controller.moveTab(tab.panelId, Math.min(floating.record.tabs.length - 1, tab.index + 1))
                        }
                        CatButton {
                            theme: floating.workspace.theme
                            height: parent.height
                            implicitWidth: floating.workspace.headerHeight
                            padding: 0
                            text: "×"
                            visible: tab.panel ? tab.panel.closable : false
                            Accessible.name: qsTr("Close %1").arg(tab.panel ? tab.panel.title : "")
                            onClicked: if (tab.panel)
                                tab.panel.closeRequested()
                        }
                    }
                }
            }
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
}
