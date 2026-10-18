// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    required property string componentId
    property int variant: 0
    property bool showTicks: true
    property bool showTips: true
    readonly property bool rangeMode: componentId === "rangeslider"
    readonly property bool fieldMode: componentId === "sliderfield"
    readonly property string exampleCode: {
        const ticks = showTicks ? "    tickInterval: " + (fieldMode ? "12" : "20") + "; tickLabelsVisible: true\n" : "";
        const tip = "    valueToolTipVisible: " + showTips + "\n";
        if (fieldMode) return "CatSliderField {\n    width: 480\n    from: -60; to: 12; value: -12; stepSize: 0.5\n"
            + "    decimals: 1; suffix: \" dB\"\n    defaultValue: -12; resetVisible: true\n" + ticks + tip
            + "    Accessible.name: qsTr(\"Gain\")\n}";
        if (rangeMode) return "CatRangeSlider {\n    width: 480\n    from: 0; to: 100\n    first.value: 20; second.value: 70\n"
            + "    rangeDragEnabled: true\n" + (variant === 1 ? "    minimumSpan: 10; maximumSpan: 60\n" : variant === 2 ? "    fixedSpan: 30\n" : "")
            + ticks + tip + "    Accessible.name: qsTr(\"Selected interval\")\n}";
        return "CatSlider {\n    " + (variant === 3 ? "height: 240; orientation: Qt.Vertical" : "width: 480")
            + "\n    from: " + (variant === 2 ? "-100" : "0") + "; to: 100; value: 40\n"
            + (variant === 1 ? "    stepSize: 20; snapMode: CatSlider.SnapAlways\n" : "")
            + (variant === 2 ? "    fillOrigin: 0\n" : "")
            + "    defaultValue: 40; defaultIndicatorVisible: true\n" + ticks + tip
            + "    Accessible.name: qsTr(\"Value\")\n}";
    }
    spacing: 16
    GalleryComboBox {
        objectName: "sliderVariantPicker"
        Layout.fillWidth: true
        visible: !control.fieldMode
        model: control.rangeMode ? [qsTr("Free interval"), qsTr("Limited span"), qsTr("Fixed span")]
            : [qsTr("Continuous"), qsTr("Stepped"), qsTr("Centered fill"), qsTr("Vertical")]
        currentIndex: control.variant
        onActivated: control.variant = currentIndex
        Accessible.name: qsTr("Slider variant")
    }
    Flow {
        Layout.fillWidth: true
        spacing: 16
        CatCheckBox { text: qsTr("Tick labels"); checked: control.showTicks; onToggled: control.showTicks = checked }
        CatCheckBox { text: qsTr("Value tooltip"); checked: control.showTips; onToggled: control.showTips = checked }
    }
    Loader {
        Layout.fillWidth: true
        sourceComponent: control.fieldMode ? fieldScene : control.rangeMode ? rangeScene : sliderScene
    }
    Component {
        id: sliderScene
        ColumnLayout {
            CatLabel { text: qsTr("Value: %1").arg(slider.textFromValue(slider.value)) }
            CatSlider {
                id: slider
                objectName: "demoSlider"
                Layout.fillWidth: horizontal
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: horizontal ? implicitHeight : 240
                from: control.variant === 2 ? -100 : 0
                to: 100; value: 40
                orientation: control.variant === 3 ? Qt.Vertical : Qt.Horizontal
                stepSize: control.variant === 1 ? 20 : 0
                snapMode: control.variant === 1 ? CatSlider.SnapAlways : CatSlider.NoSnap
                fillOrigin: control.variant === 2 ? 0 : from
                defaultValue: 40; defaultIndicatorVisible: true
                tickInterval: control.showTicks ? 20 : 0
                tickLabelsVisible: control.showTicks
                valueToolTipVisible: control.showTips
                Accessible.name: qsTr("Value")
            }
            CatButton { text: qsTr("Restore default"); onClicked: slider.reset() }
            CatSlider { Layout.fillWidth: true; from: 0; to: 100; value: 35; enabled: false; Accessible.name: qsTr("Disabled") }
        }
    }
    Component {
        id: rangeScene
        ColumnLayout {
            CatLabel {
                Layout.fillWidth: true; wrapMode: Text.Wrap
                text: qsTr("Selected: %1 – %2").arg(range.textFromValue(range.first.value)).arg(range.textFromValue(range.second.value))
            }
            CatRangeSlider {
                id: range
                objectName: "demoRangeSlider"
                Layout.fillWidth: true
                from: 0; to: 100; first.value: 20; second.value: 70
                defaultFirstValue: 20; defaultSecondValue: 70
                minimumSpan: control.variant === 1 ? 10 : 0
                maximumSpan: control.variant === 1 ? 60 : Infinity
                fixedSpan: control.variant === 2 ? 30 : -1
                rangeDragEnabled: true
                tickInterval: control.showTicks ? 20 : 0
                tickLabelsVisible: control.showTicks
                valueToolTipVisible: control.showTips
                Accessible.name: qsTr("Selected interval")
            }
            CatLabel {
                Layout.fillWidth: true; wrapMode: Text.Wrap
                color: Theme.tokens.textSecondary
                text: control.variant === 1 ? qsTr("Span: 10–60. Drag between the handles to move the interval.")
                    : control.variant === 2 ? qsTr("Span locked to 30. Move either handle or drag the selected track.")
                    : qsTr("Drag either handle to resize; drag between them to move the interval.")
            }
            CatButton { text: qsTr("Restore default"); onClicked: range.reset() }
        }
    }
    Component {
        id: fieldScene
        ColumnLayout {
            CatLabel { text: qsTr("Gain") }
            CatSliderField {
                objectName: "demoSliderField"
                Layout.fillWidth: true
                from: -60; to: 12; value: -12; stepSize: 0.5
                decimals: 1; suffix: " dB"
                defaultValue: -12; resetVisible: true
                tickInterval: control.showTicks ? 12 : 0
                tickLabelsVisible: control.showTicks
                valueToolTipVisible: control.showTips
                Accessible.name: qsTr("Gain")
            }
            CatLabel {
                Layout.fillWidth: true; wrapMode: Text.Wrap
                text: qsTr("Drag for a quick adjustment, or type an exact value. The editors stay synchronized.")
                color: Theme.tokens.textSecondary
            }
        }
    }
}
