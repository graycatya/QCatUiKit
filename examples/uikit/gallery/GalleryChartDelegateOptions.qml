// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    required property CatChart targetChart
    property bool pointAvailable: false
    property bool barAvailable: false
    property bool labelAvailable: false
    property bool customPoints: false
    property bool customBars: false
    property bool customLabels: false
    property int budget: 48
    function reset() { customPoints = false; customBars = false; customLabels = false; budget = 48; }
    readonly property Component pointComponent: Component {
        Rectangle {
            required property var chartData
            color: chartData.color
            radius: width / 4
            border.color: Theme.tokens.surface
            border.width: 2
            Rectangle { anchors.centerIn: parent; width: parent.width / 3; height: width; radius: width / 2; color: Theme.tokens.surface }
        }
    }
    readonly property Component barComponent: Component {
        Rectangle {
            id: bar
            required property var chartData
            radius: Math.min(6, width / 4, height / 4)
            gradient: Gradient {
                GradientStop { position: 0; color: Qt.lighter(bar.chartData.color, 1.5) }
                GradientStop { position: 1; color: bar.chartData.color }
            }
        }
    }
    readonly property Component labelComponent: Component {
        Rectangle {
            id: label
            required property var chartData
            radius: 4
            color: Theme.tokens.surfaceHover
            border.color: Theme.tokens.border
            Text { anchors.fill: parent; text: label.chartData.text; color: Theme.tokens.text; font.pixelSize: Theme.fontSize; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
        }
    }
    Layout.fillWidth: true
    spacing: 8
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 620 ? 3 : 1
        CatCheckBox { objectName: "chartPointDelegate"; visible: control.pointAvailable; text: qsTr("Custom point delegate"); checked: control.customPoints; onClicked: control.customPoints = checked }
        CatCheckBox { objectName: "chartBarDelegate"; visible: control.barAvailable; text: qsTr("Custom bar delegate"); checked: control.customBars; onClicked: control.customBars = checked }
        CatCheckBox { objectName: "chartLabelDelegate"; visible: control.labelAvailable; text: qsTr("Custom label delegate"); checked: control.customLabels; onClicked: control.customLabels = checked }
    }
    RowLayout {
        Layout.fillWidth: true
        visible: control.customPoints || control.customBars || control.customLabels
        CatLabel { text: qsTr("Delegate budget") }
        CatSpinBox { objectName: "chartDelegateBudget"; Layout.fillWidth: true; from: 0; to: 256; value: control.budget; onValueModified: control.budget = value }
    }
    CatLabel {
        Layout.fillWidth: true
        visible: control.customPoints || control.customBars || control.customLabels
        text: qsTr("Active delegates: %1 / %2. Remaining records use built-in rendering.").arg(control.targetChart.activeDelegateCount).arg(control.budget)
        wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
    }
    CatLabel { Layout.fillWidth: true; visible: text.length > 0; text: control.targetChart.delegateError; color: Theme.tokens.danger; wrapMode: Text.Wrap }
}
