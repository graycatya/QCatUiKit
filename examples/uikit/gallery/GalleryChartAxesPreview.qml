// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    readonly property string exampleCode: Examples.code(18, {independent: independent, extraVisible: extraVisible, logarithmic: logarithmic, reversed: reversed, farSide: farSide})
    objectName: "chartAxesPreview"
    property bool independent: true
    property bool extraVisible: true
    property bool logarithmic: false
    property bool reversed: false
    property bool farSide: true
    property int count: 24
    spacing: 12
    function refresh() {
        const temperatures = [], pressures = [];
        for (let i = 0; i < count; ++i) {
            temperatures.push({x: i, y: 20 + 8 * Math.sin(i * 0.4)});
            pressures.push({x: i / 24, y: 800 + 500 * Math.cos(i * 0.3)});
        }
        temperature.setData(temperatures); pressure.setData(pressures);
    }
    function restore() {
        independent = true; extraVisible = true; logarithmic = false;
        reversed = false; farSide = true; count = 24;
        interaction.reset(); refresh(); chart.resetView();
    }
    Component.onCompleted: refresh()
    Flow {
        Layout.fillWidth: true
        spacing: 12
        CatCheckBox {
            objectName: "chartIndependentAxes"
            text: qsTr("Independent axes for pressure")
            checked: control.independent
            onToggled: control.independent = checked
        }
        CatCheckBox {
            objectName: "chartExtraAxesVisible"
            text: qsTr("Show extra axes")
            checked: control.extraVisible
            onToggled: control.extraVisible = checked
        }
        CatCheckBox {
            objectName: "chartExtraAxesFar"
            text: qsTr("Extra axes at top / right")
            checked: control.farSide
            onToggled: control.farSide = checked
        }
        CatCheckBox {
            objectName: "chartExtraAxesReversed"
            text: qsTr("Reverse extra axes")
            checked: control.reversed
            onToggled: control.reversed = checked
        }
        CatCheckBox {
            objectName: "chartExtraAxisLog"
            text: qsTr("Logarithmic pressure axis")
            checked: control.logarithmic
            onToggled: control.logarithmic = checked
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        CatButton {
            objectName: "chartAxesAdd"
            text: qsTr("Add data")
            enabled: control.count < 96
            onClicked: { control.count += 4; control.refresh(); }
        }
        CatButton {
            objectName: "chartAxesClear"
            text: qsTr("Clear data")
            onClicked: { control.count = 0; control.refresh(); }
        }
        CatButton { objectName: "chartAxesRestore"; text: qsTr("Restore demo"); onClicked: control.restore() }
        CatButton { text: qsTr("Reset view"); onClicked: chart.resetView() }
    }
    CatLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
        text: qsTr("Each series can use its own X and Y scales. Pan, pinch and rectangle zoom update every associated axis; hiding an axis preserves its data mapping.")
    }
    GalleryChartInteractionOptions { id: interaction; Layout.fillWidth: true; targetChart: chart }
    CatChart {
        id: chart
        objectName: "multiAxisChart"
        Layout.fillWidth: true
        Layout.preferredHeight: 440
        title: qsTr("Temperature and pressure")
        xAxis.title: qsTr("Sample")
        yAxis.title: qsTr("Temperature")
        xAxis.tickCount: width < 480 ? 3 : 6
        CatChartSeries {
            id: temperature
            name: qsTr("Temperature")
            curveMode: CatChartSeries.MonotoneSpline
            markersVisible: true
        }
        CatChartSeries {
            id: pressure
            name: qsTr("Pressure")
            curveMode: CatChartSeries.NaturalSpline
            markersVisible: true
            xAxis: control.independent ? normalizedAxis : null
            yAxis: control.independent ? pressureAxis : null
        }
    }
    property CatChartAxis normalizedAxis: CatChartAxis {
        id: normalizedAxis
        orientation: Qt.Horizontal
        title: qsTr("Normalized time")
        visible: control.extraVisible
        side: control.farSide ? CatChartAxis.FarSide : CatChartAxis.NearSide
        reversed: control.reversed
        tickCount: chart.width < 480 ? 3 : 6
    }
    property CatChartAxis pressureAxis: CatChartAxis {
        id: pressureAxis
        title: qsTr("Pressure")
        visible: control.extraVisible
        side: control.farSide ? CatChartAxis.FarSide : CatChartAxis.NearSide
        reversed: control.reversed
        scale: control.logarithmic ? CatChartAxis.Logarithmic : CatChartAxis.Linear
        minorTickCount: control.logarithmic ? 3 : 0
    }
}
