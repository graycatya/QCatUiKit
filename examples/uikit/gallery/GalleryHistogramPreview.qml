// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    readonly property alias exportChart: chart
    objectName: "histogramPreview"
    readonly property alias dataAdapter: histogram
    property int bins: 12
    property int normalization: CatHistogram.Count
    property bool customEdges: false
    property bool outliers: false
    property real barGap: 0.04
    readonly property string exampleCode: Examples.code(21, {
        bins: bins, normalization: normalization, customEdges: customEdges, outliers: outliers, barGap: barGap
    })
    spacing: 12
    function populate() {
        const samples = [];
        for (let i = 0; i < 300; ++i) samples.push(5 + 2 * Math.sin(i * .17) + Math.cos(i * .71));
        if (outliers) samples.push(-2, 12);
        histogram.setData(samples);
    }
    Component.onCompleted: populate()
    onOutliersChanged: populate()
    CatLabel { text: qsTr("Bin count") }
    CatSliderField {
        objectName: "histogramBins"; Layout.fillWidth: true
        enabled: !control.customEdges
        from: 2; to: 40; stepSize: 1; decimals: 0; value: control.bins
        defaultValue: 12; resetVisible: true; valueToolTipVisible: true
        Accessible.name: qsTr("Bin count")
        onValueEdited: value => control.bins = Math.round(value)
    }
    CatLabel { text: qsTr("Normalization") }
    CatComboBox {
        objectName: "histogramNormalization"; Layout.fillWidth: true
        model: [qsTr("Count"), qsTr("Frequency"), qsTr("Probability density")]
        currentIndex: control.normalization
        Accessible.name: qsTr("Normalization")
        onActivated: index => control.normalization = index
    }
    Flow {
        Layout.fillWidth: true; spacing: 12
        CatCheckBox { objectName: "histogramCustomEdges"; text: qsTr("Unequal bins: 0, 3, 4, 6, 10"); checked: control.customEdges; onToggled: control.customEdges = checked }
        CatCheckBox { objectName: "histogramOutliers"; text: qsTr("Add outliers"); checked: control.outliers; onToggled: control.outliers = checked }
    }
    CatLabel { text: qsTr("Bar gap") }
    CatSliderField {
        objectName: "histogramGap"; Layout.fillWidth: true
        from: 0; to: 50; stepSize: 1; decimals: 0; value: control.barGap * 100; suffix: "%"
        defaultValue: 4; resetVisible: true; valueToolTipVisible: true; tickInterval: 10
        Accessible.name: qsTr("Bar gap")
        onValueEdited: value => control.barGap = value / 100
    }
    CatHistogram {
        id: histogram; objectName: "histogramAdapter"
        binCount: control.bins
        binEdges: control.customEdges ? [0, 3, 4, 6, 10] : []
        normalization: control.normalization
        series.name: qsTr("Observations")
        series.color: Theme.tokens.primary
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap
        text: qsTr("%1 observations · %2 included · %3 below / %4 above the bin range")
            .arg(histogram.sampleCount).arg(histogram.includedCount).arg(histogram.underflowCount).arg(histogram.overflowCount)
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
        text: qsTr("Bins include the left edge; only the last bin includes the right edge. Frequency sums to 1 and density integrates to 1 over included observations. Hover to inspect bin boundaries and counts.")
    }
    CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: histogram.error; visible: text.length > 0; color: Theme.tokens.danger }
    CatChart {
        id: chart; objectName: "histogramChart"
        Layout.fillWidth: true; Layout.preferredHeight: 420
        title: qsTr("Histogram")
        barSpacing: control.barGap
        series: [histogram.series]
        xAxis.title: qsTr("Observation")
        yAxis.title: control.normalization === CatHistogram.Count ? qsTr("Count")
            : control.normalization === CatHistogram.Frequency ? qsTr("Frequency") : qsTr("Probability density")
    }
    CatButton { text: qsTr("Reset view"); onClicked: chart.resetView() }
}
