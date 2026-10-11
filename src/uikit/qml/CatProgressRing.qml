// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Item {
    id: control
    property ThemePalette theme: Theme
    property bool isActive: true
    property bool indeterminate: true
    property real minimum: 0
    property real maximum: 100
    property real value: 0
    property real strokeWidth: 3
    implicitWidth: 32
    implicitHeight: 32
    CatBusyIndicator {
        theme: control.theme
        anchors.fill: parent
        visible: control.indeterminate
        running: control.isActive
    }
    CatCircularProgress {
        theme: control.theme
        anchors.fill: parent
        visible: !control.indeterminate
        from: control.minimum
        to: control.maximum
        value: control.value
        strokeWidth: control.strokeWidth
        showValue: false
    }
}
