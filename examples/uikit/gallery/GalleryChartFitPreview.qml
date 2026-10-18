// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    objectName: "chartFitPreview"
    readonly property alias dataAdapter: fit
    property real noise: 3
    property real multiplier: 2
    property bool bandVisible: true
    property bool constantX: false
    property bool empty: false
    readonly property string exampleCode: Examples.code(22, {noise: noise, multiplier: multiplier,
        bandVisible: bandVisible, constantX: constantX, empty: empty})
    spacing: 12
    function populate() {
        const points = [];
        if (!empty) for (let i = 0; i < 40; ++i)
            points.push({x: constantX ? 1 : i, y: 2 + i * .3 + noise * Math.sin(i * 1.7)});
        observations.setData(points);
    }
    Component.onCompleted: populate()
    onNoiseChanged: populate()
    onConstantXChanged: populate()
    onEmptyChanged: populate()
    CatLabel { text: qsTr("Observation noise") }
    CatSliderField {
        objectName: "fitNoise"; Layout.fillWidth: true
        from: 0; to: 10; stepSize: .1; decimals: 1; value: control.noise
        defaultValue: 3; resetVisible: true; valueToolTipVisible: true
        Accessible.name: qsTr("Observation noise")
        onValueEdited: value => control.noise = value
    }
    CatLabel { text: qsTr("Residual band multiplier") }
    CatSliderField {
        objectName: "fitMultiplier"; Layout.fillWidth: true
        from: 0; to: 4; stepSize: .1; decimals: 1; value: control.multiplier
        defaultValue: 2; resetVisible: true; valueToolTipVisible: true
        Accessible.name: qsTr("Residual band multiplier")
        onValueEdited: value => control.multiplier = value
    }
    Flow {
        Layout.fillWidth: true; spacing: 12
        CatCheckBox { objectName: "fitBand"; text: qsTr("Show residual band"); checked: control.bandVisible; onToggled: control.bandVisible = checked }
        CatCheckBox { objectName: "fitConstantX"; text: qsTr("Identical X values"); checked: control.constantX; onToggled: control.constantX = checked }
        CatCheckBox { objectName: "fitEmpty"; text: qsTr("Empty data"); checked: control.empty; onToggled: control.empty = checked }
    }
    CatChartFit {
        id: fit
        source: CatChartSeries { id: observations; type: CatChartSeries.Scatter; name: qsTr("Observations"); color: Theme.tokens.primary; markerSize: 6 }
        residualMultiplier: control.multiplier
        series.name: qsTr("Linear fit")
        series.color: Theme.tokens.danger
        band.name: qsTr("Residual band")
        band.color: Theme.tokens.danger
        band.visible: control.bandVisible
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap
        text: fit.valid ? qsTr("%1 observations · slope %2 · R² %3 · RMSE %4")
            .arg(fit.sampleCount).arg(fit.slope.toFixed(4)).arg(fit.rSquared.toFixed(4)).arg(fit.rootMeanSquareError.toFixed(4))
            : qsTr("No valid fit")
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
        text: qsTr("Ordinary least squares uses every source point in data coordinates. The band is fitted Y ± multiplier × RMSE; it is not a confidence or prediction interval.")
    }
    CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: fit.error; visible: text.length > 0; color: Theme.tokens.danger }
    CatChart {
        objectName: "fitChart"
        Layout.fillWidth: true; Layout.preferredHeight: 420
        title: qsTr("Observations, fit and residual band")
        series: [fit.band, observations, fit.series]
    }
}
