// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "chartPreview"
    required property string componentId
    readonly property GalleryChartData dataController: demo
    property int mode: componentId === "chart-axis" ? 11 : 0
    property bool ready: false
    readonly property bool radial: mode >= 6 && mode <= 8
    readonly property var modes: [qsTr("Line"), qsTr("Area"), qsTr("Grouped bars"), qsTr(
            "Stacked bars"), qsTr("Scatter"), qsTr("Bubble"), qsTr("Pie"), qsTr("Doughnut"), qsTr(
            "Radar"), qsTr("Heatmap"), qsTr("Bar + line"), qsTr("Time axis"), qsTr("Log axis"), qsTr(
            "Positive / negative"), qsTr("Scatter + trend")]
    spacing: 12

    function formatCount(value: real): string {
        return value.toLocaleString(Qt.locale(GalleryLanguage.localeName), "f", 0);
    }
    function loadDemo() {
        demo.load(mode, chart, first, second);
    }
    function toggleStress() {
        if (demo.running)
            demo.stopStress();
        else
            demo.startStress();
    }
    Component.onCompleted: {
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
        model: control.modes
        currentIndex: control.mode
        onActivated: index => control.mode = index
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
            onClicked: demo.resetData()
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
    CatChart {
        id: chart
        objectName: "demoChart"
        Layout.fillWidth: true
        Layout.preferredHeight: 360
        xAxis.tickCount: width < 420 ? 3 : 6
        CatChartSeries { id: first; objectName: "demoChartFirstSeries" }
        CatChartSeries { id: second; objectName: "demoChartSecondSeries" }
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
