// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    objectName: "waterfallPreview"
    readonly property alias dataAdapter: waterfall
    property real initial: 0
    property real cost: -45
    property real barWidth: .8
    property bool resetSubtotal: false
    property bool connectors: true
    property bool labels: true
    readonly property string exampleCode: Examples.code(23, {initial: initial, cost: cost, barWidth: barWidth,
        resetSubtotal: resetSubtotal, connectors: connectors, labels: labels})
    spacing: 12
    CatLabel { text: qsTr("Initial value") }
    CatSliderField {
        objectName: "waterfallInitial"; Layout.fillWidth: true
        from: -50; to: 50; stepSize: 1; decimals: 0; value: control.initial
        defaultValue: 0; resetVisible: true; valueToolTipVisible: true; fillOrigin: 0
        Accessible.name: qsTr("Initial value")
        onValueEdited: value => control.initial = value
    }
    CatLabel { text: qsTr("Cost change") }
    CatSliderField {
        objectName: "waterfallCost"; Layout.fillWidth: true
        from: -180; to: 0; stepSize: 1; decimals: 0; value: control.cost
        defaultValue: -45; resetVisible: true; valueToolTipVisible: true
        Accessible.name: qsTr("Cost change")
        onValueEdited: value => control.cost = value
    }
    CatLabel { text: qsTr("Bar width") }
    CatSliderField {
        objectName: "waterfallWidth"; Layout.fillWidth: true
        from: 20; to: 100; stepSize: 1; decimals: 0; value: control.barWidth * 100; suffix: "%"
        defaultValue: 80; resetVisible: true; valueToolTipVisible: true
        Accessible.name: qsTr("Bar width")
        onValueEdited: value => control.barWidth = value / 100
    }
    Flow {
        Layout.fillWidth: true; spacing: 12
        CatCheckBox { objectName: "waterfallReset"; text: qsTr("Reset after subtotal"); checked: control.resetSubtotal; onToggled: control.resetSubtotal = checked }
        CatCheckBox { objectName: "waterfallConnectors"; text: qsTr("Connectors"); checked: control.connectors; onToggled: control.connectors = checked }
        CatCheckBox { objectName: "waterfallLabels"; text: qsTr("Value labels"); checked: control.labels; onToggled: control.labels = checked }
    }
    CatWaterfall {
        id: waterfall
        initialValue: control.initial
        resetOnSubtotal: control.resetSubtotal
        connectorsVisible: control.connectors
        increaseColor: Theme.tokens.success; decreaseColor: Theme.tokens.danger
        subtotalColor: Theme.tokens.warning; totalColor: Theme.tokens.primary
        series.name: qsTr("Net change")
        series.borderColor: Theme.tokens.textSecondary
        series.barBorderColor: Theme.tokens.border
        series.barBorderWidth: 1
        series.barLabelsVisible: control.labels
        steps: [{value:120,label:qsTr("Sales")}, {value:control.cost,label:qsTr("Costs")},
            {kind:CatWaterfall.Subtotal,label:qsTr("Subtotal")}, {value:-25,label:qsTr("Adjustment")},
            {value:30,label:qsTr("Other")}, {kind:CatWaterfall.Total,label:qsTr("Total")}]
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
        text: qsTr("Changes run from the previous total to the next. Subtotals and totals start at zero. Resetting a subtotal restores the initial value and breaks its outgoing connector.")
    }
    CatLabel { text: qsTr("Final accumulated value: %1").arg(waterfall.finalValue) }
    CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: waterfall.error; visible: text.length > 0; color: Theme.tokens.danger }
    CatChart {
        objectName: "waterfallChart"
        Layout.fillWidth: true; Layout.preferredHeight: 420
        title: qsTr("Waterfall")
        series: [waterfall.series]
        barGroupWidth: control.barWidth
        xAxis.scale: CatChartAxis.Category
        xAxis.categories: waterfall.categories
        xAxis.tickCount: width >= 600 ? 7 : 6
    }
}
