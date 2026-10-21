// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Item {
    id: overlay
    required property var tokens
    property int position: DockController.Center
    property bool tabsOnly: false
    property bool outer: false
    property bool guidesOnly: false
    property bool allOuterGuides: false
    property bool guidesVisible: true
    property real ratio: 0.5
    readonly property color accent: tokens.primary
    readonly property real unit: Math.min(tokens.controlHeight, Math.max(18, Math.min(width, height) / 4))
    Rectangle {
        visible: !overlay.guidesOnly
        x: overlay.position === DockController.Right ? parent.width * overlay.ratio : 0
        y: overlay.position === DockController.Bottom ? parent.height * overlay.ratio : 0
        width: parent.width * (overlay.position === DockController.Left ? overlay.ratio : overlay.position === DockController.Right ? 1 - overlay.ratio : 1)
        height: parent.height * (overlay.position === DockController.Top ? overlay.ratio : overlay.position === DockController.Bottom ? 1 - overlay.ratio : 1)
        color: Qt.rgba(overlay.accent.r, overlay.accent.g, overlay.accent.b, 0.16)
        border.width: 2
        border.color: overlay.tokens.primary
        radius: 3
    }
    Repeater {
        model: !overlay.guidesVisible ? [] : overlay.tabsOnly ? [DockController.Center]
               : overlay.outer ? overlay.allOuterGuides ? [DockController.Left, DockController.Right, DockController.Top, DockController.Bottom] : [overlay.position]
               : [DockController.Center, DockController.Left, DockController.Right, DockController.Top, DockController.Bottom]
        delegate: Rectangle {
            id: guide
            required property int modelData
            readonly property bool selected: modelData === overlay.position
            x: overlay.outer ? (modelData === DockController.Left ? 8 : modelData === DockController.Right ? overlay.width - width - 8 : (overlay.width - width) / 2)
                             : (overlay.width - width) / 2 + (modelData === DockController.Left ? -overlay.unit - 4 : modelData === DockController.Right ? overlay.unit + 4 : 0)
            y: overlay.outer ? (modelData === DockController.Top ? 8 : modelData === DockController.Bottom ? overlay.height - height - 8 : (overlay.height - height) / 2)
                             : (overlay.height - height) / 2 + (modelData === DockController.Top ? -overlay.unit - 4 : modelData === DockController.Bottom ? overlay.unit + 4 : 0)
            width: overlay.unit
            height: overlay.unit
            radius: 4
            color: selected ? overlay.tokens.primary : overlay.tokens.surface
            border.color: selected ? overlay.tokens.primary : overlay.tokens.border
            border.width: 1
            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.56
                height: parent.height * 0.56
                radius: 1
                color: "transparent"
                border.width: 1
                border.color: guide.selected ? overlay.tokens.onPrimary : overlay.tokens.textSecondary
                Rectangle {
                    x: guide.modelData === DockController.Right ? parent.width / 2 : 0
                    y: guide.modelData === DockController.Bottom ? parent.height / 2 : 0
                    width: parent.width / (guide.modelData === DockController.Left || guide.modelData === DockController.Right ? 2 : 1)
                    height: parent.height / (guide.modelData === DockController.Top || guide.modelData === DockController.Bottom ? 2 : 1)
                    color: guide.selected ? overlay.tokens.onPrimary : overlay.tokens.primary
                    opacity: 0.75
                }
            }
        }
    }
}
