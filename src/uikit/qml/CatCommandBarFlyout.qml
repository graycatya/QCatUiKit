// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatFlyout {
    id: control
    property var actions: []
    contentItem: Column {
        spacing: 4
        Repeater {
            model: control.actions
            CatButton {
                required property var modelData
                theme: control.theme
                width: parent.width
                action: modelData
                flat: true
                onClicked: control.close()
            }
        }
    }
}
