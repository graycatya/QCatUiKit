// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    objectName: "chartPreview"
    required property string componentId
    readonly property GalleryChartData dataController: demo
    property int requestedMode: -1
    readonly property var allowedModes: GalleryChartCatalog.entryForId(componentId)?.modes || [0]
    property int mode: 0
    readonly property string exampleCode: (specializedLoader.item as GalleryChartM4Preview)?.exampleCode
        || (specializedLoader.item as GalleryChart3DPreview)?.exampleCode
        || (specializedLoader.item as GalleryWaterfallPreview)?.exampleCode
        || (specializedLoader.item as GalleryChartFitPreview)?.exampleCode
        || (specializedLoader.item as GalleryHistogramPreview)?.exampleCode
        || (specializedLoader.item as GalleryChartStackPreview)?.exampleCode
        || (specializedLoader.item as GalleryChartAxesPreview)?.exampleCode
        || (specializedLoader.item as GalleryChartRadialPreview)?.exampleCode
        || (specializedLoader.item as GalleryChartAdvancedPreview)?.exampleCode
        || Examples.code(mode, {
            curve: curveChoice, tension: tension, dashed: dashed, markers: showMarkers,
            marker: markerChoice, markerSize: markerDiameter, barVariant: demo.barVariant, barLabels: barLabels,
            barWidth: barWidth, barGap: barGap, fillAlpha: areaOptions.fillAlpha,
            baselineMode: areaOptions.baselineMode, baselineValue: areaOptions.baselineValue
        })
    function selectRequestedMode() {
        const modes = GalleryChartCatalog.entryForId(componentId)?.modes || [0];
        const selected = modes.includes(requestedMode) ? requestedMode : modes[0];
        if (mode === selected) { if (ready) loadDemo(); }
        else mode = selected;
    }
    onComponentIdChanged: if (ready) selectRequestedMode()
    onRequestedModeChanged: if (ready && requestedMode >= 0) selectRequestedMode()
    property bool ready: false
    property int curveChoice: 0
    property int markerChoice: 0
    property int markerDiameter: 8
    property bool showMarkers: false
    property real tension: 0
    property bool dashed: false
    property int dashLength: 16
    property int dashGap: 8
    property int dashPhase: 0
    property real barWidth: 0.8
    property real barGap: 0
    property int barBorder: 0
    property int barBorderChoice: 0
    property bool barLabels: false
    property int barPosition: 0
    property int barFormat: 0
    property int barPrecision: 6
    property int barMargin: 4
    property int barRotation: 0
    readonly property string barFormatText: barFormat === 1 ? "@category: @value"
        : barFormat === 2 ? "@series: @value" : barFormat === 3 ? "@value (@percentage%)" : "@value"
    readonly property color barBorderColor: barBorderChoice === 1 ? Theme.tokens.primary
        : barBorderChoice === 2 ? "#ffffff" : barBorderChoice === 3 ? "#000000" : Theme.tokens.text
    readonly property bool percentBars: demo.barVariant === 2 || demo.barVariant === 5
    onPercentBarsChanged: { if (!percentBars && barFormat === 3) barFormat = 0; }
    readonly property bool curveDemo: mode === 0 || mode === 1 || mode === 10 || mode === 11 || mode === 12 || mode === 14
    readonly property bool barDemo: mode === 2 || mode === 3 || mode === 13
    readonly property bool markerDemo: curveDemo || mode === 4 || mode === 5
    function resetVariants() {
        areaOptions.reset(); interactionOptions.reset(); strokeOptions.reset(); delegateOptions.reset();
        barWidth = 0.8; barGap = 0; barBorder = 0; barBorderChoice = 0; barLabels = false; barPosition = 0; barFormat = 0; barPrecision = 6; barMargin = 4; barRotation = 0;
        curveChoice = 0; markerChoice = 0; markerDiameter = 8; showMarkers = false; tension = 0; dashed = false; dashLength = 16; dashGap = 8; dashPhase = 0;
    }
    readonly property bool radial: mode >= 6 && mode <= 8
    spacing: 12

    function formatCount(value: real): string {
        return value.toLocaleString(Qt.locale(GalleryLanguage.localeName), "f", 0);
    }
    function loadDemo() {
        resetVariants();
        if (mode >= 15) { demo.stopStress(); return; }
        demo.load(mode, chart, first, second);
    }
    function toggleStress() {
        if (demo.running)
            demo.stopStress();
        else
            demo.startStress();
    }
    Component.onCompleted: {
        selectRequestedMode();
        ready = true;
        loadDemo();
    }
    onModeChanged: {
        if (ready)
            loadDemo();
    }
    onVisibleChanged: {
        if (!visible)
            demo.stopStress();
    }

    GalleryChartData {
        id: demo
        objectName: "chartDemoData"
    }
    Connections {
        target: GalleryLanguage
        function onLanguageChanged() { demo.retranslate(); }
    }
    Connections {
        target: control.Window.window
        function onVisibilityChanged() {
            if (control.Window.window.visibility === Window.Hidden
                    || control.Window.window.visibility === Window.Minimized)
                demo.stopStress();
        }
    }
    CatComboBox {
        objectName: "chartDemoPicker"
        Layout.fillWidth: true
        textRole: "text"; valueRole: "value"
        visible: control.allowedModes.length > 1
        Accessible.name: qsTr("Chart variant")
        model: control.allowedModes.map(mode => ({text: GalleryChartCatalog.modeNames[mode], value: mode}))
        currentValue: control.mode
        onActivated: index => control.mode = model[index].value
    }
    Loader {
        id: specializedLoader
        Layout.fillWidth: true
        active: control.mode >= 15
        visible: active
        sourceComponent: control.mode >= 24 ? m4Preview : control.mode === 23 ? waterfallPreview : control.mode === 22 ? fitPreview : control.mode === 21 ? histogramPreview : control.mode === 20 ? stackPreview : control.mode === 19 ? threeDPreview : control.mode === 18 ? axesPreview : control.mode === 17 ? radialPreview : advancedPreview
    }
    Component { id: m4Preview; GalleryChartM4Preview { mode: control.mode } }
    Component { id: radialPreview; GalleryChartRadialPreview {} }
    Component { id: waterfallPreview; GalleryWaterfallPreview {} }
    Component { id: fitPreview; GalleryChartFitPreview {} }
    Component { id: histogramPreview; GalleryHistogramPreview {} }
    Component { id: stackPreview; GalleryChartStackPreview {} }
    Component { id: threeDPreview; GalleryChart3DPreview {} }
    Component { id: axesPreview; GalleryChartAxesPreview {} }
    Component { id: advancedPreview; GalleryChartAdvancedPreview { rangeArea: control.mode === 15 } }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.mode < 15
        spacing: 12
        GridLayout {
            Layout.fillWidth: true
            visible: control.curveDemo || control.barDemo || control.markerDemo
            columns: control.width >= 620 ? 3 : 1
            columnSpacing: 12; rowSpacing: 8
            ColumnLayout {
                visible: control.curveDemo
                Layout.fillWidth: true
                CatLabel { text: qsTr("Curve style") }
                CatComboBox {
                    objectName: "chartCurveStyle"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Curve style")
                    textRole: "text"; valueRole: "value"
                    model: [{text: qsTr("Straight"), value: 0}, {text: qsTr("Step left"), value: 1},
                            {text: qsTr("Step right"), value: 2}, {text: qsTr("Step center"), value: 3},
                            {text: qsTr("Monotone spline"), value: 4}, {text: qsTr("Catmull-Rom spline"), value: 5},
                            {text: qsTr("Natural cubic spline"), value: 6}]
                    currentValue: control.curveChoice
                    onActivated: index => control.curveChoice = index
                }
            }
            ColumnLayout {
                visible: control.barDemo
                Layout.fillWidth: true
                CatLabel { text: qsTr("Bar layout") }
                CatComboBox {
                    objectName: "chartBarLayout"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Bar layout")
                    textRole: "text"; valueRole: "value"
                    model: [{text: qsTr("Vertical grouped"), value: 0}, {text: qsTr("Vertical stacked"), value: 1},
                            {text: qsTr("Vertical percent"), value: 2}, {text: qsTr("Horizontal grouped"), value: 3},
                            {text: qsTr("Horizontal stacked"), value: 4}, {text: qsTr("Horizontal percent"), value: 5}]
                    currentValue: demo.barVariant
                    onActivated: index => demo.barVariant = index
                }
            }
            ColumnLayout {
                visible: control.markerDemo
                Layout.fillWidth: true
                CatLabel { text: qsTr("Marker shape") }
                CatComboBox {
                    objectName: "chartMarkerShape"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Marker shape")
                    textRole: "text"; valueRole: "value"
                    model: [{text: qsTr("Circle"), value: 0}, {text: qsTr("Square"), value: 1},
                            {text: qsTr("Triangle"), value: 2}, {text: qsTr("Diamond"), value: 3}, {text: qsTr("Texture"), value: 4}]
                    currentValue: control.markerChoice
                    onActivated: index => control.markerChoice = index
                }
            }
            ColumnLayout {
                visible: control.markerDemo && control.mode !== 5
                Layout.fillWidth: true
                CatLabel { text: qsTr("Marker diameter (px)") }
                CatSliderField {
                    objectName: "chartMarkerSize"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Marker diameter (px)")
                    from: 1; to: 128; value: control.markerDiameter
                    onValueEdited: value => control.markerDiameter = value
                    decimals: 0; stepSize: 1; suffix: " px"; defaultValue: 8; tickValues: [1, 32, 64, 96, 128]
                    valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                    locale: Qt.locale(GalleryLanguage.localeName)
                }
            }
            CatCheckBox {
                objectName: "chartCurveMarkers"
                visible: control.curveDemo
                text: qsTr("Show data markers")
                checked: control.showMarkers
                onToggled: control.showMarkers = checked
            }
            CatCheckBox {
                objectName: "chartDashedStroke"
                visible: control.curveDemo
                text: qsTr("Dashed stroke")
                checked: control.dashed
                onToggled: control.dashed = checked
            }
            Repeater {
                model: [qsTr("Dash length (px)"), qsTr("Dash gap (px)"), qsTr("Dash offset (px)")]
                ColumnLayout {
                    required property int index
                    required property string modelData
                    visible: control.curveDemo && control.dashed
                    Layout.fillWidth: true
                    CatLabel { text: parent.modelData }
                    CatSpinBox {
                        objectName: "chartDashParameter" + parent.index
                        Layout.fillWidth: true
                        Accessible.name: parent.modelData
                        from: parent.index === 2 ? -128 : 1; to: 128
                        value: parent.index === 0 ? control.dashLength : parent.index === 1 ? control.dashGap : control.dashPhase
                        onValueModified: {
                            if (parent.index === 0) control.dashLength = value;
                            else if (parent.index === 1) control.dashGap = value;
                            else control.dashPhase = value;
                        }
                    }
                }
            }
            ColumnLayout {
                visible: control.curveDemo && control.curveChoice === CatChartSeries.CatmullRomSpline
                Layout.fillWidth: true
                CatLabel { text: qsTr("Spline tension: %1").arg(control.tension.toFixed(2)) }
                CatSliderField {
                    objectName: "chartSplineTension"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Spline tension")
                    from: 0; to: 1; stepSize: 0.01; value: control.tension
                    onValueEdited: value => control.tension = value
                    decimals: 2; defaultValue: 0; tickInterval: 0.25
                    valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                    locale: Qt.locale(GalleryLanguage.localeName)
                }
            }
        }
        GalleryChartStrokeOptions {
            id: strokeOptions
            Layout.fillWidth: true
            visible: control.curveDemo
        }
        GalleryChartAreaOptions {
            id: areaOptions
            Layout.fillWidth: true
            visible: control.mode === 1
            baselineEnabled: true
        }
        GridLayout {
            Layout.fillWidth: true
            visible: control.barDemo
            columns: control.width >= 620 ? 3 : 1
            columnSpacing: 12; rowSpacing: 8
            ColumnLayout {
                Layout.fillWidth: true
                CatLabel { text: qsTr("Group width: %1%").arg(Math.round(control.barWidth * 100)) }
                CatSliderField {
                    objectName: "chartBarWidth"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Group width")
                    from: 0; to: 100; stepSize: 1; value: control.barWidth * 100
                    onValueEdited: value => control.barWidth = value / 100
                    decimals: 1; suffix: "%"; defaultValue: 80; tickInterval: 25
                    valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                    locale: Qt.locale(GalleryLanguage.localeName)
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                CatLabel { text: qsTr("Bar spacing: %1%").arg(Math.round(control.barGap * 100)) }
                CatSliderField {
                    objectName: "chartBarSpacing"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Bar spacing")
                    from: 0; to: 100; stepSize: 1; value: control.barGap * 100
                    onValueEdited: value => control.barGap = value / 100
                    decimals: 1; suffix: "%"; defaultValue: 0; tickInterval: 25
                    valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                    locale: Qt.locale(GalleryLanguage.localeName)
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                CatLabel { text: qsTr("Border width (px)") }
                CatSpinBox {
                    objectName: "chartBarBorderWidth"; Layout.fillWidth: true
                    Accessible.name: qsTr("Border width (px)")
                    from: 0; to: 16; value: control.barBorder
                    onValueModified: control.barBorder = value
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                enabled: control.barBorder > 0
                CatLabel { text: qsTr("Border color") }
                CatComboBox {
                    objectName: "chartBarBorderColor"; Layout.fillWidth: true
                    Accessible.name: qsTr("Border color")
                    textRole: "text"; valueRole: "value"
                    model: [{text: qsTr("Theme text"), value: 0}, {text: qsTr("Primary"), value: 1}, {text: qsTr("White"), value: 2}, {text: qsTr("Black"), value: 3}]
                    currentValue: control.barBorderChoice
                    onActivated: index => control.barBorderChoice = index
                }
            }
            CatCheckBox {
                objectName: "chartBarLabels"
                text: qsTr("Show bar labels"); checked: control.barLabels
                onToggled: control.barLabels = checked
            }
            ColumnLayout {
                Layout.fillWidth: true
                enabled: control.barLabels
                CatLabel { text: qsTr("Label position") }
                CatComboBox {
                    objectName: "chartBarLabelPosition"; Layout.fillWidth: true
                    Accessible.name: qsTr("Label position")
                    textRole: "text"; valueRole: "value"
                    model: [{text: qsTr("Center"), value: 0}, {text: qsTr("Inside end"), value: 1}, {text: qsTr("Inside base"), value: 2}, {text: qsTr("Outside end"), value: 3}]
                    currentValue: control.barPosition
                    onActivated: index => control.barPosition = index
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                visible: control.barLabels
                CatLabel { text: qsTr("Label content") }
                CatComboBox {
                    objectName: "chartBarLabelFormat"; Layout.fillWidth: true
                    Accessible.name: qsTr("Label content")
                    textRole: "text"; valueRole: "value"
                    model: {
                        const items = [{text: qsTr("Value"), value: 0}, {text: qsTr("Category and value"), value: 1},
                                       {text: qsTr("Series and value"), value: 2}];
                        if (control.percentBars) items.push({text: qsTr("Value and percentage"), value: 3});
                        return items;
                    }
                    currentValue: control.barFormat
                    onActivated: index => control.barFormat = index
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                visible: control.barLabels
                CatLabel { text: qsTr("Significant digits") }
                CatSpinBox {
                    objectName: "chartBarLabelPrecision"; Layout.fillWidth: true
                    Accessible.name: qsTr("Significant digits")
                    from: 1; to: 15; value: control.barPrecision
                    onValueModified: control.barPrecision = value
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                visible: control.barLabels
                CatLabel { text: qsTr("Label rotation (degrees)") }
                CatSpinBox {
                    objectName: "chartBarLabelRotation"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Label rotation (degrees)")
                    from: -180; to: 180; stepSize: 15; value: control.barRotation
                    onValueModified: control.barRotation = value
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                visible: control.barLabels
                CatLabel { text: qsTr("Label margin (px)") }
                CatSpinBox {
                    objectName: "chartBarLabelMargin"; Layout.fillWidth: true
                    Accessible.name: qsTr("Label margin (px)")
                    from: 0; to: 64; value: control.barMargin
                    onValueModified: control.barMargin = value
                }
            }
        }
        CatLabel {
            Layout.fillWidth: true
            visible: control.barDemo && control.barLabels
            text: qsTr("Labels use source values; percent layouts can also show the signed share. Labels that do not fit are omitted.")
            wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatButton {
                objectName: "chartAddButton"
                text: qsTr("Add data")
                enabled: !demo.running
                onClicked: demo.addData()
            }
            CatButton {
                objectName: "chartRemoveButton"
                text: qsTr("Remove latest")
                enabled: !demo.running && demo.unitCount > 0
                onClicked: demo.removeData()
            }
            CatButton {
                objectName: "chartClearButton"
                text: qsTr("Clear data")
                enabled: !demo.running && demo.unitCount > 0
                onClicked: demo.clearData()
            }
            CatButton {
                objectName: "chartRestoreButton"
                text: qsTr("Restore demo")
                onClicked: { control.resetVariants(); demo.resetData(); }
            }
            CatButton {
                objectName: "chartMillionPreset"
                text: qsTr("Million-sample preset")
                enabled: !demo.running
                onClicked: demo.configureMillionSamples()
            }
            CatButton {
                objectName: "chartStressButton"
                text: demo.running ? qsTr("Stop stress test") : qsTr("Start stress test")
                primary: true
                onClicked: control.toggleStress()
            }
        }
        CatLabel {
            objectName: "chartDataSummary"
            Layout.fillWidth: true
            text: qsTr("%1 / %2 %3 · %4 samples in total")
                .arg(control.formatCount(demo.unitCount)).arg(control.formatCount(demo.capacity))
                .arg(demo.unitName).arg(control.formatCount(demo.sampleCount))
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
        }
        GalleryChartDelegateOptions {
            id: delegateOptions
            targetChart: chart; pointAvailable: control.markerDemo; barAvailable: control.barDemo
            labelAvailable: control.barDemo || control.mode === 6 || control.mode === 7
        }
        GalleryChartInteractionOptions {
            id: interactionOptions
            Layout.fillWidth: true
            visible: !control.radial
            targetChart: chart
        }
        CatChart {
            id: chart
            delegateBudget: delegateOptions.budget
            objectName: "demoChart"
            Layout.fillWidth: true
            Layout.preferredHeight: control.barDemo && demo.barVariant >= 3 && control.barLabels
                ? Math.min(720, Math.max(360, 170 + demo.unitCount
                    * Math.max(44, (Theme.tokens.fontSize + 12 + 2 * control.barBorder)
                        * (demo.barVariant === 3 ? 2 : 1) / Math.max(0.15, control.barWidth * (1 - control.barGap)))))
                : 360
            barGroupWidth: control.barWidth; barSpacing: control.barGap
            xAxis.tickCount: width < 420 ? 3 : 6
            CatChartSeries {
                id: first; objectName: "demoChartFirstSeries"
                curveMode: control.curveChoice; splineTension: control.tension
                markerShape: control.markerChoice; markerSize: control.markerDiameter
                pointDelegate: delegateOptions.customPoints ? delegateOptions.pointComponent : null
                barDelegate: delegateOptions.customBars ? delegateOptions.barComponent : null
                labelDelegate: delegateOptions.customLabels ? delegateOptions.labelComponent : null
                markerTexture: control.markerChoice === CatChartSeries.TextureMarker ? Qt.resolvedUrl("assets/chart-marker.png") : ""
                markersVisible: control.showMarkers
                lineCap: strokeOptions.cap; lineJoin: strokeOptions.join
                lineWidth: strokeOptions.strokeWidth; miterLimit: strokeOptions.limit
                strokeStyle: control.dashed ? CatChartSeries.DashLine : CatChartSeries.SolidLine
                dashPattern: [control.dashLength, control.dashGap]
                dashOffset: control.dashPhase
                areaBaselineMode: areaOptions.baselineMode; areaBaseline: areaOptions.baselineValue
                fillStyle: areaOptions.style; fillOpacity: areaOptions.fillAlpha
                fillGradientStops: areaOptions.gradientStops
                fillTexture: areaOptions.style === 3 ? areaOptions.texture : ""
                fillTextureMode: areaOptions.textureMode; fillTextureSize: Qt.size(areaOptions.tileSize, areaOptions.tileSize)
                fillColor: areaOptions.fillPalette === 0 ? areaOptions.defaultStyle.fillColor : areaOptions.startColor
                fillEndColor: areaOptions.fade ? "transparent" : areaOptions.endColor
                borderColor: areaOptions.outline === 0 ? areaOptions.defaultStyle.borderColor : areaOptions.outlineColor
                barBorderWidth: control.barBorder; barBorderColor: control.barBorderColor
                barLabelsVisible: control.barLabels; barLabelPosition: control.barPosition
                barLabelFormat: control.barFormatText; barLabelPrecision: control.barPrecision
                barLabelMargin: control.barMargin; barLabelRotation: control.barRotation
            }
            CatChartSeries {
                id: second; objectName: "demoChartSecondSeries"
                curveMode: control.curveChoice; splineTension: control.tension
                markerShape: control.markerChoice; markerSize: control.markerDiameter
                pointDelegate: delegateOptions.customPoints ? delegateOptions.pointComponent : null
                barDelegate: delegateOptions.customBars ? delegateOptions.barComponent : null
                labelDelegate: delegateOptions.customLabels ? delegateOptions.labelComponent : null
                markerTexture: control.markerChoice === CatChartSeries.TextureMarker ? Qt.resolvedUrl("assets/chart-marker.png") : ""
                markersVisible: control.showMarkers
                lineCap: strokeOptions.cap; lineJoin: strokeOptions.join
                lineWidth: strokeOptions.strokeWidth; miterLimit: strokeOptions.limit
                strokeStyle: control.dashed ? CatChartSeries.DashLine : CatChartSeries.SolidLine
                dashPattern: [control.dashLength, control.dashGap]
                dashOffset: control.dashPhase
                areaBaselineMode: areaOptions.baselineMode; areaBaseline: areaOptions.baselineValue
                fillStyle: areaOptions.style; fillOpacity: areaOptions.fillAlpha
                fillGradientStops: areaOptions.gradientStops
                fillTexture: areaOptions.style === 3 ? areaOptions.texture : ""
                fillTextureMode: areaOptions.textureMode; fillTextureSize: Qt.size(areaOptions.tileSize, areaOptions.tileSize)
                fillColor: areaOptions.fillPalette === 0 ? areaOptions.defaultStyle.fillColor : areaOptions.startColor
                fillEndColor: areaOptions.fade ? "transparent" : areaOptions.endColor
                borderColor: areaOptions.outline === 0 ? areaOptions.defaultStyle.borderColor : areaOptions.outlineColor
                barBorderWidth: control.barBorder; barBorderColor: control.barBorderColor
                barLabelsVisible: control.barLabels; barLabelPosition: control.barPosition
                barLabelFormat: control.barFormatText; barLabelPrecision: control.barPrecision
                barLabelMargin: control.barMargin; barLabelRotation: control.barRotation
            }
            data: CatLabel {
                anchors.centerIn: parent
                width: parent.width - 40
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                visible: demo.unitCount === 0 || (control.mode === 8 && demo.unitCount < 3)
                text: control.mode === 8 && demo.unitCount > 0
                    ? qsTr("Radar needs at least 3 dimensions. Add more data.")
                    : qsTr("No data. Add data or restore the demo to begin.")
            }
        }
        GalleryChartRangeControls {
            Layout.fillWidth: true
            visible: control.mode === 0
            targetChart: chart
        }
        CatLabel {
            objectName: "chartOverviewSummary"
            Layout.fillWidth: true
            visible: demo.aggregated
            text: qsTr("%1 raw samples · %2 preview samples\n%3")
                .arg(control.formatCount(demo.sampleCount)).arg(control.formatCount(demo.displayedSampleCount))
                .arg(demo.previewDescription)
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatButton {
                text: qsTr("Zoom in")
                enabled: !control.radial
                onClicked: chart.zoomAt(1.5, Qt.point(chart.plotRect().x + chart.plotRect().width / 2,
                                                      chart.plotRect().y + chart.plotRect().height / 2))
            }
            CatButton {
                text: qsTr("Zoom out")
                enabled: !control.radial
                onClicked: chart.zoomAt(1 / 1.5, Qt.point(chart.plotRect().x + chart.plotRect().width / 2,
                                                          chart.plotRect().y + chart.plotRect().height / 2))
            }
            CatButton { text: qsTr("Reset view"); onClicked: chart.resetView() }
            CatCheckBox {
                text: qsTr("Legend")
                checked: true
                onToggled: chart.legendVisible = checked
            }
            CatCheckBox {
                text: qsTr("Grid")
                checked: true
                onToggled: chart.gridVisible = checked
            }
            CatCheckBox {
                text: qsTr("Crosshair")
                checked: true
                enabled: !control.radial
                onToggled: chart.crosshairEnabled = checked
            }
            CatCheckBox {
                text: qsTr("Tooltips")
                checked: true
                onToggled: chart.tooltipsEnabled = checked
            }
        }
        GridLayout {
            Layout.fillWidth: true
            columns: control.width >= 620 ? 3 : 1
            columnSpacing: 16
            rowSpacing: 12
            ColumnLayout {
                Layout.fillWidth: true
                CatLabel { text: qsTr("Batch size"); Layout.fillWidth: true }
                CatSpinBox {
                    objectName: "chartBatchSize"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Batch size")
                    editable: true
                    from: 1; to: demo.maximumBatchSize
                    value: demo.batchSize
                    enabled: !demo.running
                    onValueModified: demo.batchSize = value
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                CatLabel { text: qsTr("Data limit"); Layout.fillWidth: true }
                CatSpinBox {
                    objectName: "chartCapacity"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Data limit")
                    editable: true
                    from: control.mode === 8 ? 3 : 1; to: demo.maximumCapacity
                    stepSize: demo.maximumCapacity >= 10000 ? 1000 : 1
                    value: demo.capacity
                    enabled: !demo.running
                    onValueModified: demo.capacity = value
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                CatLabel { text: qsTr("Update interval (ms)"); Layout.fillWidth: true }
                CatSpinBox {
                    objectName: "chartInterval"
                    Layout.fillWidth: true
                    Accessible.name: qsTr("Update interval (ms)")
                    editable: true
                    from: 16; to: 1000; stepSize: 10
                    value: demo.interval
                    enabled: !demo.running
                    onValueModified: demo.interval = value
                }
            }
        }
        CatLabel {
            Layout.fillWidth: true
            text: qsTr("Batch size and data limit use %1. At the limit, new data replaces the oldest data.")
                .arg(demo.unitName)
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
        }
        CatLabel {
            Layout.fillWidth: true
            text: qsTr("The million-sample preset sets the total raw-data limit, batch size and interval. Start the stress test to fill it gradually. Large datasets use a sampled or grouped overview; all raw records are retained up to the limit.")
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: stressInfo.implicitHeight + 24
            radius: 8
            color: Theme.tokens.background
            border.color: demo.running ? Theme.tokens.primary : Theme.tokens.border
            ColumnLayout {
                id: stressInfo
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 12
                spacing: 6
                CatLabel {
                    objectName: "chartStressStatus"
                    Layout.fillWidth: true
                    font.bold: true
                    text: demo.running ? qsTr("Stress test running — stop at any time")
                        : demo.updates > 0 ? qsTr("Stress test stopped — results retained")
                                          : qsTr("Continuous stress test")
                    wrapMode: Text.Wrap
                }
                CatButton {
                    objectName: "chartStressPanelButton"
                    text: demo.running ? qsTr("Stop stress test") : qsTr("Start stress test")
                    primary: true
                    onClicked: control.toggleStress()
                }
                CatLabel {
                    objectName: "chartStressStatistics"
                    Layout.fillWidth: true
                    text: qsTr("%1 s · %2 updates · %3 updates/s\nTotal added: %4 samples · total removed/replaced: %5 samples\nSlowest data batch: %6 ms")
                        .arg(demo.elapsedSeconds.toFixed(1)).arg(control.formatCount(demo.updates))
                        .arg(demo.updatesPerSecond.toFixed(1)).arg(control.formatCount(demo.added))
                        .arg(control.formatCount(demo.removed))
                        .arg(demo.maximumBatchMs.toFixed(2))
                    wrapMode: Text.Wrap
                }
                CatLabel {
                    Layout.fillWidth: true
                    text: qsTr("Continuously adds 4 batches, then removes 1. Switching charts or leaving this page stops the test. Statistics measure data updates, not rendering FPS.")
                    wrapMode: Text.Wrap
                    color: Theme.tokens.textSecondary
                }
            }
        }
        CatLabel { Layout.fillWidth: true; text: demo.hint; wrapMode: Text.Wrap }
        CatLabel {
            Layout.fillWidth: true
            text: qsTr("Scroll to zoom, drag to pan, click a legend to toggle visibility. Use +/−, arrow keys and Home when focused.")
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
        }
    }
}
