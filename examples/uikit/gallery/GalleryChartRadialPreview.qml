// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    readonly property alias exportChart: chart
    readonly property string exampleCode: Examples.code(17, {variant: variant, start: start, sweep: sweep, hole: hole, gap: gap})
    objectName: "chartRadialPreview"
    property int variant: 2
    property int start: 0
    property int sweep: 300
    property real radius: 0.84
    property real hole: 0.28
    property int gap: 8
    property int separation: 16
    property bool bordered: true
    property bool labels: true
    property int labelPosition: CatChart.SliceLabelOutside
    property int sampleCount: 4
    spacing: 12
    function refresh(preserveVisibility = false) {
        const masks = [];
        if (preserveVisibility)
            for (let si = 0; si < 3; ++si) {
                const visible = [];
                for (let i = 0; i < sampleCount; ++i) visible.push(chart.isSliceVisible(si, i));
                masks.push(visible);
            }
        const a = [], b = [], c = [];
        for (let i = 0; i < sampleCount; ++i) {
            const label = qsTr("Slice %1").arg(i + 1);
            a.push({x: i, y: 12 + i * 3, label: label});
            b.push({x: i, y: 22 - i % 4 * 4, label: label});
            c.push({x: i, y: 7 + i % 3 * 5, label: label});
        }
        outer.setData(a); middle.setData(b); inner.setData(c);
        styleSlices();
        if (preserveVisibility)
            for (let si = 0; si < 3; ++si)
                for (let i = 0; i < sampleCount; ++i) chart.setSliceVisible(si, i, masks[si][i]);
    }
    function styleSlices() {
        const colors = [Theme.tokens.primary, Theme.tokens.success, Theme.tokens.warning, Theme.tokens.danger];
        for (const series of [outer, middle, inner]) {
            for (let i = 0; i < series.count; ++i)
                series.setSliceStyle(i, {color: colors[i % 4], borderColor: Theme.tokens.surface,
                    borderWidth: bordered ? 2 : 0, explodeDistance: i === 0 ? separation : 0});
        }
    }
    function restore() {
        delegateOptions.reset(); chart.animationsEnabled = false; chart.clearSelection(); chart.selectionMode = CatChart.NoSelection;
        variant = 2; start = 0; sweep = 300; radius = 0.84; hole = 0.28;
        gap = 8; separation = 16; bordered = true; labels = true; sampleCount = 4;
        labelPosition = CatChart.SliceLabelOutside;
        refresh();
    }
    function updateAngles() { chart.setPieAngles(start, start + sweep); }
    onStartChanged: updateAngles()
    onSweepChanged: updateAngles()
    onSeparationChanged: styleSlices()
    onBorderedChanged: styleSlices()
    Component.onCompleted: { refresh(); updateAngles(); }
    Connections {
        target: Theme
        function onTokensChanged() { control.styleSlices(); }
    }
    Connections {
        target: GalleryLanguage
        function onLanguageChanged() { control.refresh(true); }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 900 ? 3 : control.width >= 620 ? 2 : 1
        columnSpacing: 12; rowSpacing: 8
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Radial layout") }
            CatComboBox {
                objectName: "chartRadialVariant"
                Layout.fillWidth: true
                Accessible.name: qsTr("Radial layout")
                model: [qsTr("Pie"), qsTr("Doughnut"), qsTr("Two rings"), qsTr("Three rings")]
                currentIndex: control.variant
                onActivated: index => control.variant = index
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Start angle") }
            CatSliderField {
                objectName: "chartRadialStart"
                Layout.fillWidth: true
                Accessible.name: qsTr("Start angle")
                from: -360; to: 360; stepSize: 15; value: control.start;                 onValueEdited: value => control.start = value
                decimals: 0; suffix: "°"; defaultValue: 0; fillOrigin: 0; defaultIndicatorVisible: true; tickInterval: 180
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Sweep angle") }
            CatSliderField {
                objectName: "chartRadialSweep"
                Layout.fillWidth: true
                Accessible.name: qsTr("Sweep angle")
                from: 0; to: 360; stepSize: 15; value: control.sweep;                 onValueEdited: value => control.sweep = value
                decimals: 0; suffix: "°"; defaultValue: 300; tickInterval: 90
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Separation (px)") }
            CatSpinBox {
                objectName: "chartRadialSeparation"
                Layout.fillWidth: true
                Accessible.name: qsTr("Separation (px)")
                from: 0; to: 40; value: control.separation
                onValueModified: control.separation = value
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Radius: %1%").arg(Math.round(control.radius * 100)) }
            CatSliderField {
                objectName: "chartRadialRadius"
                Layout.fillWidth: true
                Accessible.name: qsTr("Radius")
                from: 20; to: 100; stepSize: 1; value: control.radius * 100
                onValueEdited: value => control.radius = value / 100
                decimals: 1; suffix: "%"; defaultValue: 84; tickInterval: 20
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            enabled: control.variant !== 0
            CatLabel { text: qsTr("Hole: %1%").arg(Math.round(control.hole * 100)) }
            CatSliderField {
                objectName: "chartRadialHole"
                Layout.fillWidth: true
                Accessible.name: qsTr("Hole size")
                from: 0; to: 85; stepSize: 1; value: control.hole * 100
                onValueEdited: value => control.hole = value / 100
                decimals: 1; suffix: "%"; defaultValue: 28; tickValues: [0, 25, 50, 75, 85]
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            enabled: control.variant >= 2
            CatLabel { text: qsTr("Ring gap (px)") }
            CatSpinBox {
                objectName: "chartRadialGap"
                Layout.fillWidth: true
                Accessible.name: qsTr("Ring gap (px)")
                from: 0; to: 32; value: control.gap
                onValueModified: control.gap = value
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            enabled: control.labels
            CatLabel { text: qsTr("Label position") }
            CatComboBox {
                objectName: "chartRadialLabelPosition"
                Layout.fillWidth: true
                Accessible.name: qsTr("Label position")
                textRole: "text"; valueRole: "value"
                model: [
                    {text: qsTr("Inside slices"), value: CatChart.SliceLabelInside},
                    {text: qsTr("Outside with leaders"), value: CatChart.SliceLabelOutside}
                ]
                currentValue: control.labelPosition
                onActivated: index => control.labelPosition = valueAt(index)
            }
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        CatCheckBox {
            objectName: "chartRadialBorder"
            text: qsTr("Slice borders"); checked: control.bordered
            onToggled: control.bordered = checked
        }
        CatCheckBox {
            objectName: "chartRadialLabels"
            text: qsTr("Slice labels"); checked: control.labels
            onToggled: control.labels = checked
        }
        CatButton {
            objectName: "chartRadialAdd"
            text: qsTr("Add slice"); enabled: control.sampleCount < 12
            onClicked: { ++control.sampleCount; control.refresh(); }
        }
        CatButton {
            objectName: "chartRadialRemove"
            text: qsTr("Remove latest"); enabled: control.sampleCount > 0
            onClicked: { --control.sampleCount; control.refresh(); }
        }
        CatButton {
            objectName: "chartRadialClear"
            text: qsTr("Clear data")
            onClicked: { control.sampleCount = 0; control.refresh(); }
        }
        CatButton { objectName: "chartRadialRestore"; text: qsTr("Restore demo"); onClicked: control.restore() }
    }
    CatLabel {
        Layout.fillWidth: true
        text: qsTr("Each ring has its own proportions. Outside labels keep percentages and shorten long names; crowded labels favor larger slices. Click legend entries to hide slices.")
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
    RowLayout {
        Layout.fillWidth: true
        CatComboBox {
            objectName: "chartRadialSelectionMode"; Layout.fillWidth: true
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("No selection"), value: CatChart.NoSelection},
                    {text: qsTr("Single selection"), value: CatChart.SingleSelection},
                    {text: qsTr("Multiple selection"), value: CatChart.MultiSelection}]
            currentValue: chart.selectionMode; onActivated: index => chart.selectionMode = index
        }
        CatButton { text: qsTr("Clear selection (%1)").arg(chart.selectedPoints.length); enabled: chart.selectedPoints.length > 0; onClicked: chart.clearSelection() }
    }
    GalleryChartMotionOptions { targetChart: chart }
    GalleryChartDelegateOptions { id: delegateOptions; targetChart: chart; labelAvailable: true }
    CatChart {
        id: chart
        delegateBudget: delegateOptions.budget
        objectName: "radialChart"
        Layout.fillWidth: true
        Layout.preferredHeight: control.width < 500 ? 440 : 500
        chartType: control.variant === 0 ? CatChartSeries.Pie : CatChartSeries.Doughnut
        multiRing: control.variant >= 2
        pieRadius: control.radius
        holeSize: control.hole
        ringGap: control.gap
        sliceLabelsVisible: control.labels
        sliceLabelPosition: control.labelPosition
        title: control.variant >= 2 ? qsTr("Concentric rings") : control.variant === 1 ? qsTr("Doughnut") : qsTr("Pie")
        CatChartSeries { id: outer; name: qsTr("Outer"); capacity: 12; labelDelegate: delegateOptions.customLabels ? delegateOptions.labelComponent : null }
        CatChartSeries { id: middle; name: qsTr("Middle"); capacity: 12; labelDelegate: delegateOptions.customLabels ? delegateOptions.labelComponent : null }
        CatChartSeries { id: inner; name: qsTr("Inner"); capacity: 12; labelDelegate: delegateOptions.customLabels ? delegateOptions.labelComponent : null; visible: control.variant === 3 }
        data: CatLabel {
            anchors.centerIn: parent
            visible: control.sampleCount === 0
            text: qsTr("No data. Add a slice or restore the demo.")
        }
    }
}
