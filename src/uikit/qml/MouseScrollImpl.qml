// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick

// Mouse drags must be claimed by the innermost view before a parent Flickable
// steals the gesture. Touch flicking and wheel events stay with the native view.
DragHandler {
    id: handler
    required property Flickable view
    parent: view
    target: null
    enabled: view && view.interactive && (xAxis.enabled || yAxis.enabled)
    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
    acceptedButtons: Qt.LeftButton
    // Respect grabs held by text editors, charts, resize and nested drag handlers.
    grabPermissions: PointerHandler.CanTakeOverFromItems
    xAxis.enabled: view && view.contentWidth + view.leftMargin + view.rightMargin > view.width
                   && view.flickableDirection !== Flickable.VerticalFlick
    yAxis.enabled: view && view.contentHeight + view.topMargin + view.bottomMargin > view.height
                   && view.flickableDirection !== Flickable.HorizontalFlick
    onActiveChanged: if (active) view.cancelFlick()
    onTranslationChanged: delta => {
        if (!active) return;
        if (xAxis.enabled)
            view.contentX = Math.max(view.originX - view.leftMargin,
                                    Math.min(view.originX + view.contentWidth - view.width + view.rightMargin,
                                             view.contentX - delta.x));
        if (yAxis.enabled)
            view.contentY = Math.max(view.originY - view.topMargin,
                                    Math.min(view.originY + view.contentHeight - view.height + view.bottomMargin,
                                             view.contentY - delta.y));
    }
}
