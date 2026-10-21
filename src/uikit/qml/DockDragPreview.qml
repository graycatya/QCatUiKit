// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window

Window {
    id: preview
    required property var workspace
    objectName: "dockDragPreview"
    readonly property var panel: workspace.registry[workspace.dragId]
    flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowTransparentForInput | Qt.WindowDoesNotAcceptFocus
    transientParent: workspace.Window.window
    color: "transparent"
    visible: workspace.ready && workspace.dragging && workspace.hasDragPoint && workspace.visible && workspace.enabled
             && transientParent !== null && transientParent.visible && transientParent.visibility !== Window.Minimized
    x: Math.round(workspace.dragGlobalPoint.x + 20)
    y: Math.round(workspace.dragGlobalPoint.y + 24)
    width: Math.min(360, Math.max(220, panel ? panel.width * 0.55 : 220))
    height: workspace.tokens.controlHeight + 16
            + (panel && panel.width > 0 ? Math.max(100, Math.min(240, (width - 16) * panel.height / panel.width)) : 140)
    Rectangle {
        anchors.fill: parent
        anchors.margins: 3
        radius: 6
        color: preview.workspace.tokens.surface
        border.color: preview.workspace.tokens.primary
        border.width: 2
        clip: true
        Rectangle {
            x: 2
            y: 2
            width: parent.width - 4
            height: preview.workspace.tokens.controlHeight
            color: preview.workspace.tokens.surfaceHover
            Text {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                text: preview.panel ? preview.panel.title : ""
                font.pixelSize: preview.workspace.tokens.fontSize
                color: preview.workspace.tokens.text
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
        Image {
            objectName: "dockDragThumbnail"
            x: 8
            y: preview.workspace.tokens.controlHeight + 8
            width: parent.width - 16
            height: parent.height - y - 8
            source: preview.workspace.dragImageResult ? preview.workspace.dragImageResult.url : ""
            fillMode: Image.PreserveAspectFit
            cache: false
        }
        Text {
            anchors.centerIn: parent
            width: parent.width - 24
            visible: !preview.workspace.dragImageResult
            text: preview.panel ? preview.panel.title : ""
            font.pixelSize: preview.workspace.tokens.fontSize
            color: preview.workspace.tokens.textSecondary
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }
}
