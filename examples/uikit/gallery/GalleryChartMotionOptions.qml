// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    required property CatChart targetChart
    Layout.fillWidth: true
    Layout.minimumWidth: 0
    spacing: 8
    CatCheckBox {
        objectName: "chartAnimations"
        text: qsTr("Chart transitions")
        checked: control.targetChart.animationsEnabled
        onToggled: control.targetChart.animationsEnabled = checked
    }
    CatCheckBox {
        objectName: "chartReducedMotion"
        text: qsTr("Reduced motion (global theme)")
        checked: Theme.reducedMotion
        onToggled: Theme.reducedMotion = checked
    }
    CatLabel {
        Layout.fillWidth: true; Layout.minimumWidth: 0
        Layout.preferredWidth: 0
        text: qsTr("Theme and appearance changes fade in. Panning and incoming data update immediately.")
        wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
    }
}
