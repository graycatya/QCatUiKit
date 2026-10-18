// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

GridLayout {
    id: control
    objectName: "chartStrokeOptions"
    property int cap: CatChartSeries.AutoCap
    property int join: CatChartSeries.AutoJoin
    property int strokeWidth: 2
    property int limit: 4
    columns: width >= 620 ? 4 : 2
    columnSpacing: 12; rowSpacing: 8
    function reset() { cap = CatChartSeries.AutoCap; join = CatChartSeries.AutoJoin; strokeWidth = 2; limit = 4; }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Line cap") }
        CatComboBox {
            objectName: "chartLineCap"
            Layout.fillWidth: true
            Accessible.name: qsTr("Line cap")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Automatic"), value: 0}, {text: qsTr("Flat"), value: 1},
                    {text: qsTr("Square"), value: 2}, {text: qsTr("Round"), value: 3}]
            currentValue: control.cap
            onActivated: index => control.cap = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Line join") }
        CatComboBox {
            objectName: "chartLineJoin"
            Layout.fillWidth: true
            Accessible.name: qsTr("Line join")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Automatic"), value: 0}, {text: qsTr("Bevel"), value: 1},
                    {text: qsTr("Miter"), value: 2}, {text: qsTr("Round"), value: 3}]
            currentValue: control.join
            onActivated: index => control.join = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Line width (px)") }
        CatSpinBox {
            objectName: "chartLineWidth"
            Layout.fillWidth: true
            Accessible.name: qsTr("Line width (px)")
            from: 1; to: 64; value: control.strokeWidth
            onValueModified: control.strokeWidth = value
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Miter limit") }
        CatSpinBox {
            objectName: "chartMiterLimit"
            Layout.fillWidth: true
            Accessible.name: qsTr("Miter limit")
            enabled: control.join === CatChartSeries.MiterJoin
            from: 1; to: 16; value: control.limit
            onValueModified: control.limit = value
        }
    }
}
