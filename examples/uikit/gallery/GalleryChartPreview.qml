// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    required property string componentId
    property int mode: componentId === "chart-axis" ? 11 : 0
    property bool live: false
    property bool ready: false
    readonly property var modes: [qsTr("Line"), qsTr("Area"), qsTr("Grouped bars"), qsTr(
            "Stacked bars"), qsTr("Scatter"), qsTr("Bubble"), qsTr("Pie"), qsTr("Doughnut"), qsTr(
            "Radar"), qsTr("Heatmap"), qsTr("Bar + line"), qsTr("Time axis"), qsTr("Log axis"), qsTr(
            "Positive / negative"), qsTr("Scatter + trend")]
    spacing: 12

    function loadDemo() {
        live = false;
        demo.load(mode, chart, first, second);
    }
    Component.onCompleted: {
        ready = true;
        loadDemo();
    }
    onModeChanged: {
        if (ready)
            loadDemo();
    }

    GalleryChartData {
        id: demo
    }
    Connections {
        target: GalleryLanguage
        function onLanguageChanged() {
            demo.retranslate();
        }
    }
    Timer {
        interval: 100
        repeat: true
        running: control.live && control.visible
        onTriggered: demo.advance()
    }
    CatComboBox {
        objectName: "chartDemoPicker"
        Layout.fillWidth: true
        model: control.modes
        currentIndex: control.mode
        onActivated: index => control.mode = index
    }
    CatChart {
        id: chart
        objectName: "demoChart"
        Layout.fillWidth: true
        Layout.preferredHeight: 360
        xAxis.tickCount: width < 420 ? 3 : 6
        CatChartSeries {
            id: first
            objectName: "demoChartFirstSeries"
        }
        CatChartSeries {
            id: second
            objectName: "demoChartSecondSeries"
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        CatButton {
            text: qsTr("Zoom in")
            enabled: control.mode !== 6 && control.mode !== 7 && control.mode !== 8
            onClicked: chart.zoomAt(1.5, Qt.point(chart.plotRect().x + chart.plotRect().width / 2,
                                                  chart.plotRect().y + chart.plotRect().height / 2))
        }
        CatButton {
            text: qsTr("Zoom out")
            enabled: control.mode !== 6 && control.mode !== 7 && control.mode !== 8
            onClicked: chart.zoomAt(1 / 1.5, Qt.point(chart.plotRect().x + chart.plotRect().width
                                                      / 2, chart.plotRect().y + chart.plotRect(
                                                          ).height / 2))
        }
        CatButton {
            text: qsTr("Reset view")
            onClicked: chart.resetView()
        }
        CatButton {
            objectName: "chartLiveButton"
            text: control.live ? qsTr("Pause live") : qsTr("Start live")
            primary: true
            visible: control.mode === 0
            onClicked: control.live = !control.live
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 12
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
            onToggled: chart.crosshairEnabled = checked
        }
    }
    RowLayout {
        visible: control.componentId === "chart-series"
        Layout.fillWidth: true
        CatLabel {
            text: qsTr("Capacity")
        }
        CatSpinBox {
            objectName: "chartCapacity"
            Layout.fillWidth: true
            from: 1000
            to: 100000
            stepSize: 1000
            value: 100000
            onValueModified: {
                first.capacity = value;
                second.capacity = value;
            }
        }
    }
    CatLabel {
        Layout.fillWidth: true
        text: demo.hint
        wrapMode: Text.Wrap
    }
    CatLabel {
        Layout.fillWidth: true
        text: qsTr("%1 retained samples per series · display budget %2").arg(first.count).arg(
                  chart.displayBudget)
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
    CatLabel {
        Layout.fillWidth: true
        text: qsTr(
                  "Scroll to zoom, drag to pan, click a legend to toggle visibility. Use +/−, arrow keys and Home when focused.")
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
}
