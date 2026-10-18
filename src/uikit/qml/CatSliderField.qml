// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

FocusScope {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property alias from: valueSlider.from
    property alias to: valueSlider.to
    property alias value: valueSlider.value
    property alias stepSize: valueSlider.stepSize
    property alias snapMode: valueSlider.snapMode
    property alias live: valueSlider.live
    property alias decimals: valueSlider.decimals
    property alias suffix: valueSlider.suffix
    property alias locale: valueSlider.locale
    property alias tickInterval: valueSlider.tickInterval
    property alias tickValues: valueSlider.tickValues
    property alias tickLabelsVisible: valueSlider.tickLabelsVisible
    property alias valueToolTipVisible: valueSlider.valueToolTipVisible
    property alias valueFormatter: valueSlider.valueFormatter
    property alias defaultValue: valueSlider.defaultValue
    property alias defaultIndicatorVisible: valueSlider.defaultIndicatorVisible
    property alias fillOrigin: valueSlider.fillOrigin
    property bool resetVisible: false
    property real inputWidth: 120
    readonly property bool compact: width < inputWidth + (resetVisible ? resetButton.implicitWidth : 0) + 180
    readonly property CatSlider slider: valueSlider
    readonly property CatNumberBox numberBox: valueInput
    signal valueEdited(real value)
    signal valueCommitted(real value)
    function reset(): void { valueSlider.reset(); }
    implicitWidth: 360
    implicitHeight: fields.implicitHeight
    GridLayout {
        id: fields
        width: control.width
        columns: control.compact ? 1 : 2
        columnSpacing: control.tokens.spacing
        rowSpacing: control.tokens.spacing
        CatSlider {
            id: valueSlider
            theme: control.theme
            focus: true
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Accessible.name: control.Accessible.name
            onMoved: control.valueEdited(value)
            onValueCommitted: value => control.valueCommitted(value)
        }
        RowLayout {
            Layout.fillWidth: control.compact
            spacing: control.tokens.spacing
            CatNumberBox {
                id: valueInput
                theme: control.theme
                Layout.preferredWidth: control.inputWidth
                Layout.fillWidth: control.compact
                Layout.minimumWidth: 0
                minimum: Math.min(valueSlider.from, valueSlider.to)
                maximum: Math.max(valueSlider.from, valueSlider.to)
                smallChange: valueSlider.stepSize > 0 ? valueSlider.stepSize : Math.pow(10, -valueSlider.decimals)
                decimals: valueSlider.decimals
                locale: valueSlider.locale
                Accessible.name: control.Accessible.name
                // Connections preserve synchronization after either editor changes value.
                Component.onCompleted: value = valueSlider.value
                onValueEdited: value => {
                    valueSlider.editValue(value);
                }
                Connections {
                    target: valueSlider
                    function onValueChanged() { valueInput.value = valueSlider.value; }
                }
            }
            CatLabel { theme: control.theme; text: control.suffix; visible: text.length > 0 }
            CatButton {
                id: resetButton
                theme: control.theme
                text: qsTr("Reset")
                visible: control.resetVisible
                enabled: control.value !== Math.max(Math.min(control.from, control.to), Math.min(Math.max(control.from, control.to), control.defaultValue))
                onClicked: control.reset()
            }
        }
    }
}
