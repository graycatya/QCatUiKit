// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

GridLayout {
    id: control
    objectName: "chartInteractionOptions"
    required property CatChart targetChart
    columns: width >= 420 ? 2 : 1
    columnSpacing: 12; rowSpacing: 8
    function reset() {
        targetChart.animationsEnabled = false;
        targetChart.clearSelection(); targetChart.selectionMode = CatChart.NoSelection;
        targetChart.xAxis.side = CatChartAxis.NearSide;
        targetChart.yAxis.side = CatChartAxis.NearSide;
        targetChart.xAxis.visible = true; targetChart.yAxis.visible = true;
        targetChart.xAxis.reversed = false;
        targetChart.yAxis.reversed = false;
        targetChart.xAxis.minorTickCount = 0;
        targetChart.yAxis.minorTickCount = 0;
        targetChart.cancelZoomSelection();
        targetChart.touchNavigationEnabled = true;
        targetChart.dragMode = CatChart.PanDrag;
        targetChart.rectangleZoomAxes = CatChart.ZoomBothAxes;
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("X axis position") }
        CatComboBox {
            objectName: "chartXAxisSide"
            Layout.fillWidth: true
            Accessible.name: qsTr("X axis position")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Bottom"), value: CatChartAxis.NearSide},
                    {text: qsTr("Top"), value: CatChartAxis.FarSide}]
            currentValue: control.targetChart.xAxis.side
            onActivated: index => control.targetChart.xAxis.side = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Y axis position") }
        CatComboBox {
            objectName: "chartYAxisSide"
            Layout.fillWidth: true
            Accessible.name: qsTr("Y axis position")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Left"), value: CatChartAxis.NearSide},
                    {text: qsTr("Right"), value: CatChartAxis.FarSide}]
            currentValue: control.targetChart.yAxis.side
            onActivated: index => control.targetChart.yAxis.side = index
        }
    }
    CatCheckBox {
        objectName: "chartXReversed"
        Layout.fillWidth: true
        text: qsTr("Reverse X axis")
        checked: control.targetChart.xAxis.reversed
        onToggled: control.targetChart.xAxis.reversed = checked
    }
    CatCheckBox {
        objectName: "chartYReversed"
        Layout.fillWidth: true
        text: qsTr("Reverse Y axis")
        checked: control.targetChart.yAxis.reversed
        onToggled: control.targetChart.yAxis.reversed = checked
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("X axis minor ticks") }
        CatSpinBox {
            objectName: "chartXMinorTicks"
            Layout.fillWidth: true
            Accessible.name: qsTr("X axis minor ticks")
            from: 0; to: 10
            enabled: control.targetChart.xAxis.scale !== CatChartAxis.Category
            value: control.targetChart.xAxis.minorTickCount
            onValueModified: control.targetChart.xAxis.minorTickCount = value
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Y axis minor ticks") }
        CatSpinBox {
            objectName: "chartYMinorTicks"
            Layout.fillWidth: true
            Accessible.name: qsTr("Y axis minor ticks")
            from: 0; to: 10
            enabled: control.targetChart.yAxis.scale !== CatChartAxis.Category
            value: control.targetChart.yAxis.minorTickCount
            onValueModified: control.targetChart.yAxis.minorTickCount = value
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Drag action") }
        CatComboBox {
            objectName: "chartDragMode"
            Layout.fillWidth: true
            Accessible.name: qsTr("Drag action")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Pan"), value: CatChart.PanDrag},
                    {text: qsTr("Rectangle zoom"), value: CatChart.RectangleZoomDrag}]
            currentValue: control.targetChart.dragMode
            onActivated: index => control.targetChart.dragMode = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Rectangle zoom axes") }
        CatComboBox {
            objectName: "chartRectangleZoomAxes"
            Layout.fillWidth: true
            enabled: control.targetChart.dragMode === CatChart.RectangleZoomDrag
            Accessible.name: qsTr("Rectangle zoom axes")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Both axes"), value: CatChart.ZoomBothAxes},
                    {text: qsTr("X axis only"), value: CatChart.ZoomXAxis},
                    {text: qsTr("Y axis only"), value: CatChart.ZoomYAxis}]
            currentValue: control.targetChart.rectangleZoomAxes
            onActivated: index => control.targetChart.rectangleZoomAxes = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Point selection") }
        CatComboBox {
            objectName: "chartSelectionMode"; Layout.fillWidth: true
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Disabled"), value: CatChart.NoSelection},
                    {text: qsTr("Single selection"), value: CatChart.SingleSelection},
                    {text: qsTr("Multiple selection"), value: CatChart.MultiSelection}]
            currentValue: control.targetChart.selectionMode
            onActivated: index => control.targetChart.selectionMode = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Selected records: %1").arg(control.targetChart.selectedPoints.length) }
        CatButton { objectName: "chartClearSelection"; text: qsTr("Clear selection"); enabled: control.targetChart.selectedPoints.length > 0; onClicked: control.targetChart.clearSelection() }
    }
    CatLabel {
        Layout.fillWidth: true; Layout.columnSpan: control.columns
        visible: control.targetChart.selectionMode !== CatChart.NoSelection
        wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
        text: qsTr("Click or tap a record to select it. Multiple selection toggles each record; data changes clear that series' selection.")
    }
    CatCheckBox {
        objectName: "chartTouchNavigation"
        Layout.fillWidth: true
        Layout.columnSpan: control.columns
        text: qsTr("Touch pan and pinch zoom")
        checked: control.targetChart.touchNavigationEnabled
        onToggled: control.targetChart.touchNavigationEnabled = checked
    }
    CatLabel {
        objectName: "chartTouchHint"
        Layout.fillWidth: true
        Layout.columnSpan: control.columns
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
        text: control.targetChart.touchNavigationEnabled
            ? qsTr("Touch: drag with one finger to pan; pinch with two fingers to zoom. Mouse drag settings apply separately.")
            : qsTr("Touch: tap to inspect a point; drag to scroll the surrounding page.")
    }
    CatLabel {
        Layout.fillWidth: true
        Layout.columnSpan: control.columns
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
        text: control.targetChart.dragMode === CatChart.RectangleZoomDrag
            ? qsTr("Drag inside the plot and release to zoom. Press Esc to cancel; double-click or press Home to reset.")
            : qsTr("Drag inside the plot to pan. Use the wheel or + / - to zoom; double-click or press Home to reset.")
    }
    GalleryChartMotionOptions { Layout.columnSpan: control.columns; targetChart: control.targetChart }
}
