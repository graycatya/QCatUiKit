// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    readonly property string exampleCode: Examples.code(rangeArea ? 15 : 16,
        {curve: curve, tension: tension, dashed: dashed, fillAlpha: areaOptions.fillAlpha})
    objectName: "chartAdvancedPreview"
    required property bool rangeArea
    property bool ready: false
    property int sampleCount: 12
    property int recordCount: 0
    property int boundaryInput: 0
    property bool inputRejected: false
    property bool largePreset: false
    property bool broken: true
    property bool crossed: false
    property bool dashed: false
    property int curve: 0
    property real tension: 0
    spacing: 12
    GalleryChartData { id: rangeData }
    function refresh() {
        inputRejected = false;
        if (largePreset) { rangeData.populateRangeSeries(series, sampleCount, broken, crossed); recordCount = series.count; return; }
        const points = [], lower = [], upper = [];
        for (let i = 0; i < sampleCount; ++i) {
            const low = 2 + Math.sin(i * 0.7);
            const high = crossed ? 2 + 4 * Math.cos(i * 0.55) : 7 + Math.cos(i * 0.5);
            const gap = broken && i > 0 && i % 6 === 0;
            lower.push({x: i, y: low, breakBefore: gap});
            upper.push({x: boundaryInput >= 2 && i > 0 && i < sampleCount - 1 ? i + 0.35 : i, y: high, breakBefore: gap});
            points.push(rangeArea ? {x: i, lower: low, upper: high, breakBefore: gap}
                                 : {x: i, y: high, breakBefore: gap});
        }
        if (rangeArea && boundaryInput > 0) inputRejected = !series.setRangeBoundaries(lower, upper,
            boundaryInput === 3 ? CatChartSeries.LinearAlignment : CatChartSeries.ExactAlignment);
        else if (rangeArea) series.setRangeData(points);
        else series.setData(points);
        recordCount = series.count;
    }
    function restore() {
        largePreset = false; boundaryInput = 0; inputRejected = false; sampleCount = 12; broken = true; crossed = false; dashed = false; curve = 0; tension = 0; areaOptions.reset(); interactionOptions.reset(); strokeOptions.reset();
        refresh(); chart.resetView();
    }
    Component.onCompleted: { ready = true; refresh(); }
    onRangeAreaChanged: { if (ready) restore(); }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.rangeArea
        CatLabel { text: qsTr("Boundary input") }
        CatComboBox {
            objectName: "chartBoundaryInput"
            Layout.fillWidth: true
            enabled: !control.largePreset
            Accessible.name: qsTr("Boundary input")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Paired records"), value: 0}, {text: qsTr("Independent, matching X"), value: 1},
                {text: qsTr("Mismatched X, strict rejection"), value: 2}, {text: qsTr("Mismatched X, linear alignment"), value: 3}]
            currentValue: control.boundaryInput
            onActivated: index => { control.boundaryInput = index; control.refresh(); }
        }
        CatLabel {
            objectName: "chartBoundaryError"
            Layout.fillWidth: true
            visible: control.inputRejected
            text: qsTr("X values do not match. The previous valid data is retained; choose linear alignment to interpolate the shared domain.")
            wrapMode: Text.Wrap
            color: Theme.tokens.danger
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 12
        CatCheckBox {
            objectName: "chartAdvancedBreaks"
            text: control.largePreset ? qsTr("Break every 10,000 records") : qsTr("Break every six records")
            checked: control.broken
            onToggled: { control.broken = checked; control.refresh(); }
        }
        CatCheckBox {
            objectName: "chartAdvancedCrossing"
            visible: control.rangeArea
            text: qsTr("Cross boundaries")
            checked: control.crossed
            onToggled: { control.crossed = checked; control.refresh(); }
        }
        CatCheckBox {
            objectName: "chartAdvancedDashed"
            text: qsTr("Dashed stroke")
            checked: control.dashed
            onToggled: control.dashed = checked
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Curve style") }
        CatComboBox {
            objectName: "chartAdvancedCurve"
            Layout.fillWidth: true
            Accessible.name: qsTr("Curve style")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Straight"), value: 0}, {text: qsTr("Step left"), value: 1},
                    {text: qsTr("Step right"), value: 2}, {text: qsTr("Step center"), value: 3},
                    {text: qsTr("Monotone spline"), value: 4}, {text: qsTr("Catmull-Rom spline"), value: 5},
                    {text: qsTr("Natural cubic spline"), value: 6}]
            currentValue: control.curve
            onActivated: index => control.curve = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.curve === CatChartSeries.CatmullRomSpline
        CatLabel { text: qsTr("Spline tension: %1").arg(control.tension.toFixed(2)) }
        CatSliderField {
            objectName: "chartAdvancedTension"
            Layout.fillWidth: true
            Accessible.name: qsTr("Spline tension")
            from: 0; to: 1; stepSize: 0.01; value: control.tension
            onValueEdited: value => control.tension = value
            decimals: 2; defaultValue: 0; tickInterval: 0.25
            valueToolTipVisible: true; resetVisible: true; inputWidth: 90
            locale: Qt.locale(GalleryLanguage.localeName)
        }
    }
    GalleryChartStrokeOptions {
        id: strokeOptions
        Layout.fillWidth: true
        visible: true
    }
    GalleryChartAreaOptions {
        id: areaOptions
        Layout.fillWidth: true
        visible: control.rangeArea
        defaultAlpha: 0.3
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        CatButton {
            objectName: "chartAdvancedAdd"
            text: qsTr("Add data")
            enabled: !control.largePreset && control.sampleCount < 512
            onClicked: { control.sampleCount = Math.min(512, control.sampleCount + 6); control.refresh(); }
        }
        CatButton {
            objectName: "chartAdvancedRemove"
            text: qsTr("Remove latest")
            enabled: !control.largePreset && control.sampleCount > 0
            onClicked: { control.sampleCount = Math.max(0, control.sampleCount - 6); control.refresh(); }
        }
        CatButton {
            objectName: "chartAdvancedClear"
            text: qsTr("Clear data")
            onClicked: { control.largePreset = false; control.sampleCount = 0; control.refresh(); }
        }
        CatButton {
            objectName: "chartRangeLargePreset"
            visible: control.rangeArea
            text: qsTr("100,000 intervals")
            onClicked: { control.largePreset = true; control.sampleCount = 100000; control.refresh(); chart.resetView(); }
        }
        CatButton { objectName: "chartAdvancedRestore"; text: qsTr("Restore demo"); onClicked: control.restore() }
        CatButton { text: qsTr("Reset view"); onClicked: chart.resetView() }
    }
    CatLabel {
        Layout.fillWidth: true
        text: control.rangeArea
            ? qsTr("Paired boundaries share the curve style and X. Crossing preserves boundary names; gaps interrupt both curves and the fill. Hover to inspect original values.")
            : qsTr("Explicit breaks interrupt every curve mode. Dashes are a visual style; they do not remove records.")
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
    GalleryChartInteractionOptions {
        id: interactionOptions
        Layout.fillWidth: true
        targetChart: chart
    }
    CatChart {
        id: chart
        objectName: "advancedChart"
        Layout.fillWidth: true
        Layout.preferredHeight: 360
        chartType: control.rangeArea ? CatChartSeries.RangeArea : CatChartSeries.Line
        title: control.rangeArea ? qsTr("Range area") : qsTr("Segmented curves")
        xAxis.title: qsTr("Sample index")
        yAxis.title: qsTr("Value")
        xAxis.tickCount: width < 420 ? 3 : 6
        CatChartSeries {
            id: series
            name: control.rangeArea ? qsTr("Value interval") : qsTr("Signal")
            capacity: control.largePreset ? 100000 : 512
            curveMode: control.curve; splineTension: control.tension
            lineCap: strokeOptions.cap; lineJoin: strokeOptions.join
            lineWidth: strokeOptions.strokeWidth; miterLimit: strokeOptions.limit
            strokeStyle: control.dashed ? CatChartSeries.DashLine : CatChartSeries.SolidLine
            dashPattern: [16, 8]
            fillStyle: areaOptions.style; fillOpacity: areaOptions.fillAlpha
            fillGradientStops: areaOptions.gradientStops
            fillTexture: areaOptions.style === 3 ? areaOptions.texture : ""
            fillTextureMode: areaOptions.textureMode; fillTextureSize: Qt.size(areaOptions.tileSize, areaOptions.tileSize)
            fillColor: areaOptions.fillPalette === 0 ? areaOptions.defaultStyle.fillColor : areaOptions.startColor
            fillEndColor: areaOptions.fade ? "transparent" : areaOptions.endColor
            borderColor: areaOptions.outline === 0 ? areaOptions.defaultStyle.borderColor : areaOptions.outlineColor
            markersVisible: !control.rangeArea
        }
        data: CatLabel {
            anchors.centerIn: parent
            visible: control.sampleCount === 0
            text: qsTr("No data. Add data or restore the demo to begin.")
        }
    }
    GalleryChartRangeControls {
        Layout.fillWidth: true
        visible: !control.rangeArea
        targetChart: chart
    }
    CatLabel {
        Layout.fillWidth: true
        text: qsTr("%1 / %2 records").arg(control.recordCount).arg(control.largePreset ? 100000 : 512)
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
}
