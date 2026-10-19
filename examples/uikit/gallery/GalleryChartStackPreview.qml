// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    readonly property alias exportChart: chart
    objectName: "chartStackPreview"
    readonly property alias dataAdapter: stack
    property bool percentage: false
    property bool signedValues: false
    property bool gap: false
    property bool secondVisible: true
    property real bandOpacity: 0.65
    readonly property string exampleCode: Examples.code(20, {percentage: percentage, signed: signedValues, gap: gap, opacity: bandOpacity, secondVisible: secondVisible})
    spacing: 12
    function populate() {
        first.setData([{x:0,y:3}, {x:1,y:5}, {x:2,y:control.signedValues ? -4 : 4}, {x:3,y:6,breakBefore:control.gap}, {x:4,y:3}]);
        second.setData([{x:0,y:2}, {x:1,y:3}, {x:2,y:2}, {x:3,y:control.signedValues ? -3 : 3}, {x:4,y:4}]);
    }
    Component.onCompleted: populate()
    onSignedValuesChanged: populate()
    onGapChanged: populate()
    Flow {
        Layout.fillWidth: true; spacing: 12
        CatCheckBox { objectName: "stackPercentage"; text: qsTr("Percentage stack"); checked: control.percentage; onToggled: control.percentage = checked }
        CatCheckBox { objectName: "stackSigned"; text: qsTr("Positive and negative values"); checked: control.signedValues; onToggled: control.signedValues = checked }
        CatCheckBox { objectName: "stackGap"; text: qsTr("Shared gap"); checked: control.gap; onToggled: control.gap = checked }
        CatCheckBox { objectName: "stackSecond"; text: qsTr("Second series"); checked: control.secondVisible; onToggled: control.secondVisible = checked }
    }
    CatLabel { text: qsTr("Fill opacity") }
    CatSliderField {
        objectName: "stackOpacity"; Layout.fillWidth: true
        from: 0; to: 100; value: control.bandOpacity * 100; decimals: 0; stepSize: 1; suffix: "%"
        defaultValue: 65; resetVisible: true; valueToolTipVisible: true; tickInterval: 25
        Accessible.name: qsTr("Fill opacity")
        onValueEdited: value => control.bandOpacity = value / 100
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap
        text: qsTr("Sources share strictly increasing X values. Positive and negative values stack separately; zero crossings split the bands. Hidden series leave the totals. Hover to inspect the original value and share.")
        color: Theme.tokens.textSecondary
    }
    CatChartStack {
        id: stack
        objectName: "stackAdapter"
        percentage: control.percentage
        CatChartSeries { id: first; name: qsTr("Series A"); color: Theme.tokens.primary; fillOpacity: control.bandOpacity }
        CatChartSeries { id: second; name: qsTr("Series B"); color: Theme.tokens.success; fillOpacity: control.bandOpacity; visible: control.secondVisible; onVisibleChanged: control.secondVisible = visible }
    }
    CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: stack.error; visible: text.length > 0; color: Theme.tokens.danger }
    CatChart {
        id: chart; objectName: "stackChart"
        Layout.fillWidth: true; Layout.preferredHeight: 420
        title: control.percentage ? qsTr("Percentage stacked area") : qsTr("Stacked area")
        series: stack.series
        yAxis.title: control.percentage ? qsTr("Share (%)") : qsTr("Value")
    }
    CatButton { text: qsTr("Reset view"); onClicked: chart.resetView() }
}
