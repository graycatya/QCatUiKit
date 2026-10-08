// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: control
    property Item target
    property alias active: handler.active
    implicitWidth: 32
    implicitHeight: 32
    DragHandler {
        id: handler
        target: control.target
    }
    HoverHandler {
        cursorShape: Qt.SizeAllCursor
    }
}
