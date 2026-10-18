// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "chartRangeControls"
    required property CatChart targetChart
    readonly property bool hasData: {
        for (const series of targetChart.series)
            if (series.visible && series.count > 0) return true;
        return false;
    }
    spacing: 8
    // Coalesce data/style/axis notifications. Reading the mapping uses the chart's
    // cached preparation; it never copies or scans the source samples in QML.
    function scheduleSync() { Qt.callLater(synchronize); }
    function synchronize() {
        if (!visible || targetChart.width <= 0 || targetChart.height <= 0) return;
        if (!hasData) { xRange.clear(); yRange.clear(); return; }
        const rect = targetChart.plotRect();
        const a = targetChart.mapToValue(Qt.point(rect.x, rect.y));
        const b = targetChart.mapToValue(Qt.point(rect.x + rect.width, rect.y + rect.height));
        xRange.synchronize(Math.min(a.x, b.x), Math.max(a.x, b.x));
        yRange.synchronize(Math.min(a.y, b.y), Math.max(a.y, b.y));
    }
    onVisibleChanged: scheduleSync()
    onHasDataChanged: scheduleSync()
    Component.onCompleted: scheduleSync()
    Connections {
        target: control.targetChart
        function onViewChanged() { control.scheduleSync(); }
        function onSeriesChanged() { control.scheduleSync(); }
        function onWidthChanged() { control.scheduleSync(); }
        function onHeightChanged() { control.scheduleSync(); }
    }
    Instantiator {
        model: control.targetChart.series
        delegate: Connections {
            required property CatChartSeries modelData
            target: modelData
            function onDataChanged() { control.scheduleSync(); }
            function onStyleChanged() { control.scheduleSync(); }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Axis ranges"); font.bold: true; Layout.fillWidth: true }
        CatButton {
            objectName: "chartRangeReset"
            text: qsTr("Show all data")
            enabled: control.hasData
            onClicked: { control.targetChart.resetView(); control.scheduleSync(); }
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 620 ? 2 : 1
        uniformCellWidths: true
        columnSpacing: 24
        rowSpacing: 8
        GalleryChartAxisRange {
            id: xRange
            Layout.fillWidth: true
            enabled: control.hasData
            axis: control.targetChart.xAxis
            axisName: qsTr("X axis")
            sliderName: "chartXRangeSlider"
        }
        GalleryChartAxisRange {
            id: yRange
            Layout.fillWidth: true
            enabled: control.hasData
            axis: control.targetChart.yAxis
            axisName: qsTr("Y axis")
            sliderName: "chartYRangeSlider"
        }
    }
    CatLabel {
        Layout.fillWidth: true
        text: control.hasData
            ? qsTr("Drag handles to resize or drag the selected range to pan. Lock the span to keep its size. Chart pan and zoom stay in sync.")
            : qsTr("Add data to enable axis range controls.")
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
}
